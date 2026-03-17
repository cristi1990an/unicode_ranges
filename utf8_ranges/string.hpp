#pragma once

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
		std::array<char8_t, 4> bytes{};
		const auto size = ch.encode_utf8<char8_t>(bytes.begin());
		return byte_view().contains(std::u8string_view{ bytes.data(), size });
	}

	template <typename T>
		requires (std::same_as<T, char8_t> || std::same_as<T, utf8_char>)
	constexpr size_type find(T ch) const noexcept
	{
		if constexpr (std::same_as<T, char8_t>)
		{
			return byte_view().find(ch);
		}
		else
		{
			std::array<char8_t, 4> bytes{};
			const auto size = ch.template encode_utf8<char8_t>(bytes.begin());
			return byte_view().find(std::u8string_view{ bytes.data(), size });
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
		for (const auto& [idx, ch] : char_indices())
		{
			if (idx == index) [[unlikely]]
			{
				return ch;
			}
		}

		return std::nullopt;
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
class utf8_string : public utf8_string_crtp<utf8_string<Allocator>, utf8_string_view>
{
	using base_type = std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>;
	using equivalent_utf8_string_view = utf8_string_view;
	using equivalent_string_view = std::u8string_view;

public:
	using base_class = utf8_string_crtp<utf8_string<Allocator>, utf8_string_view>;
	using value_type = char8_t;
	using size_type = typename base_class::size_type;
	using difference_type = typename base_class::difference_type;
	static constexpr size_type npos = base_class::npos;

	utf8_string() = default;
	utf8_string(const utf8_string&) = default;
	utf8_string(utf8_string&&) = default;
	utf8_string& operator=(const utf8_string&) = default;
	utf8_string& operator=(utf8_string&&) = default;

	constexpr utf8_string(const Allocator& alloc)
		: base_(alloc)
	{ }

	constexpr utf8_string(const utf8_string& other, const Allocator& alloc)
		: base_(other.base_, alloc)
	{ }

	constexpr utf8_string(utf8_string&& other, const Allocator& alloc)
		noexcept(std::allocator_traits<Allocator>::is_always_equal)
		: base_(std::move(other.base_), alloc)
	{ }

	constexpr utf8_string(utf8_string_view view, const Allocator& alloc = Allocator())
		: base_(view.base(), alloc)
	{}

	constexpr utf8_string(std::size_t count, utf8_char ch, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(count, ch);
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr utf8_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append_range(std::forward<R>(rg));
	}

	constexpr utf8_string(std::initializer_list<utf8_char> ilist, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(ilist);
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr utf8_string(It it, Sent sent, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(std::move(it), std::move(sent));
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr utf8_string& append_range(R&& rg)
	{
		for (utf8_char ch : std::forward<R>(rg))
		{
			base_.append(ch.as_view());
		}
		return *this;
	}

	constexpr utf8_string& append(size_type count, utf8_char ch)
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

	constexpr utf8_string& append(utf8_string_view sv)
	{
		base_.append(sv.base());
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr utf8_string& append(It it, Sent sent)
	{
		return append_range(std::ranges::subrange(std::move(it), std::move(sent)));
	}

	constexpr utf8_string& append(std::initializer_list<utf8_char> ilist)
	{
		return append_range(ilist);
	}

	constexpr utf8_string& operator=(utf8_string_view sv)
	{
		base_.replace(sv.base());
		return *this;
	}

	constexpr utf8_string& operator=(utf8_char ch)
	{
		base_.replace(ch.as_view());
		return *this;
	}

	constexpr utf8_string& operator=(std::initializer_list<utf8_char> ilist)
	{
		base_.clear();
		base_.append(ilist);
		return *this;
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
	constexpr size_type size() const
	{
		return base_.size();
	}

	constexpr void pop_back()
	{
		const auto bytes_to_remove = (*(this->reversed_chars().begin())).byte_count();
		const auto where_idx = base_.size() - bytes_to_remove;
		base_.erase(where_idx, bytes_to_remove);
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

	constexpr void swap(utf8_string& other)
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
	{
		base_.swap(other.base_);
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
inline std::ostream& operator<<(std::ostream& os, const utf8_string<Allocator>& value)
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
	struct formatter<utf8_ranges::utf8_string<Allocator>, char> : formatter<utf8_ranges::utf8_string_view, char>
	{
		template<typename FormatContext>
		auto format(const utf8_ranges::utf8_string<Allocator>& value, FormatContext& ctx) const
		{
			return formatter<utf8_ranges::utf8_string_view, char>::format(value.as_view(), ctx);
		}
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
