#ifndef UTF8_RANGES_UTF32_VIEWS_HPP
#define UTF8_RANGES_UTF32_VIEWS_HPP

#include "utf32_char.hpp"

namespace unicode_ranges
{

namespace views
{
	template<typename R>
	concept lossy_utf32_viewable_range =
		std::ranges::viewable_range<R> &&
		std::ranges::contiguous_range<R> &&
		std::ranges::sized_range<R> &&
		std::is_integral_v<std::ranges::range_value_t<R>> &&
		!std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<R>>, bool>;

	template<typename R>
	concept utf32_viewable_range = lossy_utf32_viewable_range<R>;

	class utf32_view : public std::ranges::view_interface<utf32_view>
	{
	public:
		[[nodiscard]]
		constexpr std::u32string_view base() const noexcept
		{
			return base_;
		}

		class iterator
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using iterator_concept = std::random_access_iterator_tag;
			using value_type = utf32_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf32_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				return utf32_char::from_scalar_unchecked(static_cast<std::uint32_t>(base_[current_]));
			}

			constexpr reference operator[](difference_type offset) const noexcept
			{
				const auto index = static_cast<std::size_t>(static_cast<difference_type>(current_) + offset);
				return utf32_char::from_scalar_unchecked(static_cast<std::uint32_t>(base_[index]));
			}

			constexpr iterator& operator++() noexcept
			{
				++current_;
				return *this;
			}

			constexpr iterator operator++(int) noexcept
			{
				iterator old = *this;
				++(*this);
				return old;
			}

			constexpr iterator& operator--() noexcept
			{
				--current_;
				return *this;
			}

			constexpr iterator operator--(int) noexcept
			{
				iterator old = *this;
				--(*this);
				return old;
			}

			constexpr iterator& operator+=(difference_type offset) noexcept
			{
				current_ = static_cast<std::size_t>(static_cast<difference_type>(current_) + offset);
				return *this;
			}

			constexpr iterator& operator-=(difference_type offset) noexcept
			{
				current_ = static_cast<std::size_t>(static_cast<difference_type>(current_) - offset);
				return *this;
			}

			friend constexpr iterator operator+(iterator it, difference_type offset) noexcept
			{
				it += offset;
				return it;
			}

			friend constexpr iterator operator+(difference_type offset, iterator it) noexcept
			{
				it += offset;
				return it;
			}

			friend constexpr iterator operator-(iterator it, difference_type offset) noexcept
			{
				it -= offset;
				return it;
			}

			friend constexpr difference_type operator-(const iterator& lhs, const iterator& rhs) noexcept
			{
				return static_cast<difference_type>(lhs.current_) - static_cast<difference_type>(rhs.current_);
			}

			friend constexpr bool operator==(const iterator&, const iterator&) noexcept = default;
			friend constexpr auto operator<=>(const iterator&, const iterator&) noexcept = default;

		private:
			friend class utf32_view;

			constexpr iterator(const char32_t* base, std::size_t current) noexcept
				: base_(base), current_(current)
			{}

			const char32_t* base_ = nullptr;
			std::size_t current_ = 0;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator{ base_.data(), 0 };
		}

		constexpr iterator end() const noexcept
		{
			return iterator{ base_.data(), base_.size() };
		}

		constexpr std::size_t size() const noexcept
		{
			return base_.size();
		}

		constexpr std::size_t reserve_hint() const noexcept
		{
			return base_.size();
		}

	private:
		template <typename Derived, typename View>
		friend class details::utf32_string_crtp;

		static constexpr utf32_view from_code_points_unchecked(std::u32string_view base) noexcept
		{
			UTF8_RANGES_DEBUG_ASSERT(details::validate_utf32(base).has_value());
			return utf32_view{ base };
		}

		constexpr explicit utf32_view(std::u32string_view base) noexcept
			: base_(base)
		{}

