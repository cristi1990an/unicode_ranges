#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_SIMDUTF_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_SIMDUTF_HPP

#include "../config.hpp"

#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF

#include <string>

#include <simdutf.h>

#include "../framework.hpp"

namespace comparative_benchmarks::adapters
{

inline std::size_t validate_utf8_simdutf(const corpus& input)
{
	const bool is_valid = simdutf::validate_utf8(
		reinterpret_cast<const char*>(input.bytes.data()), input.bytes.size());
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(is_valid == input.valid_utf8);
	return is_valid ? 1u : 0u;
}

inline std::size_t utf8_to_utf16_owned_simdutf(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	const auto* utf8 = reinterpret_cast<const char*>(input.bytes.data());
	const auto utf8_size = input.bytes.size();

	std::u16string result{};
	result.resize(simdutf::utf16_length_from_utf8(utf8, utf8_size));

	const simdutf::result conversion =
		simdutf::convert_utf8_to_utf16_with_errors(utf8, utf8_size, result.data());
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(conversion.error == simdutf::SUCCESS);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(conversion.count == result.size());

	return checksum(std::u16string_view{ result });
}

inline std::size_t utf8_to_utf32_owned_simdutf(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	const auto* utf8 = reinterpret_cast<const char*>(input.bytes.data());
	const auto utf8_size = input.bytes.size();

	std::u32string result{};
	result.resize(simdutf::utf32_length_from_utf8(utf8, utf8_size));

	const simdutf::result conversion =
		simdutf::convert_utf8_to_utf32_with_errors(utf8, utf8_size, result.data());
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(conversion.error == simdutf::SUCCESS);
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(conversion.count == result.size());

	return checksum(std::u32string_view{ result });
}

} // namespace comparative_benchmarks::adapters

#endif // UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_SIMDUTF_HPP
