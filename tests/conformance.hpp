#ifndef UNICODE_RANGES_CONFORMANCE_TESTS_HPP
#define UNICODE_RANGES_CONFORMANCE_TESTS_HPP

#include "edge_cases.hpp"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace unicode_ranges_edge_tests
{
inline std::uint32_t parse_scalar(std::string_view token)
{
	std::uint32_t cp = 0;
	const auto parsed = std::from_chars(token.data(), token.data() + token.size(), cp, 16);
	if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size() || !scalar_valid(cp))
		throw std::runtime_error("Invalid scalar in conformance data at line " + std::to_string(current_case));
	return cp;
}

inline std::ifstream open_conformance(const std::filesystem::path& path, std::string_view name)
{
	std::ifstream input{path};
	if (!input) throw std::runtime_error("Cannot open conformance file: " + path.string());
	std::string header;
	std::getline(input, header);
	const auto [major, minor, patch] = details::unicode::unicode_version;
	const auto expected = "# " + std::string{name} + "-" + std::to_string(major) + "."
		+ std::to_string(minor) + "." + std::to_string(patch) + ".txt";
	if (header != expected)
		throw std::runtime_error("Conformance data version mismatch: expected " + expected);
	return input;
}

template <typename Unit>
void check_normalization_row(const std::array<std::u32string, 5>& columns)
{
	constexpr std::array forms{normalization_form::nfc, normalization_form::nfd,
		normalization_form::nfkc, normalization_form::nfkd};
	for (std::size_t source = 0; source < columns.size(); ++source)
	{
		const auto raw = reference_encode<Unit>(columns[source]);
		const auto view = checked_view(raw);
		// The four cross-column invariants specified in NormalizationTest.txt.
		const std::array<std::size_t, 4> targets{source < 3 ? 1u : 3u, source < 3 ? 2u : 4u, 3, 4};
		for (std::size_t f = 0; f < forms.size(); ++f)
		{
			const auto result = view.normalize(forms[f]);
			if (result.base() != reference_encode<Unit>(columns[targets[f]]))
			{
				std::fprintf(stderr, "normalization: unit bytes=%zu source column=%zu form=%zu\n", sizeof(Unit), source + 1, f);
				check(false, "Unicode normalization cross-column invariant", __LINE__);
			}
			++checks;
		}
	}
}

inline void normalization_conformance(const std::filesystem::path& root)
{
	current_suite = "normalization-conformance";
	current_case = 1;
	checks = 0;
	auto input = open_conformance(root / "NormalizationTest.txt", "NormalizationTest");
	std::size_t rows = 0;
	std::string line;
	while (std::getline(input, line))
	{
		++current_case;
		line.erase(line.find('#') == line.npos ? line.size() : line.find('#'));
		if (line.find_first_not_of(" \t\r") == line.npos || line.front() == '@') continue;
		std::istringstream row{line};
		std::array<std::u32string, 5> columns;
		for (auto& column : columns)
		{
			std::string field;
			if (!std::getline(row, field, ';')) throw std::runtime_error("Missing normalization column");
			std::istringstream scalars{field};
			for (std::string token; scalars >> token;) column.push_back(static_cast<char32_t>(parse_scalar(token)));
			if (column.empty()) throw std::runtime_error("Empty normalization column");
		}
		check_normalization_row<char8_t>(columns);
		check_normalization_row<char16_t>(columns);
		check_normalization_row<char32_t>(columns);
		++rows;
	}
	check(input.eof() && rows > 0, "nonempty normalization file read completely", __LINE__);
	std::printf("[       OK ] normalization-conformance (%zu rows, %llu checks)\n", rows, static_cast<unsigned long long>(checks));
}

template <typename Unit>
void check_grapheme_row(const std::u32string& scalars, const std::vector<std::size_t>& breaks)
{
	const auto raw = reference_encode<Unit>(scalars);
	const auto view = checked_view(raw);
	std::vector<std::basic_string<Unit>> expected;
	std::vector<std::size_t> unit_offsets{0};
	for (std::size_t i = 1; i < breaks.size(); ++i)
	{
		expected.push_back(reference_encode<Unit>(std::u32string_view{scalars}.substr(breaks[i - 1], breaks[i] - breaks[i - 1])));
		unit_offsets.push_back(unit_offsets.back() + expected.back().size());
	}
	if (parts_of(view.graphemes()) != expected)
	{
		std::fprintf(stderr, "grapheme: unit bytes=%zu scalars=", sizeof(Unit));
		for (const auto cp : scalars) std::fprintf(stderr, " %04X", static_cast<unsigned>(cp));
		std::fprintf(stderr, " expected break positions:");
		for (const auto pos : breaks) std::fprintf(stderr, " %zu", pos);
		std::fprintf(stderr, " actual cluster lengths:");
		for (const auto& cluster : parts_of(view.graphemes())) std::fprintf(stderr, " %zu", cluster.size());
		std::fprintf(stderr, "\n");
		check(false, "Unicode grapheme break sequence", __LINE__);
	}
	check(view.grapheme_count() == expected.size(), "Unicode grapheme count", __LINE__);
	for (std::size_t pos = 0; pos <= raw.size(); ++pos)
		check(view.is_grapheme_boundary(pos) == (std::ranges::find(unit_offsets, pos) != unit_offsets.end()),
			"Unicode grapheme boundary at every code unit", __LINE__);
}

inline void grapheme_conformance(const std::filesystem::path& root)
{
	current_suite = "grapheme-conformance";
	current_case = 1;
	checks = 0;
	auto input = open_conformance(root / "auxiliary" / "GraphemeBreakTest.txt", "GraphemeBreakTest");
	std::size_t rows = 0;
	std::string line;
	while (std::getline(input, line))
	{
		++current_case;
		line.erase(line.find('#') == line.npos ? line.size() : line.find('#'));
		if (line.find_first_not_of(" \t\r") == line.npos) continue;
		std::istringstream row{line};
		std::u32string scalars;
		std::vector<std::size_t> breaks;
		bool expect_marker = true;
		std::string token;
		while (row >> token)
		{
			if (expect_marker)
			{
				if (token == "\xC3\xB7") breaks.push_back(scalars.size()); // division: break
				else if (token != "\xC3\x97") throw std::runtime_error("Invalid grapheme break marker");
			}
			else scalars.push_back(static_cast<char32_t>(parse_scalar(token)));
			expect_marker = !expect_marker;
		}
		check(!expect_marker && !scalars.empty() && breaks.size() >= 2
			&& breaks.front() == 0 && breaks.back() == scalars.size(), "complete grapheme test row", __LINE__);
		check_grapheme_row<char8_t>(scalars, breaks);
		check_grapheme_row<char16_t>(scalars, breaks);
		check_grapheme_row<char32_t>(scalars, breaks);
		++rows;
	}
	check(input.eof() && rows > 0, "nonempty grapheme file read completely", __LINE__);
	std::printf("[       OK ] grapheme-conformance (%zu rows, %llu checks)\n", rows, static_cast<unsigned long long>(checks));
}

inline void conformance(const std::filesystem::path& root)
{
	grapheme_conformance(root);
	normalization_conformance(root);
}
}
#endif
