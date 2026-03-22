#ifndef UTF8_RANGES_CORE_HPP
#define UTF8_RANGES_CORE_HPP

#include <ranges>
#include <algorithm>

#include <array>
#include <charconv>
#include <compare>
#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <uchar.h>

namespace unicode_ranges
{

class utf8_string_view;
class utf16_string_view;
struct utf16_char;

template <typename Allocator = std::allocator<char8_t>>
class basic_utf8_string;

using utf8_string = basic_utf8_string<>;

template <typename Allocator = std::allocator<char16_t>>
class basic_utf16_string;

using utf16_string = basic_utf16_string<>;

enum class utf8_error_code
{
	invalid_lead_byte,
	truncated_sequence,
	invalid_sequence
};

struct utf8_error
{
	utf8_error_code code{};
	std::size_t first_invalid_byte_index = 0;
};

enum class utf16_error_code
{
	truncated_surrogate_pair,
	invalid_sequence
};

struct utf16_error
{
	utf16_error_code code{};
	std::size_t first_invalid_code_unit_index = 0;
};

namespace views
{
	class utf8_view;

	class reversed_utf8_view;

	class utf16_view;

	class reversed_utf16_view;

	template <typename CharT>
	class lossy_utf8_view;

	template <typename CharT>
	class lossy_utf16_view;
}

namespace details
{
	template <typename Derived, typename View = utf8_string_view>
	class utf8_string_crtp;

	template <typename Derived, typename View = utf16_string_view>
	class utf16_string_crtp;

	template<typename From, typename To>
	concept non_narrowing_convertible =
		requires(From value)
	{
		To{ value };
	};

	template<typename CharT>
	inline constexpr bool is_single_valid_utf8_char(std::basic_string_view<CharT> value) noexcept
	{
		if (value.empty()) [[unlikely]]
		{
			return false;
		}

		const auto byte = [&value](std::size_t index) noexcept -> std::uint8_t
		{
			return static_cast<std::uint8_t>(value[index]);
		};

		const auto is_cont = [&byte](std::size_t index) noexcept -> bool
		{
			return (byte(index) & 0xC0u) == 0x80u;
		};

		const std::size_t size = value.size();
		const unsigned char b1 = byte(0);

		if (size == 1) [[likely]]
		{
			return b1 <= 0x7Fu;
		}

		if (size == 2)
		{
			return b1 >= 0xC2u && b1 <= 0xDFu && is_cont(1);
		}

		if (size == 3)
		{
			const unsigned char b2 = byte(1);
			return
				(
					b1 == 0xE0u &&
					b2 >= 0xA0u && b2 <= 0xBFu &&
					is_cont(2)
				) ||
				(
					b1 >= 0xE1u && b1 <= 0xECu &&
					is_cont(1) && is_cont(2)
				) ||
				(
					b1 == 0xEDu &&
					b2 >= 0x80u && b2 <= 0x9Fu &&
					is_cont(2)
				) ||
				(
					b1 >= 0xEEu && b1 <= 0xEFu &&
					is_cont(1) && is_cont(2)
				);
		}

		if (size == 4)
		{
			const unsigned char b2 = byte(1);
			return
				(
					b1 == 0xF0u &&
					b2 >= 0x90u && b2 <= 0xBFu &&
					is_cont(2) && is_cont(3)
				) ||
				(
					b1 >= 0xF1u && b1 <= 0xF3u &&
					is_cont(1) && is_cont(2) && is_cont(3)
				) ||
				(
					b1 == 0xF4u &&
					b2 >= 0x80u && b2 <= 0x8Fu &&
					is_cont(2) && is_cont(3)
				);
		}

		return false;
	}

	template <typename R, typename T>
	concept container_compatible_range =
		std::ranges::input_range<R> &&
		std::convertible_to<std::ranges::range_reference_t<R>, T>;

	inline constexpr bool is_valid_unicode_scalar(std::uint32_t scalar) noexcept
	{
		return scalar <= 0x10FFFFu && !(scalar >= 0xD800u && scalar <= 0xDFFFu);
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf8_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		if (scalar <= 0x7Fu) [[likely]]
		{
			out[0] = static_cast<CharT>(scalar);
			return 1;
		}

		if (scalar <= 0x7FFu)
		{
			out[0] = static_cast<CharT>(0xC0u | ((scalar >> 6) & 0x1Fu));
			out[1] = static_cast<CharT>(0x80u | (scalar & 0x3Fu));
			return 2;
		}

		if (scalar <= 0xFFFFu)
		{
			out[0] = static_cast<CharT>(0xE0u | ((scalar >> 12) & 0x0Fu));
			out[1] = static_cast<CharT>(0x80u | ((scalar >> 6) & 0x3Fu));
			out[2] = static_cast<CharT>(0x80u | (scalar & 0x3Fu));
			return 3;
		}

		out[0] = static_cast<CharT>(0xF0u | ((scalar >> 18) & 0x07u));
		out[1] = static_cast<CharT>(0x80u | ((scalar >> 12) & 0x3Fu));
		out[2] = static_cast<CharT>(0x80u | ((scalar >> 6) & 0x3Fu));
		out[3] = static_cast<CharT>(0x80u | (scalar & 0x3Fu));
		return 4;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf8(std::uint32_t scalar, CharT* out) noexcept
	{
		if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return 0;
		}
		return encode_unicode_scalar_utf8_unchecked(scalar, out);
	}

