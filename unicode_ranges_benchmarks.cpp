#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
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

[[noreturn]] void benchmark_assert_fail(const char* expression, const char* file, int line)
{
	std::fprintf(stderr, "benchmark assertion failed: %s (%s:%d)\n", expression, file, line);
	std::abort();
}

#define UTF8_RANGES_BENCHMARK_ASSERT(expr) \
	do \
	{ \
		if (!(expr)) \
		{ \
			benchmark_assert_fail(#expr, __FILE__, __LINE__); \
		} \
	} while (false)

volatile std::size_t benchmark_sink = 0;

struct benchmark_options
{
	std::chrono::milliseconds min_duration{ 250 };
	std::size_t min_iterations = 2;
	std::size_t sample_count = 3;
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
		else if (arg.starts_with("--samples="))
		{
			options.sample_count = (std::max)(std::size_t{ 1 }, static_cast<std::size_t>(
				std::strtoull(arg.substr("--samples="sv.size()).data(), nullptr, 10)));
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

std::vector<utf32_char> make_utf32_char_vector(std::u32string_view chunk, std::size_t count)
{
	const auto text = utf32_string_view::from_code_points_unchecked(chunk);
	std::vector<utf32_char> result;
	result.reserve(text.char_count() * count);
	for (std::size_t i = 0; i != count; ++i)
	{
		for (const utf32_char ch : text.chars())
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

std::size_t checksum(std::u32string_view text) noexcept
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

	std::vector<double> ns_samples{};
	std::vector<double> mib_samples{};
	std::vector<std::size_t> iteration_samples{};
	ns_samples.reserve(options.sample_count);
	mib_samples.reserve(options.sample_count);
	iteration_samples.reserve(options.sample_count);

	for (std::size_t sample = 0; sample != options.sample_count; ++sample)
	{
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
		ns_samples.push_back((elapsed * 1'000'000'000.0) / static_cast<double>(iterations));
		mib_samples.push_back(c.bytes_per_iteration == 0
			? 0.0
			: (static_cast<double>(c.bytes_per_iteration) * static_cast<double>(iterations))
				/ elapsed
				/ (1024.0 * 1024.0));
		iteration_samples.push_back(iterations);
	}

	const auto median_double = [](std::vector<double> values) -> double
	{
		const auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
		std::nth_element(values.begin(), middle, values.end());
		if ((values.size() & 1u) != 0)
		{
			return *middle;
		}

		const auto upper = *middle;
		const auto lower = *std::max_element(values.begin(), middle);
		return (lower + upper) / 2.0;
	};

	const auto median_size_t = [](std::vector<std::size_t> values) -> std::size_t
	{
		const auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
		std::nth_element(values.begin(), middle, values.end());
		if ((values.size() & 1u) != 0)
		{
			return *middle;
		}

		const auto upper = *middle;
		const auto lower = *std::max_element(values.begin(), middle);
		return (lower + upper) / 2;
	};

	return benchmark_result{
		c.name,
		median_double(std::move(ns_samples)),
		median_double(std::move(mib_samples)),
		median_size_t(std::move(iteration_samples))
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
	const auto utf32_find_haystack_storage = repeat_text(
		U"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		2048);
	const auto utf8_find_haystack = utf8_string_view::from_bytes_unchecked(utf8_find_haystack_storage);
	const auto utf16_find_haystack = utf16_string_view::from_code_units_unchecked(utf16_find_haystack_storage);
	const auto utf32_find_haystack = utf32_string_view::from_code_points_unchecked(utf32_find_haystack_storage);
	constexpr auto utf8_long_needle = u8"abcdefghij"_utf8_sv;
	constexpr auto utf16_long_needle = u"abcdefghij"_utf16_sv;
	constexpr auto utf32_long_needle = U"abcdefghij"_utf32_sv;
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
	constexpr std::array utf32_small_any_of{
		U"\u00E9"_u32c,
		U"\u00DF"_u32c,
		U"\u0103"_u32c,
		U"\u0111"_u32c,
		U"\u03C9"_u32c,
		U"\u0416"_u32c,
		U"\u05D0"_u32c,
		U"\u20AC"_u32c
	};
	auto utf8_span_find_haystack_storage = repeat_text(
		u8"plain ascii words and \u03B1\u03B2\u03B3 "sv,
		4096);
	utf8_span_find_haystack_storage += u8"\u20AC";
	auto utf16_span_find_haystack_storage = repeat_text(
		u"plain ascii words and \u03B1\u03B2\u03B3 "sv,
		4096);
	utf16_span_find_haystack_storage += u"\u20AC";
	auto utf32_span_find_haystack_storage = repeat_text(
		U"plain ascii words and \u03B1\u03B2\u03B3 "sv,
		4096);
	utf32_span_find_haystack_storage += U"\u20AC";
	const auto utf8_span_find_haystack = utf8_string_view::from_bytes_unchecked(utf8_span_find_haystack_storage);
	const auto utf16_span_find_haystack = utf16_string_view::from_code_units_unchecked(utf16_span_find_haystack_storage);
	const auto utf32_span_find_haystack = utf32_string_view::from_code_points_unchecked(utf32_span_find_haystack_storage);

	UTF8_RANGES_BENCHMARK_ASSERT(utf8_find_haystack.find(utf8_long_needle) == 6);
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_find_haystack.rfind(utf8_long_needle) == utf8_find_haystack_storage.size() - 17);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_find_haystack.find(utf16_long_needle) == 6);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_find_haystack.rfind(utf16_long_needle) == utf16_find_haystack_storage.size() - 17);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_find_haystack.find(utf32_long_needle) == 6);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_find_haystack.rfind(utf32_long_needle) == utf32_find_haystack_storage.size() - 17);
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_span_find_haystack.find(std::span{ utf8_small_any_of }) == utf8_span_find_haystack_storage.size() - 3);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_span_find_haystack.find(std::span{ utf16_small_any_of }) == utf16_span_find_haystack_storage.size() - 1);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_span_find_haystack.find(std::span{ utf32_small_any_of }) == utf32_span_find_haystack_storage.size() - 1);

	const auto utf8_replace_same_storage = repeat_text(
		u8"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		1024);
	const auto utf16_replace_same_storage = repeat_text(
		u"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		1024);
	const auto utf32_replace_same_storage = repeat_text(
		U"prefixabcdefghijmiddleabcdefghijsuffix-"sv,
		1024);
	const auto utf8_replace_same_expected = repeat_text(
		u8"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix-"sv,
		1024);
	const auto utf16_replace_same_expected = repeat_text(
		u"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix-"sv,
		1024);
	const auto utf32_replace_same_expected = repeat_text(
		U"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix-"sv,
		1024);
	const auto utf8_replace_grow_expected = repeat_text(
		u8"prefixABCDEFGHIJ++middleABCDEFGHIJ++suffix-"sv,
		1024);
	const auto utf16_replace_grow_expected = repeat_text(
		u"prefixABCDEFGHIJ++middleABCDEFGHIJ++suffix-"sv,
		1024);
	const auto utf32_replace_grow_expected = repeat_text(
		U"prefixABCDEFGHIJ++middleABCDEFGHIJ++suffix-"sv,
		1024);

	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_replace_same_storage) }
		.replace_all(utf8_long_needle, u8"ABCDEFGHIJ"_utf8_sv)
		== utf8_string_view::from_bytes_unchecked(utf8_replace_same_expected));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_replace_same_storage) }
		.replace_all(utf16_long_needle, u"ABCDEFGHIJ"_utf16_sv)
		== utf16_string_view::from_code_units_unchecked(utf16_replace_same_expected));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string{ utf32_string_view::from_code_points_unchecked(utf32_replace_same_storage) }
		.replace_all(utf32_long_needle, U"ABCDEFGHIJ"_utf32_sv)
		== utf32_string_view::from_code_points_unchecked(utf32_replace_same_expected));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string{ utf8_string_view::from_bytes_unchecked(utf8_replace_same_storage) }
		.replace_all(utf8_long_needle, u8"ABCDEFGHIJ++"_utf8_sv)
		== utf8_string_view::from_bytes_unchecked(utf8_replace_grow_expected));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string{ utf16_string_view::from_code_units_unchecked(utf16_replace_same_storage) }
		.replace_all(utf16_long_needle, u"ABCDEFGHIJ++"_utf16_sv)
		== utf16_string_view::from_code_units_unchecked(utf16_replace_grow_expected));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string{ utf32_string_view::from_code_points_unchecked(utf32_replace_same_storage) }
		.replace_all(utf32_long_needle, U"ABCDEFGHIJ++"_utf32_sv)
		== utf32_string_view::from_code_points_unchecked(utf32_replace_grow_expected));

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
	const auto utf32_ascii_upper_storage = repeat_text(
		U"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 0123456789 "sv,
		4096);
	const auto utf32_ascii_lower_storage = repeat_text(
		U"the quick brown fox jumps over the lazy dog 0123456789 "sv,
		4096);
	const auto utf8_mixed_upper_storage = repeat_text(
		u8"ÉLAN ΑΒΓ DÉJÀ VU STRASSE CAFÉ "sv,
		2048);
	const auto utf8_mixed_lower_storage = repeat_text(
		u8"élan αβγ déjà vu strasse café "sv,
		2048);
	const auto utf16_mixed_upper_storage = repeat_text(
		u"ÉLAN ΑΒΓ DÉJÀ VU STRASSE CAFÉ "sv,
		2048);
	const auto utf16_mixed_lower_storage = repeat_text(
		u"élan αβγ déjà vu strasse café "sv,
		2048);

	const auto utf32_mixed_upper_storage = repeat_text(
		U"\u00C9LAN \u0391\u0392\u0393 D\u00C9J\u00C0 VU STRASSE CAF\u00C9 "sv,
		2048);
	const auto utf32_mixed_lower_storage = repeat_text(
		U"\u00E9lan \u03B1\u03B2\u03B3 d\u00E9j\u00E0 vu strasse caf\u00E9 "sv,
		2048);

	const auto utf8_validate_storage = repeat_text(
		u8"ASCII caf\u00E9 \u03A9mega \U0001F600 done "sv,
		4096);
	const auto boundary_ascii_encoding_storage = repeat_text(
		u8"ASCII boundary payload 0123456789 "sv,
		4096);
	const auto boundary_latin1_encoding_storage = repeat_text(
		u8"Caf\u00E9 d\u00E9j\u00E0 vu \u00C5ngstr\u00F6m \u00FF "sv,
		2048);
	const auto boundary_windows_1252_encoding_storage = repeat_text(
		u8"Price: \u20AC Caf\u00E9 \u2014 na\u00EFve \u201Cquotes\u201D "sv,
		2048);
	const auto boundary_windows_1252_large_encoding_storage = repeat_text(
		u8"Price: \u20AC Caf\u00E9 \u2014 na\u00EFve \u201Cquotes\u201D "sv,
		8192);
	const auto utf8_grapheme_storage = repeat_text(
		u8"e\u0301 \U0001F469\u200D\U0001F4BB \U0001F1F7\U0001F1F4 "sv,
		2048);
	const auto utf16_grapheme_storage = repeat_text(
		u"e\u0301 \U0001F469\u200D\U0001F4BB \U0001F1F7\U0001F1F4 "sv,
		2048);
	const auto utf32_grapheme_storage = repeat_text(
		U"e\u0301 \U0001F469\u200D\U0001F4BB \U0001F1F7\U0001F1F4 "sv,
		2048);
	const auto utf8_char_count_storage = repeat_text(
		u8"AbC-\u00E9\u00DF\U0001F642 "sv,
		4096);
	const auto utf16_char_count_storage = repeat_text(
		u"AbC-\u00E9\u00DF\U0001F642 "sv,
		4096);
	const auto utf32_char_count_storage = repeat_text(
		U"AbC-\u00E9\u00DF\U0001F642 "sv,
		4096);
	const auto utf8_normalize_nfc_storage = repeat_text(
		u8"e\u0301 A\u030A \u1E9B\u0323 "sv,
		2048);
	const auto utf16_normalize_nfc_storage = repeat_text(
		u"e\u0301 A\u030A \u1E9B\u0323 "sv,
		2048);
	const auto utf32_normalize_nfc_storage = repeat_text(
		U"e\u0301 A\u030A \u1E9B\u0323 "sv,
		2048);
	const auto utf8_already_nfc_storage = repeat_text(
		u8"Caf\u00E9 \u00C5ngstr\u00F6m d\u00E9j\u00E0 vu \U0001F642 "sv,
		2048);
	const auto utf16_already_nfc_storage = repeat_text(
		u"Caf\u00E9 \u00C5ngstr\u00F6m d\u00E9j\u00E0 vu \U0001F642 "sv,
		2048);
	const auto utf32_already_nfc_storage = repeat_text(
		U"Caf\u00E9 \u00C5ngstr\u00F6m d\u00E9j\u00E0 vu \U0001F642 "sv,
		2048);
	const auto utf32_parallel_large_storage = repeat_text(
		U"ASCII \u00C4 \u03A9 STRASSE \u1E9E \U0001F642 done "sv,
		32768);
	const auto utf8_split_whitespace_storage = repeat_text(
		u8"alpha  beta\tcaf\u00E9 \n\U0001F642  \u03A9mega\r\n"sv,
		2048);
	const auto utf16_split_whitespace_storage = repeat_text(
		u"alpha  beta\tcaf\u00E9 \n\U0001F642  \u03A9mega\r\n"sv,
		2048);
	const auto utf32_split_whitespace_storage = repeat_text(
		U"alpha  beta\tcaf\u00E9 \n\U0001F642  \u03A9mega\r\n"sv,
		2048);
	const auto utf8_grapheme_text = utf8_string_view::from_bytes_unchecked(utf8_grapheme_storage);
	const auto utf16_grapheme_text = utf16_string_view::from_code_units_unchecked(utf16_grapheme_storage);
	const auto utf32_grapheme_text = utf32_string_view::from_code_points_unchecked(utf32_grapheme_storage);
	const auto utf8_char_count_text = utf8_string_view::from_bytes_unchecked(utf8_char_count_storage);
	const auto utf16_char_count_text = utf16_string_view::from_code_units_unchecked(utf16_char_count_storage);
	const auto utf32_char_count_text = utf32_string_view::from_code_points_unchecked(utf32_char_count_storage);
	const auto boundary_ascii_encoding_text =
		utf8_string_view::from_bytes_unchecked(boundary_ascii_encoding_storage).to_utf8_owned();
	const auto boundary_latin1_encoding_text =
		utf8_string_view::from_bytes_unchecked(boundary_latin1_encoding_storage).to_utf8_owned();
	const auto boundary_windows_1252_encoding_text =
		utf8_string_view::from_bytes_unchecked(boundary_windows_1252_encoding_storage).to_utf8_owned();
	const auto boundary_windows_1252_large_encoding_text =
		utf8_string_view::from_bytes_unchecked(boundary_windows_1252_large_encoding_storage).to_utf8_owned();
	const auto utf8_ascii_upper_text = utf8_string_view::from_bytes_unchecked(utf8_ascii_upper_storage);
	const auto utf8_ascii_lower_text = utf8_string_view::from_bytes_unchecked(utf8_ascii_lower_storage);
	const auto utf16_ascii_upper_text = utf16_string_view::from_code_units_unchecked(utf16_ascii_upper_storage);
	const auto utf16_ascii_lower_text = utf16_string_view::from_code_units_unchecked(utf16_ascii_lower_storage);
	const auto utf32_ascii_upper_text = utf32_string_view::from_code_points_unchecked(utf32_ascii_upper_storage);
	const auto utf32_ascii_lower_text = utf32_string_view::from_code_points_unchecked(utf32_ascii_lower_storage);
	const auto utf8_normalize_nfc_text = utf8_string_view::from_bytes_unchecked(utf8_normalize_nfc_storage);
	const auto utf16_normalize_nfc_text = utf16_string_view::from_code_units_unchecked(utf16_normalize_nfc_storage);
	const auto utf32_normalize_nfc_text = utf32_string_view::from_code_points_unchecked(utf32_normalize_nfc_storage);
	const auto utf8_already_nfc_text = utf8_string_view::from_bytes_unchecked(utf8_already_nfc_storage);
	const auto utf16_already_nfc_text = utf16_string_view::from_code_units_unchecked(utf16_already_nfc_storage);
	const auto utf32_already_nfc_text = utf32_string_view::from_code_points_unchecked(utf32_already_nfc_storage);
	const auto utf32_parallel_large_text = utf32_string_view::from_code_points_unchecked(utf32_parallel_large_storage);
	const auto utf8_split_whitespace_text = utf8_string_view::from_bytes_unchecked(utf8_split_whitespace_storage);
	const auto utf16_split_whitespace_text = utf16_string_view::from_code_units_unchecked(utf16_split_whitespace_storage);
	const auto utf32_split_whitespace_text = utf32_string_view::from_code_points_unchecked(utf32_split_whitespace_storage);
	constexpr std::size_t ascii_ignore_case_trim = 64;
	const auto utf8_ascii_prefix = utf8_string_view::from_bytes_unchecked(
		std::u8string_view{ utf8_ascii_lower_storage.data(), utf8_ascii_lower_storage.size() - ascii_ignore_case_trim });
	const auto utf16_ascii_prefix = utf16_string_view::from_code_units_unchecked(
		std::u16string_view{ utf16_ascii_lower_storage.data(), utf16_ascii_lower_storage.size() - ascii_ignore_case_trim });
	const auto utf32_ascii_prefix = utf32_string_view::from_code_points_unchecked(
		std::u32string_view{ utf32_ascii_lower_storage.data(), utf32_ascii_lower_storage.size() - ascii_ignore_case_trim });
	const auto utf8_ascii_suffix = utf8_string_view::from_bytes_unchecked(
		std::u8string_view{ utf8_ascii_lower_storage.data() + ascii_ignore_case_trim, utf8_ascii_lower_storage.size() - ascii_ignore_case_trim });
	const auto utf16_ascii_suffix = utf16_string_view::from_code_units_unchecked(
		std::u16string_view{ utf16_ascii_lower_storage.data() + ascii_ignore_case_trim, utf16_ascii_lower_storage.size() - ascii_ignore_case_trim });
	const auto utf32_ascii_suffix = utf32_string_view::from_code_points_unchecked(
		std::u32string_view{ utf32_ascii_lower_storage.data() + ascii_ignore_case_trim, utf32_ascii_lower_storage.size() - ascii_ignore_case_trim });

	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string_view::from_bytes_unchecked(utf8_ascii_upper_storage).to_ascii_lowercase()
		== utf8_string_view::from_bytes_unchecked(utf8_ascii_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string_view::from_code_units_unchecked(utf16_ascii_upper_storage).to_ascii_lowercase()
		== utf16_string_view::from_code_units_unchecked(utf16_ascii_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_ascii_upper_storage).to_ascii_lowercase()
		== utf32_string_view::from_code_points_unchecked(utf32_ascii_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_ascii_upper_text.eq_ignore_case(utf8_ascii_lower_text));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_ascii_upper_text.eq_ignore_case(utf16_ascii_lower_text));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_ascii_upper_text.eq_ignore_case(utf32_ascii_lower_text));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_ascii_upper_text.compare_ignore_case(utf8_ascii_lower_text) == std::weak_ordering::equivalent);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_ascii_upper_text.compare_ignore_case(utf16_ascii_lower_text) == std::weak_ordering::equivalent);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_ascii_upper_text.compare_ignore_case(utf32_ascii_lower_text) == std::weak_ordering::equivalent);
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_ascii_upper_text.starts_with_ignore_case(utf8_ascii_prefix));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_ascii_upper_text.starts_with_ignore_case(utf16_ascii_prefix));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_ascii_upper_text.starts_with_ignore_case(utf32_ascii_prefix));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_ascii_upper_text.ends_with_ignore_case(utf8_ascii_suffix));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_ascii_upper_text.ends_with_ignore_case(utf16_ascii_suffix));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_ascii_upper_text.ends_with_ignore_case(utf32_ascii_suffix));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).to_lowercase()
		== utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).to_lowercase()
		== utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).to_lowercase()
		== utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(details::validate_utf8(std::u8string_view{ utf8_validate_storage }).has_value());
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_grapheme_text.grapheme_count() != 0);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_grapheme_text.grapheme_count() != 0);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_grapheme_text.grapheme_count() != 0);
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_char_count_text.char_count() != 0);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_char_count_text.char_count() != 0);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_char_count_text.char_count() != 0);
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_normalize_nfc_text.to_nfc().is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_normalize_nfc_text.to_nfc().is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_normalize_nfc_text.to_nfc().is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_already_nfc_text.is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_already_nfc_text.is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_already_nfc_text.is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_already_nfc_text.to_nfc() == utf8_already_nfc_text);
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_already_nfc_text.to_nfc() == utf16_already_nfc_text);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_already_nfc_text.to_nfc() == utf32_already_nfc_text);
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage).is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage).is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage).is_nfc());
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage).to_nfc()
		== utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage).to_nfc()
		== utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage).to_nfc()
		== utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).eq_ignore_case(
		utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage)));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).eq_ignore_case(
		utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage)));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).eq_ignore_case(
		utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage)));
	UTF8_RANGES_BENCHMARK_ASSERT(utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).to_utf32()
		== utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).to_utf32()
		== utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).to_utf8()
		== utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage));
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).to_utf16()
		== utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage));

	const auto utf8_chars = make_utf8_char_vector(u8"AbC-éß🙂 "sv, 4096);
	const auto utf16_chars = make_utf16_char_vector(u"AbC-éß🙂 "sv, 4096);

	const auto utf32_chars = make_utf32_char_vector(U"AbC-\u00E9\u00DF\U0001F642 "sv, 4096);

	UTF8_RANGES_BENCHMARK_ASSERT(utf32_parallel_large_text.to_utf8().to_utf32() == utf32_parallel_large_text);
	UTF8_RANGES_BENCHMARK_ASSERT(utf32_parallel_large_text.to_utf16().to_utf32() == utf32_parallel_large_text);
	UTF8_RANGES_BENCHMARK_ASSERT(!utf32_parallel_large_text.case_fold().empty());

	const auto boundary_ascii_expected = boundary_ascii_encoding_text.to_encoded<encodings::ascii_strict>();
	UTF8_RANGES_BENCHMARK_ASSERT(boundary_ascii_expected.has_value());
	const auto boundary_latin1_expected = boundary_latin1_encoding_text.to_encoded<encodings::iso_8859_1>();
	UTF8_RANGES_BENCHMARK_ASSERT(boundary_latin1_expected.has_value());
	const auto boundary_windows_1252_expected = boundary_windows_1252_encoding_text.to_encoded<encodings::windows_1252>();
	UTF8_RANGES_BENCHMARK_ASSERT(boundary_windows_1252_expected.has_value());
	const auto boundary_windows_1252_large_expected = boundary_windows_1252_large_encoding_text.to_encoded<encodings::windows_1252>();
	UTF8_RANGES_BENCHMARK_ASSERT(boundary_windows_1252_large_expected.has_value());

	std::vector<std::uint32_t> utf8_char_scalars;
	utf8_char_scalars.reserve(utf8_chars.size());
	for (const auto ch : utf8_chars)
	{
		utf8_char_scalars.push_back(ch.as_scalar());
	}

	{
		utf8_string s;
		s.append_range(utf8_chars);
		UTF8_RANGES_BENCHMARK_ASSERT(s.char_count() == utf8_chars.size());
	}
	{
		utf16_string s;
		s.append_range(utf16_chars);
		UTF8_RANGES_BENCHMARK_ASSERT(s.char_count() == utf16_chars.size());
	}
	{
		utf32_string s;
		s.append_range(utf32_chars);
		UTF8_RANGES_BENCHMARK_ASSERT(s.char_count() == utf32_chars.size());
	}

	std::vector<benchmark_case> cases;
	cases.reserve(96);

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
	cases.push_back({
		"utf32.find.long_needle",
		utf32_find_haystack_storage.size() * sizeof(char32_t),
		64,
		[&]() -> std::size_t
		{
			return utf32_find_haystack.find(utf32_long_needle);
		}
	});
	cases.push_back({
		"utf32.rfind.long_needle",
		utf32_find_haystack_storage.size() * sizeof(char32_t),
		64,
		[&]() -> std::size_t
		{
			return utf32_find_haystack.rfind(utf32_long_needle);
		}
	});
	cases.push_back({
		"utf32.find.small_non_ascii_set",
		utf32_span_find_haystack_storage.size() * sizeof(char32_t),
		16,
		[&]() -> std::size_t
		{
			return utf32_span_find_haystack.find(std::span{ utf32_small_any_of });
		}
	});
	cases.push_back({
		"utf32.grapheme_count.mixed",
		utf32_grapheme_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			return utf32_grapheme_text.grapheme_count();
		}
	});
	cases.push_back({
		"utf32.char_count.mixed",
		utf32_char_count_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			return utf32_char_count_text.char_count();
		}
	});
	cases.push_back({
		"utf32.to_ascii_lowercase.view",
		utf32_ascii_upper_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_ascii_upper_storage).to_ascii_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_ascii_uppercase.view",
		utf32_ascii_lower_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_ascii_lower_storage).to_ascii_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_lowercase.ascii.rvalue",
		utf32_ascii_upper_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string{ utf32_string_view::from_code_points_unchecked(utf32_ascii_upper_storage) }.to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_lowercase.mixed.view",
		utf32_mixed_upper_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_uppercase.ascii.rvalue",
		utf32_ascii_lower_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string{ utf32_string_view::from_code_points_unchecked(utf32_ascii_lower_storage) }.to_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_uppercase.mixed.view",
		utf32_mixed_lower_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage).to_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.replace_all.same_width.rvalue",
		utf32_replace_same_storage.size() * sizeof(char32_t),
		4,
		[&]() -> std::size_t
		{
			auto result = utf32_string{ utf32_string_view::from_code_points_unchecked(utf32_replace_same_storage) }
				.replace_all(utf32_long_needle, U"ABCDEFGHIJ"_utf32_sv);
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.replace_all.growing.rvalue",
		utf32_replace_same_storage.size() * sizeof(char32_t),
		4,
		[&]() -> std::size_t
		{
			auto result = utf32_string{ utf32_string_view::from_code_points_unchecked(utf32_replace_same_storage) }
				.replace_all(utf32_long_needle, U"ABCDEFGHIJ++"_utf32_sv);
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_utf16.mixed.view",
		utf8_mixed_upper_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).to_utf16();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_utf32.mixed.view",
		utf8_mixed_upper_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).to_utf32();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_utf8.mixed.view",
		utf16_mixed_upper_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).to_utf8();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_utf32.mixed.view",
		utf16_mixed_upper_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).to_utf32();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_utf8.mixed.view",
		utf32_mixed_upper_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).to_utf8();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_utf16.mixed.view",
		utf32_mixed_upper_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).to_utf16();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_utf8.large.view",
		utf32_parallel_large_storage.size() * sizeof(char32_t),
		1,
		[&]() -> std::size_t
		{
			auto result = utf32_parallel_large_text.to_utf8();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_utf16.large.view",
		utf32_parallel_large_storage.size() * sizeof(char32_t),
		1,
		[&]() -> std::size_t
		{
			auto result = utf32_parallel_large_text.to_utf16();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"encoding.to_encoded.ascii_strict",
		boundary_ascii_expected->size(),
		4,
		[&]() -> std::size_t
		{
			auto result = boundary_ascii_encoding_text.to_encoded<encodings::ascii_strict>();
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(*result);
		}
	});
	cases.push_back({
		"encoding.encode_to.ascii_strict",
		boundary_ascii_expected->size(),
		4,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes(boundary_ascii_expected->size());
			encodings::ascii_strict encoder{};
			auto result = boundary_ascii_encoding_text.encode_to(std::span<char8_t>{ bytes.data(), bytes.size() }, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.encode_append_to.ascii_strict",
		boundary_ascii_expected->size(),
		4,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes{};
			bytes.reserve(boundary_ascii_expected->size());
			encodings::ascii_strict encoder{};
			auto result = boundary_ascii_encoding_text.encode_append_to(bytes, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.to_encoded.windows_1252",
		boundary_windows_1252_expected->size(),
		4,
		[&]() -> std::size_t
		{
			auto result = boundary_windows_1252_encoding_text.to_encoded<encodings::windows_1252>();
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(*result);
		}
	});
	cases.push_back({
		"encoding.encode_to.windows_1252",
		boundary_windows_1252_expected->size(),
		4,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes(boundary_windows_1252_expected->size());
			encodings::windows_1252 encoder{};
			auto result = boundary_windows_1252_encoding_text.encode_to(std::span<char8_t>{ bytes.data(), bytes.size() }, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.encode_append_to.windows_1252",
		boundary_windows_1252_expected->size(),
		4,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes{};
			bytes.reserve(boundary_windows_1252_expected->size());
			encodings::windows_1252 encoder{};
			auto result = boundary_windows_1252_encoding_text.encode_append_to(bytes, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.to_encoded.iso_8859_1",
		boundary_latin1_expected->size(),
		4,
		[&]() -> std::size_t
		{
			auto result = boundary_latin1_encoding_text.to_encoded<encodings::iso_8859_1>();
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(*result);
		}
	});
	cases.push_back({
		"encoding.encode_to.iso_8859_1",
		boundary_latin1_expected->size(),
		4,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes(boundary_latin1_expected->size());
			encodings::iso_8859_1 encoder{};
			auto result = boundary_latin1_encoding_text.encode_to(std::span<char8_t>{ bytes.data(), bytes.size() }, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.encode_append_to.iso_8859_1",
		boundary_latin1_expected->size(),
		4,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes{};
			bytes.reserve(boundary_latin1_expected->size());
			encodings::iso_8859_1 encoder{};
			auto result = boundary_latin1_encoding_text.encode_append_to(bytes, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.to_encoded.windows_1252.large",
		boundary_windows_1252_large_expected->size(),
		1,
		[&]() -> std::size_t
		{
			auto result = boundary_windows_1252_large_encoding_text.to_encoded<encodings::windows_1252>();
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(*result);
		}
	});
	cases.push_back({
		"encoding.encode_to.windows_1252.large",
		boundary_windows_1252_large_expected->size(),
		1,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes(boundary_windows_1252_large_expected->size());
			encodings::windows_1252 encoder{};
			auto result = boundary_windows_1252_large_encoding_text.encode_to(std::span<char8_t>{ bytes.data(), bytes.size() }, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"encoding.encode_append_to.windows_1252.large",
		boundary_windows_1252_large_expected->size(),
		1,
		[&]() -> std::size_t
		{
			std::vector<char8_t> bytes{};
			bytes.reserve(boundary_windows_1252_large_expected->size());
			encodings::windows_1252 encoder{};
			auto result = boundary_windows_1252_large_encoding_text.encode_append_to(bytes, encoder);
			UTF8_RANGES_BENCHMARK_ASSERT(result.has_value());
			return checksum(std::u8string_view{ bytes.data(), bytes.size() });
		}
	});
	cases.push_back({
		"utf32.to_lowercase.large.view",
		utf32_parallel_large_storage.size() * sizeof(char32_t),
		1,
		[&]() -> std::size_t
		{
			auto result = utf32_parallel_large_text.to_lowercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_uppercase.large.view",
		utf32_parallel_large_storage.size() * sizeof(char32_t),
		1,
		[&]() -> std::size_t
		{
			auto result = utf32_parallel_large_text.to_uppercase();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.case_fold.mixed.view",
		utf8_mixed_upper_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).case_fold();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.case_fold.mixed.view",
		utf16_mixed_upper_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).case_fold();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.case_fold.mixed.view",
		utf32_mixed_upper_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).case_fold();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.case_fold.large.view",
		utf32_parallel_large_storage.size() * sizeof(char32_t),
		1,
		[&]() -> std::size_t
		{
			auto result = utf32_parallel_large_text.case_fold();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.eq_ignore_case.ascii",
		utf8_ascii_upper_storage.size() + utf8_ascii_lower_storage.size(),
		16,
		[&]() -> std::size_t
		{
			return utf8_ascii_upper_text.eq_ignore_case(utf8_ascii_lower_text) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf16.eq_ignore_case.ascii",
		(utf16_ascii_upper_storage.size() + utf16_ascii_lower_storage.size()) * sizeof(char16_t),
		16,
		[&]() -> std::size_t
		{
			return utf16_ascii_upper_text.eq_ignore_case(utf16_ascii_lower_text) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf32.eq_ignore_case.ascii",
		(utf32_ascii_upper_storage.size() + utf32_ascii_lower_storage.size()) * sizeof(char32_t),
		16,
		[&]() -> std::size_t
		{
			return utf32_ascii_upper_text.eq_ignore_case(utf32_ascii_lower_text) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf8.compare_ignore_case.ascii",
		utf8_ascii_upper_storage.size() + utf8_ascii_lower_storage.size(),
		16,
		[&]() -> std::size_t
		{
			return utf8_ascii_upper_text.compare_ignore_case(utf8_ascii_lower_text) == std::weak_ordering::equivalent ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf16.compare_ignore_case.ascii",
		(utf16_ascii_upper_storage.size() + utf16_ascii_lower_storage.size()) * sizeof(char16_t),
		16,
		[&]() -> std::size_t
		{
			return utf16_ascii_upper_text.compare_ignore_case(utf16_ascii_lower_text) == std::weak_ordering::equivalent ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf32.compare_ignore_case.ascii",
		(utf32_ascii_upper_storage.size() + utf32_ascii_lower_storage.size()) * sizeof(char32_t),
		16,
		[&]() -> std::size_t
		{
			return utf32_ascii_upper_text.compare_ignore_case(utf32_ascii_lower_text) == std::weak_ordering::equivalent ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf8.starts_with_ignore_case.ascii",
		utf8_ascii_upper_storage.size() + utf8_ascii_prefix.size(),
		16,
		[&]() -> std::size_t
		{
			return utf8_ascii_upper_text.starts_with_ignore_case(utf8_ascii_prefix) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf16.starts_with_ignore_case.ascii",
		(utf16_ascii_upper_storage.size() + utf16_ascii_prefix.size()) * sizeof(char16_t),
		16,
		[&]() -> std::size_t
		{
			return utf16_ascii_upper_text.starts_with_ignore_case(utf16_ascii_prefix) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf32.starts_with_ignore_case.ascii",
		(utf32_ascii_upper_storage.size() + utf32_ascii_prefix.size()) * sizeof(char32_t),
		16,
		[&]() -> std::size_t
		{
			return utf32_ascii_upper_text.starts_with_ignore_case(utf32_ascii_prefix) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf8.ends_with_ignore_case.ascii",
		utf8_ascii_upper_storage.size() + utf8_ascii_suffix.size(),
		16,
		[&]() -> std::size_t
		{
			return utf8_ascii_upper_text.ends_with_ignore_case(utf8_ascii_suffix) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf16.ends_with_ignore_case.ascii",
		(utf16_ascii_upper_storage.size() + utf16_ascii_suffix.size()) * sizeof(char16_t),
		16,
		[&]() -> std::size_t
		{
			return utf16_ascii_upper_text.ends_with_ignore_case(utf16_ascii_suffix) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf32.ends_with_ignore_case.ascii",
		(utf32_ascii_upper_storage.size() + utf32_ascii_suffix.size()) * sizeof(char32_t),
		16,
		[&]() -> std::size_t
		{
			return utf32_ascii_upper_text.ends_with_ignore_case(utf32_ascii_suffix) ? 1u : 0u;
		}
	});
	cases.push_back({
		"utf8.eq_ignore_case.mixed",
		utf8_mixed_upper_storage.size() + utf8_mixed_lower_storage.size(),
		16,
		[&]() -> std::size_t
		{
			return utf8_string_view::from_bytes_unchecked(utf8_mixed_upper_storage).eq_ignore_case(
				utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage))
				? 1u
				: 0u;
		}
	});
	cases.push_back({
		"utf16.eq_ignore_case.mixed",
		(utf16_mixed_upper_storage.size() + utf16_mixed_lower_storage.size()) * sizeof(char16_t),
		16,
		[&]() -> std::size_t
		{
			return utf16_string_view::from_code_units_unchecked(utf16_mixed_upper_storage).eq_ignore_case(
				utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage))
				? 1u
				: 0u;
		}
	});
	cases.push_back({
		"utf32.eq_ignore_case.mixed",
		(utf32_mixed_upper_storage.size() + utf32_mixed_lower_storage.size()) * sizeof(char32_t),
		16,
		[&]() -> std::size_t
		{
			return utf32_string_view::from_code_points_unchecked(utf32_mixed_upper_storage).eq_ignore_case(
				utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage))
				? 1u
				: 0u;
		}
	});
	cases.push_back({
		"utf8.to_nfc.decomposed.view",
		utf8_normalize_nfc_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_normalize_nfc_text.to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_nfc.decomposed.view",
		utf16_normalize_nfc_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_normalize_nfc_text.to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_nfc.decomposed.view",
		utf32_normalize_nfc_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_normalize_nfc_text.to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_nfc.already_nfc.view",
		utf8_already_nfc_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_already_nfc_text.to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.to_nfc.already_nfc.mixed",
		utf8_mixed_lower_storage.size(),
		8,
		[&]() -> std::size_t
		{
			auto result = utf8_string_view::from_bytes_unchecked(utf8_mixed_lower_storage).to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_nfc.already_nfc.view",
		utf16_already_nfc_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_already_nfc_text.to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf16.to_nfc.already_nfc.mixed",
		utf16_mixed_lower_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf16_string_view::from_code_units_unchecked(utf16_mixed_lower_storage).to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_nfc.already_nfc.view",
		utf32_already_nfc_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_already_nfc_text.to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf32.to_nfc.already_nfc.mixed",
		utf32_mixed_lower_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			auto result = utf32_string_view::from_code_points_unchecked(utf32_mixed_lower_storage).to_nfc();
			return checksum(result.base());
		}
	});
	cases.push_back({
		"utf8.split_whitespace.mixed",
		utf8_split_whitespace_storage.size(),
		8,
		[&]() -> std::size_t
		{
			std::size_t sum = 0;
			for (const auto part : utf8_split_whitespace_text.split_whitespace())
			{
				sum += part.size();
			}
			return sum;
		}
	});
	cases.push_back({
		"utf16.split_whitespace.mixed",
		utf16_split_whitespace_storage.size() * sizeof(char16_t),
		8,
		[&]() -> std::size_t
		{
			std::size_t sum = 0;
			for (const auto part : utf16_split_whitespace_text.split_whitespace())
			{
				sum += part.size();
			}
			return sum;
		}
	});
	cases.push_back({
		"utf32.split_whitespace.mixed",
		utf32_split_whitespace_storage.size() * sizeof(char32_t),
		8,
		[&]() -> std::size_t
		{
			std::size_t sum = 0;
			for (const auto part : utf32_split_whitespace_text.split_whitespace())
			{
				sum += part.size();
			}
			return sum;
		}
	});
	cases.push_back({
		"utf32.append_range.utf32_char_vector",
		utf32_chars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			utf32_string s;
			s.reserve(utf32_chars.size() + 16u);
			s.append(U"prefix"_utf32_sv);
			s.append_range(utf32_chars);
			return checksum(s.base());
		}
	});
	cases.push_back({
		"utf32.assign_range.utf32_char_vector",
		utf32_chars.size() * 4u,
		8,
		[&]() -> std::size_t
		{
			utf32_string s;
			s.assign_range(utf32_chars);
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
	std::cout << ", samples: " << options.sample_count << " (median)";
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
		<< std::setw(12) << "iters/smp"
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
