#ifndef UTF8_RANGES_UTF16_STRING_CRTP_HPP
#define UTF8_RANGES_UTF16_STRING_CRTP_HPP

#include "utf16_views.hpp"

namespace unicode_ranges
{

namespace details
{

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
		return chars() | std::views::enumerate;
	}

	constexpr bool contains(utf16_char ch) const noexcept
	{
		return find(ch) != npos;
	}

	constexpr bool contains(View sv) const noexcept
	{
		return find(sv) != npos;
	}

	constexpr size_type find(char16_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		return code_unit_view().find(ch, pos);
	}

	constexpr size_type find(utf16_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		return code_unit_view().find(ch.as_view(), pos);
	}

	constexpr size_type find(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		return code_unit_view().find(sv.base(), pos);
	}

	constexpr size_type rfind(char16_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		return code_unit_view().rfind(ch, pos);
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
		return code_unit_view().rfind(needle, pos);
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
		return code_unit_view().rfind(needle, pos);
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

	constexpr utf16_char front() const noexcept
	{
		return *chars().begin();
	}

	constexpr utf16_char back() const noexcept
	{
		return *reversed_chars().begin();
	}

	constexpr bool starts_with(char16_t ch) const noexcept
	{
		return !empty() && (front() == ch);
	}

	constexpr bool starts_with(utf16_char ch) const noexcept
	{
		return !empty() && (front() == ch);
	}

	constexpr bool starts_with(View sv) const noexcept
	{
		return code_unit_view().starts_with(sv.base());
	}

	constexpr bool ends_with(char16_t ch) const noexcept
	{
		return !empty() && (back() == ch);
	}

	constexpr bool ends_with(utf16_char ch) const noexcept
	{
		return !empty() && (back() == ch);
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
