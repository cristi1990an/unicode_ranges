#ifndef UTF8_RANGES_GRAPHEME_CLUSTER_VIEW_HPP
#define UTF8_RANGES_GRAPHEME_CLUSTER_VIEW_HPP

#include <vector>

#include "utf8_string_view.hpp"
#include "utf16_string_view.hpp"

namespace unicode_ranges
{

namespace views
{
	template <typename CharT>
	class grapheme_cluster_view : public std::ranges::view_interface<grapheme_cluster_view<CharT>>
	{
	public:
		static_assert(std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t>);

		using code_unit_type = CharT;
		using cluster_type = std::conditional_t<std::same_as<CharT, char8_t>, utf8_string_view, utf16_string_view>;

		static constexpr grapheme_cluster_view from_code_units_unchecked(std::basic_string_view<CharT> base) noexcept
		{
			return grapheme_cluster_view{ base };
		}

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

			constexpr iterator(std::basic_string_view<CharT> base, std::size_t current, std::size_t next) noexcept
				: base_(base), current_(current), next_(next)
			{}

			constexpr reference operator*() const noexcept
			{
				const auto cluster = base_.substr(current_, next_ - current_);
				if constexpr (std::same_as<CharT, char8_t>)
				{
					return details::utf8_string_view_from_bytes_unchecked(cluster);
				}
				else
				{
					return details::utf16_string_view_from_code_units_unchecked(cluster);
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

	private:
		constexpr explicit grapheme_cluster_view(std::basic_string_view<CharT> base) noexcept
			: base_(base)
		{}

		std::basic_string_view<CharT> base_{};
	};

	template <typename CharT>
	class reversed_grapheme_cluster_view : public std::ranges::view_interface<reversed_grapheme_cluster_view<CharT>>
	{
	public:
		static_assert(std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t>);

		using code_unit_type = CharT;
		using cluster_type = std::conditional_t<std::same_as<CharT, char8_t>, utf8_string_view, utf16_string_view>;

		static constexpr reversed_grapheme_cluster_view from_code_units_unchecked(std::basic_string_view<CharT> base)
		{
			return reversed_grapheme_cluster_view{ base };
		}

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

			constexpr iterator(std::basic_string_view<CharT> base, std::size_t current, std::size_t next) noexcept
				: base_(base), current_(current), next_(next)
			{}

			constexpr iterator(
				std::basic_string_view<CharT> base,
				const std::vector<std::size_t>* boundaries,
				std::size_t boundary_index) noexcept
				: base_(base), boundaries_(boundaries), boundary_index_(boundary_index)
			{}

			constexpr reference operator*() const noexcept
			{
				const auto current = boundaries_ ? (*boundaries_)[boundary_index_] : current_;
				const auto next = boundaries_
					? (boundary_index_ + 1u < boundaries_->size() ? (*boundaries_)[boundary_index_ + 1u] : base_.size())
					: next_;
				const auto cluster = base_.substr(current, next - current);
				if constexpr (std::same_as<CharT, char8_t>)
				{
					return details::utf8_string_view_from_bytes_unchecked(cluster);
				}
				else
				{
					return details::utf16_string_view_from_code_units_unchecked(cluster);
				}
			}

			constexpr iterator& operator++() noexcept
			{
				if (boundaries_)
				{
					if (boundary_index_ == 0)
					{
						boundary_index_ = boundaries_->size();
					}
					else
					{
						--boundary_index_;
					}

					return *this;
				}

				if (current_ == 0)
				{
					current_ = base_.size();
					next_ = base_.size();
				}
				else
				{
					next_ = current_;
					current_ = details::previous_grapheme_boundary(base_, current_ - 1);
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
				if (it.boundaries_)
				{
					return it.boundary_index_ == it.boundaries_->size();
				}

				return it.current_ == it.base_.size();
			}

			friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
			{
				if (lhs.boundaries_ || rhs.boundaries_)
				{
					return lhs.boundaries_ == rhs.boundaries_
						&& lhs.boundary_index_ == rhs.boundary_index_
						&& lhs.base_.data() == rhs.base_.data()
						&& lhs.base_.size() == rhs.base_.size();
				}

				return lhs.current_ == rhs.current_ && lhs.next_ == rhs.next_
					&& lhs.base_.data() == rhs.base_.data() && lhs.base_.size() == rhs.base_.size();
			}

			friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
			{
				if (it.boundaries_)
				{
					return it.boundary_index_ == it.boundaries_->size();
				}

				return it.current_ == it.base_.size();
			}

		private:
			std::basic_string_view<CharT> base_{};
			const std::vector<std::size_t>* boundaries_ = nullptr;
			std::size_t boundary_index_ = 0;
			std::size_t current_ = 0;
			std::size_t next_ = 0;
		};

		constexpr iterator begin() const noexcept
		{
			if (base_.empty())
			{
				return iterator{ base_, base_.size(), base_.size() };
			}

			if consteval
			{
				const auto current = details::previous_grapheme_boundary(base_, base_.size() - 1);
				return iterator{ base_, current, base_.size() };
			}
			else
			{
				return iterator{ base_, &boundaries_, boundaries_.size() - 1u };
			}
		}

		constexpr std::default_sentinel_t end() const noexcept
		{
			return std::default_sentinel;
		}

	private:
		constexpr explicit reversed_grapheme_cluster_view(std::basic_string_view<CharT> base)
			: base_(base)
		{
			if consteval
			{
				return;
			}

			if (base_.empty())
			{
				return;
			}

			boundaries_.reserve(base_.size());
			for (std::size_t current = 0; current < base_.size(); current = details::next_grapheme_boundary(base_, current))
			{
				boundaries_.push_back(current);
			}
		}

		std::basic_string_view<CharT> base_{};
		std::vector<std::size_t> boundaries_{};
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
	constexpr auto utf8_string_crtp<Derived, View>::reversed_graphemes() const -> views::reversed_grapheme_cluster_view<char8_t>
	{
		return views::reversed_grapheme_cluster_view<char8_t>::from_code_units_unchecked(byte_view());
	}


	template <typename Derived, typename View>
	constexpr auto utf16_string_crtp<Derived, View>::graphemes() const noexcept -> views::grapheme_cluster_view<char16_t>
	{
		return views::grapheme_cluster_view<char16_t>::from_code_units_unchecked(code_unit_view());
	}

	template <typename Derived, typename View>
	constexpr auto utf16_string_crtp<Derived, View>::reversed_graphemes() const -> views::reversed_grapheme_cluster_view<char16_t>
	{
		return views::reversed_grapheme_cluster_view<char16_t>::from_code_units_unchecked(code_unit_view());
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
}

}

namespace std::ranges
{
	template <typename CharT>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::grapheme_cluster_view<CharT>> = true;
}

#endif // UTF8_RANGES_GRAPHEME_CLUSTER_VIEW_HPP
