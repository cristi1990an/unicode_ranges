#ifndef UTF8_RANGES_UTF16_CHAR_HPP
#define UTF8_RANGES_UTF16_CHAR_HPP

#include "utf8_char.hpp"

namespace unicode_ranges
{

struct utf16_char;

namespace details
{
	[[nodiscard]]
	constexpr std::u16string_view utf16_char_view(const utf16_char& ch) noexcept;
}

struct utf16_char
{
public:
	utf16_char() = default;
	static const utf16_char replacement_character;
	static const utf16_char null_terminator;

	[[nodiscard]]
	static constexpr std::optional<utf16_char> from_scalar(std::uint32_t scalar) noexcept
	{
		utf16_char value;
		if (!value.assign_scalar(scalar)) [[unlikely]]
		{
			return std::nullopt;
		}

		return value;
	}

	[[nodiscard]]
	static constexpr utf16_char from_scalar_unchecked(std::uint32_t scalar) noexcept
	{
		utf16_char value;
		value.assign_scalar_unchecked(scalar);
		return value;
	}

	template<typename CharT>
	[[nodiscard]]
	static constexpr std::optional<utf16_char> from_utf16_code_units(const CharT* code_units, std::size_t size) noexcept
	{
		if (!details::is_single_valid_utf16_char(std::basic_string_view<CharT>{ code_units, size })) [[unlikely]]
		{
			return std::nullopt;
		}

		return from_utf16_code_units_unchecked(code_units, size);
	}

	template<typename CharT>
	[[nodiscard]]
	static constexpr utf16_char from_utf16_code_units_unchecked(const CharT* code_units, std::size_t size) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(code_units != nullptr);
		UTF8_RANGES_DEBUG_ASSERT(details::is_single_valid_utf16_char(std::basic_string_view<CharT>{ code_units, size }));

		utf16_char value;
		std::ranges::copy_n(code_units, size, value.code_units_.begin());
		return value;
	}

	[[nodiscard]]
	constexpr std::uint32_t as_scalar() const noexcept
	{
		const auto first = static_cast<std::uint16_t>(code_units_[0]);
		if (code_units_[1] == 0) [[likely]]
		{
			return first;
		}

		const auto second = static_cast<std::uint16_t>(code_units_[1]);
		return 0x10000u
			+ ((static_cast<std::uint32_t>(first) - 0xD800u) << 10)
			+ (static_cast<std::uint32_t>(second) - 0xDC00u);
	}

	constexpr operator utf8_char() const noexcept;

	template <typename Allocator = std::allocator<char16_t>>
	constexpr basic_utf16_string<Allocator> to_utf16_owned(const Allocator& alloc = Allocator()) const;

	constexpr utf16_char& operator++() noexcept
	{
		const auto first = static_cast<std::uint16_t>(code_units_[0]);
		if (!details::is_utf16_high_surrogate(first)) [[likely]]
		{
			if (first == details::encoding_constants::bmp_scalar_max)
			{
				code_units_[0] = static_cast<char16_t>(details::encoding_constants::high_surrogate_min);
				code_units_[1] = static_cast<char16_t>(details::encoding_constants::low_surrogate_min);
				return *this;
			}

			const auto next = first + 1u;
			code_units_[0] = static_cast<char16_t>(
				next == details::encoding_constants::high_surrogate_min
					? details::encoding_constants::scalar_after_surrogate_range
					: next);
			return *this;
		}

		const auto second = static_cast<std::uint16_t>(code_units_[1]);
		if (second < details::encoding_constants::low_surrogate_max)
		{
			code_units_[1] = static_cast<char16_t>(second + 1u);
			return *this;
		}

		code_units_[1] = static_cast<char16_t>(details::encoding_constants::low_surrogate_min);
		if (first < details::encoding_constants::high_surrogate_max)
		{
			code_units_[0] = static_cast<char16_t>(first + 1u);
		}
		else
		{
			code_units_[0] = 0;
		}

		return *this;
	}

	constexpr utf16_char operator++(int) noexcept
	{
		utf16_char old = *this;
		++(*this);
		return old;
	}

	constexpr utf16_char& operator--() noexcept
	{
		const auto first = static_cast<std::uint16_t>(code_units_[0]);
		if (!details::is_utf16_high_surrogate(first)) [[likely]]
		{
			if (first == 0u)
			{
				code_units_[0] = static_cast<char16_t>(details::encoding_constants::high_surrogate_max);
				code_units_[1] = static_cast<char16_t>(details::encoding_constants::low_surrogate_max);
				return *this;
			}

			code_units_[0] = static_cast<char16_t>(
				first == details::encoding_constants::scalar_after_surrogate_range
					? details::encoding_constants::scalar_before_surrogate_range
					: first - 1u);
			return *this;
		}

		const auto second = static_cast<std::uint16_t>(code_units_[1]);
		if (second > details::encoding_constants::low_surrogate_min)
		{
			code_units_[1] = static_cast<char16_t>(second - 1u);
			return *this;
		}

		if (first > details::encoding_constants::high_surrogate_min)
		{
			code_units_[0] = static_cast<char16_t>(first - 1u);
			code_units_[1] = static_cast<char16_t>(details::encoding_constants::low_surrogate_max);
		}
		else
		{
			code_units_[0] = static_cast<char16_t>(details::encoding_constants::scalar_before_surrogate_range);
			code_units_[1] = 0;
		}

		return *this;
	}

