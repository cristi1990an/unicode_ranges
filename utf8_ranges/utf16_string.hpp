#ifndef UTF8_RANGES_UTF16_STRING_HPP
#define UTF8_RANGES_UTF16_STRING_HPP

#include "utf16_string_view.hpp"

namespace unicode_ranges
{

template <typename Allocator>
class basic_utf16_string : public details::utf16_string_crtp<basic_utf16_string<Allocator>, utf16_string_view>
{
	using base_type = std::basic_string<char16_t, std::char_traits<char16_t>, Allocator>;
	using equivalent_utf16_string_view = utf16_string_view;
	using equivalent_string_view = std::u16string_view;

public:
	using allocator_type = Allocator;
	using value_type = utf16_char;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	static constexpr basic_utf16_string from_code_units_unchecked(base_type code_units) noexcept
	{
		basic_utf16_string result;
		result.base_ = std::move(code_units);
		return result;
	}

	static constexpr basic_utf16_string from_code_units_unchecked(equivalent_string_view code_units, Allocator alloc) noexcept
	{
		basic_utf16_string result;
		result.base_ = base_type{ code_units, std::move(alloc) };
		return result;
	}

	basic_utf16_string() = default;
	basic_utf16_string(const basic_utf16_string&) = default;
	basic_utf16_string(basic_utf16_string&&) = default;
	basic_utf16_string& operator=(const basic_utf16_string&) = default;
	basic_utf16_string& operator=(basic_utf16_string&&) = default;

	constexpr basic_utf16_string(const Allocator& alloc)
		: base_(alloc)
	{ }

	constexpr basic_utf16_string(const basic_utf16_string& other, const Allocator& alloc)
		: base_(other.base_, alloc)
	{ }

	constexpr basic_utf16_string(basic_utf16_string&& other, const Allocator& alloc)
		noexcept(std::allocator_traits<Allocator>::is_always_equal)
		: base_(std::move(other.base_), alloc)
	{ }

	constexpr basic_utf16_string(utf16_string_view view, const Allocator& alloc = Allocator())
		: base_(view.base(), alloc)
	{}

	constexpr basic_utf16_string(std::size_t count, utf16_char ch, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(count, ch);
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append_range(std::forward<R>(rg));
	}

