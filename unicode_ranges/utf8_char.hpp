#ifndef UTF8_RANGES_UTF8_CHAR_HPP
#define UTF8_RANGES_UTF8_CHAR_HPP

#include "core.hpp"
#include "unicode_tables.hpp"

namespace unicode_ranges
{

struct utf8_char;

namespace details
{
	[[nodiscard]]
	constexpr std::u8string_view utf8_char_view(const utf8_char& ch) noexcept;
}

inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version = details::unicode::unicode_version;

struct utf8_char
{
public:
	utf8_char() = default;
	static const utf8_char replacement_character;
	static const utf8_char null_terminator;

	[[nodiscard]]
	static constexpr std::optional<utf8_char> from_scalar(std::uint32_t scalar) noexcept
	{
		utf8_char value;
		if (!value.assign_scalar(scalar)) [[unlikely]]
		{
			return std::nullopt;
		}

		return value;
	}

	[[nodiscard]]
	static constexpr utf8_char from_scalar_unchecked(std::uint32_t scalar) noexcept
	{
		utf8_char value;
		value.assign_scalar_unchecked(scalar);
		return value;
	}

	template<typename CharT>
	[[nodiscard]]
	static constexpr utf8_char from_utf8_bytes_unchecked(const CharT* bytes, std::size_t size) noexcept
	{
		assert(bytes != nullptr);
		assert(details::is_single_valid_utf8_char(std::basic_string_view<CharT>{ bytes, size }));

		utf8_char value;
		switch (size)
		{
		case details::encoding_constants::single_code_unit_count:
			value.bytes_[0] = static_cast<char8_t>(bytes[0]);
			break;
		case details::encoding_constants::two_code_unit_count:
			value.bytes_[0] = static_cast<char8_t>(bytes[0]);
			value.bytes_[1] = static_cast<char8_t>(bytes[1]);
			break;
		case details::encoding_constants::three_code_unit_count:
			value.bytes_[0] = static_cast<char8_t>(bytes[0]);
			value.bytes_[1] = static_cast<char8_t>(bytes[1]);
			value.bytes_[2] = static_cast<char8_t>(bytes[2]);
			break;
		default:
			value.bytes_[0] = static_cast<char8_t>(bytes[0]);
			value.bytes_[1] = static_cast<char8_t>(bytes[1]);
			value.bytes_[2] = static_cast<char8_t>(bytes[2]);
			value.bytes_[3] = static_cast<char8_t>(bytes[3]);
			break;
		}
		return value;
	}

	[[nodiscard]]
	constexpr std::uint32_t as_scalar() const noexcept
	{
		const auto b0 = static_cast<std::uint8_t>(bytes_[0]);
		if (b0 <= details::encoding_constants::ascii_scalar_max) [[likely]]
		{
			return b0;
		}

		if (b0 <= details::encoding_constants::utf8_two_byte_lead_max)
		{
			return (static_cast<std::uint32_t>(b0 & details::encoding_constants::utf8_two_byte_payload_mask)
				<< details::encoding_constants::utf8_two_byte_lead_shift)
				| (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes_[1]) & details::encoding_constants::utf8_continuation_payload_mask));
		}

