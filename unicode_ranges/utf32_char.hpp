#ifndef UTF8_RANGES_UTF32_CHAR_HPP
#define UTF8_RANGES_UTF32_CHAR_HPP

#include "utf8_char.hpp"

namespace unicode_ranges
{

struct utf32_char;

namespace details
{
	[[nodiscard]]
	constexpr std::u32string_view utf32_char_view(const utf32_char& ch) noexcept;
}

struct utf32_char
{
public:
	utf32_char() = default;
	static const utf32_char replacement_character;
	static const utf32_char null_terminator;

	[[nodiscard]]
	static constexpr std::optional<utf32_char> from_scalar(std::uint32_t scalar) noexcept
	{
		utf32_char value;
		if (!value.assign_scalar(scalar)) [[unlikely]]
		{
			return std::nullopt;
		}

		return value;
	}

	[[nodiscard]]
	static constexpr utf32_char from_scalar_unchecked(std::uint32_t scalar) noexcept
	{
		utf32_char value;
		value.assign_scalar_unchecked(scalar);
		return value;
	}

	template<typename CharT>
	[[nodiscard]]
	static constexpr std::optional<utf32_char> from_utf32_code_points(const CharT* code_points, std::size_t size) noexcept
	{
		if (!details::is_single_valid_utf32_char(std::basic_string_view<CharT>{ code_points, size })) [[unlikely]]
		{
			return std::nullopt;
		}

		return from_utf32_code_points_unchecked(code_points, size);
	}

	template<typename CharT>
	[[nodiscard]]
	static constexpr utf32_char from_utf32_code_points_unchecked(const CharT* code_points, std::size_t size) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(code_points != nullptr);
		UTF8_RANGES_DEBUG_ASSERT(details::is_single_valid_utf32_char(std::basic_string_view<CharT>{ code_points, size }));

		utf32_char value;
		std::ranges::copy_n(code_points, size, value.code_points_.begin());
		return value;
	}

	[[nodiscard]]
	constexpr std::uint32_t as_scalar() const noexcept
	{
		return static_cast<std::uint32_t>(code_points_[0]);
	}

	constexpr operator utf8_char() const noexcept;
	constexpr operator utf16_char() const noexcept;

	template <typename Allocator = std::allocator<char32_t>>
	[[nodiscard]]
	constexpr basic_utf32_string<Allocator> to_utf32_owned(const Allocator& alloc = Allocator()) const;

	constexpr utf32_char& operator++() noexcept
	{
		const auto scalar = as_scalar();
		if (scalar == details::encoding_constants::max_unicode_scalar)
		{
			code_points_[0] = 0;
			return *this;
		}

		auto next = scalar + 1u;
		if (next >= details::encoding_constants::high_surrogate_min
			&& next <= details::encoding_constants::low_surrogate_max)
		{
			next = details::encoding_constants::scalar_after_surrogate_range;
		}

		code_points_[0] = static_cast<char32_t>(next);
		return *this;
	}

	constexpr utf32_char operator++(int) noexcept
	{
		utf32_char old = *this;
		++(*this);
		return old;
	}

	constexpr utf32_char& operator--() noexcept
	{
		const auto scalar = as_scalar();
		if (scalar == 0u)
		{
			code_points_[0] = static_cast<char32_t>(details::encoding_constants::max_unicode_scalar);
			return *this;
		}

		auto previous = scalar - 1u;
		if (previous >= details::encoding_constants::high_surrogate_min
			&& previous <= details::encoding_constants::low_surrogate_max)
		{
			previous = details::encoding_constants::scalar_before_surrogate_range;
		}

		code_points_[0] = static_cast<char32_t>(previous);
		return *this;
	}

	constexpr utf32_char operator--(int) noexcept
	{
		utf32_char old = *this;
		--(*this);
		return old;
	}

	[[nodiscard]]
	constexpr bool is_ascii() const noexcept
	{
		return static_cast<std::uint32_t>(code_points_[0]) <= details::encoding_constants::ascii_scalar_max;
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

		const auto value = static_cast<std::uint8_t>(code_points_[0]);
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

		const auto value = static_cast<std::uint8_t>(code_points_[0]);
		return value <= details::encoding_constants::ascii_control_max || value == details::encoding_constants::ascii_delete;
	}