		std::u32string_view base_{};
	};

	using reversed_utf32_view = std::ranges::reverse_view<utf32_view>;

	template <typename CharT>
	class lossy_utf32_view : public std::ranges::view_interface<lossy_utf32_view<CharT>>
	{
	public:
		lossy_utf32_view() = default;
		constexpr lossy_utf32_view(std::basic_string_view<CharT> base) noexcept : base_(base) {}

		class iterator
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using iterator_concept = std::random_access_iterator_tag;
			using value_type = utf32_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf32_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				return decode(base_, current_);
			}

			constexpr reference operator[](difference_type offset) const noexcept
			{
				const auto index = static_cast<std::size_t>(static_cast<difference_type>(current_) + offset);
				return decode(base_, index);
			}

			constexpr iterator& operator++() noexcept
			{
				++current_;
				return *this;
			}

			constexpr iterator operator++(int) noexcept
			{
				iterator old = *this;
				++(*this);
				return old;
			}

			constexpr iterator& operator--() noexcept
			{
				--current_;
				return *this;
			}

			constexpr iterator operator--(int) noexcept
			{
				iterator old = *this;
				--(*this);
				return old;
			}

			constexpr iterator& operator+=(difference_type offset) noexcept
			{
				current_ = static_cast<std::size_t>(static_cast<difference_type>(current_) + offset);
				return *this;
			}

			constexpr iterator& operator-=(difference_type offset) noexcept
			{
				current_ = static_cast<std::size_t>(static_cast<difference_type>(current_) - offset);
				return *this;
			}

			friend constexpr iterator operator+(iterator it, difference_type offset) noexcept
			{
				it += offset;
				return it;
			}

			friend constexpr iterator operator+(difference_type offset, iterator it) noexcept
			{
				it += offset;
				return it;
			}

			friend constexpr iterator operator-(iterator it, difference_type offset) noexcept
			{
				it -= offset;
				return it;
			}

			friend constexpr difference_type operator-(const iterator& lhs, const iterator& rhs) noexcept
			{
				return static_cast<difference_type>(lhs.current_) - static_cast<difference_type>(rhs.current_);
			}

			friend constexpr bool operator==(const iterator&, const iterator&) noexcept = default;
			friend constexpr auto operator<=>(const iterator&, const iterator&) noexcept = default;

		private:
			friend class lossy_utf32_view<CharT>;

			static constexpr utf32_char decode(const CharT* base, std::size_t current) noexcept
			{
				const auto scalar = static_cast<std::uint32_t>(base[current]);
				if (details::is_valid_unicode_scalar(scalar))
				{
					return utf32_char::from_scalar_unchecked(scalar);
				}

				return utf32_char::replacement_character;
			}

			constexpr iterator(const CharT* base, std::size_t current) noexcept
				: base_(base), current_(current)
			{
			}

			const CharT* base_ = nullptr;
			std::size_t current_ = 0;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator{ base_.data(), 0 };
		}

		constexpr iterator end() const noexcept
		{
			return iterator{ base_.data(), base_.size() };
		}

		constexpr std::size_t size() const noexcept
		{
			return base_.size();
		}

		constexpr std::size_t reserve_hint() const noexcept
		{
			return base_.size();
		}

	private:
		std::basic_string_view<CharT> base_;
	};

	struct lossy_utf32_fn : std::ranges::range_adaptor_closure<lossy_utf32_fn>
	{
		template<lossy_utf32_viewable_range R>
		constexpr auto operator()(R&& range) const noexcept
		{
			using char_type = std::remove_cv_t<std::ranges::range_value_t<R>>;
			return lossy_utf32_view<char_type>{
				std::basic_string_view<char_type>{
					std::ranges::data(range),
					static_cast<std::size_t>(std::ranges::size(range))
				}
			};
		}
	};

	inline constexpr lossy_utf32_fn lossy_utf32{};
}

}

namespace std::ranges
{
	template <>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::utf32_view> = true;

	template <typename CharT>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::lossy_utf32_view<CharT>> = true;
}

#endif // UTF8_RANGES_UTF32_VIEWS_HPP
