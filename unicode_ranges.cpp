#include "unicode_ranges.hpp"

#if defined(__has_include)
#if !__has_include(<simdutf.h>) || !__has_include(<simdutf.cpp>)
#error "unicode_ranges.cpp requires simdutf.h and simdutf.cpp on the include path"
#endif
#endif

#include <simdutf.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include <simdutf.cpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace unicode_ranges::details
{

namespace
{
	[[nodiscard]]
	auto map_simdutf_utf8_error(simdutf::result result) noexcept -> utf8_error
	{
		switch (result.error)
		{
		case simdutf::SUCCESS:
			return utf8_error{};

		case simdutf::HEADER_BITS:
		case simdutf::TOO_LONG:
			return utf8_error{
				.code = utf8_error_code::invalid_lead_byte,
				.first_invalid_byte_index = result.count
			};

		case simdutf::TOO_SHORT:
			return utf8_error{
				.code = utf8_error_code::truncated_sequence,
				.first_invalid_byte_index = result.count
			};

		case simdutf::OVERLONG:
		case simdutf::TOO_LARGE:
		case simdutf::SURROGATE:
			return utf8_error{
				.code = utf8_error_code::invalid_sequence,
				.first_invalid_byte_index = result.count
			};

		case simdutf::OUTPUT_BUFFER_TOO_SMALL:
			UTF8_RANGES_DEBUG_ASSERT(false && "simdutf output buffer unexpectedly too small");
			[[fallthrough]];

		default:
			return utf8_error{
				.code = utf8_error_code::invalid_sequence,
				.first_invalid_byte_index = result.count
			};
		}
	}

	[[nodiscard]]
	auto validate_utf8_runtime_impl(const char* bytes, std::size_t size) noexcept
		-> std::expected<void, utf8_error>
	{
		const auto result = simdutf::validate_utf8_with_errors(bytes, size);
		if (result.error == simdutf::SUCCESS) [[likely]]
		{
			return {};
		}

		return std::unexpected(map_simdutf_utf8_error(result));
	}
}

auto simdutf_validate_utf8_runtime(std::string_view bytes) noexcept
	-> std::expected<void, utf8_error>
{
	return validate_utf8_runtime_impl(bytes.data(), bytes.size());
}

auto simdutf_validate_utf8_runtime(std::u8string_view bytes) noexcept
	-> std::expected<void, utf8_error>
{
	return validate_utf8_runtime_impl(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

std::size_t simdutf_utf16_length_from_valid_utf8_runtime(std::u8string_view bytes) noexcept
{
	return simdutf::utf16_length_from_utf8(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size());
}

std::size_t simdutf_utf32_length_from_valid_utf8_runtime(std::u8string_view bytes) noexcept
{
	return simdutf::utf32_length_from_utf8(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size());
}

std::size_t simdutf_convert_valid_utf8_to_utf16_runtime(
	std::u8string_view bytes,
	char16_t* output) noexcept
{
	const auto count = simdutf::convert_valid_utf8_to_utf16(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size(),
		output);
	UTF8_RANGES_DEBUG_ASSERT(count == simdutf_utf16_length_from_valid_utf8_runtime(bytes));
	return count;
}

std::size_t simdutf_convert_valid_utf8_to_utf32_runtime(
	std::u8string_view bytes,
	char32_t* output) noexcept
{
	const auto count = simdutf::convert_valid_utf8_to_utf32(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size(),
		output);
	UTF8_RANGES_DEBUG_ASSERT(count == simdutf_utf32_length_from_valid_utf8_runtime(bytes));
	return count;
}

auto simdutf_convert_utf8_to_utf16_checked_runtime(
	std::string_view bytes,
	char16_t* output) noexcept
	-> std::expected<std::size_t, utf8_error>
{
	const auto result = simdutf::convert_utf8_to_utf16_with_errors(bytes.data(), bytes.size(), output);
	if (result.error == simdutf::SUCCESS) [[likely]]
	{
		return result.count;
	}

	return std::unexpected(map_simdutf_utf8_error(result));
}

auto simdutf_convert_utf8_to_utf32_checked_runtime(
	std::string_view bytes,
	char32_t* output) noexcept
	-> std::expected<std::size_t, utf8_error>
{
	const auto result = simdutf::convert_utf8_to_utf32_with_errors(bytes.data(), bytes.size(), output);
	if (result.error == simdutf::SUCCESS) [[likely]]
	{
		return result.count;
	}

	return std::unexpected(map_simdutf_utf8_error(result));
}

}