	[[nodiscard]]
	constexpr bool is_ascii_digit() const noexcept
	{
		return is_ascii() && is_ascii_digit_byte(static_cast<std::uint8_t>(code_points_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_graphic() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_points_[0]);
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

		const auto value = static_cast<std::uint8_t>(code_points_[0]);
		return is_ascii_digit_byte(value)
			|| (value >= 'a' && value <= 'f')
			|| (value >= 'A' && value <= 'F');
	}

	[[nodiscard]]
	constexpr bool is_ascii_lowercase() const noexcept
	{
		return is_ascii() && is_ascii_lower_alpha(static_cast<std::uint8_t>(code_points_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_octdigit() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_points_[0]);
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
		return is_ascii() && is_ascii_upper_alpha(static_cast<std::uint8_t>(code_points_[0]));
	}

	[[nodiscard]]
	constexpr bool is_ascii_whitespace() const noexcept
	{
		if (!is_ascii())
		{
			return false;
		}

		const auto value = static_cast<std::uint8_t>(code_points_[0]);
		return value == ' '
			|| (value >= '\t' && value <= '\r');
	}

	[[nodiscard]]
	constexpr utf32_char ascii_lowercase() const noexcept
	{
		if (!is_ascii())
		{
			return *this;
		}

		utf32_char result = *this;
		const auto value = static_cast<std::uint8_t>(result.code_points_[0]);
		if (is_ascii_upper_alpha(value))
		{
			result.code_points_[0] = static_cast<char32_t>(value + ('a' - 'A'));
		}
		return result;
	}

	[[nodiscard]]
	constexpr utf32_char ascii_uppercase() const noexcept
	{
		if (!is_ascii())
		{
			return *this;
		}

		utf32_char result = *this;
		const auto value = static_cast<std::uint8_t>(result.code_points_[0]);
		if (is_ascii_lower_alpha(value))
		{
			result.code_points_[0] = static_cast<char32_t>(value - ('a' - 'A'));
		}
		return result;
	}

	[[nodiscard]]
	constexpr bool eq_ignore_ascii_case(utf32_char other) const noexcept
	{
		return ascii_lowercase() == other.ascii_lowercase();
	}

	constexpr void swap(utf32_char& other) noexcept
	{
		code_points_.swap(other.code_points_);
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
		return details::encoding_constants::single_code_unit_count;
	}

	template<typename CharT, typename OutIt>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& details::non_narrowing_convertible<char32_t, CharT>
		&& std::output_iterator<OutIt, CharT>)
	constexpr std::size_t encode_utf32(OutIt out) const noexcept
	{
		*out++ = static_cast<CharT>(code_points_[0]);
		return details::encoding_constants::single_code_unit_count;
	}

	template<typename CharT, typename OutIt>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& details::non_narrowing_convertible<char16_t, CharT>
		&& std::output_iterator<OutIt, CharT>)
	constexpr std::size_t encode_utf16(OutIt out) const noexcept
	{
		std::array<CharT, details::encoding_constants::utf16_surrogate_code_unit_count> buffer{};
		const auto len = details::encode_unicode_scalar_utf16_unchecked(as_scalar(), buffer.data());
		std::ranges::copy_n(buffer.data(), len, out);
		return len;
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

	friend constexpr bool operator==(const utf32_char&, const utf32_char&) = default;
	friend constexpr auto operator<=>(const utf32_char&, const utf32_char&) = default;

	friend constexpr bool operator==(const utf32_char& lhs, char32_t rhs) noexcept
	{
		return lhs.code_points_[0] == rhs;
	}

	friend std::ostream& operator<<(std::ostream& os, const utf32_char& ch)
	{
		std::array<char, details::encoding_constants::max_utf8_code_units> buffer{};
		const auto len = ch.encode_utf8<char>(buffer.begin());
		os.write(buffer.data(), static_cast<std::streamsize>(len));
		return os;
	}

private:
	friend constexpr std::u32string_view details::utf32_char_view(const utf32_char& ch) noexcept;

	[[nodiscard]]
	constexpr std::u32string_view as_view() const noexcept
	{
		return { code_points_.data(), details::encoding_constants::single_code_unit_count };
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

		code_points_.fill(0);
		details::encode_unicode_scalar_utf32_unchecked(scalar, code_points_.data());
		return true;
	}

	constexpr void assign_scalar_unchecked(std::uint32_t scalar) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::is_valid_unicode_scalar(scalar));

