#ifndef UTF8_RANGES_UTF16_STRING_CRTP_HPP
#define UTF8_RANGES_UTF16_STRING_CRTP_HPP

#include "utf16_views.hpp"

namespace unicode_ranges
{

namespace details
{

class utf16_char_indices_view : public std::ranges::view_interface<utf16_char_indices_view>
{
public:
	static constexpr utf16_char_indices_view from_code_units_unchecked(std::u16string_view base) noexcept
	{
		return utf16_char_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, utf16_char>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(std::u16string_view base, std::size_t current) noexcept
			: base_(base), current_(current)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto first = static_cast<std::uint16_t>(base_[current_]);
			const auto count = details::is_utf16_high_surrogate(first) ? 2u : 1u;
			return { current_, utf16_char::from_utf16_code_units_unchecked(base_.data() + current_, count) };
		}

		constexpr iterator& operator++() noexcept
		{
			const auto first = static_cast<std::uint16_t>(base_[current_]);
			current_ += details::is_utf16_high_surrogate(first) ? 2u : 1u;
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

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == it.base_.size();
		}

	private:
		std::u16string_view base_{};
		std::size_t current_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		return iterator{ base_, 0 };
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit utf16_char_indices_view(std::u16string_view base) noexcept
		: base_(base)
	{}

	std::u16string_view base_{};
};

template <typename View>
class utf16_grapheme_indices_view : public std::ranges::view_interface<utf16_grapheme_indices_view<View>>
{
public:
	static constexpr utf16_grapheme_indices_view from_code_units_unchecked(std::u16string_view base) noexcept
	{
		return utf16_grapheme_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, View>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(std::u16string_view base, std::size_t current, std::size_t next) noexcept
			: base_(base), current_(current), next_(next)
		{}

		constexpr reference operator*() const noexcept
		{
			return value_type{
				current_,
				View::from_code_units_unchecked(base_.substr(current_, next_ - current_))
			};
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

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == it.base_.size();
		}

	private:
		std::u16string_view base_{};
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
	constexpr explicit utf16_grapheme_indices_view(std::u16string_view base) noexcept
		: base_(base)
	{}

	std::u16string_view base_{};
};

template <typename Derived, typename View>
class utf16_string_crtp
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	constexpr auto chars() const noexcept
	{
		return views::utf16_view::from_code_units_unchecked(code_unit_view());
	}

	constexpr auto reversed_chars() const noexcept
	{
		return views::reversed_utf16_view::from_code_units_unchecked(code_unit_view());
	}

	constexpr auto graphemes() const noexcept -> views::grapheme_cluster_view<char16_t>;
	constexpr auto reversed_graphemes() const noexcept -> views::reversed_grapheme_cluster_view<char16_t>;
	template <typename Allocator = std::allocator<char8_t>>
	constexpr basic_utf8_string<Allocator> to_utf8(const Allocator& alloc = Allocator()) const;

	constexpr size_type size() const noexcept
	{
		return code_unit_view().size();
	}

	constexpr bool empty() const noexcept
	{
		return code_unit_view().empty();
	}

	constexpr bool is_ascii() const noexcept
	{
		return std::ranges::all_of(chars(), [](utf16_char ch) noexcept { return ch.is_ascii(); });
	}

	constexpr auto char_indices() const noexcept
	{
		return utf16_char_indices_view::from_code_units_unchecked(code_unit_view());
	}

	constexpr auto grapheme_indices() const noexcept
	{
		return utf16_grapheme_indices_view<View>::from_code_units_unchecked(code_unit_view());
	}

	constexpr bool is_grapheme_boundary(size_type index) const noexcept
	{
		return details::is_grapheme_boundary(code_unit_view(), index);
	}

	constexpr bool contains(utf16_char ch) const noexcept
	{
		return find(ch) != npos;
	}

	constexpr bool contains(View sv) const noexcept
	{
		return find(sv) != npos;
	}

	constexpr bool contains_grapheme(utf16_char ch) const noexcept
	{
		return find_grapheme(ch) != npos;
	}

	constexpr bool contains_grapheme(View sv) const noexcept
	{
		return find_grapheme(sv) != npos;
	}

	constexpr size_type find(char16_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		if consteval
		{
			for (size_type index = pos; index != size(); ++index)
			{
				if (code_unit_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return code_unit_view().find(ch, pos);
		}
	}

	constexpr size_type find(utf16_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		const auto needle = ch.as_view();
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
					if (code_unit_view()[index + needle_index] != needle[needle_index])
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
			return code_unit_view().find(needle, pos);
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
					if (code_unit_view()[index + needle_index] != needle[needle_index])
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
			return code_unit_view().find(needle, pos);
		}
	}

	constexpr size_type find_grapheme(utf16_char ch, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(code_unit_view(), ch.as_view(), pos);
	}

	constexpr size_type find_grapheme(View sv, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(code_unit_view(), sv.base(), pos);
	}

	constexpr size_type find_first_of(char16_t ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	constexpr size_type find_first_of(utf16_char ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	constexpr size_type find_first_of(View sv, size_type pos = 0) const noexcept
	{
		if (sv.empty())
		{
			return npos;
		}

		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && sv.contains(current);
		});

		return it == indices.end() ? npos : (*it).first;
	}

	constexpr size_type find_first_not_of(char16_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		for (size_type index = pos; index != size(); ++index)
		{
			if (code_unit_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	constexpr size_type find_first_not_of(utf16_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && current != ch;
		});

		return it == indices.end() ? npos : (*it).first;
	}

	constexpr size_type find_first_not_of(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && !sv.contains(current);
		});

		return it == indices.end() ? npos : (*it).first;
	}

