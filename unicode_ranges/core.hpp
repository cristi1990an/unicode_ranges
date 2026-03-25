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
#include <memory_resource>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <uchar.h>

#include "unicode_tables.hpp"

namespace unicode_ranges
{

struct utf8_char;
class utf8_string_view;
class utf16_string_view;
struct utf16_char;

template <typename Allocator = std::allocator<char8_t>>
class basic_utf8_string;

using utf8_string = basic_utf8_string<>;

template <typename Allocator = std::allocator<char16_t>>
class basic_utf16_string;

using utf16_string = basic_utf16_string<>;

namespace pmr
{

using utf8_string = basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>;
using utf16_string = basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>;

}

template <typename T>
concept unicode_character =
	std::same_as<std::remove_cvref_t<T>, utf8_char>
	|| std::same_as<std::remove_cvref_t<T>, utf16_char>;

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

enum class unicode_scalar_error_code
{
	invalid_scalar
};

struct unicode_scalar_error
{
	unicode_scalar_error_code code{};
	std::size_t first_invalid_element_index = 0;
};

namespace views
{
	class utf8_view;

	class reversed_utf8_view;

	template <typename CharT>
	class grapheme_cluster_view;

	template <typename CharT>
	class reversed_grapheme_cluster_view;

	class utf16_view;

	class reversed_utf16_view;

	template <typename CharT>
	class lossy_utf8_view;

	template <typename CharT>
	class lossy_utf16_view;
}

namespace details
{
	[[nodiscard]]
	constexpr utf8_string_view utf8_string_view_from_bytes_unchecked(std::u8string_view bytes) noexcept;

	[[nodiscard]]
	constexpr utf16_string_view utf16_string_view_from_code_units_unchecked(std::u16string_view code_units) noexcept;

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

	namespace encoding_constants
	{
		inline constexpr std::uint32_t ascii_scalar_max = 0x7Fu;
		inline constexpr std::uint32_t two_byte_scalar_max = 0x7FFu;
		inline constexpr std::uint32_t bmp_scalar_max = 0xFFFFu;
		inline constexpr std::uint32_t supplementary_plane_base = 0x10000u;
		inline constexpr std::uint32_t max_unicode_scalar = 0x10FFFFu;
		inline constexpr std::uint32_t replacement_character_scalar = 0xFFFDu;
		inline constexpr std::uint32_t high_surrogate_min = 0xD800u;
		inline constexpr std::uint32_t high_surrogate_max = 0xDBFFu;
		inline constexpr std::uint32_t low_surrogate_min = 0xDC00u;
		inline constexpr std::uint32_t low_surrogate_max = 0xDFFFu;
		inline constexpr std::uint32_t scalar_before_surrogate_range = 0xD7FFu;
		inline constexpr std::uint32_t scalar_after_surrogate_range = 0xE000u;
		inline constexpr std::uint32_t surrogate_payload_mask = 0x3FFu;
		inline constexpr std::size_t single_code_unit_count = 1;
		inline constexpr std::size_t two_code_unit_count = 2;
		inline constexpr std::size_t three_code_unit_count = 3;
		inline constexpr std::size_t four_code_unit_count = 4;
		inline constexpr std::size_t utf16_surrogate_code_unit_count = two_code_unit_count;
		inline constexpr std::size_t max_utf8_code_units = four_code_unit_count;
		inline constexpr std::uint32_t utf8_continuation_payload_bits = 6u;
		inline constexpr std::uint32_t utf8_two_byte_lead_shift = utf8_continuation_payload_bits;
		inline constexpr std::uint32_t utf8_three_byte_lead_shift = utf8_continuation_payload_bits * 2u;
		inline constexpr std::uint32_t utf8_four_byte_lead_shift = utf8_continuation_payload_bits * 3u;
		inline constexpr std::uint32_t utf16_high_surrogate_shift = 10u;
		inline constexpr std::uint8_t ascii_control_max = 0x1Fu;
		inline constexpr std::uint8_t ascii_delete = 0x7Fu;
		inline constexpr std::uint8_t ascii_graphic_first = 0x21u;
		inline constexpr std::uint8_t ascii_graphic_last = 0x7Eu;
		inline constexpr std::uint8_t utf8_continuation_mask = 0xC0u;
		inline constexpr std::uint8_t utf8_continuation_tag = 0x80u;
		inline constexpr std::uint8_t utf8_continuation_min = 0x80u;
		inline constexpr std::uint8_t utf8_continuation_max = 0xBFu;
		inline constexpr std::uint8_t utf8_two_byte_lead_min = 0xC2u;
		inline constexpr std::uint8_t utf8_two_byte_lead_max = 0xDFu;
		inline constexpr std::uint8_t utf8_three_byte_lead_min = 0xE0u;
		inline constexpr std::uint8_t utf8_three_byte_lead_max = 0xEFu;
		inline constexpr std::uint8_t utf8_four_byte_lead_min = 0xF0u;
		inline constexpr std::uint8_t utf8_four_byte_lead_max = 0xF4u;
		inline constexpr std::uint8_t utf8_three_byte_lead_after_e0_min = 0xE1u;
		inline constexpr std::uint8_t utf8_three_byte_lead_before_surrogate_max = 0xECu;
		inline constexpr std::uint8_t utf8_surrogate_boundary_lead = 0xEDu;
		inline constexpr std::uint8_t utf8_three_byte_lead_after_surrogate_min = 0xEEu;
		inline constexpr std::uint8_t utf8_four_byte_lead_after_f0_min = 0xF1u;
		inline constexpr std::uint8_t utf8_four_byte_lead_before_f4_max = 0xF3u;
		inline constexpr std::uint8_t utf8_two_byte_prefix_mask = 0xE0u;
		inline constexpr std::uint8_t utf8_three_byte_prefix_mask = 0xF0u;
		inline constexpr std::uint8_t utf8_four_byte_prefix_mask = 0xF8u;
		inline constexpr std::uint8_t utf8_two_byte_prefix_value = 0xC0u;
		inline constexpr std::uint8_t utf8_three_byte_prefix_value = 0xE0u;
		inline constexpr std::uint8_t utf8_four_byte_prefix_value = 0xF0u;
		inline constexpr std::uint8_t utf8_two_byte_payload_mask = 0x1Fu;
		inline constexpr std::uint8_t utf8_three_byte_payload_mask = 0x0Fu;
		inline constexpr std::uint8_t utf8_four_byte_payload_mask = 0x07u;
		inline constexpr std::uint8_t utf8_continuation_payload_mask = 0x3Fu;
		inline constexpr std::uint8_t utf8_e0_second_byte_min = 0xA0u;
		inline constexpr std::uint8_t utf8_ed_second_byte_max = 0x9Fu;
		inline constexpr std::uint8_t utf8_f0_second_byte_min = 0x90u;
		inline constexpr std::uint8_t utf8_f4_second_byte_max = 0x8Fu;
	}

