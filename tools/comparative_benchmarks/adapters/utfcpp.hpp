#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UTFCPP_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UTFCPP_HPP

#include "../config.hpp"

#if UTF8_RANGES_COMPARATIVE_WITH_UTFCPP

#include <span>
#include <string>

#include <utf8.h>
#include <utf8/cpp20.h>

#include "../framework.hpp"

namespace comparative_benchmarks::adapters
{

inline std::size_t validate_utf8_utfcpp(const corpus& input)
{
	const bool is_valid = utf8::is_valid(input.bytes.begin(), input.bytes.end());
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(is_valid == input.valid_utf8);
	return is_valid ? 1u : 0u;
}

inline std::size_t utf8_to_utf16_owned_utfcpp(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	const std::u16string result = utf8::utf8to16(std::u8string_view{ input.bytes });
	return checksum(std::u16string_view{ result });
}

inline std::size_t utf8_to_utf32_owned_utfcpp(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	const std::u32string result = utf8::utf8to32(std::u8string_view{ input.bytes });
	return checksum(std::u32string_view{ result });
}

inline std::size_t utf8_to_utf16_buffer_utfcpp(const corpus& input, std::span<char16_t> output)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	char16_t* const start = output.data();
	char16_t* const finish = utf8::utf8to16(input.bytes.begin(), input.bytes.end(), start);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(finish >= start);
	const auto count = static_cast<std::size_t>(finish - start);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(count <= output.size());
	return checksum(std::u16string_view{ output.data(), count });
}

inline std::size_t utf8_to_utf32_buffer_utfcpp(const corpus& input, std::span<char32_t> output)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	char32_t* const start = output.data();
	char32_t* const finish = utf8::utf8to32(input.bytes.begin(), input.bytes.end(), start);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(finish >= start);
	const auto count = static_cast<std::size_t>(finish - start);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(count <= output.size());
	return checksum(std::u32string_view{ output.data(), count });
}

} // namespace comparative_benchmarks::adapters

#endif // UTF8_RANGES_COMPARATIVE_WITH_UTFCPP

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UTFCPP_HPP
