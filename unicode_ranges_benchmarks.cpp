#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory_resource>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "unicode_ranges.hpp"

using namespace std::literals;
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

namespace
{

volatile std::size_t benchmark_sink = 0;

struct benchmark_options
{
	std::chrono::milliseconds min_duration{ 250 };
	std::size_t min_iterations = 2;
	std::string_view filter{};
	bool list_only = false;
};

struct benchmark_case
{
	std::string_view name;
	std::size_t bytes_per_iteration = 0;
	std::size_t batch_size = 1;
	std::function<std::size_t()> run;
};

struct benchmark_result
{
	std::string_view name;
	double nanoseconds_per_iteration = 0.0;
	double mib_per_second = 0.0;
	std::size_t iterations = 0;
};

benchmark_options parse_options(int argc, char** argv)
{
	benchmark_options options{};
	for (int i = 1; i != argc; ++i)
	{
		const std::string_view arg{ argv[i] };
		if (arg == "--quick")
		{
			options.min_duration = std::chrono::milliseconds{ 60 };
			options.min_iterations = 1;
		}
		else if (arg == "--list")
		{
			options.list_only = true;
		}
		else if (arg.starts_with("--filter="))
		{
			options.filter = arg.substr("--filter="sv.size());
		}
		else if (arg.starts_with("--min-ms="))
		{
			options.min_duration = std::chrono::milliseconds{
				static_cast<std::chrono::milliseconds::rep>(std::strtoll(arg.substr("--min-ms="sv.size()).data(), nullptr, 10))
			};
		}
	}

	return options;
}

template <typename CharT>
std::basic_string<CharT> repeat_text(std::basic_string_view<CharT> chunk, std::size_t count)
{
	std::basic_string<CharT> result;
	result.reserve(chunk.size() * count);
	for (std::size_t i = 0; i != count; ++i)
	{
		result.append(chunk);
	}

	return result;
}

std::vector<utf8_char> make_utf8_char_vector(std::u8string_view chunk, std::size_t count)
{
	const auto text = utf8_string_view::from_bytes_unchecked(chunk);
	std::vector<utf8_char> result;
	result.reserve(text.char_count() * count);
	for (std::size_t i = 0; i != count; ++i)
	{
		for (const utf8_char ch : text.chars())
		{
			result.push_back(ch);
		}
	}

	return result;
}

std::vector<utf16_char> make_utf16_char_vector(std::u16string_view chunk, std::size_t count)
{
	const auto text = utf16_string_view::from_code_units_unchecked(chunk);
	std::vector<utf16_char> result;
	result.reserve(text.char_count() * count);
	for (std::size_t i = 0; i != count; ++i)
	{
		for (const utf16_char ch : text.chars())
		{
			result.push_back(ch);
		}
	}

	return result;
}

std::size_t checksum(std::u8string_view text) noexcept
{
	if (text.empty())
	{
		return 0;
	}

	return text.size() * 1315423911u
		^ static_cast<std::size_t>(text.front())
		^ (static_cast<std::size_t>(text.back()) << 8u);
}

std::size_t checksum(std::u16string_view text) noexcept
{
	if (text.empty())
	{
		return 0;
	}

	return text.size() * 1315423911u
		^ static_cast<std::size_t>(text.front())
		^ (static_cast<std::size_t>(text.back()) << 8u);
}

benchmark_result run_case(const benchmark_case& c, const benchmark_options& options)
{
	using clock = std::chrono::steady_clock;

	std::size_t iterations = 0;
	const auto start = clock::now();
	do
	{
		for (std::size_t i = 0; i != c.batch_size; ++i)
		{
			benchmark_sink ^= c.run() + 0x9E3779B97F4A7C15ull;
		}

		iterations += c.batch_size;
	}
	while (iterations < options.min_iterations || clock::now() - start < options.min_duration);

	const auto elapsed = std::chrono::duration<double>(clock::now() - start).count();
	const auto ns_per_iteration = (elapsed * 1'000'000'000.0) / static_cast<double>(iterations);
	const auto mib_per_second = c.bytes_per_iteration == 0
		? 0.0
		: (static_cast<double>(c.bytes_per_iteration) * static_cast<double>(iterations))
			/ elapsed
			/ (1024.0 * 1024.0);

	return benchmark_result{
		c.name,
		ns_per_iteration,
		mib_per_second,
		iterations
	};
}

} // namespace

int main(int argc, char** argv)
{
	const auto options = parse_options(argc, argv);

	const auto utf8_find_haystack_storage = repeat_text(
		u8"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		2048);
	const auto utf16_find_haystack_storage = repeat_text(
		u"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		2048);
	const auto utf8_find_haystack = utf8_string_view::from_bytes_unchecked(utf8_find_haystack_storage);
	const auto utf16_find_haystack = utf16_string_view::from_code_units_unchecked(utf16_find_haystack_storage);
	constexpr auto utf8_long_needle = u8"abcdefghij"_utf8_sv;
	constexpr auto utf16_long_needle = u"abcdefghij"_utf16_sv;
	constexpr std::array utf8_small_any_of{
		u8"\u00E9"_u8c,
		u8"\u00DF"_u8c,
		u8"\u0103"_u8c,
		u8"\u0111"_u8c,
		u8"\u03C9"_u8c,
		u8"\u0416"_u8c,
		u8"\u05D0"_u8c,
		u8"\u20AC"_u8c
	};
	constexpr std::array utf16_small_any_of{
		u"\u00E9"_u16c,
		u"\u00DF"_u16c,
		u"\u0103"_u16c,
		u"\u0111"_u16c,
		u"\u03C9"_u16c,
		u"\u0416"_u16c,
		u"\u05D0"_u16c,
		u"\u20AC"_u16c
	};
	auto utf8_span_find_haystack_storage = repeat_text(
		u8"plain ascii words and \u03B1\u03B2\u03B3 "sv,
		4096);
	utf8_span_find_haystack_storage += u8"\u20AC";
	auto utf16_span_find_haystack_storage = repeat_text(
		u"plain ascii words and \u03B1\u03B2\u03B3 "sv,
		4096);
	utf16_span_find_haystack_storage += u"\u20AC";
	const auto utf8_span_find_haystack = utf8_string_view::from_bytes_unchecked(utf8_span_find_haystack_storage);
	const auto utf16_span_find_haystack = utf16_string_view::from_code_units_unchecked(utf16_span_find_haystack_storage);

	assert(utf8_find_haystack.find(utf8_long_needle) == 6);
	assert(utf8_find_haystack.rfind(utf8_long_needle) == utf8_find_haystack_storage.size() - 17);
	assert(utf16_find_haystack.find(utf16_long_needle) == 6);
	assert(utf16_find_haystack.rfind(utf16_long_needle) == utf16_find_haystack_storage.size() - 17);
	assert(utf8_span_find_haystack.find(std::span{ utf8_small_any_of }) == utf8_span_find_haystack_storage.size() - 3);
	assert(utf16_span_find_haystack.find(std::span{ utf16_small_any_of }) == utf16_span_find_haystack_storage.size() - 1);

	const auto utf8_replace_same_storage = repeat_text(
		u8"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		1024);
	const auto utf16_replace_same_storage = repeat_text(
		u"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		1024);
	const auto utf8_replace_same_expected = repeat_text(
		u8"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix-"sv,
		1024);
	const auto utf16_replace_same_expected = repeat_text(
		u"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix-"sv,
		1024);
	const auto utf8_replace_grow_expected = repeat_text(
		u8"prefixABCDEFGHIJ++middleABCDEFGHIJ++suffix-"sv,
		1024);
	const auto utf16_replace_grow_expected = repeat_text(
		u"prefixABCDEFGHIJ++middleABCDEFGHIJ++suffix-"sv,
		1024);

	assert(utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_replace_same_storage) }
		.replace_all(utf8_long_needle, u8"ABCDEFGHIJ"_utf8_sv)
		== utf8_string_view::from_bytes_unchecked(utf8_replace_same_expected));
	assert(utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_replace_same_storage) }
		.replace_all(utf16_long_needle, u"ABCDEFGHIJ"_utf16_sv)
		== utf16_string_view::from_code_units_unchecked(utf16_replace_same_expected));

	const auto utf8_ascii_upper_storage = repeat_text(
		u8"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 0123456789 "sv,
		4096);
	const auto utf8_ascii_lower_storage = repeat_text(
		u8"the quick brown fox jumps over the lazy dog 0123456789 "sv,
		4096);
	const auto utf16_ascii_upper_storage = repeat_text(
		u"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 0123456789 "sv,
		4096);
	const auto utf16_ascii_lower_storage = repeat_text(
		u"the quick brown fox jumps over the lazy dog 0123456789 "sv,
		4096);
	const auto utf8_mixed_upper_storage = repeat_text(
		u8"ÉLAN ΑΒΓ DÉJÀ VU STRASSE CAFÉ "sv,
		2048);
	const auto utf8_mixed_lower_storage = repeat_text(
		u8"élan αβγ déjà vu straße café "sv,
		2048);
	const auto utf16_mixed_upper_storage = repeat_text(
		u"ÉLAN ΑΒΓ DÉJÀ VU STRASSE CAFÉ "sv,
		2048);
	const auto utf16_mixed_lower_storage = repeat_text(
		u"élan αβγ déjà vu straße café "sv,
		2048);

	const auto utf8_validate_storage = repeat_text(
		u8"ASCII caf\u00E9 \u03A9mega \U0001F600 done "sv,
		4096);
	const auto utf8_grapheme_storage = repeat_text(
		u8"e\u0301 \U0001F469\u200D\U0001F4BB \U0001F1F7\U0001F1F4 "sv,
		2048);
	const auto utf16_grapheme_storage = repeat_text(
		u"e\u0301 \U0001F469\u200D\U0001F4BB \U0001F1F7\U0001F1F4 "sv,
		2048);
	const auto utf8_char_count_storage = repeat_text(
		u8"AbC-\u00E9\u00DF\U0001F642 "sv,
		4096);
	const auto utf16_char_count_storage = repeat_text(
		u"AbC-\u00E9\u00DF\U0001F642 "sv,
		4096);
	const auto utf8_grapheme_text = utf8_string_view::from_bytes_unchecked(utf8_grapheme_storage);
	const auto utf16_grapheme_text = utf16_string_view::from_code_units_unchecked(utf16_grapheme_storage);
	const auto utf8_char_count_text = utf8_string_view::from_bytes_unchecked(utf8_char_count_storage);
	const auto utf16_char_count_text = utf16_string_view::from_code_units_unchecked(utf16_char_count_storage);

	assert(utf8_string_view::from_bytes_unchecked(utf8_ascii_upper_storage).to_ascii_lowercase()
		== utf8_string_view::from_bytes_unchecked(utf8_ascii_lower_storage));
	assert(utf16_string_view::from_code_units_unchecked(utf16_ascii_upper_storage).to_ascii_lowercase()
		== utf16_string_view::from_code_units_unchecked(utf16_ascii_lower_storage));
	assert(utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).to_lowercase()
		== utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage));
	assert(utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).to_lowercase()
		== utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage));
	assert(details::validate_utf8(std::u8string_view{ utf8_validate_storage }).has_value());
	assert(utf8_grapheme_text.grapheme_count() != 0);
	assert(utf16_grapheme_text.grapheme_count() != 0);
	assert(utf8_char_count_text.char_count() != 0);
	assert(utf16_char_count_text.char_count() != 0);

	const auto utf8_chars = make_utf8_char_vector(u8"AbC-éß🙂 "sv, 4096);
	const auto utf16_chars = make_utf16_char_vector(u"AbC-éß🙂 "sv, 4096);

	std::vector<std::uint32_t> utf8_char_scalars;
	utf8_char_scalars.reserve(utf8_chars.size());
	for (const auto ch : utf8_chars)
	{
		utf8_char_scalars.push_back(ch.as_scalar());
	}

	{
		utf8_string s;
		s.append_range(utf8_chars);
		assert(s.char_count() == utf8_chars.size());
	}
	{
		utf16_string s;
		s.append_range(utf16_chars);
		assert(s.char_count() == utf16_chars.size());
	}

	std::vector<benchmark_case> cases;
	cases.reserve(29);

	cases.push_back({
		"utf8.find.long_needle",
		utf8_find_haystack_storage.size(),
		64,
		[&]() -> std::size_t
		{
			return utf8_find_haystack.find(utf8_long_needle);
		}
	});
	cases.push_back({
		"utf8.rfind.long_needle",
		utf8_find_haystack_storage.size(),
		64,
		[&]() -> std::size_t
		{
			return utf8_find_haystack.rfind(utf8_long_needle);
		}
	});
	cases.push_back({
		"utf16.find.long_needle",
		utf16_find_haystack_storage.size() * sizeof(char16_t),
		64,
		[&]() -> std::size_t
		{
			return utf16_find_haystack.find(utf16_long_needle);
		}
	});
	cases.push_back({
		"utf16.rfind.long_needle",
		utf16_find_haystack_storage.size() * sizeof(char16_t),
		64,
		[&]() -> std::size_t
		{
			return utf16_find_haystack.rfind(utf16_long_needle);
		}
	});
	cases.push_back({
		"utf8.find.small_non_ascii_set",
		utf8_span_find_haystack_storage.size(),
		16,
		[&]() -> std::size_t
		{
			return utf8_span_find_haystack.find(std::span{ utf8_small_any_of });
		}
	});
	cases.push_back({
		"utf16.find.small_non_ascii_set",
		utf16_span_find_haystack_storage.size() * sizeof(char16_t),
		16,
		[&]() -> std::size_t
		{
			return utf16_span_find_haystack.find(std::span{ utf16_small_any_of });
		}
	});
	cases.push_back({
		"utf8.validate.mixed",
		utf8_validate_storage.size(),
		16,
		[&]() -> std::size_t
		{
			return details::validate_utf8(std::u8string_view{ utf8_validate_storage }).has_value()
				? utf8_validate_storage.size()
				: 0u;
		}
	});
	cases.push_back({
		"utf8.grapheme_count.mixed",
		utf8_grapheme_storage.size(),
		8,
		[&]() -> std::size_t
		{
			return utf8_grapheme_text.grapheme_count();
		}
	});
	cases.push_back({
		"utf8.char_count.mixed",
		utf8_char_count_storage.size(),
		8,
		[&]() -> std::size_t
		{
			return utf8_char_count_text.char_count();
		}
	});
	cases.push_back({
		"utf16.grapheme_count.mixed",
		utf16_grapheme_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			return utf16_grapheme_text.grapheme_count();
		}
	});
	cases.push_back({
		"utf16.char_count.mixed",
		utf16_char_count_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			return utf16_char_count_text.char_count();
		}
	});
	cases.push_back({
		"utf8_char.as_scalar.mixed",
		utf8_chars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			std::size_t sum = 0;
			for (const auto ch : utf8_chars)
			{
				sum += ch.as_scalar();
			}
			return sum;
		}
	});
	cases.push_back({
		"utf16_char.as_scalar.mixed",
		utf16_chars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			std::size_t sum = 0;
			for (const auto ch : utf16_chars)
			{
				sum += ch.as_scalar();
			}
			return sum;
		}
	});
	cases.push_back({
		"utf8_char.from_scalar.mixed",
		utf8_char_scalars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			std::size_t sum = 0;
			for (const auto scalar : utf8_char_scalars)
			{
				sum += utf8_char::from_scalar_unchecked(scalar).code_unit_count();
			}
			return sum;
		}
	});

	cases.push_back({
		"utf8.replace_all.same_width.rvalue",
		utf8_replace_same_storage.size(),
		4,
		[&]() -> std::size_t
		{
			auto result = utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_replace_same_storage) }
				.replace_all(utf8_long_needle, u8"ABCDEFGHIJ"_utf8_sv);
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.replace_all.growing.rvalue",
		utf8_replace_same_storage.size(),
		4,
		[&]() -> std::size_t
		{
			auto result = utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_replace_same_storage) }
				.replace_all(utf8_long_needle, u8"ABCDEFGHIJ++"_utf8_sv);
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.replace_all.same_width.rvalue",
		utf16_replace_same_storage.size() * sizeof(char16_t),
		4,
		[&]() -> std::size_t
		{
			auto result = utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_replace_same_storage) }
				.replace_all(utf16_long_needle, u"ABCDEFGHIJ"_utf16_sv);
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.replace_all.growing.rvalue",
		utf16_replace_same_storage.size() * sizeof(char16_t),
		4,
		[&]() -> std::size_t
		{
			auto result = utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_replace_same_storage) }
				.replace_all(utf16_long_needle, u"ABCDEFGHIJ++"_utf16_sv);
			return checksum(result.base());
		}
	});

	cases.push_back({
		"utf8.to_ascii_lowercase.view",
		utf8_ascii_upper_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_ascii_upper_storage).to_ascii_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_ascii_uppercase.view",
		utf8_ascii_lower_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_ascii_lower_storage).to_ascii_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_lowercase.ascii.rvalue",
		utf8_ascii_upper_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_ascii_upper_storage) }.to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_lowercase.mixed.view",
		utf8_mixed_upper_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_uppercase.ascii.rvalue",
		utf8_ascii_lower_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_ascii_lower_storage) }.to_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_uppercase.mixed.view",
		utf8_mixed_lower_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage).to_uppercase();
			return checksum(result.base());
		}
	});

	cases.push_back({
		"utf16.to_ascii_lowercase.view",
		utf16_ascii_upper_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_ascii_upper_storage).to_ascii_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_ascii_uppercase.view",
		utf16_ascii_lower_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_ascii_lower_storage).to_ascii_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_lowercase.ascii.rvalue",
		utf16_ascii_upper_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_ascii_upper_storage) }.to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_lowercase.mixed.view",
		utf16_mixed_upper_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_uppercase.ascii.rvalue",
		utf16_ascii_lower_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_ascii_lower_storage) }.to_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_uppercase.mixed.view",
		utf16_mixed_lower_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage).to_uppercase();
			return checksum(result.base());
		}
	});

	cases.push_back({
		"utf8.append_range.utf8_char_vector",
		utf8_chars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			utf8_string s;
			s.reserve(utf8_chars.size() * 4u + 16u);
			s.append(u8"prefix"_utf8_sv);
			s.append_range(utf8_chars);
			return checksum(s.base());
		}
	});
	cases.push_back({
		"utf8.assign_range.utf8_char_vector",
		utf8_chars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			utf8_string s;
			s.assign_range(utf8_chars);
			return checksum(s.base());
		}
	});
	cases.push_back({
		"utf16.append_range.utf16_char_vector",
		utf16_chars.size() * 2u,
		8,
		[&]() -> std::size_t
		{
			utf16_string s;
			s.reserve(utf16_chars.size() * 2u + 16u);
			s.append(u"prefix"_utf16_sv);
			s.append_range(utf16_chars);
			return checksum(s.base());
		}
	});
	cases.push_back({
		"utf16.assign_range.utf16_char_vector",
		utf16_chars.size() * 2u,
		8,
		[&]() -> std::size_t
		{
			utf16_string s;
			s.assign_range(utf16_chars);
			return checksum(s.base());
		}
	});

	if (options.list_only)
	{
		for (const auto& c : cases)
		{
			if (!options.filter.empty() && !c.name.contains(options.filter))
			{
				continue;
			}

			std::cout << c.name << '\n';
		}

		return 0;
	}

	std::cout << "unicode_ranges benchmark suite\n";
	std::cout << "min duration: " << options.min_duration.count() << " ms";
	if (!options.filter.empty())
	{
		std::cout << ", filter: " << options.filter;
	}
	std::cout << "\n\n";

	std::cout << std::left
		<< std::setw(40) << "case"
		<< std::right
		<< std::setw(14) << "ns/op"
		<< std::setw(14) << "MiB/s"
		<< std::setw(12) << "iters"
		<< '\n';
	std::cout << std::string(80, '-') << '\n';

	for (const auto& c : cases)
	{
		if (!options.filter.empty() && !c.name.contains(options.filter))
		{
			continue;
		}

		const auto result = run_case(c, options);
		std::cout << std::left << std::setw(40) << result.name
			<< std::right << std::setw(14) << std::fixed << std::setprecision(2) << result.nanoseconds_per_iteration
			<< std::setw(14) << std::fixed << std::setprecision(2) << result.mib_per_second
			<< std::setw(12) << result.iterations
			<< '\n';
	}

	std::cout << "\nbenchmark sink: " << benchmark_sink << '\n';
	return 0;
}