	constexpr utf16_char operator--(int) noexcept
	{
		utf16_char old = *this;
		--(*this);
		return old;
	}

	[[nodiscard]]
	constexpr bool is_ascii() const noexcept
	{
		return code_units_[1] == 0
			&& static_cast<std::uint16_t>(code_units_[0]) <= details::encoding_constants::ascii_scalar_max;
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
	constexpr unicode_general_category general_category() const noexcept
	{
		return details::unicode::general_category(as_scalar());
	}

	[[nodiscard]]
	constexpr std::uint8_t canonical_combining_class() const noexcept
	{
		return details::unicode::canonical_combining_class(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_grapheme_break_property grapheme_break_property() const noexcept
	{
		return details::unicode::grapheme_break_property(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_script script() const noexcept
	{
		return details::unicode::script(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_east_asian_width east_asian_width() const noexcept
	{
		return details::unicode::east_asian_width(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_line_break_class line_break_class() const noexcept
	{
		return details::unicode::line_break_class(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_bidi_class bidi_class() const noexcept
	{
		return details::unicode::bidi_class(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_word_break_property word_break_property() const noexcept
	{
		return details::unicode::word_break_property(as_scalar());
	}

	[[nodiscard]]
	constexpr unicode_sentence_break_property sentence_break_property() const noexcept
	{
		return details::unicode::sentence_break_property(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_emoji() const noexcept
	{
		return details::unicode::is_emoji(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_emoji_presentation() const noexcept
	{
		return details::unicode::is_emoji_presentation(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_extended_pictographic() const noexcept
	{
		return details::unicode::is_extended_pictographic(as_scalar());
	}

	[[nodiscard]]
	constexpr bool is_ascii_alphabetic() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_units_[0]);
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

		const auto value = static_cast<std::uint8_t>(code_units_[0]);
		return value <= details::encoding_constants::ascii_control_max || value == details::encoding_constants::ascii_delete;
	}

	[[nodiscard]]
	constexpr bool is_ascii_digit() const noexcept
	{
		return is_ascii() && is_ascii_digit_byte(static_cast<std::uint8_t>(code_units_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_graphic() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_units_[0]);
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

		const auto value = static_cast<std::uint8_t>(code_units_[0]);
		return is_ascii_digit_byte(value)
			|| (value >= 'a' && value <= 'f')
			|| (value >= 'A' && value <= 'F');
	}

	[[nodiscard]]
	constexpr bool is_ascii_lowercase() const noexcept
	{
		return is_ascii() && is_ascii_lower_alpha(static_cast<std::uint8_t>(code_units_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_octdigit() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_units_[0]);
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
		return is_ascii() && is_ascii_upper_alpha(static_cast<std::uint8_t>(code_units_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_whitespace() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_units_[0]);
		return value == ' '
			|| (value >= '\t' && value <= '\r');
	}

	[[nodiscard]]
	constexpr utf16_char ascii_lowercase() const noexcept
	{
		if (!is_ascii())
		{
			return *this;
		}

		utf16_char result = *this;
		const auto value = static_cast<std::uint8_t>(result.code_units_[0]);
		if (is_ascii_upper_alpha(value))
		{
			result.code_units_[0] = static_cast<char16_t>(value + ('a' - 'A'));
		}
		return result;
	}

	[[nodiscard]]
	constexpr utf16_char ascii_uppercase() const noexcept
	{
		if (!is_ascii())
		{
			return *this;
		}

		utf16_char result = *this;
		const auto value = static_cast<std::uint8_t>(result.code_units_[0]);
		if (is_ascii_lower_alpha(value))
		{
			result.code_units_[0] = static_cast<char16_t>(value - ('a' - 'A'));
		}
		return result;
	}

	[[nodiscard]]
	constexpr bool eq_ignore_ascii_case(utf16_char other) const noexcept
	{
		return ascii_lowercase() == other.ascii_lowercase();
	}

	constexpr void swap(utf16_char& other) noexcept
	{
		code_units_.swap(other.code_units_);
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
		const auto first = static_cast<std::uint16_t>(code_units_[0]);
		return (first >= details::encoding_constants::high_surrogate_min
			&& first <= details::encoding_constants::high_surrogate_max)
			? details::encoding_constants::utf16_surrogate_code_unit_count
			: details::encoding_constants::single_code_unit_count;
	}

	template<typename CharT, typename OutIt>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& details::non_narrowing_convertible<char16_t, CharT>
		&& std::output_iterator<OutIt, CharT>)
	constexpr std::size_t encode_utf16(OutIt out) const noexcept
	{
		const auto text = as_view();
		std::ranges::copy_n(text.data(), text.size(), out);
		return text.size();
	}

	template<typename CharT, typename OutIt>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>
		&& std::output_iterator<OutIt, CharT>)
	constexpr std::size_t encode_utf8(OutIt out) const noexcept
	{
		std::array<CharT, details::encoding_constants::max_utf8_code_units> buffer{};
		const auto len = details::encode_unicode_scalar_utf8_unchecked(as_scalar(), buffer.data());
		std::ranges::copy_n(buffer.data(), len, out);
		return len;
	}

	friend constexpr bool operator==(const utf16_char&, const utf16_char&) = default;
	friend constexpr auto operator<=>(const utf16_char&, const utf16_char&) = default;

	friend constexpr bool operator==(const utf16_char& lhs, char16_t rhs) noexcept
	{
		return lhs.code_unit_count() == details::encoding_constants::single_code_unit_count && lhs.code_units_[0] == rhs;
	}

	friend std::ostream& operator<<(std::ostream& os, const utf16_char& ch)
	{
		std::array<char, details::encoding_constants::max_utf8_code_units> buffer{};
		const auto len = ch.encode_utf8<char>(buffer.begin());
		os.write(buffer.data(), static_cast<std::streamsize>(len));
		return os;
	}

private:
	friend constexpr std::u16string_view details::utf16_char_view(const utf16_char& ch) noexcept;

	[[nodiscard]]
	constexpr std::u16string_view as_view() const noexcept
	{
		return { code_units_.data(), code_unit_count() };
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

	constexpr bool assign_scalar(std::uint32_t scalar) noexcept
	{
		if (!details::is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return false;
		}

		code_units_.fill(0);
		details::encode_unicode_scalar_utf16_unchecked(scalar, code_units_.data());
		return true;
	}

	constexpr void assign_scalar_unchecked(std::uint32_t scalar) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::is_valid_unicode_scalar(scalar));

		code_units_.fill(0);
		details::encode_unicode_scalar_utf16_unchecked(scalar, code_units_.data());
	}

	std::array<char16_t, details::encoding_constants::utf16_surrogate_code_unit_count> code_units_{};
};

static_assert(std::is_trivially_copyable_v<utf16_char>);
inline constexpr utf16_char utf16_char::replacement_character = utf16_char::from_scalar_unchecked(details::encoding_constants::replacement_character_scalar);
inline constexpr utf16_char utf16_char::null_terminator = utf16_char{};

inline constexpr utf8_char::operator utf16_char() const noexcept
{
	return utf16_char::from_scalar_unchecked(as_scalar());
}

inline constexpr utf16_char::operator utf8_char() const noexcept
{
	return utf8_char::from_scalar_unchecked(as_scalar());
}

namespace details
{
	[[nodiscard]]
	inline constexpr std::u16string_view utf16_char_view(const utf16_char& ch) noexcept
	{
		return ch.as_view();
	}
}

namespace literals
{
	template<details::literals::constexpr_utf16_character Str>
	consteval utf16_char operator ""_u16c()
	{
		const auto sv = std::basic_string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		return utf16_char::from_utf16_code_units_unchecked(sv.data(), sv.size());
	}
}

}

namespace std
{
	template<>
	struct hash<unicode_ranges::utf16_char>
	{
		std::size_t operator()(const unicode_ranges::utf16_char& value) const noexcept
		{
			return std::hash<std::u16string_view>{}(unicode_ranges::details::utf16_char_view(value));
		}
	};

	template<>
	struct formatter<unicode_ranges::utf16_char, char>
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
					throw std::format_error("utf16_char format specifier is too long");
				}
				spec_[spec_len_++] = *it++;
			}

			if (it == end) [[unlikely]]
			{
				throw std::format_error("missing closing brace in utf16_char format specifier");
			}

			if (spec_len_ > 0 && is_ascii_alpha(spec_[spec_len_ - 1]))
			{
				presentation_ = spec_[spec_len_ - 1];
			}

			if (presentation_ != '\0' && presentation_ != 'c' && !is_numeric_presentation(presentation_)) [[unlikely]]
			{
				throw std::format_error("unsupported utf16_char presentation type");
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
		auto format(const unicode_ranges::utf16_char& value, FormatContext& ctx) const
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
	struct formatter<unicode_ranges::utf16_char, wchar_t>
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
					throw std::format_error("utf16_char format specifier is too long");
				}
				spec_[spec_len_++] = *it++;
			}

			if (it == end) [[unlikely]]
			{
				throw std::format_error("missing closing brace in utf16_char format specifier");
			}

			if (spec_len_ > 0 && is_ascii_alpha(spec_[spec_len_ - 1]))
			{
				presentation_ = spec_[spec_len_ - 1];
			}

			if (presentation_ != L'\0' && presentation_ != L'c' && !is_numeric_presentation(presentation_)) [[unlikely]]
			{
				throw std::format_error("unsupported utf16_char presentation type");
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
		auto format(const unicode_ranges::utf16_char& value, FormatContext& ctx) const
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

#endif // UTF8_RANGES_UTF16_CHAR_HPP
