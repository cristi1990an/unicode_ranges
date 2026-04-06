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
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf32_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf32_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				return utf32_char::from_utf32_code_points_unchecked(current_, 1);
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

			friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
			{
				return it.current_ == it.end_;
			}

			friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
			{
				return lhs.current_ == rhs.current_;
			}

			friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
			{
				return it.current_ == it.end_;
			}

		private:
			friend class utf32_view;

			constexpr iterator(const char32_t* current, const char32_t* end) noexcept
				: current_(current), end_(end)
			{}

			const char32_t* current_ = nullptr;
			const char32_t* end_ = nullptr;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator{ base_.data(), base_.data() + base_.size() };
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

	class reversed_utf32_view : public std::ranges::view_interface<reversed_utf32_view>
	{
	public:
		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf32_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf32_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				return utf32_char::from_utf32_code_points_unchecked(current_, 1);
			}

			constexpr iterator& operator++() noexcept
			{
				if (current_ == nullptr) [[unlikely]]
				{
					return *this;
				}

				if (current_ == begin_)
				{
					current_ = nullptr;
					return *this;
				}

				--current_;
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
				return it.current_ == nullptr;
			}

			friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
			{
				return lhs.current_ == rhs.current_;
			}

			friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
			{
				return it.current_ == nullptr;
			}

		private:
			friend class reversed_utf32_view;

			constexpr iterator(const char32_t* begin, const char32_t* current) noexcept
				: begin_(begin), current_(current)
			{}

			const char32_t* begin_ = nullptr;
			const char32_t* current_ = nullptr;
		};

		constexpr iterator begin() const noexcept
		{
			const char32_t* begin = base_.data();
			const char32_t* current = begin + base_.size();
			if (current == begin)
			{
				return iterator{ begin, nullptr };
			}

			--current;
			return iterator{ begin, current };
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
		friend class details::utf32_string_crtp;

		static constexpr reversed_utf32_view from_code_points_unchecked(std::u32string_view base) noexcept
		{
			UTF8_RANGES_DEBUG_ASSERT(details::validate_utf32(base).has_value());
			return reversed_utf32_view{ base };
		}

		constexpr explicit reversed_utf32_view(std::u32string_view base) noexcept
			: base_(base)
		{}

		std::u32string_view base_{};
	};

	template <typename CharT>
	class lossy_utf32_view : public std::ranges::view_interface<lossy_utf32_view<CharT>>
	{
	public:
		lossy_utf32_view() = default;
		constexpr lossy_utf32_view(std::basic_string_view<CharT> base) noexcept : base_(base) {}

		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf32_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf32_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				return current_valid_ ? current_char_ : utf32_char::replacement_character;
			}

			constexpr iterator& operator++() noexcept
			{
				current_ += static_cast<difference_type>(current_width_);
				load_current();
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
				return it.current_ == it.end_;
			}

			friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
			{
				return lhs.current_ == rhs.current_;
			}

			friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
			{
				return it.current_ == it.end_;
			}

		private:
			friend class lossy_utf32_view<CharT>;

			constexpr iterator(const CharT* current, const CharT* end) noexcept
				: current_(current), end_(end)
			{
				load_current();
			}

			constexpr void load_current() noexcept
			{
				current_width_ = 0;
				current_valid_ = false;

				if (current_ == end_) [[unlikely]]
				{
					return;
				}

				current_width_ = 1;
				const auto scalar = static_cast<std::uint32_t>(*current_);
				if (details::is_valid_unicode_scalar(scalar))
				{
					current_char_ = utf32_char::from_utf32_code_points_unchecked(current_, 1);
					current_valid_ = true;
				}
			}

			const CharT* current_ = nullptr;
			const CharT* end_ = nullptr;
			utf32_char current_char_{};
			std::size_t current_width_ = 0;
			bool current_valid_ = false;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator{ base_.data(), base_.data() + base_.size() };
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

	template <>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::reversed_utf32_view> = true;

	template <typename CharT>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::lossy_utf32_view<CharT>> = true;
}

#endif // UTF8_RANGES_UTF32_VIEWS_HPP
