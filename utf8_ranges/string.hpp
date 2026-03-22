#ifndef UTF8_RANGES_STRING_HPP
#define UTF8_RANGES_STRING_HPP

#include "views.hpp"

namespace utf8_ranges
{

template <typename Derived, typename View>
class utf8_string_crtp
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	constexpr auto chars() const noexcept
	{
		return views::utf8_view::from_bytes_unchecked(byte_view());
	}

	constexpr auto reversed_chars() const noexcept
	{
		return views::reversed_utf8_view::from_bytes_unchecked(byte_view());
	}

	constexpr size_type size() const noexcept
	{
		return byte_view().size();
	}

	constexpr bool empty() const noexcept
	{
		return byte_view().empty();
	}

	constexpr bool is_ascii() const noexcept
	{
		return std::ranges::all_of(chars(), [](utf8_char ch) noexcept { return ch.is_ascii(); });
	}

	constexpr auto char_indices() const noexcept
	{
		return chars() | std::views::enumerate;
	}

	constexpr bool contains(utf8_char ch) const noexcept
	{
		return find(ch) != npos;
	}

	constexpr bool contains(View sv) const noexcept
	{
		return find(sv) != npos;
	}

	constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		if consteval
		{
			for (size_type index = pos; index != size(); ++index)
			{
				if (byte_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(ch, pos);
		}
	}

	constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		std::array<char8_t, 4> bytes{};
		const auto needle_size = ch.encode_utf8<char8_t>(bytes.begin());
		if consteval
		{
			if (needle_size > size() - pos)
			{
				return npos;
			}

			for (size_type index = pos; index + needle_size <= size(); ++index)
			{
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle_size; ++needle_index)
				{
					if (byte_view()[index + needle_index] != bytes[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(std::u8string_view{ bytes.data(), needle_size }, pos);
		}
	}

	constexpr size_type find(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		const auto needle = sv.base();
		if (needle.empty())
		{
			return pos;
		}

		if consteval
		{
			if (needle.size() > size() - pos)
			{
				return npos;
			}

			for (size_type index = pos; index + needle.size() <= size(); ++index)
			{
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle.size(); ++needle_index)
				{
					if (byte_view()[index + needle_index] != needle[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(needle, pos);
		}
	}

	constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				if (byte_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(ch, pos);
		}
	}

	constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept
	{
		std::array<char8_t, 4> bytes{};
		const auto needle_size = ch.encode_utf8<char8_t>(bytes.begin());
		if (needle_size > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		pos = floor_char_boundary((std::min)(pos, size() - needle_size));
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle_size; ++needle_index)
				{
					if (byte_view()[index + needle_index] != bytes[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(std::u8string_view{ bytes.data(), needle_size }, pos);
		}
	}

	constexpr size_type rfind(View sv, size_type pos = npos) const noexcept
	{
		const auto needle = sv.base();
		pos = floor_char_boundary((std::min)(size(), pos));
		if (needle.empty())
		{
			return pos;
		}

		if (needle.size() > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(pos, size() - needle.size()));
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle.size(); ++needle_index)
				{
					if (byte_view()[index + needle_index] != needle[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(needle, pos);
		}
	}

	constexpr bool is_char_boundary(size_type index) const noexcept
	{
		if (index > size()) [[unlikely]]
		{
			return false;
		}

		if (index == 0 || index == size()) [[unlikely]]
		{
			return true;
		}

		return details::is_utf8_lead_byte(static_cast<std::uint8_t>(byte_view()[index]));
	}

	constexpr size_type char_count() const noexcept
	{
		return static_cast<size_type>(std::ranges::distance(chars()));
	}

	constexpr std::pair<View, View> split(size_type delim) const
	{
		if (!is_char_boundary(delim)) [[unlikely]]
		{
			throw std::out_of_range("split index must be at a UTF-8 character boundary");
		}

		const auto lhs = byte_view().substr(0, delim);
		const auto rhs = byte_view().substr(delim);
		return {
			View::from_bytes_unchecked(lhs),
			View::from_bytes_unchecked(rhs)
		};
	}

	constexpr std::optional<utf8_char> char_at(size_type index) const noexcept
	{
		if (index >= size() || !is_char_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		return char_at_unchecked(index);
	}

	constexpr utf8_char char_at_unchecked(size_type index) const noexcept
	{
		const auto len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(byte_view()[index]));
		return utf8_char::from_utf8_bytes_unchecked(byte_view().data() + index, len);
	}

	constexpr std::optional<View> substr(size_type pos, size_type count = npos) const noexcept
	{
		if (!is_char_boundary(pos)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = (count == npos)
			? size()
			: (std::min)(size(), pos + count);

		if (!is_char_boundary(end)) [[unlikely]]
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(pos, end - pos));
	}

	constexpr utf8_char front() const noexcept
	{
		return *chars().begin();
	}

	constexpr utf8_char back() const noexcept
	{
		return *reversed_chars().begin();
	}

	constexpr bool starts_with(char ch) const noexcept
	{
		return !empty() && (front() == ch);
	}

	constexpr bool starts_with(char8_t ch) const noexcept
	{
		return !empty() && (front() == ch);
	}

	constexpr bool starts_with(utf8_char ch) const noexcept
	{
		return !empty() && (front() == ch);
	}

	constexpr bool starts_with(View sv) const noexcept
	{
		return byte_view().starts_with(sv.base());
	}

	constexpr bool ends_with(char ch) const noexcept
	{
		return !empty() && (back() == ch);
	}

	constexpr bool ends_with(char8_t ch) const noexcept
	{
		return !empty() && (back() == ch);
	}

	constexpr bool ends_with(utf8_char ch) const noexcept
	{
		return !empty() && (back() == ch);
	}

	constexpr bool ends_with(View sv) const noexcept
	{
		return byte_view().ends_with(sv.base());
	}

	constexpr size_type ceil_char_boundary(size_type pos) const noexcept
	{
		pos = (std::min)(size(), pos);
		while (pos != size() && !is_char_boundary(pos))
		{
			++pos;
		}

		return pos;
	}

	constexpr size_type floor_char_boundary(size_type pos) const noexcept
	{
		pos = (std::min)(size(), pos);
		while (pos != 0 && !is_char_boundary(pos))
		{
			--pos;
		}

		return pos;
	}

protected:
	constexpr const Derived& self() const noexcept
	{
		return static_cast<const Derived&>(*this);
	}

	constexpr std::u8string_view byte_view() const noexcept
	{
		return std::u8string_view{ self().base() };
	}
};

class utf8_string_view : public utf8_string_crtp<utf8_string_view, utf8_string_view>
{
public:
	using base_class = utf8_string_crtp<utf8_string_view, utf8_string_view>;
	using size_type = typename base_class::size_type;
	using difference_type = typename base_class::difference_type;
	static constexpr size_type npos = base_class::npos;

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
	constexpr explicit utf8_string_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_;
};

template <typename Allocator>
class basic_utf8_string : public utf8_string_crtp<basic_utf8_string<Allocator>, utf8_string_view>
{
	using base_type = std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>;
	using equivalent_utf8_string_view = utf8_string_view;
	using equivalent_string_view = std::u8string_view;

public:
	using base_class = utf8_string_crtp<basic_utf8_string<Allocator>, utf8_string_view>;
	using allocator_type = Allocator;
	using value_type = char8_t;
	using size_type = typename base_class::size_type;
	using difference_type = typename base_class::difference_type;
	static constexpr size_type npos = base_class::npos;

	static constexpr basic_utf8_string from_bytes_unchecked(base_type bytes) noexcept
	{
		basic_utf8_string result;
		result.base_ = std::move(bytes);
		return result;
	}

	static constexpr basic_utf8_string from_bytes_unchecked(equivalent_string_view bytes, Allocator alloc) noexcept
	{
		basic_utf8_string result;
		result.base_ = base_type{ bytes, std::move(alloc) };
		return result;
	}

	basic_utf8_string() = default;
	basic_utf8_string(const basic_utf8_string&) = default;
	basic_utf8_string(basic_utf8_string&&) = default;
	basic_utf8_string& operator=(const basic_utf8_string&) = default;
	basic_utf8_string& operator=(basic_utf8_string&&) = default;

	constexpr basic_utf8_string(const Allocator& alloc)
		: base_(alloc)
	{ }

	constexpr basic_utf8_string(const basic_utf8_string& other, const Allocator& alloc)
		: base_(other.base_, alloc)
	{ }

	constexpr basic_utf8_string(basic_utf8_string&& other, const Allocator& alloc)
		noexcept(std::allocator_traits<Allocator>::is_always_equal)
		: base_(std::move(other.base_), alloc)
	{ }

	constexpr basic_utf8_string(utf8_string_view view, const Allocator& alloc = Allocator())
		: base_(view.base(), alloc)
	{}

	constexpr basic_utf8_string(std::size_t count, utf8_char ch, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(count, ch);
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append_range(std::forward<R>(rg));
	}

	constexpr basic_utf8_string(std::initializer_list<utf8_char> ilist, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(ilist);
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string(It it, Sent sent, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(std::move(it), std::move(sent));
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& append_range(R&& rg)
	{
		for (utf8_char ch : std::forward<R>(rg))
		{
			base_.append(ch.as_view());
		}
		return *this;
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& assign_range(R&& rg)
	{
		base_.clear();
		return append_range(std::forward<R>(rg));
	}

	constexpr basic_utf8_string& append(size_type count, utf8_char ch)
	{
		const auto sv = ch.as_view();
		const auto total_size = sv.size() * count;
		const auto old_size = base_.size();
		
		base_.resize_and_overwrite(old_size + total_size,
			[&](char8_t* buffer, std::size_t)
			{
				buffer = buffer + old_size;
				for (size_type i = 0; i != count; i++)
				{
					std::ranges::copy(sv, buffer);
					buffer += sv.size();
				}

				return total_size;
			});

		return *this;
	}

	constexpr basic_utf8_string& assign(size_type count, utf8_char ch)
	{
		base_.clear();
		return append(count, ch);
	}

	constexpr basic_utf8_string& append(utf8_string_view sv)
	{
		base_.append(sv.base());
		return *this;
	}

	constexpr basic_utf8_string& assign(utf8_string_view sv)
	{
		base_.assign(sv.base());
		return *this;
	}

	constexpr basic_utf8_string& assign(utf8_char ch)
	{
		base_.assign(ch.as_view());
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string& append(It it, Sent sent)
	{
		return append_range(std::ranges::subrange(std::move(it), std::move(sent)));
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string& assign(It it, Sent sent)
	{
		base_.clear();
		return append(std::move(it), std::move(sent));
	}

	constexpr basic_utf8_string& append(std::initializer_list<utf8_char> ilist)
	{
		return append_range(ilist);
	}

	constexpr basic_utf8_string& assign(std::initializer_list<utf8_char> ilist)
	{
		return assign_range(ilist);
	}

	constexpr basic_utf8_string& operator=(utf8_string_view sv)
	{
		return assign(sv);
	}

	constexpr basic_utf8_string& operator=(utf8_char ch)
	{
		return assign(ch);
	}

	constexpr basic_utf8_string& operator=(std::initializer_list<utf8_char> ilist)
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

	constexpr basic_utf8_string& insert(size_type index, utf8_string_view sv)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
		}

		base_.insert(index, sv.base());
		return *this;
	}

	constexpr basic_utf8_string& insert(size_type index, utf8_char ch)
	{
		return insert(index, ch.as_utf8_view());
	}

	constexpr basic_utf8_string& insert(size_type index, size_type count, utf8_char ch)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
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

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& insert_range(size_type index, R&& rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf8_char_range
		{
			utf8_char ch;

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
					return encoded_utf8_char_range{ static_cast<utf8_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.insert_range(base_.begin() + static_cast<difference_type>(index), inserted);
#else
		const utf8_string inserted(std::from_range, std::forward<R>(rg));
		base_.insert(index, inserted.base());
#endif
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string& insert(size_type index, It first, Sent last)
	{
		return insert_range(index, std::ranges::subrange(std::move(first), std::move(last)));
	}

	constexpr basic_utf8_string& insert(size_type index, std::initializer_list<utf8_char> ilist)
	{
		return insert_range(index, ilist);
	}

	constexpr void pop_back()
	{
		const auto bytes_to_remove = (*(this->reversed_chars().begin())).byte_count();
		const auto where_idx = base_.size() - bytes_to_remove;
		base_.erase(where_idx, bytes_to_remove);
	}

	constexpr basic_utf8_string& erase(size_type index, size_type count = npos)
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
			throw std::out_of_range("erase range must be a valid UTF-8 substring");
		}

		base_.erase(index, erase_count);
		return *this;
	}

	constexpr basic_utf8_string& replace(size_type pos, size_type count, utf8_string_view other)
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
			throw std::out_of_range("replace range must be a valid UTF-8 substring");
		}

		base_.replace(pos, replace_count, other.base());
		return *this;
	}

	constexpr basic_utf8_string& replace(size_type pos, size_type count, utf8_char other)
	{
		return replace(pos, count, other.as_utf8_view());
	}

	constexpr basic_utf8_string& replace(size_type pos, utf8_string_view other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).byte_count();
		base_.replace(pos, replace_count, other.base());
		return *this;
	}

	constexpr basic_utf8_string& replace(size_type pos, utf8_char other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).byte_count();
		base_.replace(pos, replace_count, other.as_view());
		return *this;
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& replace_with_range(size_type pos, size_type count, R&& rg)
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
			throw std::out_of_range("replace range must be a valid UTF-8 substring");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf8_char_range
		{
			utf8_char ch;

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
					return encoded_utf8_char_range{ static_cast<utf8_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(end),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf8_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& replace_with_range(size_type pos, R&& rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).byte_count();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf8_char_range
		{
			utf8_char ch;

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
					return encoded_utf8_char_range{ static_cast<utf8_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(pos + replace_count),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf8_char ch : std::forward<R>(rg))
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
	constexpr const char8_t* data() const noexcept
	{
		return base_.data();
	}

	[[nodiscard]]
	constexpr const char8_t* c_str() const noexcept
	{
		return data();
	}

	constexpr operator utf8_string_view() const noexcept
	{
		return as_view();
	}

	[[nodiscard]]
	constexpr equivalent_utf8_string_view as_view() const noexcept
	{
		return equivalent_utf8_string_view::from_bytes_unchecked(equivalent_string_view{ base_ });
	}

	constexpr void push_back(utf8_char ch)
	{
		std::array<char8_t, 4> tmp{};
		const auto size = ch.encode_utf8<char8_t>(tmp.begin());
		base_ += equivalent_string_view{ tmp.data(), size };
	}

	constexpr void swap(basic_utf8_string& other)
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
	{
		base_.swap(other.base_);
	}

	friend constexpr basic_utf8_string operator+(basic_utf8_string lhs, utf8_string_view rhs)
	{
		return from_bytes_unchecked(std::move(lhs.base_) + base_type{ rhs.base() });
	}

	friend constexpr basic_utf8_string operator+(utf8_string_view lhs, basic_utf8_string rhs)
	{
		return from_bytes_unchecked(base_type{ lhs.base() } + std::move(rhs.base_));
	}

	friend constexpr basic_utf8_string operator+(basic_utf8_string lhs, utf8_char rhs)
	{
		return from_bytes_unchecked(std::move(lhs.base_) + base_type{ rhs.as_view() });
	}

	friend constexpr basic_utf8_string operator+(utf8_char lhs, basic_utf8_string rhs)
	{
		return from_bytes_unchecked(base_type{ lhs.as_view() } + std::move(rhs.base_));
	}

private:
	base_type base_;
};

inline std::ostream& operator<<(std::ostream& os, utf8_string_view value)
{
	const auto text = value.base();
	os.write(reinterpret_cast<const char*>(text.data()), static_cast<std::streamsize>(text.size()));
	return os;
}

template <typename Allocator>
inline std::ostream& operator<<(std::ostream& os, const basic_utf8_string<Allocator>& value)
{
	return os << value.as_view();
}

[[nodiscard]]
inline constexpr utf8_string_view utf8_char::as_utf8_view() const noexcept
{
	return utf8_string_view::from_bytes_unchecked(as_view());
}

}

namespace std
{
	template<>
	struct hash<utf8_ranges::utf8_string_view>
	{
		std::size_t operator()(utf8_ranges::utf8_string_view value) const noexcept
		{
			return std::hash<std::u8string_view>{}(value.base());
		}
	};

	template<>
	struct formatter<utf8_ranges::utf8_string_view, char> : formatter<std::string_view, char>
	{
		template<typename FormatContext>
		auto format(utf8_ranges::utf8_string_view value, FormatContext& ctx) const
		{
			const auto text = value.base();
			return formatter<std::string_view, char>::format(
				std::string_view{ reinterpret_cast<const char*>(text.data()), text.size() },
				ctx);
		}
	};

	template<typename Allocator>
	struct formatter<utf8_ranges::basic_utf8_string<Allocator>, char> : formatter<utf8_ranges::utf8_string_view, char>
	{
		template<typename FormatContext>
		auto format(const utf8_ranges::basic_utf8_string<Allocator>& value, FormatContext& ctx) const
		{
			return formatter<utf8_ranges::utf8_string_view, char>::format(value.as_view(), ctx);
		}
	};

	template<typename Allocator, typename OtherAllocator>
	struct uses_allocator<utf8_ranges::basic_utf8_string<Allocator>, OtherAllocator> : true_type
	{
	};
}

namespace utf8_ranges
{

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

	template<details::literals::constexpr_utf8_string Str>
	consteval auto operator ""_utf8_s()
	{
		const auto sv = std::u8string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf8_string_view::from_bytes(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-8");
		}
		return utf8_string(result.value());
	}
}

}

#endif // UTF8_RANGES_STRING_HPP
