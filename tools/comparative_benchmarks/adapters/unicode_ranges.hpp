#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UNICODE_RANGES_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UNICODE_RANGES_HPP

#include "../../../unicode_ranges.hpp"
#include "../framework.hpp"

namespace comparative_benchmarks::adapters
{

inline std::size_t validate_utf8_public_factory(const corpus& input)
{
	const auto result = unicode_ranges::utf8_string_view::from_bytes(
		std::u8string_view{ input.bytes.data(), input.bytes.size() });
	const bool is_valid = result.has_value();
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(is_valid == input.valid_utf8);
	return is_valid ? 1u : 0u;
}

inline std::size_t utf8_to_utf16_owned(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	const auto validated = unicode_ranges::utf8_string_view::from_bytes(
		std::u8string_view{ input.bytes.data(), input.bytes.size() });
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(validated.has_value());
	const auto result = validated->to_utf16();
	return checksum(result.base());
}

inline std::size_t utf8_to_utf32_owned(const corpus& input)
{
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(input.valid_utf8);
	const auto validated = unicode_ranges::utf8_string_view::from_bytes(
		std::u8string_view{ input.bytes.data(), input.bytes.size() });
	UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(validated.has_value());
	const auto result = validated->to_utf32();
	return checksum(result.base());
}

} // namespace comparative_benchmarks::adapters

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_ADAPTERS_UNICODE_RANGES_HPP