	template<typename CharT>
	inline constexpr bool is_single_valid_utf16_char(std::basic_string_view<CharT> value) noexcept
	{
		if (value.empty()) [[unlikely]]
		{
			return false;
		}

		const auto code_unit = [&value](std::size_t index) noexcept -> std::uint16_t
		{
			return static_cast<std::uint16_t>(value[index]);
		};

		const auto first = code_unit(0);
		const bool first_is_high_surrogate = first >= 0xD800u && first <= 0xDBFFu;
		const bool first_is_low_surrogate = first >= 0xDC00u && first <= 0xDFFFu;

		if (value.size() == 1) [[likely]]
		{
			return !first_is_high_surrogate && !first_is_low_surrogate;
		}

		if (value.size() == 2)
		{
			const auto second = code_unit(1);
			const bool second_is_low_surrogate = second >= 0xDC00u && second <= 0xDFFFu;
			return first_is_high_surrogate && second_is_low_surrogate;
		}

		return false;
	}

	inline constexpr bool is_utf16_high_surrogate(std::uint16_t value) noexcept
	{
		return value >= 0xD800u && value <= 0xDBFFu;
	}

	inline constexpr bool is_utf16_low_surrogate(std::uint16_t value) noexcept
	{
		return value >= 0xDC00u && value <= 0xDFFFu;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char16_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf16_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		if (scalar <= 0xFFFFu) [[likely]]
		{
			out[0] = static_cast<CharT>(scalar);
			return 1;
		}

		const auto shifted = scalar - 0x10000u;
		out[0] = static_cast<CharT>(0xD800u + (shifted >> 10));
		out[1] = static_cast<CharT>(0xDC00u + (shifted & 0x3FFu));
		return 2;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char16_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf16(std::uint32_t scalar, CharT* out) noexcept
	{
		if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return 0;
		}
		return encode_unicode_scalar_utf16_unchecked(scalar, out);
	}

