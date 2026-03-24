#ifndef UTF8_RANGES_UTF8_STRING_CRTP_HPP
#define UTF8_RANGES_UTF8_STRING_CRTP_HPP

#include "utf8_views.hpp"

namespace unicode_ranges
{

namespace details
{

class utf8_char_indices_view : public std::ranges::view_interface<utf8_char_indices_view>
{
public:
	static constexpr utf8_char_indices_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		return utf8_char_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, utf8_char>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(std::u8string_view base, std::size_t current) noexcept
			: base_(base), current_(current)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base_[current_]));
			return { current_, utf8_char::from_utf8_bytes_unchecked(base_.data() + current_, count) };
		}

		constexpr iterator& operator++() noexcept
		{
			current_ += details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base_[current_]));
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
		std::u8string_view base_{};
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
	constexpr explicit utf8_char_indices_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

template <typename View>
class utf8_grapheme_indices_view : public std::ranges::view_interface<utf8_grapheme_indices_view<View>>
{
public:
	static constexpr utf8_grapheme_indices_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		return utf8_grapheme_indices_view{ base };
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

		constexpr iterator(std::u8string_view base, std::size_t current, std::size_t next) noexcept
			: base_(base), current_(current), next_(next)
		{}

		constexpr reference operator*() const noexcept
		{
			return value_type{
				current_,
				View::from_bytes_unchecked(base_.substr(current_, next_ - current_))
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
		std::u8string_view base_{};
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
	constexpr explicit utf8_grapheme_indices_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

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

	constexpr auto graphemes() const noexcept -> views::grapheme_cluster_view<char8_t>;
	constexpr auto reversed_graphemes() const noexcept -> views::reversed_grapheme_cluster_view<char8_t>;
	template <typename Allocator = std::allocator<char16_t>>
	constexpr basic_utf16_string<Allocator> to_utf16(const Allocator& alloc = Allocator()) const;

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
		return utf8_char_indices_view::from_bytes_unchecked(byte_view());
	}

	constexpr auto grapheme_indices() const noexcept
	{
		return utf8_grapheme_indices_view<View>::from_bytes_unchecked(byte_view());
	}

	constexpr bool is_grapheme_boundary(size_type index) const noexcept
	{
		return details::is_grapheme_boundary(byte_view(), index);
	}

	constexpr bool contains(utf8_char ch) const noexcept
	{
		return find(ch) != npos;
	}

	constexpr bool contains(View sv) const noexcept
	{
		return find(sv) != npos;
	}

	constexpr bool contains_grapheme(utf8_char ch) const noexcept
	{
		return find_grapheme(ch) != npos;
	}

	constexpr bool contains_grapheme(View sv) const noexcept
	{
		return find_grapheme(sv) != npos;
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

	constexpr size_type find_grapheme(utf8_char ch, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(byte_view(), ch.as_view(), pos);
	}

	constexpr size_type find_grapheme(View sv, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(byte_view(), sv.base(), pos);
	}

	constexpr size_type find_first_of(char8_t ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	constexpr size_type find_first_of(utf8_char ch, size_type pos = 0) const noexcept
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

	constexpr size_type find_first_not_of(char8_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		for (size_type index = pos; index != size(); ++index)
		{
			if (byte_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	constexpr size_type find_first_not_of(utf8_char ch, size_type pos = 0) const noexcept
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

	constexpr size_type rfind_grapheme(utf8_char ch, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(byte_view(), ch.as_view(), pos);
	}

	constexpr size_type rfind_grapheme(View sv, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(byte_view(), sv.base(), pos);
	}

	constexpr size_type find_last_of(char8_t ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	constexpr size_type find_last_of(utf8_char ch, size_type pos = npos) const noexcept
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

	constexpr size_type find_last_not_of(char8_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		for (size_type index = pos + 1; index != 0;)
		{
			--index;
			if (byte_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	constexpr size_type find_last_not_of(utf8_char ch, size_type pos = npos) const noexcept
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

		return details::is_utf8_lead_byte(static_cast<std::uint8_t>(byte_view()[index]));
	}

	constexpr size_type char_count() const noexcept
	{
		return static_cast<size_type>(std::ranges::distance(chars()));
	}

	constexpr size_type grapheme_count() const noexcept
	{
		return details::grapheme_count(byte_view());
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

	constexpr std::optional<View> grapheme_at(size_type index) const noexcept
	{
		if (index >= size() || !is_grapheme_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = details::next_grapheme_boundary(byte_view(), index);
		return View::from_bytes_unchecked(byte_view().substr(index, end - index));
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

		return View::from_bytes_unchecked(byte_view().substr(pos, end - pos));
	}

	constexpr std::optional<utf8_char> front() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return front_unchecked();
	}

	constexpr utf8_char front_unchecked() const noexcept
	{
		return *chars().begin();
	}

	constexpr std::optional<utf8_char> back() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return back_unchecked();
	}

	constexpr utf8_char back_unchecked() const noexcept
	{
		return *reversed_chars().begin();
	}

	constexpr bool starts_with(char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(char8_t ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(utf8_char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(View sv) const noexcept
	{
		return byte_view().starts_with(sv.base());
	}

	constexpr bool ends_with(char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(char8_t ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(utf8_char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
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

	constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept
	{
		return details::ceil_grapheme_boundary(byte_view(), pos);
	}

	constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept
	{
		return details::floor_grapheme_boundary(byte_view(), pos);
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

}

}

#endif // UTF8_RANGES_UTF8_STRING_CRTP_HPP
