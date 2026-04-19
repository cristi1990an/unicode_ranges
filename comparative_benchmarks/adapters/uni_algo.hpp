#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UNI_ALGO_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UNI_ALGO_HPP

#include "../config.hpp"

#if UTF8_RANGES_COMPARATIVE_WITH_UNI_ALGO

#include <uni_algo/conv.h>

#include "../framework.hpp"

namespace comparative_benchmarks::adapters
{

inline std::size_t validate_utf8_uni_algo(const corpus& input)
{
	const bool is_valid = una::is_valid_utf8(std::u8string_view{ input.bytes });
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(is_valid == input.valid_utf8);
	return is_valid ? 1u : 0u;
}

inline std::size_t utf8_to_utf16_owned_uni_algo(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	una::error error{};
	const std::u16string result = una::strict::utf8to16u(std::u8string_view{ input.bytes }, error);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(!error);
	return checksum(std::u16string_view{ result });
}

inline std::size_t utf8_to_utf32_owned_uni_algo(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	una::error error{};
	const std::u32string result = una::strict::utf8to32u(std::u8string_view{ input.bytes }, error);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(!error);
	return checksum(std::u32string_view{ result });
}

} // namespace comparative_benchmarks::adapters

#endif // UTF8_RANGES_COMPARATIVE_WITH_UNI_ALGO

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UNI_ALGO_HPP