		code_points_.fill(0);
		details::encode_unicode_scalar_utf32_unchecked(scalar, code_points_.data());
	}

	std::array<char32_t, details::encoding_constants::single_code_unit_count> code_points_{};
};

static_assert(std::is_trivially_copyable_v<utf32_char>);
inline constexpr utf32_char utf32_char::replacement_character = utf32_char::from_scalar_unchecked(details::encoding_constants::replacement_character_scalar);
inline constexpr utf32_char utf32_char::null_terminator = utf32_char{};

inline constexpr utf8_char::operator utf32_char() const noexcept
{
	return utf32_char::from_scalar_unchecked(as_scalar());
}

inline constexpr utf16_char::operator utf32_char() const noexcept
{
	return utf32_char::from_scalar_unchecked(as_scalar());
}

inline constexpr utf32_char::operator utf8_char() const noexcept
{
	return utf8_char::from_scalar_unchecked(as_scalar());
}

inline constexpr utf32_char::operator utf16_char() const noexcept
{
	return utf16_char::from_scalar_unchecked(as_scalar());
}

namespace details
{
	[[nodiscard]]
	inline constexpr std::u32string_view utf32_char_view(const utf32_char& ch) noexcept
	{
		return ch.as_view();
	}
}

namespace literals
{
	template<details::literals::constexpr_utf32_character Str>
	consteval utf32_char operator ""_u32c()
	{
		const auto sv = std::basic_string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		return utf32_char::from_utf32_code_points_unchecked(sv.data(), sv.size());
	}
}

}

namespace std
{
	template<>
	struct hash<unicode_ranges::utf32_char>
	{
		std::size_t operator()(const unicode_ranges::utf32_char& value) const noexcept
		{
			return std::hash<std::u32string_view>{}(unicode_ranges::details::utf32_char_view(value));
		}
	};

	template<>
	struct formatter<unicode_ranges::utf32_char, char>
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
					throw std::format_error("utf32_char format specifier is too long");
				}
				spec_[spec_len_++] = *it++;
			}

			if (it == end) [[unlikely]]
			{
				throw std::format_error("missing closing brace in utf32_char format specifier");
			}

			if (spec_len_ > 0 && is_ascii_alpha(spec_[spec_len_ - 1]))
			{
				presentation_ = spec_[spec_len_ - 1];
			}

			if (presentation_ != '\0' && presentation_ != 'c' && !is_numeric_presentation(presentation_)) [[unlikely]]
			{
				throw std::format_error("unsupported utf32_char presentation type");
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
		auto format(const unicode_ranges::utf32_char& value, FormatContext& ctx) const
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
	struct formatter<unicode_ranges::utf32_char, wchar_t>
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
					throw std::format_error("utf32_char format specifier is too long");
				}
				spec_[spec_len_++] = *it++;
			}

			if (it == end) [[unlikely]]
			{
				throw std::format_error("missing closing brace in utf32_char format specifier");
			}

			if (spec_len_ > 0 && is_ascii_alpha(spec_[spec_len_ - 1]))
			{
				presentation_ = spec_[spec_len_ - 1];
			}

			if (presentation_ != L'\0' && presentation_ != L'c' && !is_numeric_presentation(presentation_)) [[unlikely]]
			{
				throw std::format_error("unsupported utf32_char presentation type");
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
		auto format(const unicode_ranges::utf32_char& value, FormatContext& ctx) const
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

#endif // UTF8_RANGES_UTF32_CHAR_HPP
