#ifndef UTF8_RANGES_INTERNAL_SIMDUTF_RUNTIME_HPP
#define UTF8_RANGES_INTERNAL_SIMDUTF_RUNTIME_HPP

namespace details
{
	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr bool simdutf_runtime_enabled_for_target() noexcept
	{
#if defined(_M_IX86) || defined(__i386__)
		// The current compiled simdutf bridge regresses the 32-bit x86 benchmark rows.
		return false;
#else
		return true;
#endif
	}

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

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_validate_utf8_if_available(std::string_view bytes) noexcept
		-> std::optional<std::expected<void, utf8_error>>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_validate_utf8_runtime(bytes);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_validate_utf8_if_available(std::u8string_view bytes) noexcept
		-> std::optional<std::expected<void, utf8_error>>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_validate_utf8_runtime(bytes);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_utf16_length_from_valid_utf8_if_available(std::u8string_view bytes) noexcept
		-> std::optional<std::size_t>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_utf16_length_from_valid_utf8_runtime(bytes);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_utf32_length_from_valid_utf8_if_available(std::u8string_view bytes) noexcept
		-> std::optional<std::size_t>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_utf32_length_from_valid_utf8_runtime(bytes);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_convert_valid_utf8_to_utf16_if_available(
		std::u8string_view bytes,
		char16_t* output) noexcept -> std::optional<std::size_t>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_convert_valid_utf8_to_utf16_runtime(bytes, output);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_convert_valid_utf8_to_utf32_if_available(
		std::u8string_view bytes,
		char32_t* output) noexcept -> std::optional<std::size_t>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_convert_valid_utf8_to_utf32_runtime(bytes, output);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_convert_utf8_to_utf16_checked_if_available(
		std::string_view bytes,
		char16_t* output) noexcept -> std::optional<std::expected<std::size_t, utf8_error>>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_convert_utf8_to_utf16_checked_runtime(bytes, output);
		}

		return std::nullopt;
	}

	[[nodiscard]]
	UTF8_RANGES_FORCEINLINE constexpr auto simdutf_convert_utf8_to_utf32_checked_if_available(
		std::string_view bytes,
		char32_t* output) noexcept -> std::optional<std::expected<std::size_t, utf8_error>>
	{
		if (!std::is_constant_evaluated() && simdutf_runtime_enabled_for_target())
		{
			return simdutf_convert_utf8_to_utf32_checked_runtime(bytes, output);
		}

		return std::nullopt;
	}
}

#endif
