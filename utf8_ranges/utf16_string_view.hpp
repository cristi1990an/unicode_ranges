#ifndef UTF8_RANGES_UTF16_STRING_VIEW_HPP
#define UTF8_RANGES_UTF16_STRING_VIEW_HPP

#include "utf16_string_crtp.hpp"

namespace unicode_ranges
{

class utf16_string_view : public details::utf16_string_crtp<utf16_string_view, utf16_string_view>
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	utf16_string_view() = default;

	static constexpr std::expected<utf16_string_view, utf16_error> from_code_units(std::u16string_view code_units) noexcept
	{
		if (auto validation = details::validate_utf16(code_units); !validation) [[unlikely]]
		{
			return std::unexpected(validation.error());
		}

		return utf16_string_view{ code_units };
	}

	static constexpr utf16_string_view from_code_units_unchecked(std::u16string_view code_units) noexcept
	{
		return utf16_string_view{ code_units };
	}

	[[nodiscard]]
	constexpr auto base() const noexcept
	{
		return base_;
	}

	[[nodiscard]]
	constexpr std::u16string_view as_view() const noexcept
	{
		return base();
	}

	constexpr operator std::u16string_view() const noexcept
	{
		return base();
	}

	friend constexpr bool operator==(const utf16_string_view& lhs, const utf16_string_view& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr auto operator<=>(const utf16_string_view& lhs, const utf16_string_view& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

private:
	using base_class = details::utf16_string_crtp<utf16_string_view, utf16_string_view>;

	constexpr explicit utf16_string_view(std::u16string_view base) noexcept
		: base_(base)
	{}

	std::u16string_view base_;
};

inline std::ostream& operator<<(std::ostream& os, utf16_string_view value)
{
	for (utf16_char ch : value.chars())
	{
		std::array<char, 4> buffer{};
		const auto size = ch.encode_utf8<char>(buffer.begin());
		os.write(buffer.data(), static_cast<std::streamsize>(size));
	}

	return os;
}

[[nodiscard]]
inline constexpr utf16_string_view details::utf16_string_view_from_code_units_unchecked(std::u16string_view code_units) noexcept
{
	return utf16_string_view::from_code_units_unchecked(code_units);
}

namespace literals
{
	template<details::literals::constexpr_utf16_string Str>
	consteval auto operator ""_utf16_sv()
	{
		const auto sv = std::u16string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf16_string_view::from_code_units(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-16");
		}
		return result.value();
	}
}

}

namespace std
{
	template<>
	struct hash<unicode_ranges::utf16_string_view>
	{
		std::size_t operator()(unicode_ranges::utf16_string_view value) const noexcept
		{
			return std::hash<std::u16string_view>{}(value.base());
		}
	};

	template<>
	struct formatter<unicode_ranges::utf16_string_view, char> : formatter<std::string_view, char>
	{
		template<typename FormatContext>
		auto format(unicode_ranges::utf16_string_view value, FormatContext& ctx) const
		{
			std::string text;
			for (unicode_ranges::utf16_char ch : value.chars())
			{
				ch.encode_utf8<char>(std::back_inserter(text));
			}

			return formatter<std::string_view, char>::format(text, ctx);
		}
	};
}

#include "grapheme_cluster_view.hpp"

#endif // UTF8_RANGES_UTF16_STRING_VIEW_HPP