	template <typename Allocator>
	using utf8_base_string = std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>;

	template <typename Allocator>
	using utf16_base_string = std::basic_string<char16_t, std::char_traits<char16_t>, Allocator>;

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
			return (byte(index) & encoding_constants::utf8_continuation_mask) == encoding_constants::utf8_continuation_tag;
		};

		const std::size_t size = value.size();
		const unsigned char b1 = byte(0);

		if (size == encoding_constants::single_code_unit_count) [[likely]]
		{
			return b1 <= encoding_constants::ascii_scalar_max;
		}

		if (size == encoding_constants::two_code_unit_count)
		{
			return b1 >= encoding_constants::utf8_two_byte_lead_min
				&& b1 <= encoding_constants::utf8_two_byte_lead_max
				&& is_cont(1);
		}

		if (size == encoding_constants::three_code_unit_count)
		{
			const unsigned char b2 = byte(1);
			return
				(
					b1 == encoding_constants::utf8_three_byte_lead_min &&
					b2 >= encoding_constants::utf8_e0_second_byte_min
					&& b2 <= encoding_constants::utf8_continuation_max &&
					is_cont(2)
				) ||
				(
					b1 >= encoding_constants::utf8_three_byte_lead_after_e0_min
					&& b1 <= encoding_constants::utf8_three_byte_lead_before_surrogate_max
					&& is_cont(1) && is_cont(2)
				) ||
				(
					b1 == encoding_constants::utf8_surrogate_boundary_lead &&
					b2 >= encoding_constants::utf8_continuation_min
					&& b2 <= encoding_constants::utf8_ed_second_byte_max &&
					is_cont(1) && is_cont(2)
				) ||
				(
					b1 >= encoding_constants::utf8_three_byte_lead_after_surrogate_min
					&& b1 <= encoding_constants::utf8_three_byte_lead_max &&
					is_cont(1) && is_cont(2)
				);
		}