	inline constexpr std::size_t encode_unicode_scalar_wchar_unchecked(std::uint32_t scalar, wchar_t* out) noexcept
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			return encode_unicode_scalar_utf16_unchecked(scalar, out);
		}
		else
		{
			out[0] = static_cast<wchar_t>(scalar);
			return 1;
		}
	}

	inline constexpr std::size_t utf8_byte_count_from_lead(std::uint8_t lead) noexcept
	{
		if (lead <= 0x7Fu) [[likely]]
		{
			return 1;
		}
		if ((lead & 0xE0u) == 0xC0u)
		{
			return 2;
		}
		if ((lead & 0xF0u) == 0xE0u)
		{
			return 3;
		}
		return 4;
	}

	inline constexpr bool is_utf8_lead_byte(std::uint8_t byte) noexcept
	{
		return (byte & 0xC0u) != 0x80u;
	}

	template<typename CharT>
	inline constexpr std::expected<void, utf8_error> validate_utf8(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size())
		{
			const std::uint8_t lead = static_cast<std::uint8_t>(value[index]);
			std::size_t expected_size = 0;
			if (lead <= 0x7Fu) [[likely]]
			{
				expected_size = 1;
			}
			else if (lead >= 0xC2u && lead <= 0xDFu)
			{
				expected_size = 2;
			}
			else if (lead >= 0xE0u && lead <= 0xEFu)
			{
				expected_size = 3;
			}
			else if (lead >= 0xF0u && lead <= 0xF4u)
			{
				expected_size = 4;
			}
			else
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::invalid_lead_byte,
					.first_invalid_byte_index = index
				});
			}

			if (expected_size > value.size() - index) [[unlikely]]
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = index
				});
			}

			if (!details::is_single_valid_utf8_char(value.substr(index, expected_size))) [[unlikely]]
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::invalid_sequence,
					.first_invalid_byte_index = index
				});
			}

			index += expected_size;
		}

		return {};
	}

	template<typename CharT>
	inline constexpr std::expected<void, utf16_error> validate_utf16(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size())
		{
			const auto first = static_cast<std::uint16_t>(value[index]);
			if (!is_utf16_high_surrogate(first) && !is_utf16_low_surrogate(first)) [[likely]]
			{
				++index;
				continue;
			}

			if (is_utf16_low_surrogate(first))
			{
				return std::unexpected(utf16_error{
					.code = utf16_error_code::invalid_sequence,
					.first_invalid_code_unit_index = index
				});
			}

			if (index + 1 >= value.size()) [[unlikely]]
			{
				return std::unexpected(utf16_error{
					.code = utf16_error_code::truncated_surrogate_pair,
					.first_invalid_code_unit_index = index
				});
			}

			const auto second = static_cast<std::uint16_t>(value[index + 1]);
			if (!is_utf16_low_surrogate(second))
			{
				return std::unexpected(utf16_error{
					.code = utf16_error_code::invalid_sequence,
					.first_invalid_code_unit_index = index
				});
			}

			index += 2;
		}

		return {};
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(std::basic_string_view<CharT> ch) noexcept
	{
		const auto byte = [&ch](std::size_t index) noexcept -> std::uint8_t
		{
			return static_cast<std::uint8_t>(ch[index]);
		};

		if (ch.size() == 1) [[likely]]
		{
			return byte(0);
		}

		if (ch.size() == 2)
		{
			return (static_cast<std::uint32_t>(byte(0) & 0x1Fu) << 6) |
				(static_cast<std::uint32_t>(byte(1) & 0x3Fu));
		}

		if (ch.size() == 3)
		{
			return (static_cast<std::uint32_t>(byte(0) & 0x0Fu) << 12) |
				(static_cast<std::uint32_t>(byte(1) & 0x3Fu) << 6) |
				(static_cast<std::uint32_t>(byte(2) & 0x3Fu));
		}

		return (static_cast<std::uint32_t>(byte(0) & 0x07u) << 18) |
			(static_cast<std::uint32_t>(byte(1) & 0x3Fu) << 12) |
			(static_cast<std::uint32_t>(byte(2) & 0x3Fu) << 6) |
			(static_cast<std::uint32_t>(byte(3) & 0x3Fu));
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(std::basic_string_view<CharT> ch) noexcept
	{
		if (ch.size() == 1) [[likely]]
		{
			return static_cast<std::uint16_t>(ch[0]);
		}

		const auto high = static_cast<std::uint16_t>(ch[0]) - 0xD800u;
		const auto low = static_cast<std::uint16_t>(ch[1]) - 0xDC00u;
		return 0x10000u + (static_cast<std::uint32_t>(high) << 10) + low;
	}

	namespace literals
	{
		template<typename CharT, std::size_t N>
		struct constexpr_utf8_character
		{
			CharT p[N]{};

			static constexpr std::size_t SIZE = N;
			using CHAR_T = CharT;

			consteval constexpr_utf8_character(CharT const(&pp)[N])
			{
				std::ranges::copy(pp, p);

				if (!details::is_single_valid_utf8_char(std::basic_string_view<CharT>{ &p[0], N - 1 }))
				{
					throw std::invalid_argument("literal must contain exactly one valid UTF-8 character");
				}
			}

			consteval const CharT* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf8_string
		{
			char8_t p[N]{};

			static constexpr std::size_t SIZE = N;

			consteval constexpr_utf8_string(CharT const(&pp)[N])
			{
				for (std::size_t i = 0; i < N; ++i)
				{
					p[i] = static_cast<char8_t>(pp[i]);
				}
			}

			consteval const char8_t* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf16_character
		{
			CharT p[N]{};

			static constexpr std::size_t SIZE = N;
			using CHAR_T = CharT;

			consteval constexpr_utf16_character(CharT const(&pp)[N])
			{
				std::ranges::copy(pp, p);

				if (!details::is_single_valid_utf16_char(std::basic_string_view<CharT>{ &p[0], N - 1 }))
				{
					throw std::invalid_argument("literal must contain exactly one valid UTF-16 character");
				}
			}

			consteval const CharT* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf16_string
		{
			char16_t p[N]{};

			static constexpr std::size_t SIZE = N;

			consteval constexpr_utf16_string(CharT const(&pp)[N])
			{
				for (std::size_t i = 0; i < N; ++i)
				{
					p[i] = static_cast<char16_t>(pp[i]);
				}
			}

			consteval const char16_t* data() const noexcept
			{
				return &p[0];
			}
		};
	}
}

}

#endif // UTF8_RANGES_CORE_HPP
