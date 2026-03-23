#ifndef UTF8_RANGES_UTF8_STRING_VIEW_HPP
#define UTF8_RANGES_UTF8_STRING_VIEW_HPP

#include "utf8_string_crtp.hpp"

namespace unicode_ranges
{

class utf8_string_view : public details::utf8_string_crtp<utf8_string_view, utf8_string_view>
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	utf8_string_view() = default;

	static constexpr std::expected<utf8_string_view, utf8_error> from_bytes(std::u8string_view bytes) noexcept
	{
		if (auto validation = details::validate_utf8(bytes); !validation) [[unlikely]]
		{
			return std::unexpected(validation.error());
		}

		return utf8_string_view{ bytes };
	}

	static constexpr utf8_string_view from_bytes_unchecked(std::u8string_view bytes) noexcept
	{
		return utf8_string_view{ bytes };
	}

	[[nodiscard]]
	constexpr auto base() const noexcept
	{
		return base_;
	}

	[[nodiscard]]
	constexpr std::u8string_view as_view() const noexcept
	{
		return base();
	}

	constexpr operator std::u8string_view() const noexcept
	{
		return base();
	}

	friend constexpr bool operator==(const utf8_string_view& lhs, const utf8_string_view& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr auto operator<=>(const utf8_string_view& lhs, const utf8_string_view& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

private:
	using base_class = details::utf8_string_crtp<utf8_string_view, utf8_string_view>;

	constexpr explicit utf8_string_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_;
};

inline std::ostream& operator<<(std::ostream& os, utf8_string_view value)
{
	const auto text = value.base();
	os.write(reinterpret_cast<const char*>(text.data()), static_cast<std::streamsize>(text.size()));
	return os;
}

[[nodiscard]]
inline constexpr utf8_string_view utf8_char::as_utf8_view() const noexcept
{
	return utf8_string_view::from_bytes_unchecked(as_view());
}

[[nodiscard]]
inline constexpr utf8_string_view details::utf8_string_view_from_bytes_unchecked(std::u8string_view bytes) noexcept
{
	return utf8_string_view::from_bytes_unchecked(bytes);
}

namespace literals
{
	template<details::literals::constexpr_utf8_string Str>
	consteval auto operator ""_utf8_sv()
	{
		const auto sv = std::u8string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf8_string_view::from_bytes(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-8");
		}
		return result.value();
	}
}

}

namespace std
{
	template<>
	struct hash<unicode_ranges::utf8_string_view>
	{
		std::size_t operator()(unicode_ranges::utf8_string_view value) const noexcept
		{
			return std::hash<std::u8string_view>{}(value.base());
		}
	};

	template<>
	struct formatter<unicode_ranges::utf8_string_view, char> : formatter<std::string_view, char>
	{
		template<typename FormatContext>
		auto format(unicode_ranges::utf8_string_view value, FormatContext& ctx) const
		{
			const auto text = value.base();
			return formatter<std::string_view, char>::format(
				std::string_view{ reinterpret_cast<const char*>(text.data()), text.size() },
				ctx);
		}
	};
}

#include "grapheme_cluster_view.hpp"

#endif // UTF8_RANGES_UTF8_STRING_VIEW_HPP