		if (b0 <= details::encoding_constants::utf8_three_byte_lead_max)
		{
			return (static_cast<std::uint32_t>(b0 & details::encoding_constants::utf8_three_byte_payload_mask)
				<< details::encoding_constants::utf8_three_byte_lead_shift)
				| (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes_[1]) & details::encoding_constants::utf8_continuation_payload_mask)
					<< details::encoding_constants::utf8_continuation_payload_bits)
				| (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes_[2]) & details::encoding_constants::utf8_continuation_payload_mask));
		}

		return (static_cast<std::uint32_t>(b0 & details::encoding_constants::utf8_four_byte_payload_mask)
			<< details::encoding_constants::utf8_four_byte_lead_shift)
			| (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes_[1]) & details::encoding_constants::utf8_continuation_payload_mask)
				<< details::encoding_constants::utf8_three_byte_lead_shift)
			| (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes_[2]) & details::encoding_constants::utf8_continuation_payload_mask)
				<< details::encoding_constants::utf8_continuation_payload_bits)
			| (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes_[3]) & details::encoding_constants::utf8_continuation_payload_mask));
	}

	constexpr operator utf16_char() const noexcept;

	template <typename Allocator = std::allocator<char8_t>>
	constexpr basic_utf8_string<Allocator> to_utf8_owned(const Allocator& alloc = Allocator()) const;

	constexpr utf8_char& operator++() noexcept
	{
		const std::uint8_t b0 = static_cast<std::uint8_t>(bytes_[0]);
		if (b0 < details::encoding_constants::ascii_scalar_max) [[likely]]
		{
			bytes_[0] = static_cast<char8_t>(b0 + 1u);
			return *this;
		}

		if (b0 == details::encoding_constants::ascii_scalar_max)
		{
			set_two(details::encoding_constants::utf8_two_byte_lead_min, details::encoding_constants::utf8_continuation_min);
			return *this;
		}

		if (b0 <= details::encoding_constants::utf8_two_byte_lead_max)
		{
			const std::uint8_t b1 = static_cast<std::uint8_t>(bytes_[1]);
			if (b1 < details::encoding_constants::utf8_continuation_max)
			{
				bytes_[1] = static_cast<char8_t>(b1 + 1u);
			}
			else if (b0 < details::encoding_constants::utf8_two_byte_lead_max)
			{
				bytes_[0] = static_cast<char8_t>(b0 + 1u);
				bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
			}
			else
			{
				set_three(
					details::encoding_constants::utf8_three_byte_lead_min,
					details::encoding_constants::utf8_e0_second_byte_min,
					details::encoding_constants::utf8_continuation_min);
			}
			return *this;
		}

		if (b0 <= details::encoding_constants::utf8_three_byte_lead_max)
		{
			const std::uint8_t b1 = static_cast<std::uint8_t>(bytes_[1]);
			const std::uint8_t b2 = static_cast<std::uint8_t>(bytes_[2]);
			if (b2 < details::encoding_constants::utf8_continuation_max)
			{
				bytes_[2] = static_cast<char8_t>(b2 + 1u);
				return *this;
			}

			bytes_[2] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
			if (b0 == details::encoding_constants::utf8_three_byte_lead_min)
			{
				if (b1 < details::encoding_constants::utf8_continuation_max)
				{
					bytes_[1] = static_cast<char8_t>(b1 + 1u);
				}
				else
				{
					set_three(
						details::encoding_constants::utf8_three_byte_lead_after_e0_min,
						details::encoding_constants::utf8_continuation_min,
						details::encoding_constants::utf8_continuation_min);
				}
			}
			else if (b0 < details::encoding_constants::utf8_surrogate_boundary_lead)
			{
				if (b1 < details::encoding_constants::utf8_continuation_max)
				{
					bytes_[1] = static_cast<char8_t>(b1 + 1u);
				}
				else if (b0 < details::encoding_constants::utf8_three_byte_lead_before_surrogate_max)
				{
					bytes_[0] = static_cast<char8_t>(b0 + 1u);
					bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
				}
				else
				{
					set_three(
						details::encoding_constants::utf8_surrogate_boundary_lead,
						details::encoding_constants::utf8_continuation_min,
						details::encoding_constants::utf8_continuation_min);
				}
			}
			else if (b0 == details::encoding_constants::utf8_surrogate_boundary_lead)
			{
				if (b1 < details::encoding_constants::utf8_ed_second_byte_max)
				{
					bytes_[1] = static_cast<char8_t>(b1 + 1u);
				}
				else
				{
					set_three(
						details::encoding_constants::utf8_three_byte_lead_after_surrogate_min,
						details::encoding_constants::utf8_continuation_min,
						details::encoding_constants::utf8_continuation_min);
				}
			}
			else if (b1 < details::encoding_constants::utf8_continuation_max)
			{
				bytes_[1] = static_cast<char8_t>(b1 + 1u);
			}
			else if (b0 < details::encoding_constants::utf8_three_byte_lead_max)
			{
				bytes_[0] = static_cast<char8_t>(b0 + 1u);
				bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
			}
			else
			{
				set_four(
					details::encoding_constants::utf8_four_byte_lead_min,
					details::encoding_constants::utf8_f0_second_byte_min,
					details::encoding_constants::utf8_continuation_min,
					details::encoding_constants::utf8_continuation_min);
			}
			return *this;
		}

		const std::uint8_t b1 = static_cast<std::uint8_t>(bytes_[1]);
		const std::uint8_t b2 = static_cast<std::uint8_t>(bytes_[2]);
		const std::uint8_t b3 = static_cast<std::uint8_t>(bytes_[3]);
		if (b3 < details::encoding_constants::utf8_continuation_max)
		{
			bytes_[3] = static_cast<char8_t>(b3 + 1u);
			return *this;
		}

		bytes_[3] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
		if (b2 < details::encoding_constants::utf8_continuation_max)
		{
			bytes_[2] = static_cast<char8_t>(b2 + 1u);
			return *this;
		}

		bytes_[2] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
		if (b0 == details::encoding_constants::utf8_four_byte_lead_min)
		{
			if (b1 < details::encoding_constants::utf8_continuation_max)
			{
				bytes_[1] = static_cast<char8_t>(b1 + 1u);
			}
			else
			{
				set_four(
					details::encoding_constants::utf8_four_byte_lead_after_f0_min,
					details::encoding_constants::utf8_continuation_min,
					details::encoding_constants::utf8_continuation_min,
					details::encoding_constants::utf8_continuation_min);
			}
		}
		else if (b0 < details::encoding_constants::utf8_four_byte_lead_max)
		{
			if (b1 < details::encoding_constants::utf8_continuation_max)
			{
				bytes_[1] = static_cast<char8_t>(b1 + 1u);
			}
			else if (b0 < details::encoding_constants::utf8_four_byte_lead_before_f4_max)
			{
				bytes_[0] = static_cast<char8_t>(b0 + 1u);
				bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_min);
			}
			else
			{
				set_four(
					details::encoding_constants::utf8_four_byte_lead_max,
					details::encoding_constants::utf8_continuation_min,
					details::encoding_constants::utf8_continuation_min,
					details::encoding_constants::utf8_continuation_min);
			}
		}
		else if (b1 < details::encoding_constants::utf8_f4_second_byte_max)
		{
			bytes_[1] = static_cast<char8_t>(b1 + 1u);
		}
		else
		{
			set_ascii(0);
		}
		return *this;
	}

	constexpr utf8_char operator++(int) noexcept
	{
		utf8_char old = *this;
		++(*this);
		return old;
	}

	constexpr utf8_char& operator--() noexcept
	{
		const std::uint8_t b0 = static_cast<std::uint8_t>(bytes_[0]);
		if (b0 == 0) [[unlikely]]
		{
			set_four(
				details::encoding_constants::utf8_four_byte_lead_max,
				details::encoding_constants::utf8_f4_second_byte_max,
				details::encoding_constants::utf8_continuation_max,
				details::encoding_constants::utf8_continuation_max);
			return *this;
		}

		if (b0 <= details::encoding_constants::ascii_scalar_max)
		{
			bytes_[0] = static_cast<char8_t>(b0 - 1u);
			return *this;
		}

		if (b0 <= details::encoding_constants::utf8_two_byte_lead_max)
		{
			const std::uint8_t b1 = static_cast<std::uint8_t>(bytes_[1]);
			if (b1 > details::encoding_constants::utf8_continuation_min)
			{
				bytes_[1] = static_cast<char8_t>(b1 - 1u);
			}
			else if (b0 > details::encoding_constants::utf8_two_byte_lead_min)
			{
				bytes_[0] = static_cast<char8_t>(b0 - 1u);
				bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
			}
			else
			{
				set_ascii(details::encoding_constants::ascii_scalar_max);
			}
			return *this;
		}

		if (b0 <= details::encoding_constants::utf8_three_byte_lead_max)
		{
			const std::uint8_t b1 = static_cast<std::uint8_t>(bytes_[1]);
			const std::uint8_t b2 = static_cast<std::uint8_t>(bytes_[2]);
			if (b2 > details::encoding_constants::utf8_continuation_min)
			{
				bytes_[2] = static_cast<char8_t>(b2 - 1u);
				return *this;
			}

			bytes_[2] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
			if (b0 == details::encoding_constants::utf8_three_byte_lead_min)
			{
				if (b1 > details::encoding_constants::utf8_e0_second_byte_min)
				{
					bytes_[1] = static_cast<char8_t>(b1 - 1u);
				}
				else
				{
					set_two(
						details::encoding_constants::utf8_two_byte_lead_max,
						details::encoding_constants::utf8_continuation_max);
				}
			}
			else if (b0 < details::encoding_constants::utf8_surrogate_boundary_lead)
			{
				if (b1 > details::encoding_constants::utf8_continuation_min)
				{
					bytes_[1] = static_cast<char8_t>(b1 - 1u);
				}
				else
				{
					bytes_[0] = static_cast<char8_t>(b0 - 1u);
					bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
				}
			}
			else if (b0 == details::encoding_constants::utf8_surrogate_boundary_lead)
			{
				if (b1 > details::encoding_constants::utf8_continuation_min)
				{
					bytes_[1] = static_cast<char8_t>(b1 - 1u);
				}
				else
				{
					set_three(
						details::encoding_constants::utf8_three_byte_lead_before_surrogate_max,
						details::encoding_constants::utf8_continuation_max,
						details::encoding_constants::utf8_continuation_max);
				}
			}
			else if (b1 > details::encoding_constants::utf8_continuation_min)
			{
				bytes_[1] = static_cast<char8_t>(b1 - 1u);
			}
			else if (b0 > details::encoding_constants::utf8_three_byte_lead_after_surrogate_min)
			{
				bytes_[0] = static_cast<char8_t>(b0 - 1u);
				bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
			}
			else
			{
				set_three(
					details::encoding_constants::utf8_surrogate_boundary_lead,
					details::encoding_constants::utf8_ed_second_byte_max,
					details::encoding_constants::utf8_continuation_max);
			}
			return *this;
		}

		const std::uint8_t b1 = static_cast<std::uint8_t>(bytes_[1]);
		const std::uint8_t b2 = static_cast<std::uint8_t>(bytes_[2]);
		const std::uint8_t b3 = static_cast<std::uint8_t>(bytes_[3]);
		if (b3 > details::encoding_constants::utf8_continuation_min)
		{
			bytes_[3] = static_cast<char8_t>(b3 - 1u);
			return *this;
		}

		bytes_[3] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
		if (b2 > details::encoding_constants::utf8_continuation_min)
		{
			bytes_[2] = static_cast<char8_t>(b2 - 1u);
			return *this;
		}

		bytes_[2] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
		if (b0 == details::encoding_constants::utf8_four_byte_lead_min)
		{
			if (b1 > details::encoding_constants::utf8_f0_second_byte_min)
			{
				bytes_[1] = static_cast<char8_t>(b1 - 1u);
			}
			else
			{
				set_three(
					details::encoding_constants::utf8_three_byte_lead_max,
					details::encoding_constants::utf8_continuation_max,
					details::encoding_constants::utf8_continuation_max);
			}
		}
		else if (b0 < details::encoding_constants::utf8_four_byte_lead_max)
		{
			if (b1 > details::encoding_constants::utf8_continuation_min)
			{
				bytes_[1] = static_cast<char8_t>(b1 - 1u);
			}
			else
			{
				bytes_[0] = static_cast<char8_t>(b0 - 1u);
				bytes_[1] = static_cast<char8_t>(details::encoding_constants::utf8_continuation_max);
			}
		}
		else if (b1 > details::encoding_constants::utf8_continuation_min)
		{
			bytes_[1] = static_cast<char8_t>(b1 - 1u);
		}
		else
		{
			set_four(
				details::encoding_constants::utf8_four_byte_lead_before_f4_max,
				details::encoding_constants::utf8_continuation_max,
				details::encoding_constants::utf8_continuation_max,
				details::encoding_constants::utf8_continuation_max);
		}
		return *this;
	}

	constexpr utf8_char operator--(int) noexcept
	{
		utf8_char old = *this;
		--(*this);
		return old;
	}

	[[nodiscard]]
	constexpr bool is_ascii() const noexcept
	{
		return static_cast<std::uint8_t>(bytes_[0]) <= details::encoding_constants::ascii_scalar_max;
	}

	[[nodiscard]]
	constexpr bool is_alphabetic() const noexcept
	{
		return details::unicode::is_alphabetic(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_alphanumeric() const noexcept
	{
		return is_alphabetic() || is_numeric();
	}

	[[nodiscard]]
	constexpr bool is_ascii_alphabetic() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(bytes_[0]);
		return is_ascii_lower_alpha(value) || is_ascii_upper_alpha(value);
	}

	[[nodiscard]]
	constexpr bool is_ascii_alphanumeric() const noexcept
	{
		return is_ascii_alphabetic() || is_ascii_digit();
	}

	[[nodiscard]]
	constexpr bool is_ascii_control() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(bytes_[0]);
		return value <= details::encoding_constants::ascii_control_max || value == details::encoding_constants::ascii_delete;
	}

	[[nodiscard]]
	constexpr bool is_ascii_digit() const noexcept
	{
		return is_ascii() && is_ascii_digit_byte(static_cast<std::uint8_t>(bytes_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_graphic() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(bytes_[0]);
		return value >= details::encoding_constants::ascii_graphic_first
			&& value <= details::encoding_constants::ascii_graphic_last;
	}

	[[nodiscard]]
	constexpr bool is_ascii_hexdigit() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(bytes_[0]);
		return is_ascii_digit_byte(value)
			|| (value >= 'a' && value <= 'f')
			|| (value >= 'A' && value <= 'F');
	}

	[[nodiscard]]
	constexpr bool is_ascii_lowercase() const noexcept
	{
		return is_ascii() && is_ascii_lower_alpha(static_cast<std::uint8_t>(bytes_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_octdigit() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(bytes_[0]);
		return value >= '0' && value <= '7';
	}

	[[nodiscard]]
	constexpr bool is_ascii_punctuation() const noexcept
	{
		return is_ascii_graphic() && !is_ascii_alphanumeric();
	}

	[[nodiscard]]
	constexpr bool is_ascii_uppercase() const noexcept
	{
		return is_ascii() && is_ascii_upper_alpha(static_cast<std::uint8_t>(bytes_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_whitespace() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(bytes_[0]);
		return value == ' '
			|| (value >= '\t' && value <= '\r');
	}

	[[nodiscard]]
	constexpr utf8_char ascii_lowercase() const noexcept
	{
		if (!is_ascii())
		{
			return *this;
		}

		utf8_char result = *this;
		const auto value = static_cast<std::uint8_t>(result.bytes_[0]);
		if (is_ascii_upper_alpha(value))
		{
			result.bytes_[0] = static_cast<char8_t>(value + ('a' - 'A'));
		}
		return result;
	}

	[[nodiscard]]
	constexpr utf8_char ascii_uppercase() const noexcept
	{
		if (!is_ascii())
		{
			return *this;
		}

		utf8_char result = *this;
		const auto value = static_cast<std::uint8_t>(result.bytes_[0]);
		if (is_ascii_lower_alpha(value))
		{
			result.bytes_[0] = static_cast<char8_t>(value - ('a' - 'A'));
		}
		return result;
	}

	[[nodiscard]]
	constexpr bool eq_ignore_ascii_case(utf8_char other) const noexcept
	{
		return ascii_lowercase() == other.ascii_lowercase();
	}

	constexpr void swap(utf8_char& other) noexcept
	{
		bytes_.swap(other.bytes_);
	}

	[[nodiscard]]
	constexpr bool is_control() const noexcept
	{
		return details::unicode::is_control(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_digit() const noexcept
	{
		return details::unicode::is_digit(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_lowercase() const noexcept
	{
		return details::unicode::is_lowercase(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_numeric() const noexcept
	{
		return details::unicode::is_numeric(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_uppercase() const noexcept
	{
		return details::unicode::is_uppercase(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_whitespace() const noexcept
	{
		return details::unicode::is_whitespace(as_scalar());
	}

	[[nodiscard]]
	constexpr std::size_t code_unit_count() const noexcept
	{
		return details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes_[0]));
	}

	template<typename CharT, typename OutIt>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>
		&& std::output_iterator<OutIt, CharT>)
	constexpr std::size_t encode_utf8(OutIt out) const noexcept
	{
		const auto size = code_unit_count();
		*out++ = static_cast<CharT>(bytes_[0]);
		if (size == details::encoding_constants::single_code_unit_count) [[likely]]
		{
			return details::encoding_constants::single_code_unit_count;
		}

		*out++ = static_cast<CharT>(bytes_[1]);
		if (size == details::encoding_constants::two_code_unit_count)
		{
			return details::encoding_constants::two_code_unit_count;
		}

		*out++ = static_cast<CharT>(bytes_[2]);
		if (size == details::encoding_constants::three_code_unit_count)
		{
			return details::encoding_constants::three_code_unit_count;
		}

		*out++ = static_cast<CharT>(bytes_[3]);
		return details::encoding_constants::max_utf8_code_units;
	}

	template<typename CharT, typename OutIt>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& details::non_narrowing_convertible<char16_t, CharT>
		&& std::output_iterator<OutIt, CharT>)
	constexpr std::size_t encode_utf16(OutIt out) const noexcept
	{
		const auto scalar = as_scalar();
		if (scalar <= details::encoding_constants::bmp_scalar_max)
		{
			*out++ = static_cast<CharT>(scalar);
			return details::encoding_constants::single_code_unit_count;
		}

		const auto shifted = scalar - details::encoding_constants::supplementary_plane_base;
		*out++ = static_cast<CharT>(details::encoding_constants::high_surrogate_min + (shifted >> details::encoding_constants::utf16_high_surrogate_shift));
		*out++ = static_cast<CharT>(details::encoding_constants::low_surrogate_min + (shifted & details::encoding_constants::surrogate_payload_mask));
		return details::encoding_constants::utf16_surrogate_code_unit_count;
	}

	friend constexpr bool operator==(const utf8_char&, const utf8_char&) = default;
	friend constexpr auto operator<=>(const utf8_char&, const utf8_char&) = default;

	friend constexpr bool operator==(const utf8_char& lhs, char rhs) noexcept
	{
		return static_cast<char>(lhs.bytes_[0]) == rhs;
	}

	friend constexpr bool operator==(const utf8_char& lhs, char8_t rhs) noexcept
	{
		return static_cast<char8_t>(lhs.bytes_[0]) == rhs;
	}

	friend std::ostream& operator<<(std::ostream& os, const utf8_char& ch)
	{
		const auto text = details::utf8_char_view(ch);
		os.write(reinterpret_cast<const char*>(text.data()), static_cast<std::streamsize>(text.size()));
		return os;
	}

private:
	template <typename CharT>
	friend class views::lossy_utf8_view;

	friend constexpr std::u8string_view details::utf8_char_view(const utf8_char& ch) noexcept;

	[[nodiscard]]
	constexpr std::u8string_view as_view() const noexcept
	{
		return { bytes_.data(), code_unit_count() };
	}

	static constexpr std::uint8_t invalid_sentinel_byte = 0xFFu;

	static constexpr utf8_char invalid_sentinel_unchecked() noexcept
	{
		utf8_char value;
		value.bytes_[0] = static_cast<char8_t>(invalid_sentinel_byte);
		return value;
	}

	static constexpr bool is_ascii_lower_alpha(std::uint8_t value) noexcept
	{
		return value >= 'a' && value <= 'z';
	}

	static constexpr bool is_ascii_upper_alpha(std::uint8_t value) noexcept
	{
		return value >= 'A' && value <= 'Z';
	}

	static constexpr bool is_ascii_digit_byte(std::uint8_t value) noexcept
	{
		return value >= '0' && value <= '9';
	}

	constexpr bool is_valid() const noexcept
	{
		return static_cast<std::uint8_t>(bytes_[0]) != invalid_sentinel_byte;
	}

	constexpr std::size_t maybe_invalid_code_unit_count() const noexcept
	{
		if (is_valid()) [[likely]]
		{
			return code_unit_count();
		}
		else
		{
			return details::encoding_constants::single_code_unit_count;
		}
	}

	constexpr void set_ascii(std::uint8_t b0) noexcept
	{
		bytes_[0] = static_cast<char8_t>(b0);
		bytes_[1] = static_cast<char8_t>(0);
		bytes_[2] = static_cast<char8_t>(0);
		bytes_[3] = static_cast<char8_t>(0);
	}

	constexpr void set_two(std::uint8_t b0, std::uint8_t b1) noexcept
	{
		bytes_[0] = static_cast<char8_t>(b0);
		bytes_[1] = static_cast<char8_t>(b1);
		bytes_[2] = static_cast<char8_t>(0);
		bytes_[3] = static_cast<char8_t>(0);
	}

	constexpr void set_three(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2) noexcept
	{
		bytes_[0] = static_cast<char8_t>(b0);
		bytes_[1] = static_cast<char8_t>(b1);
		bytes_[2] = static_cast<char8_t>(b2);
		bytes_[3] = static_cast<char8_t>(0);
	}

	constexpr void set_four(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, std::uint8_t b3) noexcept
	{
		bytes_[0] = static_cast<char8_t>(b0);
		bytes_[1] = static_cast<char8_t>(b1);
		bytes_[2] = static_cast<char8_t>(b2);
		bytes_[3] = static_cast<char8_t>(b3);
	}

	constexpr bool assign_scalar(std::uint32_t scalar) noexcept
	{
		if (!details::is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return false;
		}

		assign_scalar_unchecked(scalar);
		return true;
	}

	constexpr void assign_scalar_unchecked(std::uint32_t scalar) noexcept
	{
		assert(details::is_valid_unicode_scalar(scalar));

		if (scalar <= details::encoding_constants::ascii_scalar_max) [[likely]]
		{
			set_ascii(static_cast<std::uint8_t>(scalar));
			return;
		}

		if (scalar <= details::encoding_constants::two_byte_scalar_max)
		{
			set_two(
				static_cast<std::uint8_t>(
					details::encoding_constants::utf8_two_byte_prefix_value
					| (scalar >> details::encoding_constants::utf8_two_byte_lead_shift)),
				static_cast<std::uint8_t>(
					details::encoding_constants::utf8_continuation_tag
					| (scalar & details::encoding_constants::utf8_continuation_payload_mask)));
			return;
		}

		if (scalar <= details::encoding_constants::bmp_scalar_max)
		{
			set_three(
				static_cast<std::uint8_t>(
					details::encoding_constants::utf8_three_byte_prefix_value
					| (scalar >> details::encoding_constants::utf8_three_byte_lead_shift)),
				static_cast<std::uint8_t>(
					details::encoding_constants::utf8_continuation_tag
					| ((scalar >> details::encoding_constants::utf8_continuation_payload_bits)
						& details::encoding_constants::utf8_continuation_payload_mask)),
				static_cast<std::uint8_t>(
					details::encoding_constants::utf8_continuation_tag
					| (scalar & details::encoding_constants::utf8_continuation_payload_mask)));
			return;
		}

		set_four(
			static_cast<std::uint8_t>(
				details::encoding_constants::utf8_four_byte_prefix_value
				| (scalar >> details::encoding_constants::utf8_four_byte_lead_shift)),
			static_cast<std::uint8_t>(
				details::encoding_constants::utf8_continuation_tag
				| ((scalar >> details::encoding_constants::utf8_three_byte_lead_shift)
					& details::encoding_constants::utf8_continuation_payload_mask)),
			static_cast<std::uint8_t>(
				details::encoding_constants::utf8_continuation_tag
				| ((scalar >> details::encoding_constants::utf8_continuation_payload_bits)
					& details::encoding_constants::utf8_continuation_payload_mask)),
			static_cast<std::uint8_t>(
				details::encoding_constants::utf8_continuation_tag
				| (scalar & details::encoding_constants::utf8_continuation_payload_mask)));
	}

	std::array<char8_t, details::encoding_constants::max_utf8_code_units> bytes_{};
};

static_assert(std::is_trivially_copyable_v<utf8_char>);
inline constexpr utf8_char utf8_char::replacement_character = utf8_char::from_scalar_unchecked(details::encoding_constants::replacement_character_scalar);
inline constexpr utf8_char utf8_char::null_terminator = utf8_char{};

namespace details
{
	[[nodiscard]]
	inline constexpr std::u8string_view utf8_char_view(const utf8_char& ch) noexcept
	{
		return ch.as_view();
	}
}

namespace literals
{
	template<details::literals::constexpr_utf8_character Str>
	consteval utf8_char operator ""_u8c()
	{
		const auto sv = std::basic_string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		return utf8_char::from_utf8_bytes_unchecked(sv.data(), sv.size());
	}
}

}

namespace std
{
	template<>
	struct hash<unicode_ranges::utf8_char>
	{
		std::size_t operator()(const unicode_ranges::utf8_char& value) const noexcept
		{
			std::array<char, 4> buffer{};
			const auto len = value.encode_utf8<char>(buffer.begin());
			return std::hash<std::string_view>{}(std::string_view{ buffer.data(), len });
		}
	};

	template<>
	struct formatter<unicode_ranges::utf8_char, char>
	{
		static constexpr std::size_t max_spec_size = 64;

		static constexpr bool is_numeric_presentation(char c) noexcept
		{
			return c == 'd' || c == 'b' || c == 'B' || c == 'o' || c == 'x' || c == 'X';
		}

		static constexpr bool is_ascii_alpha(char c) noexcept
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
		}

		std::array<char, max_spec_size> spec_{};
		std::size_t spec_len_ = 0;
		char presentation_ = '\0';
		bool use_numeric_formatter_ = false;
		std::formatter<std::string_view, char> text_formatter_{};
		std::formatter<std::uint32_t, char> numeric_formatter_{};

		constexpr auto parse(std::format_parse_context& ctx)
		{
			auto it = ctx.begin();
			auto end = ctx.end();
			while (it != end && *it != '}')
			{
				if (spec_len_ >= max_spec_size) [[unlikely]]
				{
					throw std::format_error("utf8_char format specifier is too long");
				}
				spec_[spec_len_++] = *it++;
			}

			if (it == end) [[unlikely]]
			{
				throw std::format_error("missing closing brace in utf8_char format specifier");
			}

			if (spec_len_ > 0 && is_ascii_alpha(spec_[spec_len_ - 1]))
			{
				presentation_ = spec_[spec_len_ - 1];
			}

			if (presentation_ != '\0' && presentation_ != 'c' && !is_numeric_presentation(presentation_)) [[unlikely]]
			{
				throw std::format_error("unsupported utf8_char presentation type");
			}

			use_numeric_formatter_ = is_numeric_presentation(presentation_);
			if (use_numeric_formatter_)
			{
				std::format_parse_context numeric_ctx{ std::string_view{ spec_.data(), spec_len_ } };
				(void)numeric_formatter_.parse(numeric_ctx);
			}
			else
			{
				std::array<char, max_spec_size> text_spec = spec_;
				if (presentation_ == 'c')
				{
					text_spec[spec_len_ - 1] = 's';
				}
				std::format_parse_context text_ctx{ std::string_view{ text_spec.data(), spec_len_ } };
				(void)text_formatter_.parse(text_ctx);
			}

			return it;
		}

		template<typename FormatContext>
		auto format(const unicode_ranges::utf8_char& value, FormatContext& ctx) const
		{
			if (use_numeric_formatter_) [[unlikely]]
			{
				return numeric_formatter_.format(value.as_scalar(), ctx);
			}

			std::array<char, 4> buffer{};
			const auto len = value.encode_utf8<char>(buffer.begin());
			const std::string_view text{ buffer.data(), len };
			return text_formatter_.format(text, ctx);
		}
	};

	template<>
	struct formatter<unicode_ranges::utf8_char, wchar_t>
	{
		static constexpr std::size_t max_spec_size = 64;

		static constexpr bool is_numeric_presentation(wchar_t c) noexcept
		{
			return c == L'd' || c == L'b' || c == L'B' || c == L'o' || c == L'x' || c == L'X';
		}

		static constexpr bool is_ascii_alpha(wchar_t c) noexcept
		{
			return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z');
		}

		std::array<wchar_t, max_spec_size> spec_{};
		std::size_t spec_len_ = 0;
		wchar_t presentation_ = L'\0';
		bool use_numeric_formatter_ = false;
		std::formatter<std::wstring_view, wchar_t> text_formatter_{};
		std::formatter<std::uint32_t, wchar_t> numeric_formatter_{};

		constexpr auto parse(std::wformat_parse_context& ctx)
		{
			auto it = ctx.begin();
			auto end = ctx.end();
			while (it != end && *it != L'}')
			{
				if (spec_len_ >= max_spec_size) [[unlikely]]
				{
					throw std::format_error("utf8_char format specifier is too long");
				}
				spec_[spec_len_++] = *it++;
			}

			if (it == end) [[unlikely]]
			{
				throw std::format_error("missing closing brace in utf8_char format specifier");
			}

			if (spec_len_ > 0 && is_ascii_alpha(spec_[spec_len_ - 1]))
			{
				presentation_ = spec_[spec_len_ - 1];
			}

			if (presentation_ != L'\0' && presentation_ != L'c' && !is_numeric_presentation(presentation_)) [[unlikely]]
			{
				throw std::format_error("unsupported utf8_char presentation type");
			}

			use_numeric_formatter_ = is_numeric_presentation(presentation_);
			if (use_numeric_formatter_)
			{
				std::wformat_parse_context numeric_ctx{ std::wstring_view{ spec_.data(), spec_len_ } };
				(void)numeric_formatter_.parse(numeric_ctx);
			}
			else
			{
				std::array<wchar_t, max_spec_size> text_spec = spec_;
				if (presentation_ == L'c')
				{
					text_spec[spec_len_ - 1] = L's';
				}
				std::wformat_parse_context text_ctx{ std::wstring_view{ text_spec.data(), spec_len_ } };
				(void)text_formatter_.parse(text_ctx);
			}

			return it;
		}

		template<typename FormatContext>
		auto format(const unicode_ranges::utf8_char& value, FormatContext& ctx) const
		{
			if (use_numeric_formatter_) [[unlikely]]
			{
				return numeric_formatter_.format(value.as_scalar(), ctx);
			}

			std::array<wchar_t, 2> buffer{};
			const auto len = unicode_ranges::details::encode_unicode_scalar_wchar_unchecked(value.as_scalar(), buffer.data());
			const std::wstring_view text{ buffer.data(), len };
			return text_formatter_.format(text, ctx);
		}
	};
}

#endif // UTF8_RANGES_UTF8_CHAR_HPP
