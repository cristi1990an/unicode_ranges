#ifndef UTF8_RANGES_INTERNAL_SIMDUTF_RUNTIME_HPP
#define UTF8_RANGES_INTERNAL_SIMDUTF_RUNTIME_HPP

namespace details
{
	[[nodiscard]]
	std::expected<void, utf8_error> simdutf_validate_utf8_runtime(std::string_view bytes) noexcept;

	[[nodiscard]]
	std::expected<void, utf8_error> simdutf_validate_utf8_runtime(std::u8string_view bytes) noexcept;

	[[nodiscard]]
	std::size_t simdutf_utf16_length_from_valid_utf8_runtime(std::u8string_view bytes) noexcept;

	[[nodiscard]]
	std::size_t simdutf_utf32_length_from_valid_utf8_runtime(std::u8string_view bytes) noexcept;

	[[nodiscard]]
	std::size_t simdutf_convert_valid_utf8_to_utf16_runtime(
		std::u8string_view bytes,
		char16_t* output) noexcept;

	[[nodiscard]]
	std::size_t simdutf_convert_valid_utf8_to_utf32_runtime(
		std::u8string_view bytes,
		char32_t* output) noexcept;

	[[nodiscard]]
	std::expected<std::size_t, utf8_error> simdutf_convert_utf8_to_utf16_checked_runtime(
		std::string_view bytes,
		char16_t* output) noexcept;

	[[nodiscard]]
	std::expected<std::size_t, utf8_error> simdutf_convert_utf8_to_utf32_checked_runtime(
		std::string_view bytes,
		char32_t* output) noexcept;
}

#endif
