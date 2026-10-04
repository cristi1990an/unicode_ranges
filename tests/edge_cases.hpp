#ifndef UNICODE_RANGES_EDGE_CASES_HPP
#define UNICODE_RANGES_EDGE_CASES_HPP

#include "../unicode_ranges_all.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory_resource>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace unicode_ranges_edge_tests
{
using namespace unicode_ranges;

// Deliberately independent of the library's decoding, tables, and test helpers.
inline const char* current_suite = "";
inline std::uint64_t current_case = 0;
inline std::uint64_t checks = 0;

inline void check(bool condition, const char* expression, int line)
{
	++checks;
	if (!condition)
	{
		std::fprintf(stderr, "edge suite=%s case=%llu line=%d: %s\n", current_suite,
			static_cast<unsigned long long>(current_case), line, expression);
		std::fflush(stderr);
		std::_Exit(EXIT_FAILURE);
	}
}

// Unlike assert(), these checks also execute in Release builds.
#define UNICODE_EDGE_CHECK(...) ::unicode_ranges_edge_tests::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__, __LINE__)

inline constexpr bool scalar_valid(std::uint32_t cp)
{
	return cp <= 0x10FFFFu && !(cp >= 0xD800u && cp <= 0xDFFFu);
}

inline std::u8string raw_utf8(std::initializer_list<unsigned> bytes)
{
	std::u8string result;
	for (const auto byte : bytes) result.push_back(static_cast<char8_t>(byte));
	return result;
}

template <typename Unit>
std::basic_string<Unit> reference_encode(std::u32string_view scalars)
{
	std::basic_string<Unit> result;
	for (const auto value : scalars)
	{
		const auto cp = static_cast<std::uint32_t>(value);
		if constexpr (std::same_as<Unit, char8_t>)
		{
			if (cp < 0x80u) result.push_back(static_cast<Unit>(cp));
			else
			{
				const auto count = cp < 0x800u ? 2u : cp < 0x10000u ? 3u : 4u;
				const auto lead = count == 2 ? 0xC0u : count == 3 ? 0xE0u : 0xF0u;
				result.push_back(static_cast<Unit>(lead | (cp >> (6u * (count - 1u)))));
				for (auto i = count - 1; i > 0; --i)
					result.push_back(static_cast<Unit>(0x80u | ((cp >> (6u * (i - 1u))) & 0x3Fu)));
			}
		}
		else if constexpr (std::same_as<Unit, char16_t>)
		{
			if (cp < 0x10000u) result.push_back(static_cast<Unit>(cp));
			else
			{
				result.push_back(static_cast<Unit>(0xD800u + ((cp - 0x10000u) >> 10)));
				result.push_back(static_cast<Unit>(0xDC00u + ((cp - 0x10000u) & 0x3FFu)));
			}
		}
		else result.push_back(static_cast<Unit>(cp));
	}
	return result;
}

// Returns the first malformed sequence's start, or npos. Decode numerically,
// then reject overlong encodings, surrogates, and values above U+10FFFF.
inline std::size_t reference_utf8_error(std::u8string_view bytes)
{
	for (std::size_t i = 0; i < bytes.size();)
	{
		const auto first = static_cast<unsigned>(bytes[i]);
		const unsigned count = first < 0x80 ? 1 : first >= 0xC2 && first <= 0xDF ? 2
			: first >= 0xE0 && first <= 0xEF ? 3 : first >= 0xF0 && first <= 0xF4 ? 4 : 0;
		if (count == 0 || bytes.size() - i < count) return i;
		std::uint32_t cp = first & (0x7Fu >> (count == 1 ? 0 : count));
		for (unsigned j = 1; j < count; ++j)
		{
			const auto next = static_cast<unsigned>(bytes[i + j]);
			if ((next & 0xC0u) != 0x80u) return i;
			cp = (cp << 6) | (next & 0x3Fu);
		}
		constexpr std::array<std::uint32_t, 5> minimum{0, 0, 0x80, 0x800, 0x10000};
		if (cp < minimum[count] || !scalar_valid(cp)) return i;
		i += count;
	}
	return std::u8string_view::npos;
}

inline void check_utf8_validation(std::u8string_view bytes)
{
	const auto expected = reference_utf8_error(bytes);
	const auto view = utf8_string_view::from_bytes(bytes);
	UNICODE_EDGE_CHECK(view.has_value() == (expected == std::u8string_view::npos));
	if (!view) UNICODE_EDGE_CHECK(view.error().first_invalid_element_index == expected);
}

struct generator
{
	// Fixed algorithm as well as seed: reproducible across standard libraries.
	std::uint32_t state;
	std::uint32_t next()
	{
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;
		return state;
	}

	std::u32string text(std::size_t size)
	{
		constexpr std::array<char32_t, 20> alphabet{
			0, U'a', U'b', U' ', U'\r', U'\n', U'-', 0x7F, 0x80, 0x7FF,
			0x800, 0x301, 0xD7FF, 0xE000, 0xFFFF, 0x10000, 0x1F600, 0x200D, 0xFE0F, 0x10FFFF};
		std::u32string result;
		while (result.size() < size) result.push_back(alphabet[next() % alphabet.size()]);
		return result;
	}
};

template <typename Unit> struct encoding;
template <> struct encoding<char8_t>
{
	using owned = utf8_string;
	using view = utf8_string_view;
	static auto checked(std::u8string_view s) { return view::from_bytes(s); }
};
template <> struct encoding<char16_t>
{
	using owned = utf16_string;
	using view = utf16_string_view;
	static auto checked(std::u16string_view s) { return view::from_code_units(s); }
};
template <> struct encoding<char32_t>
{
	using owned = utf32_string;
	using view = utf32_string_view;
	static auto checked(std::u32string_view s) { return view::from_code_points(s); }
};

template <typename Unit>
auto checked_view(const std::basic_string<Unit>& s)
{
	const auto result = encoding<Unit>::checked(s);
	UNICODE_EDGE_CHECK(result.has_value());
	return result.value();
}

template <typename Range>
std::u32string scalars_of(Range&& chars)
{
	std::u32string result;
	for (const auto ch : chars) result.push_back(static_cast<char32_t>(ch.as_scalar()));
	return result;
}

inline void validation()
{
	check_utf8_validation({});
	std::array<char8_t, 2> bytes{};
	for (unsigned first = 0; first < 256; ++first)
	{
		current_case = first;
		bytes[0] = static_cast<char8_t>(first);
		check_utf8_validation({bytes.data(), 1});
		for (unsigned second = 0; second < 256; ++second)
		{
			current_case = (first << 8) | second;
			bytes[1] = static_cast<char8_t>(second);
			check_utf8_validation({bytes.data(), bytes.size()});
		}
	}
	// Complete 16-bit domain: every isolated surrogate is rejected.
	for (std::uint32_t cp = 0; cp <= 0xFFFF; ++cp)
	{
		current_case = cp;
		const char16_t unit = static_cast<char16_t>(cp);
		const auto result = utf16_string_view::from_code_units({&unit, 1});
		UNICODE_EDGE_CHECK(result.has_value() == scalar_valid(cp));
		if (!result)
		{
			UNICODE_EDGE_CHECK(result.error().first_invalid_element_index == 0);
			UNICODE_EDGE_CHECK(result.error().code == (cp < 0xDC00
				? unicode_error_code::truncated_surrogate_pair : unicode_error_code::invalid_sequence));
		}
	}
	// Exhaust all pairs drawn from code-unit classes and their endpoints.
	constexpr std::array<char16_t, 10> units{0, 0x7F, 0xD7FF, 0xD800, 0xDBFF, 0xDC00, 0xDFFF, 0xE000, 0xFFFE, 0xFFFF};
	for (const auto a : units) for (const auto b : units)
	{
		current_case = (static_cast<std::uint32_t>(a) << 16) | b;
		const std::array input{a, b};
		const bool pair = a >= 0xD800 && a <= 0xDBFF && b >= 0xDC00 && b <= 0xDFFF;
		const auto result = utf16_string_view::from_code_units({input.data(), input.size()});
		UNICODE_EDGE_CHECK(result.has_value() == (pair || (scalar_valid(a) && scalar_valid(b))));
		if (!result) UNICODE_EDGE_CHECK(result.error().first_invalid_element_index == (scalar_valid(a) ? 1u : 0u));
	}
	constexpr std::array<std::size_t, 18> offsets{0, 1, 2, 3, 7, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 255, 4096};
	const std::array<std::u8string, 12> invalid{
		raw_utf8({0x80}), raw_utf8({0xC0, 0xAF}), raw_utf8({0xC1, 0xBF}), raw_utf8({0xE0, 0x80, 0x80}),
		raw_utf8({0xED, 0xA0, 0x80}), raw_utf8({0xF0, 0x80, 0x80, 0x80}), raw_utf8({0xF4, 0x90, 0x80, 0x80}),
		raw_utf8({0xFF}), raw_utf8({0xC2}), raw_utf8({0xE1, 0x80}), raw_utf8({0xF1, 0x80, 0x80}), raw_utf8({0xE1, 0x80, 0x21})};
	for (const auto offset : offsets) for (std::size_t bad = 0; bad < invalid.size(); ++bad)
	{
		current_case = offset * invalid.size() + bad;
		const auto input = std::u8string(offset, u8'A') + invalid[bad];
		check_utf8_validation(input);
		const auto owned = utf8_string::from_bytes(std::u8string{input});
		UNICODE_EDGE_CHECK(!owned && owned.error().first_invalid_element_index == offset);
		const auto narrow = std::string(input.begin(), input.end());
		const auto narrow_owned = utf8_string::from_bytes(std::string_view{narrow});
		UNICODE_EDGE_CHECK(!narrow_owned && narrow_owned.error().first_invalid_element_index == offset);
	}
	for (const auto offset : offsets)
	{
		current_case = offset;
		for (const auto bad : {0xD800u, 0xDFFFu, 0x110000u, 0x7FFFFFFFu, 0xFFFFFFFFu})
		{
			auto input = std::u32string(offset, U'A');
			input.push_back(static_cast<char32_t>(bad));
			const auto result = utf32_string_view::from_code_points(input);
			UNICODE_EDGE_CHECK(!result && result.error().first_invalid_element_index == offset);
			UNICODE_EDGE_CHECK(result.error().code == unicode_error_code::invalid_scalar);
			UNICODE_EDGE_CHECK(!utf8_char::from_scalar(bad) && !utf16_char::from_scalar(bad) && !utf32_char::from_scalar(bad));
		}
	}
	// Many 3/4-byte combinations plus arbitrary malformed and truncated tails.
	generator rng{0x86C4A921u};
	for (current_case = 0; current_case < 4000; ++current_case)
	{
		auto input = std::u8string(rng.next() % 80, u8'A');
		const auto length = rng.next() % 40;
		for (std::uint32_t i = 0; i < length; ++i) input.push_back(static_cast<char8_t>(rng.next() & 0xFF));
		check_utf8_validation(input);
	}
}

inline void transcoding()
{
	// Every Unicode scalar, including NUL, noncharacters, and all plane ends.
	// Batches keep memory/runtime bounded even on 32-bit and sanitizer runners.
	for (std::uint32_t begin = 0; begin <= 0x10FFFF; begin += 4096)
	{
		current_case = begin;
		std::u32string input;
		for (auto cp = begin; cp < begin + 4096 && cp <= 0x10FFFF; ++cp)
		{
			const auto c8 = utf8_char::from_scalar(cp);
			const auto c16 = utf16_char::from_scalar(cp);
			const auto c32 = utf32_char::from_scalar(cp);
			UNICODE_EDGE_CHECK(c8.has_value() == scalar_valid(cp));
			UNICODE_EDGE_CHECK(c16.has_value() == scalar_valid(cp));
			UNICODE_EDGE_CHECK(c32.has_value() == scalar_valid(cp));
			if (!scalar_valid(cp)) continue;
			UNICODE_EDGE_CHECK(c8->as_scalar() == cp && c16->as_scalar() == cp && c32->as_scalar() == cp);
			input.push_back(static_cast<char32_t>(cp));
		}
		const auto bytes = reference_encode<char8_t>(input);
		const auto units = reference_encode<char16_t>(input);
		const auto v8 = checked_view(bytes);
		const auto v16 = checked_view(units);
		const auto v32 = checked_view(input);
		UNICODE_EDGE_CHECK(v8.to_utf16().base() == units);
		UNICODE_EDGE_CHECK(v8.to_utf32().base() == input);
		UNICODE_EDGE_CHECK(v16.to_utf8().base() == bytes);
		UNICODE_EDGE_CHECK(v16.to_utf32().base() == input);
		UNICODE_EDGE_CHECK(v32.to_utf8().base() == bytes);
		UNICODE_EDGE_CHECK(v32.to_utf16().base() == units);
		UNICODE_EDGE_CHECK(scalars_of(v8.chars()) == input);
		UNICODE_EDGE_CHECK(scalars_of(v16.chars()) == input);
		UNICODE_EDGE_CHECK(v8.char_count() == input.size() && v16.char_count() == input.size());
		std::ranges::reverse(input);
		UNICODE_EDGE_CHECK(scalars_of(v8.reversed_chars()) == input);
		UNICODE_EDGE_CHECK(scalars_of(v16.reversed_chars()) == input);
	}
	// Mixed-width text across allocation, SIMD, and parallel conversion sizes.
	for (const std::size_t size : {0u, 1u, 15u, 16u, 31u, 32u, 63u, 64u, 255u, 256u, 4095u, 4096u, 65535u, 65536u, 65537u})
	{
		current_case = size;
		generator rng{0x12345678u};
		const auto input = rng.text(size);
		const auto v32 = checked_view(input);
		const auto text8 = v32.to_utf8();
		const auto text16 = v32.to_utf16();
		UNICODE_EDGE_CHECK(text8.base() == reference_encode<char8_t>(input));
		UNICODE_EDGE_CHECK(text16.base() == reference_encode<char16_t>(input));
		UNICODE_EDGE_CHECK(text8.to_utf32().base() == input);
		UNICODE_EDGE_CHECK(text16.to_utf32().base() == input);
	}
}

inline void lossy()
{
	// Regression: an ASCII fast path must advance the view as well as the index.
	// Cover each multibyte width and repeated transitions, including embedded NUL.
	for (const auto& valid : {std::u8string{u8"A\u00E9"}, std::u8string{u8"A\u20AC"},
		std::u8string{u8"A\U0001F600"}, std::u8string{u8"a\u00E9b\u20ACc\U0001F600"},
		std::u8string{u8"A\0\u00E9", 4}})
	{
		UNICODE_EDGE_CHECK(utf8_string::from_bytes_lossy(std::u8string_view{valid}).base() == valid);
		UNICODE_EDGE_CHECK(utf8_string::from_bytes_lossy(std::u8string{valid}).base() == valid);
		const std::string narrow(valid.begin(), valid.end());
		UNICODE_EDGE_CHECK(utf8_string::from_bytes_lossy(std::string_view{narrow}).base() == valid);
	}
	struct sample { std::u8string bytes; std::u32string scalars; };
	// Maximal-subpart replacement: valid prefixes of a truncated sequence form
	// one replacement; overlong/surrogate/out-of-range bytes do not get swallowed.
	const std::array cases{
		sample{u8"", U""}, sample{raw_utf8({0xC2}), U"\uFFFD"}, sample{raw_utf8({0xE1, 0x80}), U"\uFFFD"},
		sample{raw_utf8({0xF1, 0x80, 0x80}), U"\uFFFD"}, sample{raw_utf8({0xE1, 0x80, 0x21}), U"\uFFFD!"},
		sample{raw_utf8({0xF1, 0x80, 0x21}), U"\uFFFD!"}, sample{raw_utf8({0xC0, 0xAF}), U"\uFFFD\uFFFD"},
		sample{raw_utf8({0xE0, 0x80, 0x80}), U"\uFFFD\uFFFD\uFFFD"},
		sample{raw_utf8({0xED, 0xA0, 0x80}), U"\uFFFD\uFFFD\uFFFD"},
		sample{raw_utf8({0xF4, 0x90, 0x80, 0x80}), U"\uFFFD\uFFFD\uFFFD\uFFFD"},
		sample{raw_utf8({0x80, 0xBF, 0xFF}), U"\uFFFD\uFFFD\uFFFD"},
		sample{raw_utf8({0xEF, 0xBF, 0xBD}), U"\uFFFD"}, sample{{u8'A', 0, char8_t{0xFF}, u8'B'}, {U'A', 0, 0xFFFD, U'B'}}};
	for (std::size_t i = 0; i < cases.size(); ++i)
	{
		for (const std::size_t prefix : {0u, 15u, 32u, 63u, 128u})
		{
			current_case = i * 256 + prefix;
			const auto input = std::u8string(prefix, u8'a') + cases[i].bytes;
			const auto expected = std::u32string(prefix, U'a') + cases[i].scalars;
			const auto repaired = utf8_string::from_bytes_lossy(std::u8string_view{input});
			UNICODE_EDGE_CHECK(repaired.to_utf32().base() == expected);
			UNICODE_EDGE_CHECK(scalars_of(std::u8string_view{input} | views::lossy_utf8) == expected);
			for (const bool spare_capacity : {false, true})
			{
				auto moved = input;
				if (spare_capacity) moved.reserve(input.size() * 4 + 64);
				UNICODE_EDGE_CHECK(utf8_string::from_bytes_lossy(std::move(moved)).base() == repaired.base());
			}
			UNICODE_EDGE_CHECK(utf8_string::from_bytes_lossy(repaired.as_view().base()) == repaired);
		}
	}
	const std::array<char16_t, 7> units{0, u'A', 0xD800, 0xDBFF, 0xDC00, 0xDFFF, 0xFFFF};
	for (const auto a : units) for (const auto b : units) for (const auto c : units)
	{
		current_case = (static_cast<std::uint64_t>(a) << 32) | (static_cast<std::uint32_t>(b) << 16) | c;
		const std::u16string input{a, b, c};
		std::u32string expected;
		for (std::size_t i = 0; i < input.size(); ++i)
		{
			const auto unit = input[i];
			if (unit >= 0xD800 && unit <= 0xDBFF && i + 1 < input.size() && input[i + 1] >= 0xDC00 && input[i + 1] <= 0xDFFF)
			{
				expected.push_back(static_cast<char32_t>(0x10000u + ((unit - 0xD800u) << 10) + (input[i + 1] - 0xDC00u)));
				++i;
			}
			else expected.push_back(scalar_valid(unit) ? static_cast<char32_t>(unit) : U'\uFFFD');
		}
		UNICODE_EDGE_CHECK(utf16_string::from_code_units_lossy(std::u16string_view{input}).to_utf32().base() == expected);
		UNICODE_EDGE_CHECK(utf16_string::from_code_units_lossy(std::u16string{input}).to_utf32().base() == expected);
		UNICODE_EDGE_CHECK(scalars_of(std::u16string_view{input} | views::lossy_utf16) == expected);
	}
	const std::u32string invalid{0, 0xD800, 0xDFFF, 0x10FFFF, 0x110000, 0xFFFFFFFFu};
	const std::u32string expected{0, 0xFFFD, 0xFFFD, 0x10FFFF, 0xFFFD, 0xFFFD};
	UNICODE_EDGE_CHECK(utf32_string::from_code_points_lossy(std::u32string_view{invalid}).base() == expected);
	UNICODE_EDGE_CHECK(utf32_string::from_code_points_lossy(std::u32string{invalid}).base() == expected);
	UNICODE_EDGE_CHECK(scalars_of(std::u32string_view{invalid} | views::lossy_utf32) == expected);
}

template <typename Unit>
void boundaries_for()
{
	using E = encoding<Unit>;
	generator rng{0xB01DAB1Eu};
	for (std::size_t trial = 0; trial < 80; ++trial)
	{
		const auto model = rng.text(trial % 20);
		const auto raw = reference_encode<Unit>(model);
		const auto view = checked_view(raw);
		std::vector<std::size_t> boundaries{0};
		for (const auto cp : model) boundaries.push_back(boundaries.back() + reference_encode<Unit>({&cp, 1}).size());
		for (std::size_t pos = 0; pos <= raw.size() + 1; ++pos)
		{
			current_case = sizeof(Unit) * 1000000 + trial * 10000 + pos;
			const auto ceil = std::lower_bound(boundaries.begin(), boundaries.end(), pos);
			const auto upper = std::upper_bound(boundaries.begin(), boundaries.end(), pos);
			const bool valid = std::ranges::find(boundaries, pos) != boundaries.end();
			UNICODE_EDGE_CHECK(view.is_char_boundary(pos) == valid);
			UNICODE_EDGE_CHECK(view.ceil_char_boundary(pos) == (ceil == boundaries.end() ? raw.size() : *ceil));
			UNICODE_EDGE_CHECK(view.floor_char_boundary(pos) == *std::prev(upper));
			for (std::size_t end = pos; end <= raw.size(); ++end)
			{
				const bool valid_end = std::ranges::find(boundaries, end) != boundaries.end();
				const auto slice = view.substr(pos, end - pos);
				UNICODE_EDGE_CHECK(slice.has_value() == (valid && valid_end));
				if (slice) UNICODE_EDGE_CHECK(slice->base() == std::basic_string_view<Unit>{raw}.substr(pos, end - pos));
			}
		}
		UNICODE_EDGE_CHECK(!view.substr(E::view::npos));
		UNICODE_EDGE_CHECK(view.floor_char_boundary(E::view::npos) == raw.size());
		UNICODE_EDGE_CHECK(view.ceil_char_boundary(E::view::npos) == raw.size());
		std::size_t index = 0;
		for (const auto [offset, ch] : view.char_indices())
		{
			UNICODE_EDGE_CHECK(index < model.size());
			UNICODE_EDGE_CHECK(offset == boundaries[index] && ch.as_scalar() == static_cast<std::uint32_t>(model[index]));
			++index;
		}
		UNICODE_EDGE_CHECK(index == model.size());
	}
}

inline void boundaries() { boundaries_for<char8_t>(); boundaries_for<char16_t>(); boundaries_for<char32_t>(); }

template <typename Unit>
void mutations_for()
{
	using Owned = typename encoding<Unit>::owned;
	for (const auto seed : {0x13579BDFu, 0x2468ACE1u, 0xF00DBAAAu})
	{
		generator rng{seed};
		std::u32string model;
		Owned value;
		for (std::size_t step = 0; step < 350; ++step)
		{
			current_case = (static_cast<std::uint64_t>(seed) << 16) | (sizeof(Unit) << 12) | step;
			const auto inserted = rng.text(rng.next() % 9);
			const auto raw_inserted = reference_encode<Unit>(inserted);
			const auto insert_view = checked_view(raw_inserted);
			const auto pos = rng.next() % (model.size() + 1);
			const auto count = rng.next() % (model.size() - pos + 1);
			const auto unit_pos = reference_encode<Unit>(std::u32string_view{model}.substr(0, pos)).size();
			const auto unit_count = reference_encode<Unit>(std::u32string_view{model}.substr(pos, count)).size();
			switch (rng.next() % 8)
			{
			case 0: value.append(insert_view); model.append(inserted); break;
			case 1: value.insert(unit_pos, insert_view); model.insert(pos, inserted); break;
			case 2: value.erase(unit_pos, unit_count); model.erase(pos, count); break;
			case 3:
				value = std::move(value).replace_at(unit_pos, unit_count, insert_view);
				model.replace(pos, count, inserted);
				break;
			case 4:
				value.reverse(unit_pos, unit_count);
				std::reverse(model.begin() + static_cast<std::ptrdiff_t>(pos), model.begin() + static_cast<std::ptrdiff_t>(pos + count));
				break;
			case 5:
			{
				const auto copy = value.substr(unit_pos, unit_count);
				UNICODE_EDGE_CHECK(copy && copy->base() == reference_encode<Unit>(std::u32string_view{model}.substr(pos, count)));
				break;
			}
			case 6:
			{
				const auto snapshot = model.substr(pos, count);
				const auto alias = value.as_view().substr(unit_pos, unit_count).value();
				value.append(alias);
				model.append(snapshot);
				break;
			}
			case 7:
			{
				const auto snapshot = model.substr(pos, count);
				const auto alias = value.as_view().substr(unit_pos, unit_count).value();
				value.insert(0, alias);
				model.insert(0, snapshot);
				break;
			}
			}
			UNICODE_EDGE_CHECK(value.base() == reference_encode<Unit>(model));
			UNICODE_EDGE_CHECK(value.char_count() == model.size());
			UNICODE_EDGE_CHECK(encoding<Unit>::checked(value.base()).has_value());
			if (model.size() > 160) { value.clear(); model.clear(); }
		}
	}
}

inline void mutations() { mutations_for<char8_t>(); mutations_for<char16_t>(); mutations_for<char32_t>(); }

template <typename Range>
auto parts_of(Range&& range)
{
	using Unit = typename std::remove_cvref_t<decltype((*std::ranges::begin(range)).base())>::value_type;
	std::vector<std::basic_string<Unit>> result;
	for (const auto part : range) result.emplace_back(part.base());
	return result;
}

template <typename Unit>
auto reference_split(const std::basic_string<Unit>& raw, const std::basic_string<Unit>& delimiter, std::size_t limit)
{
	std::vector<std::basic_string<Unit>> result;
	if (limit == 0) return result;
	std::size_t cursor = 0;
	while (!delimiter.empty() && result.size() + 1 < limit)
	{
		const auto match = raw.find(delimiter, cursor);
		if (match == raw.npos) break;
		result.push_back(raw.substr(cursor, match - cursor));
		cursor = match + delimiter.size();
	}
	result.push_back(raw.substr(cursor));
	return result;
}

template <typename Unit>
void search_split_for()
{
	using E = encoding<Unit>;
	generator rng{0x5EA2C4F1u};
	for (std::size_t trial = 0; trial < 150; ++trial)
	{
		const auto model = trial < 20 ? std::u32string(trial, U'a') : rng.text(rng.next() % 50);
		const auto raw = reference_encode<Unit>(model);
		const auto view = checked_view(raw);
		std::vector<std::size_t> boundaries{0};
		for (const auto cp : model) boundaries.push_back(boundaries.back() + reference_encode<Unit>({&cp, 1}).size());
		const std::array needles{std::u32string{}, std::u32string{U"a"}, std::u32string{U"aa"},
			std::u32string{U"aaa"}, std::u32string(1, U'\0'), model, model.substr(0, model.size() / 2), rng.text(2)};
		for (std::size_t n = 0; n < needles.size(); ++n)
		{
			current_case = sizeof(Unit) * 1000000 + trial * 100 + n;
			const auto needle = reference_encode<Unit>(needles[n]);
			const auto delimiter = checked_view(needle);
			for (std::size_t pos = 0; pos <= raw.size() + 1; ++pos)
			{
				// Search positions clamp to the text and round to character boundaries,
				// which matters for empty needles and positions within multibyte text.
				const auto bounded = (std::min)(pos, raw.size());
				const auto ceil = *std::lower_bound(boundaries.begin(), boundaries.end(), bounded);
				const auto floor = *std::prev(std::upper_bound(boundaries.begin(), boundaries.end(), bounded));
				UNICODE_EDGE_CHECK(view.find(delimiter, pos) == raw.find(needle, ceil));
				UNICODE_EDGE_CHECK(view.rfind(delimiter, pos) == raw.rfind(needle, floor));
			}
			UNICODE_EDGE_CHECK(view.find(delimiter, E::view::npos) == raw.find(needle, raw.size()));
			UNICODE_EDGE_CHECK(view.rfind(delimiter) == raw.rfind(needle));
			const auto expected = reference_split(raw, needle, raw.npos);
			auto split = view.split(delimiter);
			UNICODE_EDGE_CHECK(parts_of(split) == expected);
			UNICODE_EDGE_CHECK(parts_of(std::views::reverse(split)) == decltype(expected)(expected.rbegin(), expected.rend()));
			UNICODE_EDGE_CHECK(parts_of(typename E::owned{view}.split(delimiter)) == expected);
			for (const auto limit : {0u, 1u, 2u, 5u})
				UNICODE_EDGE_CHECK(parts_of(view.splitn(limit, delimiter)) == reference_split(raw, needle, limit));
		}
	}
}

inline void search_split() { search_split_for<char8_t>(); search_split_for<char16_t>(); search_split_for<char32_t>(); }

template <typename Unit>
void unicode_algorithms_for()
{
	using Owned = typename encoding<Unit>::owned;
	const std::vector<std::vector<std::u32string>> clusters{
		{}, {U"\r\n"}, {U"\r", U"a", U"\n"}, {U"\u0301\u0308", U"a"},
		{U"a\u0301\u0308", U"b"}, {U"\u1100\u1161\u11A8", U"!"},
		{U"\U0001F1E6\U0001F1E7", U"\U0001F1E8\U0001F1E9", U"\U0001F1EA"},
		{U"\U0001F469\U0001F3FD\u200D\U0001F4BB", U"!"},
		{U"1\uFE0F\u20E3", U"2"}, {std::u32string(1, U'\0'), U"\u0301", U"x"}};
	for (std::size_t trial = 0; trial < clusters.size(); ++trial)
	{
		current_case = sizeof(Unit) * 1000 + trial;
		std::u32string model;
		std::vector<std::basic_string<Unit>> expected;
		std::vector<std::size_t> offsets{0};
		for (const auto& part : clusters[trial])
		{
			model += part;
			expected.push_back(reference_encode<Unit>(part));
			offsets.push_back(offsets.back() + expected.back().size());
		}
		const auto raw = reference_encode<Unit>(model);
		const auto view = checked_view(raw);
		UNICODE_EDGE_CHECK(parts_of(view.graphemes()) == expected);
		UNICODE_EDGE_CHECK(view.grapheme_count() == expected.size());
		for (std::size_t pos = 0; pos <= raw.size(); ++pos)
			UNICODE_EDGE_CHECK(view.is_grapheme_boundary(pos) == (std::ranges::find(offsets, pos) != offsets.end()));
		Owned reversed{view};
		reversed.reverse_graphemes();
		std::basic_string<Unit> reversed_expected;
		for (const auto& part : std::views::reverse(expected)) reversed_expected += part;
		UNICODE_EDGE_CHECK(reversed.base() == reversed_expected);
	}
	struct normalization_case { std::u32string input, nfc, nfd, nfkc, nfkd; };
	const std::array cases{
		normalization_case{U"", U"", U"", U"", U""},
		normalization_case{U"e\u0301", U"\u00E9", U"e\u0301", U"\u00E9", U"e\u0301"},
		normalization_case{U"\u212B", U"\u00C5", U"A\u030A", U"\u00C5", U"A\u030A"},
		normalization_case{U"\uFB03", U"\uFB03", U"\uFB03", U"ffi", U"ffi"},
		normalization_case{U"\uAC01", U"\uAC01", U"\u1100\u1161\u11A8", U"\uAC01", U"\u1100\u1161\u11A8"},
		normalization_case{U"q\u0315\u0300", U"q\u0300\u0315", U"q\u0300\u0315", U"q\u0300\u0315", U"q\u0300\u0315"}};
	constexpr std::array forms{normalization_form::nfc, normalization_form::nfd, normalization_form::nfkc, normalization_form::nfkd};
	for (std::size_t i = 0; i < cases.size(); ++i)
	{
		current_case = sizeof(Unit) * 1000 + 100 + i;
		const auto raw = reference_encode<Unit>(cases[i].input);
		const auto view = checked_view(raw);
		const std::array expected{cases[i].nfc, cases[i].nfd, cases[i].nfkc, cases[i].nfkd};
		for (std::size_t f = 0; f < forms.size(); ++f)
		{
			const auto normalized = view.normalize(forms[f]);
			UNICODE_EDGE_CHECK(normalized.base() == reference_encode<Unit>(expected[f]));
			UNICODE_EDGE_CHECK(normalized.normalize(forms[f]) == normalized);
			UNICODE_EDGE_CHECK(normalized.is_normalized(forms[f]));
			UNICODE_EDGE_CHECK(Owned{view}.normalize(forms[f]) == normalized);
		}
	}
}

inline void unicode_algorithms() { unicode_algorithms_for<char8_t>(); unicode_algorithms_for<char16_t>(); unicode_algorithms_for<char32_t>(); }

class failing_resource final : public std::pmr::memory_resource
{
public:
	bool fail = false;
	std::size_t minimum_failing_allocation_size = 0;
	std::size_t outstanding = 0;
private:
	void* do_allocate(std::size_t bytes, std::size_t alignment) override
	{
		if (fail && bytes >= minimum_failing_allocation_size) throw std::bad_alloc{};
		auto* ptr = std::pmr::new_delete_resource()->allocate(bytes, alignment);
		++outstanding;
		return ptr;
	}
	void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override
	{
		UNICODE_EDGE_CHECK(outstanding != 0);
		--outstanding;
		std::pmr::new_delete_resource()->deallocate(ptr, bytes, alignment);
	}
	bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override { return this == &other; }
};

template <typename Unit>
void allocation_for()
{
	using Alloc = std::pmr::polymorphic_allocator<Unit>;
	using Owned = std::conditional_t<std::same_as<Unit, char8_t>, basic_utf8_string<Alloc>,
		std::conditional_t<std::same_as<Unit, char16_t>, basic_utf16_string<Alloc>, basic_utf32_string<Alloc>>>;
	const auto raw = reference_encode<Unit>(std::u32string(80, U'\U0001F600'));
	const auto view = checked_view(raw);
	failing_resource resource;
	{
		Owned value{view, Alloc{&resource}};
		const std::basic_string<Unit> snapshot{value.base()};
		const auto blocks = resource.outstanding;
		resource.fail = true;
		for (unsigned operation = 0; operation < 4; ++operation)
		{
			// Debug standard libraries may allocate small container bookkeeping
			// before the string's character buffer; fail the payload allocation.
			resource.minimum_failing_allocation_size = operation >= 2 ? 64 : 0;
			current_case = sizeof(Unit) * 10 + operation;
			bool threw = false;
			try
			{
				switch (operation)
				{
				case 0: value.reserve(value.capacity() + 1024); break;
				case 1: value.append(value.capacity() + 1024, *view.chars().begin()); break;
				case 2: static_cast<void>(value.to_utf8(std::pmr::polymorphic_allocator<char8_t>{&resource})); break;
				case 3: static_cast<void>(value.substr(0)); break;
				}
			}
			catch (const std::bad_alloc&) { threw = true; }
			UNICODE_EDGE_CHECK(threw);
			UNICODE_EDGE_CHECK(std::basic_string_view<Unit>{value.base()} == std::basic_string_view<Unit>{snapshot});
			UNICODE_EDGE_CHECK(value.get_allocator().resource() == &resource);
			UNICODE_EDGE_CHECK(resource.outstanding == blocks);
		}
		resource.fail = false;
		resource.minimum_failing_allocation_size = 0;
		value.append(view);
		UNICODE_EDGE_CHECK(std::basic_string_view<Unit>{value.base()} == std::basic_string_view<Unit>{snapshot + raw});
	}
	UNICODE_EDGE_CHECK(resource.outstanding == 0);
}

inline void allocation() { allocation_for<char8_t>(); allocation_for<char16_t>(); allocation_for<char32_t>(); }

template <typename Unit>
void encoding_output_for()
{
	using Owned = typename encoding<Unit>::owned;
	const std::array inputs{std::u32string{}, std::u32string{U"ASCII"},
		std::u32string{U'A', 0, U'B'}, std::u32string{U"a\u00E9\U0001F600z"}, std::u32string(64, U'x')};
	for (std::size_t sample = 0; sample < inputs.size(); ++sample)
	{
		const auto raw = reference_encode<Unit>(inputs[sample]);
		const Owned value{checked_view(raw)};
		std::u8string expected;
		std::size_t replacements = 0;
		for (const auto cp : inputs[sample])
		{
			expected.push_back(cp <= 0x7F ? static_cast<char8_t>(cp) : u8'?');
			replacements += cp > 0x7F ? 1 : 0;
		}
		for (std::size_t capacity = 0; capacity <= expected.size() + 2; ++capacity)
		{
			current_case = sizeof(Unit) * 100000 + sample * 1000 + capacity;
			// Guard both sides, including when the supplied span is empty.
			std::vector<char8_t> buffer(capacity + 2, char8_t{0xA5});
			encodings::ascii_lossy encoder;
			const auto result = value.encode_to(std::span<char8_t>{buffer.data() + 1, capacity}, encoder);
			UNICODE_EDGE_CHECK(result.has_value() == (capacity >= expected.size()));
			if (!result) UNICODE_EDGE_CHECK(result.error().kind == encode_to_error_kind::overflow);
			UNICODE_EDGE_CHECK(buffer.front() == char8_t{0xA5} && buffer.back() == char8_t{0xA5});
			const auto written = (std::min)(capacity, expected.size());
			UNICODE_EDGE_CHECK(std::u8string_view(buffer.data() + 1, written) == std::u8string_view{expected}.substr(0, written));
			for (auto i = written; i < capacity; ++i) UNICODE_EDGE_CHECK(buffer[i + 1] == char8_t{0xA5});
			if (result) UNICODE_EDGE_CHECK(encoder.replacement_count == replacements);
		}
		std::vector<char8_t> buffer(inputs[sample].size() + 2, char8_t{0xA5});
		encodings::ascii_strict strict;
		const auto result = value.encode_to(std::span<char8_t>{buffer.data() + 1, inputs[sample].size()}, strict);
		UNICODE_EDGE_CHECK(result.has_value() == (replacements == 0));
		if (!result)
		{
			UNICODE_EDGE_CHECK(result.error().kind == encode_to_error_kind::encoding_error);
			UNICODE_EDGE_CHECK(result.error().error == encodings::ascii_strict::encode_error::unrepresentable_scalar);
		}
		UNICODE_EDGE_CHECK(buffer.front() == char8_t{0xA5} && buffer.back() == char8_t{0xA5});
	}
}

inline void encoding_output() { encoding_output_for<char8_t>(); encoding_output_for<char16_t>(); encoding_output_for<char32_t>(); }

struct suite { const char* name; void (*run)(); };
inline constexpr std::array suites{
	suite{"validation", validation}, suite{"transcoding", transcoding}, suite{"lossy", lossy},
	suite{"boundaries", boundaries}, suite{"mutations", mutations}, suite{"search-split", search_split},
	suite{"unicode-algorithms", unicode_algorithms}, suite{"allocation", allocation}, suite{"encoding-output", encoding_output}};

inline bool run(std::string_view filter = {})
{
	bool matched = false;
	for (const auto& test : suites)
	{
		if (!filter.empty() && filter != test.name) continue;
		matched = true;
		current_suite = test.name;
		current_case = 0;
		checks = 0;
		std::printf("[ RUN      ] %s\n", test.name);
		std::fflush(stdout);
		test.run();
		std::printf("[       OK ] %s (%llu checks)\n", test.name, static_cast<unsigned long long>(checks));
	}
	return matched;
}

#undef UNICODE_EDGE_CHECK
}

#endif