	constexpr basic_utf16_string(std::initializer_list<utf16_char> ilist, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(ilist);
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string(It it, Sent sent, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(std::move(it), std::move(sent));
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& append_range(R&& rg)
	{
		for (utf16_char ch : std::forward<R>(rg))
		{
			base_.append(ch.as_view());
		}
		return *this;
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& assign_range(R&& rg)
	{
		base_.clear();
		return append_range(std::forward<R>(rg));
	}

	constexpr basic_utf16_string& append(size_type count, utf16_char ch)
	{
		const auto sv = ch.as_view();
		const auto total_size = sv.size() * count;
		const auto old_size = base_.size();

		base_.resize_and_overwrite(old_size + total_size,
			[&](char16_t* buffer, std::size_t)
			{
				buffer = buffer + old_size;
				for (size_type i = 0; i != count; ++i)
				{
					std::ranges::copy(sv, buffer);
					buffer += sv.size();
				}

				return total_size;
			});

		return *this;
	}

	constexpr basic_utf16_string& assign(size_type count, utf16_char ch)
	{
		base_.clear();
		return append(count, ch);
	}

	constexpr basic_utf16_string& append(utf16_string_view sv)
	{
		base_.append(sv.base());
		return *this;
	}

	constexpr basic_utf16_string& assign(utf16_string_view sv)
	{
		base_.assign(sv.base());
		return *this;
	}

	constexpr basic_utf16_string& assign(utf16_char ch)
	{
		base_.assign(ch.as_view());
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string& append(It it, Sent sent)
	{
		return append_range(std::ranges::subrange(std::move(it), std::move(sent)));
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string& assign(It it, Sent sent)
	{
		base_.clear();
		return append(std::move(it), std::move(sent));
	}

	constexpr basic_utf16_string& append(std::initializer_list<utf16_char> ilist)
	{
		return append_range(ilist);
	}

	constexpr basic_utf16_string& assign(std::initializer_list<utf16_char> ilist)
	{
		return assign_range(ilist);
	}

	constexpr basic_utf16_string& operator=(utf16_string_view sv)
	{
		return assign(sv);
	}

	constexpr basic_utf16_string& operator=(utf16_char ch)
	{
		return assign(ch);
	}

	constexpr basic_utf16_string& operator=(std::initializer_list<utf16_char> ilist)
	{
		return assign(ilist);
	}

	constexpr void shrink_to_fit()
	{
		base_.shrink_to_fit();
	}

	[[nodiscard]]
	constexpr size_type capacity() const
	{
		return base_.capacity();
	}

	[[nodiscard]]
	constexpr allocator_type get_allocator() const noexcept
	{
		return base_.get_allocator();
	}

	[[nodiscard]]
	constexpr size_type size() const
	{
		return base_.size();
	}

	constexpr basic_utf16_string& insert(size_type index, utf16_string_view sv)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

		base_.insert(index, sv.base());
		return *this;
	}

	constexpr basic_utf16_string& insert(size_type index, utf16_char ch)
	{
		return insert(index, equivalent_utf16_string_view::from_code_units_unchecked(ch.as_view()));
	}

	constexpr basic_utf16_string& insert(size_type index, size_type count, utf16_char ch)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

		base_type inserted{ base_.get_allocator() };
		const auto sv = ch.as_view();
		for (size_type i = 0; i != count; ++i)
		{
			inserted.append(sv);
		}

		base_.insert(index, inserted);
		return *this;
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& insert_range(size_type index, R&& rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf16_char_range
		{
			utf16_char ch;

			constexpr auto begin() const noexcept
			{
				return ch.as_view().begin();
			}

			constexpr auto end() const noexcept
			{
				return ch.as_view().end();
			}
		};

		auto inserted = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf16_char_range{ static_cast<utf16_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.insert_range(base_.begin() + static_cast<difference_type>(index), inserted);
#else
		const utf16_string inserted(std::from_range, std::forward<R>(rg));
		base_.insert(index, inserted.base());
#endif
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string& insert(size_type index, It first, Sent last)
	{
		return insert_range(index, std::ranges::subrange(std::move(first), std::move(last)));
	}

	constexpr basic_utf16_string& insert(size_type index, std::initializer_list<utf16_char> ilist)
	{
		return insert_range(index, ilist);
	}

	constexpr void pop_back()
	{
		const auto code_units_to_remove = (*(this->reversed_chars().begin())).code_unit_count();
		const auto where_idx = base_.size() - code_units_to_remove;
		base_.erase(where_idx, code_units_to_remove);
	}

	constexpr basic_utf16_string& erase(size_type index, size_type count = npos)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("erase index out of range");
		}

		const auto remaining = size() - index;
		const auto erase_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = index + erase_count;

		if (!this->is_char_boundary(index) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("erase range must be a valid UTF-16 substring");
		}

		base_.erase(index, erase_count);
		return *this;
	}

	constexpr basic_utf16_string& replace(size_type pos, size_type count, utf16_string_view other)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		const auto remaining = size() - pos;
		const auto replace_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + replace_count;

		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("replace range must be a valid UTF-16 substring");
		}

		base_.replace(pos, replace_count, other.base());
		return *this;
	}

	constexpr basic_utf16_string& replace(size_type pos, size_type count, utf16_char other)
	{
		return replace(pos, count, equivalent_utf16_string_view::from_code_units_unchecked(other.as_view()));
	}

	constexpr basic_utf16_string& replace(size_type pos, utf16_string_view other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		base_.replace(pos, replace_count, other.base());
		return *this;
	}

	constexpr basic_utf16_string& replace(size_type pos, utf16_char other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		base_.replace(pos, replace_count, other.as_view());
		return *this;
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& replace_with_range(size_type pos, size_type count, R&& rg)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		const auto remaining = size() - pos;
		const auto replace_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + replace_count;

		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("replace range must be a valid UTF-16 substring");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf16_char_range
		{
			utf16_char ch;

			constexpr auto begin() const noexcept
			{
				return ch.as_view().begin();
			}

			constexpr auto end() const noexcept
			{
				return ch.as_view().end();
			}
		};

		auto replacement = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf16_char_range{ static_cast<utf16_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(end),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf16_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& replace_with_range(size_type pos, R&& rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf16_char_range
		{
			utf16_char ch;

			constexpr auto begin() const noexcept
			{
				return ch.as_view().begin();
			}

			constexpr auto end() const noexcept
			{
				return ch.as_view().end();
			}
		};

		auto replacement = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf16_char_range{ static_cast<utf16_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(pos + replace_count),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf16_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	constexpr void reserve(size_type new_cap)
	{
		base_.reserve(new_cap);
	}

	[[nodiscard]]
	constexpr auto base() const& noexcept -> const base_type&
	{
		return base_;
	}

	[[nodiscard]]
	constexpr auto base() && noexcept -> base_type&&
	{
		return std::move(base_);
	}

	constexpr void clear()
	{
		base_.clear();
	}

	[[nodiscard]]
	constexpr const char16_t* data() const noexcept
	{
		return base_.data();
	}

	[[nodiscard]]
	constexpr const char16_t* c_str() const noexcept
	{
		return data();
	}

	constexpr operator utf16_string_view() const noexcept
	{
		return as_view();
	}

	[[nodiscard]]
	constexpr equivalent_utf16_string_view as_view() const noexcept
	{
		return equivalent_utf16_string_view::from_code_units_unchecked(equivalent_string_view{ base_ });
	}

	constexpr void push_back(utf16_char ch)
	{
		base_ += equivalent_string_view{ ch.as_view() };
	}

	constexpr void swap(basic_utf16_string& other)
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
	{
		base_.swap(other.base_);
	}

	friend constexpr bool operator==(const basic_utf16_string& lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr bool operator==(const basic_utf16_string& lhs, utf16_string_view rhs) noexcept
	{
		return lhs.base_ == rhs.base();
	}

	friend constexpr bool operator==(utf16_string_view lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base() == rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf16_string& lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf16_string& lhs, utf16_string_view rhs) noexcept
	{
		return lhs.base_ <=> rhs.base();
	}

	friend constexpr auto operator<=>(utf16_string_view lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base() <=> rhs.base_;
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(lhs.base_ + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(lhs.base_ + std::move(rhs.base_));
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + std::move(rhs.base_));
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, utf16_string_view rhs)
	{
		return from_code_units_unchecked(lhs.base_ + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, utf16_string_view rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(utf16_string_view lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(utf16_string_view lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + std::move(rhs.base_));
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, utf16_char rhs)
	{
		return from_code_units_unchecked(lhs.base_ + base_type{ rhs.as_view(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, utf16_char rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + base_type{ rhs.as_view(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(utf16_char lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.as_view(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(utf16_char lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.as_view(), rhs.get_allocator() } + std::move(rhs.base_));
	}

private:
	using base_class = details::utf16_string_crtp<basic_utf16_string<Allocator>, utf16_string_view>;

	base_type base_;
};

template <typename Allocator>
inline std::ostream& operator<<(std::ostream& os, const basic_utf16_string<Allocator>& value)
{
	return os << value.as_view();
}

}

namespace std
{
	template<typename Allocator>
	struct formatter<unicode_ranges::basic_utf16_string<Allocator>, char> : formatter<unicode_ranges::utf16_string_view, char>
	{
		template<typename FormatContext>
		auto format(const unicode_ranges::basic_utf16_string<Allocator>& value, FormatContext& ctx) const
		{
			return formatter<unicode_ranges::utf16_string_view, char>::format(value.as_view(), ctx);
		}
	};

	template<typename Allocator, typename OtherAllocator>
	struct uses_allocator<unicode_ranges::basic_utf16_string<Allocator>, OtherAllocator> : true_type
	{
	};
}

namespace unicode_ranges
{

namespace literals
{
	template<details::literals::constexpr_utf16_string Str>
	constexpr auto operator ""_utf16_s()
	{
		const auto sv = std::u16string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf16_string_view::from_code_units(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-16");
		}
		return utf16_string(result.value());
	}
}

}

#endif // UTF8_RANGES_UTF16_STRING_HPP