	constexpr size_type rfind(char16_t ch, size_type pos = npos) const noexcept
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
				if (code_unit_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return code_unit_view().rfind(ch, pos);
		}
	}

	constexpr size_type rfind(utf16_char ch, size_type pos = npos) const noexcept
	{
		const auto needle = ch.as_view();
		if (needle.size() > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		pos = floor_char_boundary((std::min)(pos, size() - needle.size()));
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle.size(); ++needle_index)
				{
					if (code_unit_view()[index + needle_index] != needle[needle_index])
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
			return code_unit_view().rfind(needle, pos);
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
					if (code_unit_view()[index + needle_index] != needle[needle_index])
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
			return code_unit_view().rfind(needle, pos);
		}
	}

	constexpr size_type rfind_grapheme(utf16_char ch, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(code_unit_view(), ch.as_view(), pos);
	}

	constexpr size_type rfind_grapheme(View sv, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(code_unit_view(), sv.base(), pos);
	}

	constexpr size_type find_last_of(char16_t ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	constexpr size_type find_last_of(utf16_char ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	constexpr size_type find_last_of(View sv, size_type pos = npos) const noexcept
	{
		if (empty() || sv.empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		if (pos == size())
		{
			pos = floor_char_boundary(size() - 1);
		}
		for (size_type index = pos;;)
		{
			if (sv.contains(char_at_unchecked(index)))
			{
				return index;
			}

			if (index == 0)
			{
				return npos;
			}

			index = floor_char_boundary(index - 1);
		}
	}

	constexpr size_type find_last_not_of(char16_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		for (size_type index = pos + 1; index != 0;)
		{
			--index;
			if (code_unit_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	constexpr size_type find_last_not_of(utf16_char ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		size_type result = npos;
		for (const auto [index, current] : char_indices())
		{
			if (index > pos)
			{
				break;
			}

			if (current != ch)
			{
				result = index;
			}
		}

		return result;
	}

	constexpr size_type find_last_not_of(View sv, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		if (pos == size())
		{
			pos = floor_char_boundary(size() - 1);
		}
		for (size_type index = pos;;)
		{
			if (!sv.contains(char_at_unchecked(index)))
			{
				return index;
			}

			if (index == 0)
			{
				return npos;
			}

			index = floor_char_boundary(index - 1);
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

		return !details::is_utf16_low_surrogate(static_cast<std::uint16_t>(code_unit_view()[index]));
	}

	constexpr size_type char_count() const noexcept
	{
		return static_cast<size_type>(std::ranges::distance(chars()));
	}

	constexpr size_type grapheme_count() const noexcept
	{
		return details::grapheme_count(code_unit_view());
	}

	constexpr std::pair<View, View> split(size_type delim) const
	{
		if (!is_char_boundary(delim)) [[unlikely]]
		{
			throw std::out_of_range("split index must be at a UTF-16 character boundary");
		}

		const auto lhs = code_unit_view().substr(0, delim);
		const auto rhs = code_unit_view().substr(delim);
		return {
			View::from_code_units_unchecked(lhs),
			View::from_code_units_unchecked(rhs)
		};
	}

	constexpr std::optional<utf16_char> char_at(size_type index) const noexcept
	{
		if (index >= size() || !is_char_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		return char_at_unchecked(index);
	}

	constexpr utf16_char char_at_unchecked(size_type index) const noexcept
	{
		const auto first = static_cast<std::uint16_t>(code_unit_view()[index]);
		const auto len = details::is_utf16_high_surrogate(first) ? 2u : 1u;
		return utf16_char::from_utf16_code_units_unchecked(code_unit_view().data() + index, len);
	}

	constexpr std::optional<View> grapheme_at(size_type index) const noexcept
	{
		if (index >= size() || !is_grapheme_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = details::next_grapheme_boundary(code_unit_view(), index);
		return View::from_code_units_unchecked(code_unit_view().substr(index, end - index));
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

		return View::from_code_units_unchecked(code_unit_view().substr(pos, end - pos));
	}

	constexpr std::optional<View> grapheme_substr(size_type pos, size_type count = npos) const noexcept
	{
		if (!is_grapheme_boundary(pos)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = (count == npos)
			? size()
			: (std::min)(size(), pos + count);

		if (!is_grapheme_boundary(end)) [[unlikely]]
		{
			return std::nullopt;
		}

		return View::from_code_units_unchecked(code_unit_view().substr(pos, end - pos));
	}

	constexpr std::optional<utf16_char> front() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return front_unchecked();
	}

	constexpr utf16_char front_unchecked() const noexcept
	{
		return *chars().begin();
	}

	constexpr std::optional<utf16_char> back() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return back_unchecked();
	}

	constexpr utf16_char back_unchecked() const noexcept
	{
		return *reversed_chars().begin();
	}

	constexpr bool starts_with(char16_t ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(utf16_char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(View sv) const noexcept
	{
		return code_unit_view().starts_with(sv.base());
	}

	constexpr bool ends_with(char16_t ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(utf16_char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(View sv) const noexcept
	{
		return code_unit_view().ends_with(sv.base());
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

	constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept
	{
		return details::ceil_grapheme_boundary(code_unit_view(), pos);
	}

	constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept
	{
		return details::floor_grapheme_boundary(code_unit_view(), pos);
	}

protected:
	constexpr const Derived& self() const noexcept
	{
		return static_cast<const Derived&>(*this);
	}

	constexpr std::u16string_view code_unit_view() const noexcept
	{
		return std::u16string_view{ self().base() };
	}
};

}

}

#endif // UTF8_RANGES_UTF16_STRING_CRTP_HPP