		if (size == encoding_constants::max_utf8_code_units)
		{
			const unsigned char b2 = byte(1);
			return
				(
					b1 == encoding_constants::utf8_four_byte_lead_min &&
					b2 >= encoding_constants::utf8_f0_second_byte_min
					&& b2 <= encoding_constants::utf8_continuation_max &&
					is_cont(2) && is_cont(3)
				) ||
				(
					b1 >= encoding_constants::utf8_four_byte_lead_after_f0_min
					&& b1 <= encoding_constants::utf8_four_byte_lead_before_f4_max &&
					is_cont(1) && is_cont(2) && is_cont(3)
				) ||
				(
					b1 == encoding_constants::utf8_four_byte_lead_max &&
					b2 >= encoding_constants::utf8_continuation_min
					&& b2 <= encoding_constants::utf8_f4_second_byte_max &&
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
		return scalar <= encoding_constants::max_unicode_scalar
			&& !(scalar >= encoding_constants::high_surrogate_min && scalar <= encoding_constants::low_surrogate_max);
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf8_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		if (scalar <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			out[0] = static_cast<CharT>(scalar);
			return encoding_constants::single_code_unit_count;
		}

		if (scalar <= encoding_constants::two_byte_scalar_max)
		{
			out[0] = static_cast<CharT>(encoding_constants::utf8_two_byte_prefix_value | ((scalar >> encoding_constants::utf8_two_byte_lead_shift) & encoding_constants::utf8_two_byte_payload_mask));
			out[1] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | (scalar & encoding_constants::utf8_continuation_payload_mask));
			return encoding_constants::two_code_unit_count;
		}

		if (scalar <= encoding_constants::bmp_scalar_max)
		{
			out[0] = static_cast<CharT>(encoding_constants::utf8_three_byte_lead_min | ((scalar >> encoding_constants::utf8_three_byte_lead_shift) & encoding_constants::utf8_three_byte_payload_mask));
			out[1] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | ((scalar >> encoding_constants::utf8_continuation_payload_bits) & encoding_constants::utf8_continuation_payload_mask));
			out[2] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | (scalar & encoding_constants::utf8_continuation_payload_mask));
			return encoding_constants::three_code_unit_count;
		}

		out[0] = static_cast<CharT>(encoding_constants::utf8_four_byte_lead_min | ((scalar >> encoding_constants::utf8_four_byte_lead_shift) & encoding_constants::utf8_four_byte_payload_mask));
		out[1] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | ((scalar >> encoding_constants::utf8_three_byte_lead_shift) & encoding_constants::utf8_continuation_payload_mask));
		out[2] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | ((scalar >> encoding_constants::utf8_continuation_payload_bits) & encoding_constants::utf8_continuation_payload_mask));
		out[3] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | (scalar & encoding_constants::utf8_continuation_payload_mask));
		return encoding_constants::max_utf8_code_units;
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
		const bool first_is_high_surrogate = first >= encoding_constants::high_surrogate_min && first <= encoding_constants::high_surrogate_max;
		const bool first_is_low_surrogate = first >= encoding_constants::low_surrogate_min && first <= encoding_constants::low_surrogate_max;

		if (value.size() == encoding_constants::single_code_unit_count) [[likely]]
		{
			return !first_is_high_surrogate && !first_is_low_surrogate;
		}

		if (value.size() == encoding_constants::utf16_surrogate_code_unit_count)
		{
			const auto second = code_unit(1);
			const bool second_is_low_surrogate = second >= encoding_constants::low_surrogate_min && second <= encoding_constants::low_surrogate_max;
			return first_is_high_surrogate && second_is_low_surrogate;
		}

		return false;
	}

	inline constexpr bool is_utf16_high_surrogate(std::uint16_t value) noexcept
	{
		return value >= encoding_constants::high_surrogate_min && value <= encoding_constants::high_surrogate_max;
	}

	inline constexpr bool is_utf16_low_surrogate(std::uint16_t value) noexcept
	{
		return value >= encoding_constants::low_surrogate_min && value <= encoding_constants::low_surrogate_max;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char16_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf16_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		if (scalar <= encoding_constants::bmp_scalar_max) [[likely]]
		{
			out[0] = static_cast<CharT>(scalar);
			return encoding_constants::single_code_unit_count;
		}

		const auto shifted = scalar - encoding_constants::supplementary_plane_base;
		out[0] = static_cast<CharT>(encoding_constants::high_surrogate_min + (shifted >> encoding_constants::utf16_high_surrogate_shift));
		out[1] = static_cast<CharT>(encoding_constants::low_surrogate_min + (shifted & encoding_constants::surrogate_payload_mask));
		return encoding_constants::utf16_surrogate_code_unit_count;
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
			return encoding_constants::single_code_unit_count;
		}
	}

	inline constexpr std::size_t utf8_byte_count_from_lead(std::uint8_t lead) noexcept
	{
		if (lead <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			return encoding_constants::single_code_unit_count;
		}
		if ((lead & encoding_constants::utf8_two_byte_prefix_mask) == encoding_constants::utf8_two_byte_prefix_value)
		{
			return encoding_constants::two_code_unit_count;
		}
		if ((lead & encoding_constants::utf8_three_byte_prefix_mask) == encoding_constants::utf8_three_byte_prefix_value)
		{
			return encoding_constants::three_code_unit_count;
		}
		return encoding_constants::max_utf8_code_units;
	}

	inline constexpr bool is_utf8_lead_byte(std::uint8_t byte) noexcept
	{
		return (byte & encoding_constants::utf8_continuation_mask) != encoding_constants::utf8_continuation_tag;
	}

	template<typename CharT>
	inline constexpr auto validate_utf8_sequence_at(
		std::basic_string_view<CharT> value,
		std::size_t index) noexcept -> std::expected<std::size_t, utf8_error>
	{
		const std::uint8_t lead = static_cast<std::uint8_t>(value[index]);
		std::size_t expected_size = 0;
		if (lead <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			expected_size = encoding_constants::single_code_unit_count;
		}
		else if (lead >= encoding_constants::utf8_two_byte_lead_min && lead <= encoding_constants::utf8_two_byte_lead_max)
		{
			expected_size = encoding_constants::two_code_unit_count;
		}
		else if (lead >= encoding_constants::utf8_three_byte_lead_min && lead <= encoding_constants::utf8_three_byte_lead_max)
		{
			expected_size = encoding_constants::three_code_unit_count;
		}
		else if (lead >= encoding_constants::utf8_four_byte_lead_min && lead <= encoding_constants::utf8_four_byte_lead_max)
		{
			expected_size = encoding_constants::max_utf8_code_units;
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

		return expected_size;
	}

	template<typename CharT>
	inline constexpr auto validate_utf16_sequence_at(
		std::basic_string_view<CharT> value,
		std::size_t index) noexcept -> std::expected<std::size_t, utf16_error>
	{
		const auto first = static_cast<std::uint16_t>(value[index]);
		if (!is_utf16_high_surrogate(first) && !is_utf16_low_surrogate(first)) [[likely]]
		{
			return encoding_constants::single_code_unit_count;
		}

		if (is_utf16_low_surrogate(first))
		{
			return std::unexpected(utf16_error{
				.code = utf16_error_code::invalid_sequence,
				.first_invalid_code_unit_index = index
			});
		}

		if (index + encoding_constants::single_code_unit_count >= value.size()) [[unlikely]]
		{
			return std::unexpected(utf16_error{
				.code = utf16_error_code::truncated_surrogate_pair,
				.first_invalid_code_unit_index = index
			});
		}

		const auto second = static_cast<std::uint16_t>(value[index + encoding_constants::single_code_unit_count]);
		if (!is_utf16_low_surrogate(second))
		{
			return std::unexpected(utf16_error{
				.code = utf16_error_code::invalid_sequence,
				.first_invalid_code_unit_index = index
			});
		}

		return encoding_constants::utf16_surrogate_code_unit_count;
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(std::basic_string_view<CharT> ch) noexcept;

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(std::basic_string_view<CharT> ch) noexcept;

	template<typename CharT>
	inline constexpr std::expected<void, utf8_error> validate_utf8(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size())
		{
			const auto expected_size = validate_utf8_sequence_at(value, index);
			if (!expected_size) [[unlikely]]
			{
				return std::unexpected(expected_size.error());
			}

			index += *expected_size;
		}

		return {};
	}

	template<typename CharT>
	inline constexpr std::expected<void, utf16_error> validate_utf16(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size())
		{
			const auto sequence_length = validate_utf16_sequence_at(value, index);
			if (!sequence_length) [[unlikely]]
			{
				return std::unexpected(sequence_length.error());
			}

			index += *sequence_length;
		}

		return {};
	}

	inline constexpr std::expected<void, unicode_scalar_error> validate_unicode_scalars(std::wstring_view value) noexcept
	{
		for (std::size_t index = 0; index != value.size(); ++index)
		{
			if (!is_valid_unicode_scalar(static_cast<std::uint32_t>(value[index])))
			{
				return std::unexpected(unicode_scalar_error{
					.code = unicode_scalar_error_code::invalid_scalar,
					.first_invalid_element_index = index
				});
			}
		}

		return {};
	}

	template <typename Allocator>
	inline constexpr auto copy_validated_utf8_bytes(
		std::string_view bytes,
		const Allocator& alloc) -> std::expected<utf8_base_string<Allocator>, utf8_error>
	{
		utf8_base_string<Allocator> result{ alloc };
		std::optional<utf8_error> error;
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto sequence_length = validate_utf8_sequence_at(bytes, read_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					for (std::size_t i = 0; i != *sequence_length; ++i)
					{
						buffer[write_index + i] = static_cast<char8_t>(bytes[read_index + i]);
					}

					write_index += *sequence_length;
					read_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_utf8_to_utf16_checked(
		std::string_view bytes,
		const Allocator& alloc) -> std::expected<utf16_base_string<Allocator>, utf8_error>
	{
		utf16_base_string<Allocator> result{ alloc };
		std::optional<utf8_error> error;
		result.resize_and_overwrite(bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto sequence_length = validate_utf8_sequence_at(bytes, read_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					const auto scalar = decode_valid_utf8_char(
						std::basic_string_view<char>{ bytes.data() + read_index, *sequence_length });
					write_index += encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_utf16_to_utf8_checked(
		std::wstring_view code_units,
		const Allocator& alloc) -> std::expected<utf8_base_string<Allocator>, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		utf8_base_string<Allocator> result{ alloc };
		std::optional<utf16_error> error;
		result.resize_and_overwrite(code_units.size() * encoding_constants::three_code_unit_count,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto sequence_length = validate_utf16_sequence_at(code_units, read_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					const auto scalar = decode_valid_utf16_char(
						std::basic_string_view<wchar_t>{ code_units.data() + read_index, *sequence_length });
					write_index += encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
					read_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto copy_validated_utf16_code_units(
		std::wstring_view code_units,
		const Allocator& alloc) -> std::expected<utf16_base_string<Allocator>, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		utf16_base_string<Allocator> result{ alloc };
		std::optional<utf16_error> error;
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				while (write_index < code_units.size())
				{
					const auto sequence_length = validate_utf16_sequence_at(code_units, write_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					for (std::size_t i = 0; i != *sequence_length; ++i)
					{
						buffer[write_index + i] = static_cast<char16_t>(code_units[write_index + i]);
					}

					write_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_unicode_scalars_to_utf8_checked(
		std::wstring_view scalars,
		const Allocator& alloc) -> std::expected<utf8_base_string<Allocator>, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		utf8_base_string<Allocator> result{ alloc };
		std::optional<unicode_scalar_error> error;
		result.resize_and_overwrite(scalars.size() * encoding_constants::max_utf8_code_units,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t read_index = 0; read_index != scalars.size(); ++read_index)
				{
					const auto scalar = static_cast<std::uint32_t>(scalars[read_index]);
					if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
					{
						error = unicode_scalar_error{
							.code = unicode_scalar_error_code::invalid_scalar,
							.first_invalid_element_index = read_index
						};
						return std::size_t{ 0 };
					}

					write_index += encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_unicode_scalars_to_utf16_checked(
		std::wstring_view scalars,
		const Allocator& alloc) -> std::expected<utf16_base_string<Allocator>, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		utf16_base_string<Allocator> result{ alloc };
		std::optional<unicode_scalar_error> error;
		result.resize_and_overwrite(scalars.size() * encoding_constants::utf16_surrogate_code_unit_count,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t read_index = 0; read_index != scalars.size(); ++read_index)
				{
					const auto scalar = static_cast<std::uint32_t>(scalars[read_index]);
					if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
					{
						error = unicode_scalar_error{
							.code = unicode_scalar_error_code::invalid_scalar,
							.first_invalid_element_index = read_index
						};
						return std::size_t{ 0 };
					}

					write_index += encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(std::basic_string_view<CharT> ch) noexcept
	{
		const auto byte = [&ch](std::size_t index) noexcept -> std::uint8_t
		{
			return static_cast<std::uint8_t>(ch[index]);
		};

		if (ch.size() == encoding_constants::single_code_unit_count) [[likely]]
		{
			return byte(0);
		}

		if (ch.size() == encoding_constants::two_code_unit_count)
		{
			return (static_cast<std::uint32_t>(byte(0) & encoding_constants::utf8_two_byte_payload_mask) << encoding_constants::utf8_two_byte_lead_shift) |
				(static_cast<std::uint32_t>(byte(1) & encoding_constants::utf8_continuation_payload_mask));
		}

		if (ch.size() == encoding_constants::three_code_unit_count)
		{
			return (static_cast<std::uint32_t>(byte(0) & encoding_constants::utf8_three_byte_payload_mask) << encoding_constants::utf8_three_byte_lead_shift) |
				(static_cast<std::uint32_t>(byte(1) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_continuation_payload_bits) |
				(static_cast<std::uint32_t>(byte(2) & encoding_constants::utf8_continuation_payload_mask));
		}

		return (static_cast<std::uint32_t>(byte(0) & encoding_constants::utf8_four_byte_payload_mask) << encoding_constants::utf8_four_byte_lead_shift) |
			(static_cast<std::uint32_t>(byte(1) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_three_byte_lead_shift) |
			(static_cast<std::uint32_t>(byte(2) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_continuation_payload_bits) |
			(static_cast<std::uint32_t>(byte(3) & encoding_constants::utf8_continuation_payload_mask));
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(std::basic_string_view<CharT> ch) noexcept
	{
		if (ch.size() == encoding_constants::single_code_unit_count) [[likely]]
		{
			return static_cast<std::uint16_t>(ch[0]);
		}

		const auto high = static_cast<std::uint16_t>(ch[0]) - encoding_constants::high_surrogate_min;
		const auto low = static_cast<std::uint16_t>(ch[1]) - encoding_constants::low_surrogate_min;
		return encoding_constants::supplementary_plane_base + (static_cast<std::uint32_t>(high) << encoding_constants::utf16_high_surrogate_shift) + low;
	}

	struct decoded_scalar
	{
		std::uint32_t scalar = 0;
		std::size_t next_index = 0;
	};

	inline constexpr decoded_scalar decode_next_scalar(std::u8string_view text, std::size_t index) noexcept
	{
		const auto len = utf8_byte_count_from_lead(static_cast<std::uint8_t>(text[index]));
		return decoded_scalar{
			.scalar = decode_valid_utf8_char(text.substr(index, len)),
			.next_index = index + len
		};
	}

	inline constexpr decoded_scalar decode_next_scalar(std::u16string_view text, std::size_t index) noexcept
	{
		const auto first = static_cast<std::uint16_t>(text[index]);
		const auto len = is_utf16_high_surrogate(first)
			? encoding_constants::utf16_surrogate_code_unit_count
			: encoding_constants::single_code_unit_count;
		return decoded_scalar{
			.scalar = decode_valid_utf16_char(text.substr(index, len)),
			.next_index = index + len
		};
	}

		enum class grapheme_emoji_suffix_state
		{
			none,
			extended_pictographic,
			extended_pictographic_extend,
			extended_pictographic_extend_zwj
		};

		enum class grapheme_indic_suffix_state
		{
			none,
			consonant_no_linker,
			consonant_linker
		};

		struct grapheme_state
		{
			unicode::grapheme_cluster_break_property previous_break = unicode::grapheme_cluster_break_property::other;
			std::size_t trailing_regional_indicator_count = 0;
			grapheme_emoji_suffix_state emoji_suffix = grapheme_emoji_suffix_state::none;
			grapheme_indic_suffix_state indic_suffix = grapheme_indic_suffix_state::none;
		};

		inline constexpr bool is_grapheme_control(unicode::grapheme_cluster_break_property value) noexcept
		{
			using enum unicode::grapheme_cluster_break_property;
			return value == cr || value == lf || value == control;
		}

		inline constexpr grapheme_emoji_suffix_state advance_emoji_suffix(
			grapheme_emoji_suffix_state state,
			std::uint32_t scalar,
			unicode::grapheme_cluster_break_property break_property) noexcept
		{
			if (unicode::is_extended_pictographic(scalar))
			{
				return grapheme_emoji_suffix_state::extended_pictographic;
			}

			using enum unicode::grapheme_cluster_break_property;
			switch (break_property)
			{
			case extend:
				if (state == grapheme_emoji_suffix_state::extended_pictographic
					|| state == grapheme_emoji_suffix_state::extended_pictographic_extend)
				{
					return grapheme_emoji_suffix_state::extended_pictographic_extend;
				}
				return grapheme_emoji_suffix_state::none;
			case zwj:
				if (state == grapheme_emoji_suffix_state::extended_pictographic
					|| state == grapheme_emoji_suffix_state::extended_pictographic_extend)
				{
					return grapheme_emoji_suffix_state::extended_pictographic_extend_zwj;
				}
				return grapheme_emoji_suffix_state::none;
			default:
				return grapheme_emoji_suffix_state::none;
			}
		}

		inline constexpr grapheme_indic_suffix_state advance_indic_suffix(
			grapheme_indic_suffix_state state,
			std::uint32_t scalar) noexcept
		{
			using enum unicode::indic_conjunct_break_property;
			switch (unicode::indic_conjunct_break(scalar))
			{
			case consonant:
				return grapheme_indic_suffix_state::consonant_no_linker;
			case extend:
				if (state == grapheme_indic_suffix_state::consonant_no_linker
					|| state == grapheme_indic_suffix_state::consonant_linker)
				{
					return state;
				}
				return grapheme_indic_suffix_state::none;
			case linker:
				if (state == grapheme_indic_suffix_state::consonant_no_linker
					|| state == grapheme_indic_suffix_state::consonant_linker)
				{
					return grapheme_indic_suffix_state::consonant_linker;
				}
				return grapheme_indic_suffix_state::none;
			default:
				return grapheme_indic_suffix_state::none;
			}
		}

		inline constexpr grapheme_state make_initial_grapheme_state(std::uint32_t scalar) noexcept
		{
			const auto break_property = unicode::grapheme_cluster_break(scalar);
			return grapheme_state{
				.previous_break = break_property,
				.trailing_regional_indicator_count =
					break_property == unicode::grapheme_cluster_break_property::regional_indicator ? 1u : 0u,
				.emoji_suffix = advance_emoji_suffix(grapheme_emoji_suffix_state::none, scalar, break_property),
				.indic_suffix = advance_indic_suffix(grapheme_indic_suffix_state::none, scalar)
			};
		}

		inline constexpr void consume_grapheme_scalar(grapheme_state& state, std::uint32_t scalar) noexcept
		{
			const auto break_property = unicode::grapheme_cluster_break(scalar);
			state.previous_break = break_property;
			if (break_property == unicode::grapheme_cluster_break_property::regional_indicator)
			{
				++state.trailing_regional_indicator_count;
			}
			else
			{
				state.trailing_regional_indicator_count = 0;
			}

			state.emoji_suffix = advance_emoji_suffix(state.emoji_suffix, scalar, break_property);
			state.indic_suffix = advance_indic_suffix(state.indic_suffix, scalar);
		}

		inline constexpr bool should_continue_grapheme_cluster(
			const grapheme_state& state,
			std::uint32_t scalar) noexcept
		{
			using enum unicode::grapheme_cluster_break_property;

			const auto current_break = unicode::grapheme_cluster_break(scalar);
			const auto previous_break = state.previous_break;

			if (previous_break == cr && current_break == lf)
			{
				return true;
			}

			if (is_grapheme_control(previous_break) || is_grapheme_control(current_break))
			{
				return false;
			}

			if (previous_break == l && (current_break == l || current_break == v || current_break == lv || current_break == lvt))
			{
				return true;
			}

			if ((previous_break == lv || previous_break == v) && (current_break == v || current_break == t))
			{
				return true;
			}

			if ((previous_break == lvt || previous_break == t) && current_break == t)
			{
				return true;
			}

			if (state.indic_suffix == grapheme_indic_suffix_state::consonant_linker
				&& unicode::indic_conjunct_break(scalar) == unicode::indic_conjunct_break_property::consonant)
			{
				return true;
			}

			if (current_break == extend || current_break == zwj)
			{
				return true;
			}

			if (current_break == spacing_mark)
			{
				return true;
			}

			if (previous_break == prepend)
			{
				return true;
			}

			if (state.emoji_suffix == grapheme_emoji_suffix_state::extended_pictographic_extend_zwj
				&& unicode::is_extended_pictographic(scalar))
			{
				return true;
			}

			if (previous_break == regional_indicator
				&& current_break == regional_indicator
				&& (state.trailing_regional_indicator_count % 2u) == 1u)
			{
				return true;
			}

			return false;
		}

		inline constexpr std::size_t next_grapheme_boundary(std::u8string_view text, std::size_t index) noexcept
		{
			if (index >= text.size()) [[unlikely]]
			{
				return text.size();
			}

			auto current = decode_next_scalar(text, index);
			auto state = make_initial_grapheme_state(current.scalar);
			std::size_t position = current.next_index;

			while (position < text.size())
			{
				const auto next = decode_next_scalar(text, position);
				if (!should_continue_grapheme_cluster(state, next.scalar))
				{
					return position;
				}

				consume_grapheme_scalar(state, next.scalar);
				position = next.next_index;
			}

			return text.size();
		}

		inline constexpr std::size_t next_grapheme_boundary(std::u16string_view text, std::size_t index) noexcept
		{
			if (index >= text.size()) [[unlikely]]
			{
				return text.size();
			}

			auto current = decode_next_scalar(text, index);
			auto state = make_initial_grapheme_state(current.scalar);
			std::size_t position = current.next_index;

			while (position < text.size())
			{
				const auto next = decode_next_scalar(text, position);
				if (!should_continue_grapheme_cluster(state, next.scalar))
				{
					return position;
				}

				consume_grapheme_scalar(state, next.scalar);
				position = next.next_index;
			}

			return text.size();
		}

		template <typename CharT>
		inline constexpr std::size_t previous_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			index = (std::min)(text.size(), index);
			if (index == 0 || index == text.size()) [[unlikely]]
			{
				return index;
			}

			std::size_t current = 0;
			while (current < text.size())
			{
				const auto next = next_grapheme_boundary(text, current);
				if (next >= index)
				{
					return next == index ? next : current;
				}

				current = next;
			}

			return text.size();
		}

		template <typename CharT>
		inline constexpr std::size_t grapheme_count(std::basic_string_view<CharT> text) noexcept
		{
			std::size_t count = 0;
			for (std::size_t index = 0; index < text.size(); index = next_grapheme_boundary(text, index))
			{
				++count;
			}

			return count;
		}

		template <typename CharT>
		inline constexpr std::size_t floor_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			return previous_grapheme_boundary(text, index);
		}

		template <typename CharT>
		inline constexpr std::size_t ceil_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			index = (std::min)(text.size(), index);
			if (index == 0 || index == text.size()) [[unlikely]]
			{
				return index;
			}

			std::size_t current = 0;
			while (current < text.size())
			{
				if (current >= index)
				{
					return current;
				}

				const auto next = next_grapheme_boundary(text, current);
				if (next >= index)
				{
					return next;
				}

				current = next;
			}

			return text.size();
		}

		template <typename CharT>
		inline constexpr bool is_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			if (index > text.size()) [[unlikely]]
			{
				return false;
			}

			return floor_grapheme_boundary(text, index) == index;
		}

		template <typename CharT>
		inline constexpr bool grapheme_match_at(
			std::basic_string_view<CharT> text,
			std::basic_string_view<CharT> needle,
			std::size_t index) noexcept
		{
			if (needle.size() > text.size() - index)
			{
				return false;
			}

			for (std::size_t needle_index = 0; needle_index != needle.size(); ++needle_index)
			{
				if (text[index + needle_index] != needle[needle_index])
				{
					return false;
				}
			}

			return is_grapheme_boundary(text, index + needle.size());
		}

		template <typename CharT>
		inline constexpr std::size_t find_grapheme(
			std::basic_string_view<CharT> text,
			std::basic_string_view<CharT> needle,
			std::size_t pos = std::basic_string_view<CharT>::npos) noexcept
		{
			pos = ceil_grapheme_boundary(text, (std::min)(text.size(), pos));
			if (needle.empty())
			{
				return pos;
			}

			if (needle.size() > text.size() - pos)
			{
				return std::basic_string_view<CharT>::npos;
			}

			for (std::size_t current = pos; current + needle.size() <= text.size(); current = next_grapheme_boundary(text, current))
			{
				if (grapheme_match_at(text, needle, current))
				{
					return current;
				}
			}

			return std::basic_string_view<CharT>::npos;
		}

		template <typename CharT>
		inline constexpr std::size_t rfind_grapheme(
			std::basic_string_view<CharT> text,
			std::basic_string_view<CharT> needle,
			std::size_t pos = std::basic_string_view<CharT>::npos) noexcept
		{
			pos = floor_grapheme_boundary(text, (std::min)(text.size(), pos));
			if (needle.empty())
			{
				return pos;
			}

			if (needle.size() > text.size())
			{
				return std::basic_string_view<CharT>::npos;
			}

			const auto last_start = floor_grapheme_boundary(text, text.size() - needle.size());
			pos = (std::min)(pos, last_start);
			auto last_match = std::basic_string_view<CharT>::npos;
			for (std::size_t current = 0; current <= pos; current = next_grapheme_boundary(text, current))
			{
				if (grapheme_match_at(text, needle, current))
				{
					last_match = current;
				}

				if (current == pos)
				{
					break;
				}
			}

			return last_match;
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
