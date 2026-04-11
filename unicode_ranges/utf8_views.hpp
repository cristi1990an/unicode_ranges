#ifndef UTF8_RANGES_UTF8_VIEWS_HPP
#define UTF8_RANGES_UTF8_VIEWS_HPP

#include "utf8_char.hpp"

namespace unicode_ranges
{

namespace views
{
	template<typename R>
	concept lossy_utf8_viewable_range =
		std::ranges::viewable_range<R> &&
		std::ranges::contiguous_range<R> &&
		std::ranges::sized_range<R> &&
		std::is_integral_v<std::ranges::range_value_t<R>> &&
		!std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<R>>, bool>;

	template<typename R>
	concept utf8_viewable_range = lossy_utf8_viewable_range<R>;

	class utf8_view : public std::ranges::view_interface<utf8_view>
	{
	public:
		[[nodiscard]]
		constexpr std::u8string_view base() const noexcept
		{
			return base_;
		}

		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf8_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf8_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				const std::size_t len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(*current_));
				return utf8_char::from_utf8_bytes_unchecked(current_, len);
			}

			constexpr iterator& operator++() noexcept
			{
				current_ += static_cast<difference_type>(details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(*current_)));
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
			friend class utf8_view;

			constexpr iterator(const char8_t* current, const char8_t* end) noexcept
				: current_(current), end_(end)
			{}

			const char8_t* current_ = nullptr;
			const char8_t* end_ = nullptr;
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
		friend class details::utf8_string_crtp;

		static constexpr utf8_view from_bytes_unchecked(std::u8string_view base) noexcept
		{
			UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(base).has_value());
			return utf8_view{ base };
		}

		constexpr explicit utf8_view(std::u8string_view base) noexcept
			: base_(base)
		{}

		std::u8string_view base_{};
	};

	class reversed_utf8_view : public std::ranges::view_interface<reversed_utf8_view>
	{
	public:
		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf8_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf8_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				const std::size_t len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(*current_));
				return utf8_char::from_utf8_bytes_unchecked(current_, len);
			}

			constexpr iterator& operator++() noexcept
			{
				if (current_ == begin_)
				{
					current_ = nullptr;
					return *this;
				}

				--current_;
				while (!details::is_utf8_lead_byte(static_cast<std::uint8_t>(*current_)))
				{
					--current_;
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
			friend class reversed_utf8_view;

			constexpr iterator(const char8_t* begin, const char8_t* current) noexcept
				: begin_(begin), current_(current)
			{}

			const char8_t* begin_ = nullptr;
			const char8_t* current_ = nullptr;
		};

		constexpr iterator begin() const noexcept
		{
			const char8_t* begin = base_.data();
			const char8_t* current = begin + base_.size();
			if (current == begin)
			{
				return iterator{ begin, nullptr };
			}

			--current;
			while (!details::is_utf8_lead_byte(static_cast<std::uint8_t>(*current)))
			{
				--current;
			}
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
		friend class details::utf8_string_crtp;

		static constexpr reversed_utf8_view from_bytes_unchecked(std::u8string_view base) noexcept
		{
			UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(base).has_value());
			return reversed_utf8_view{ base };
		}

		constexpr explicit reversed_utf8_view(std::u8string_view base) noexcept
			: base_(base)
		{}

		std::u8string_view base_{};
	};

	template <typename CharT>
	class lossy_utf8_view : public std::ranges::view_interface<lossy_utf8_view<CharT>>
	{
	public:
		lossy_utf8_view() = default;
		constexpr lossy_utf8_view(std::basic_string_view<CharT> base) noexcept : base_(base) {}

		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf8_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf8_char;
			using pointer = void;

			iterator() = default;

			constexpr reference operator*() const noexcept
			{
				return current_char_.is_valid() ? current_char_ : utf8_char::replacement_character;
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
			friend class lossy_utf8_view<CharT>;

			constexpr iterator(const CharT* current, const CharT* end) noexcept
				: current_(current), end_(end)
			{
				load_current();
			}

			constexpr void load_current() noexcept
			{
				current_width_ = 0;
				current_char_ = utf8_char::invalid_sentinel_unchecked();

				if (current_ == end_) [[unlikely]]
				{
					return;
				}

				const std::basic_string_view<CharT> remaining{
					current_,
					static_cast<std::size_t>(end_ - current_)
				};
				current_width_ = details::lossy_utf8_sequence_width(remaining);

				const auto traits = details::utf8_lead_validation_table[static_cast<std::uint8_t>(*current_)];
				if (traits.size != 0 && current_width_ == traits.size)
				{
					current_char_ = utf8_char::from_scalar_unchecked(
						details::decode_valid_utf8_char(remaining.data(), current_width_));
				}
			}

			const CharT* current_ = nullptr;
			const CharT* end_ = nullptr;
			utf8_char current_char_ = utf8_char::invalid_sentinel_unchecked();
			std::size_t current_width_ = 0;
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

	struct lossy_utf8_fn : std::ranges::range_adaptor_closure<lossy_utf8_fn>
	{
		template<lossy_utf8_viewable_range R>
		constexpr auto operator()(R&& range) const noexcept
		{
			using char_type = std::remove_cv_t<std::ranges::range_value_t<R>>;
			return lossy_utf8_view<char_type>{
				std::basic_string_view<char_type>{
					std::ranges::data(range),
					static_cast<std::size_t>(std::ranges::size(range))
				}
			};
		}
	};

	inline constexpr lossy_utf8_fn lossy_utf8{};
}

}

namespace std::ranges
{
	template <>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::utf8_view> = true;

	template <>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::reversed_utf8_view> = true;

	template <typename CharT>
	inline constexpr bool enable_borrowed_range<unicode_ranges::views::lossy_utf8_view<CharT>> = true;
}

#endif // UTF8_RANGES_UTF8_VIEWS_HPP
