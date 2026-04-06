#ifndef UTF8_RANGES_UTF32_STRING_VIEW_HPP
#define UTF8_RANGES_UTF32_STRING_VIEW_HPP

#include "utf32_string_crtp.hpp"

namespace unicode_ranges
{

class utf32_string_view : public details::utf32_string_crtp<utf32_string_view, utf32_string_view>
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	utf32_string_view() = default;

	static constexpr std::expected<utf32_string_view, utf32_error> from_code_points(std::u32string_view code_points) noexcept
	{
		if (auto validation = details::validate_utf32(code_points); !validation) [[unlikely]]
		{
			return std::unexpected(validation.error());
		}

		return utf32_string_view{ code_points };
	}

	static constexpr utf32_string_view from_code_points_unchecked(std::u32string_view code_points) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf32(code_points).has_value());
		return utf32_string_view{ code_points };
	}

	[[nodiscard]]
	constexpr auto base() const noexcept
	{
		return base_;
	}

	[[nodiscard]]
	constexpr std::u32string_view as_view() const noexcept
	{
		return base();
	}

	constexpr operator std::u32string_view() const noexcept
	{
		return base();
	}

	friend constexpr bool operator==(const utf32_string_view& lhs, const utf32_string_view& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr auto operator<=>(const utf32_string_view& lhs, const utf32_string_view& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

private:
	using base_class = details::utf32_string_crtp<utf32_string_view, utf32_string_view>;

	constexpr explicit utf32_string_view(std::u32string_view base) noexcept
		: base_(base)
	{}

	std::u32string_view base_;
};

inline std::ostream& operator<<(std::ostream& os, utf32_string_view value)
{
	for (utf32_char ch : value.chars())
	{
		std::array<char, 4> buffer{};
		const auto size = ch.encode_utf8<char>(buffer.begin());
		os.write(buffer.data(), static_cast<std::streamsize>(size));
	}

	return os;
}

[[nodiscard]]
inline constexpr utf32_string_view details::utf32_string_view_from_code_points_unchecked(std::u32string_view code_points) noexcept
{
	return utf32_string_view::from_code_points_unchecked(code_points);
}

namespace literals
{
	template<details::literals::constexpr_utf32_string Str>
	consteval auto operator ""_utf32_sv()
	{
		const auto sv = std::u32string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf32_string_view::from_code_points(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-32");
		}
		return result.value();
	}
}

}

namespace std
{
	template<>
	struct hash<unicode_ranges::utf32_string_view>
	{
		std::size_t operator()(unicode_ranges::utf32_string_view value) const noexcept
		{
			return std::hash<std::u32string_view>{}(value.base());
		}
	};

	template<>
	struct formatter<unicode_ranges::utf32_string_view, char> : formatter<std::string_view, char>
	{
		template<typename FormatContext>
		auto format(unicode_ranges::utf32_string_view value, FormatContext& ctx) const
		{
			const auto code_points = value.base();
			std::size_t utf8_size = 0;
			for (char32_t code_point : code_points)
			{
				const auto scalar = static_cast<std::uint32_t>(code_point);
				if (scalar <= unicode_ranges::details::encoding_constants::ascii_scalar_max) [[likely]]
				{
					++utf8_size;
				}
				else if (scalar <= unicode_ranges::details::encoding_constants::two_byte_scalar_max)
				{
					utf8_size += unicode_ranges::details::encoding_constants::two_code_unit_count;
				}
				else if (scalar <= unicode_ranges::details::encoding_constants::bmp_scalar_max)
				{
					utf8_size += unicode_ranges::details::encoding_constants::three_code_unit_count;
				}
				else
				{
					utf8_size += unicode_ranges::details::encoding_constants::max_utf8_code_units;
				}
			}

			std::string text;
			text.resize_and_overwrite(utf8_size,
				[&](char* buffer, std::size_t) noexcept
				{
					std::size_t write_index = 0;
					for (char32_t code_point : code_points)
					{
						write_index += unicode_ranges::details::encode_unicode_scalar_utf8_unchecked(
							static_cast<std::uint32_t>(code_point),
							buffer + write_index);
					}

					return write_index;
				});

			return formatter<std::string_view, char>::format(text, ctx);
		}
	};
}

#include "grapheme_cluster_view.hpp"

#endif // UTF8_RANGES_UTF32_STRING_VIEW_HPP
