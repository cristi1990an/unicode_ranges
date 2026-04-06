#ifndef UTF8_RANGES_GRAPHEME_CLUSTER_VIEW_HPP
#define UTF8_RANGES_GRAPHEME_CLUSTER_VIEW_HPP

#include "utf8_string_view.hpp"
#include "utf16_string_view.hpp"
#include "utf32_string_view.hpp"

namespace unicode_ranges
{

namespace views
{
	template <typename CharT>
	class grapheme_cluster_view : public std::ranges::view_interface<grapheme_cluster_view<CharT>>
	{
	public:
		static_assert(std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t> || std::same_as<CharT, char32_t>);

		using code_unit_type = CharT;
		using cluster_type =
			std::conditional_t<std::same_as<CharT, char8_t>, utf8_string_view,
			std::conditional_t<std::same_as<CharT, char16_t>, utf16_string_view, utf32_string_view>>;

		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = cluster_type;
			using difference_type = std::ptrdiff_t;
			using reference = cluster_type;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				const auto cluster = base_.substr(current_, next_ - current_);
				if constexpr (std::same_as<CharT, char8_t>)
				{
					return details::utf8_string_view_from_bytes_unchecked(cluster);
				}
				else if constexpr (std::same_as<CharT, char16_t>)
				{
					return details::utf16_string_view_from_code_units_unchecked(cluster);
				}
				else
				{
					return details::utf32_string_view_from_code_points_unchecked(cluster);
				}
			}

			constexpr iterator& operator++() noexcept
			{
				current_ = next_;
				if (current_ != base_.size())
				{
					next_ = details::next_grapheme_boundary(base_, current_);
				}
				return *this;
			}

			constexpr iterator operator++(int) noexcept
			{
				iterator old = *this;
				++(*this);
				return old;
			}

			friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
			{
				return it.current_ == it.base_.size();
			}

			friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
			{
				return lhs.current_ == rhs.current_ && lhs.base_.data() == rhs.base_.data() && lhs.base_.size() == rhs.base_.size();
			}

			friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
			{
				return it.current_ == it.base_.size();
			}

		private:
			friend class grapheme_cluster_view<CharT>;

			constexpr iterator(std::basic_string_view<CharT> base, std::size_t current, std::size_t next) noexcept
				: base_(base), current_(current), next_(next)
			{}

			std::basic_string_view<CharT> base_{};
			std::size_t current_ = 0;
			std::size_t next_ = 0;
		};

		constexpr iterator begin() const noexcept
		{
			if (base_.empty())
			{
				return iterator{ base_, base_.size(), base_.size() };
			}

			return iterator{ base_, 0, details::next_grapheme_boundary(base_, 0) };
		}

		constexpr std::default_sentinel_t end() const noexcept
		{
			return std::default_sentinel;
		}

		constexpr std::size_t reserve_hint() const noexcept
		{
			return base_.size();
		}

	private:
		template <typename Derived, typename View>
		friend class details::utf8_string_crtp;

		template <typename Derived, typename View>
		friend class details::utf16_string_crtp;

		template <typename Derived, typename View>
		friend class details::utf32_string_crtp;

		static constexpr grapheme_cluster_view from_code_units_unchecked(std::basic_string_view<CharT> base) noexcept
		{
			if constexpr (std::same_as<CharT, char8_t>)
			{
				UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(base).has_value());
			}
			else if constexpr (std::same_as<CharT, char16_t>)
			{
				UTF8_RANGES_DEBUG_ASSERT(details::validate_utf16(base).has_value());
			}
			else
			{
				UTF8_RANGES_DEBUG_ASSERT(details::validate_utf32(base).has_value());
			}

			return grapheme_cluster_view{ base };
		}

		constexpr explicit grapheme_cluster_view(std::basic_string_view<CharT> base) noexcept
			: base_(base)
		{}

		std::basic_string_view<CharT> base_{};
	};
}

namespace details
{
	template <typename Derived, typename View>
	constexpr auto utf8_string_crtp<Derived, View>::graphemes() const noexcept -> views::grapheme_cluster_view<char8_t>
	{
		return views::grapheme_cluster_view<char8_t>::from_code_units_unchecked(byte_view());
	}

	template <typename Derived, typename View>
	constexpr auto utf16_string_crtp<Derived, View>::graphemes() const noexcept -> views::grapheme_cluster_view<char16_t>
	{
		return views::grapheme_cluster_view<char16_t>::from_code_units_unchecked(code_unit_view());
	}

	template <typename Derived, typename View>
	constexpr auto utf32_string_crtp<Derived, View>::graphemes() const noexcept -> views::grapheme_cluster_view<char32_t>
	{
		return views::grapheme_cluster_view<char32_t>::from_code_units_unchecked(code_unit_view());
	}
}

namespace literals
{
	template<details::literals::constexpr_utf8_string Str>
	consteval auto operator ""_grapheme_utf8()
	{
		const auto sv = std::u8string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf8_string_view::from_bytes(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-8");
		}

		if (sv.empty() || details::next_grapheme_boundary(sv, 0) != sv.size())
		{
			throw std::invalid_argument("literal must contain exactly one grapheme cluster");
		}

		return result.value();
	}

	template<details::literals::constexpr_utf16_string Str>
	consteval auto operator ""_grapheme_utf16()
	{
		const auto sv = std::u16string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf16_string_view::from_code_units(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-16");
		}

		if (sv.empty() || details::next_grapheme_boundary(sv, 0) != sv.size())
		{
			throw std::invalid_argument("literal must contain exactly one grapheme cluster");
		}

		return result.value();
	}

	template<details::literals::constexpr_utf32_string Str>
	consteval auto operator ""_grapheme_utf32()
	{
		const auto sv = std::u32string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf32_string_view::from_code_points(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-32");
		}

		if (sv.empty() || details::next_grapheme_boundary(sv, 0) != sv.size())
		{
			throw std::invalid_argument("literal must contain exactly one grapheme cluster");
		}

		return result.value();
	}
}

}

namespace std::ranges
{
	template <typename CharT>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::grapheme_cluster_view<CharT>> = true;
}

#endif // UTF8_RANGES_GRAPHEME_CLUSTER_VIEW_HPP
