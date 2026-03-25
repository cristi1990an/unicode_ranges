#ifndef UTF8_RANGES_TESTS_HPP
#define UTF8_RANGES_TESTS_HPP

#include "unicode_ranges.hpp"

#include <array>
#include <cassert>
#include <format>
#include <functional>
#include <memory_resource>
#include <sstream>
#include <string>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

inline void run_unicode_ranges_tests()
{
	// Shared test helpers used by both the UTF-8 and UTF-16 sections.
	[[maybe_unused]] const auto wide_from_scalar = [](std::uint32_t scalar)
	{
		std::wstring result;
		if constexpr (sizeof(wchar_t) == 2)
		{
			if (scalar <= 0xFFFFu)
			{
				result.push_back(static_cast<wchar_t>(scalar));
			}
			else
			{
				const auto shifted = scalar - 0x10000u;
				result.push_back(static_cast<wchar_t>(0xD800u + (shifted >> 10)));
				result.push_back(static_cast<wchar_t>(0xDC00u + (shifted & 0x3FFu)));
			}
		}
		else
		{
			result.push_back(static_cast<wchar_t>(scalar));
		}

		return result;
	};
	const auto expect_out_of_range = [](auto&& fn)
	{
		try
		{
			std::forward<decltype(fn)>(fn)();
			return false;
		}
		catch (const std::out_of_range&)
		{
			return true;
		}
	};
	const auto unwrap_utf8_view = [](std::u8string_view bytes)
	{
		const auto result = utf8_string_view::from_bytes(bytes);
		assert(result.has_value());
		return result.value();
	};
	const auto unwrap_utf16_view = [](std::u16string_view code_units)
	{
		const auto result = utf16_string_view::from_code_units(code_units);
		assert(result.has_value());
		return result.value();
	};

	static_assert(std::same_as<
		pmr::utf8_string,
		basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>>);
	static_assert(std::same_as<
		pmr::utf16_string,
		basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>>);
	constexpr utf8_char latin1_ch = "é"_u8c;
	constexpr auto utf8_text = "Aé€"_utf8_sv;
	constexpr auto utf16_text = u"Aé😀"_utf16_sv;

	static_assert(unicode_character<utf8_char>);
	static_assert(std::same_as<decltype("A"_u8c.to_utf8_owned()), utf8_string>);
	static_assert(std::same_as<decltype(u"A"_u16c.to_utf16_owned()), utf16_string>);
	static_assert(unicode_character<const utf8_char&>);
	static_assert(unicode_character<utf16_char>);
	static_assert(unicode_character<utf16_char&&>);
	static_assert(!unicode_character<char8_t>);
	static_assert(!unicode_character<char16_t>);

	// Standard ranges/view concept coverage for the library view surface.
	static_assert(std::ranges::view<views::utf8_view>);
	static_assert(std::ranges::range<views::utf8_view>);
	static_assert(std::ranges::view<views::reversed_utf8_view>);
	static_assert(std::ranges::range<views::reversed_utf8_view>);
	static_assert(std::ranges::view<views::utf16_view>);
	static_assert(std::ranges::range<views::utf16_view>);
	static_assert(std::ranges::view<views::reversed_utf16_view>);
	static_assert(std::ranges::range<views::reversed_utf16_view>);
	static_assert(std::ranges::view<views::grapheme_cluster_view<char8_t>>);
	static_assert(std::ranges::range<views::grapheme_cluster_view<char8_t>>);
	static_assert(std::ranges::view<views::grapheme_cluster_view<char16_t>>);
	static_assert(std::ranges::range<views::grapheme_cluster_view<char16_t>>);
	static_assert(std::ranges::view<views::reversed_grapheme_cluster_view<char8_t>>);
	static_assert(std::ranges::range<views::reversed_grapheme_cluster_view<char8_t>>);
	static_assert(std::ranges::view<views::reversed_grapheme_cluster_view<char16_t>>);
	static_assert(std::ranges::range<views::reversed_grapheme_cluster_view<char16_t>>);
	static_assert(std::ranges::view<views::lossy_utf8_view<char>>);
	static_assert(std::ranges::range<views::lossy_utf8_view<char>>);
	static_assert(std::ranges::view<views::lossy_utf8_view<char8_t>>);
	static_assert(std::ranges::range<views::lossy_utf8_view<char8_t>>);
	static_assert(std::ranges::view<views::lossy_utf16_view<char16_t>>);
	static_assert(std::ranges::range<views::lossy_utf16_view<char16_t>>);
	static_assert(std::ranges::view<views::lossy_utf16_view<wchar_t>>);
	static_assert(std::ranges::range<views::lossy_utf16_view<wchar_t>>);

	static_assert(std::ranges::view<decltype(utf8_text.chars())>);
	static_assert(std::ranges::range<decltype(utf8_text.chars())>);
	static_assert(std::ranges::view<decltype(utf8_text.reversed_chars())>);
	static_assert(std::ranges::range<decltype(utf8_text.reversed_chars())>);
	static_assert(std::ranges::view<decltype(utf8_text.char_indices())>);
	static_assert(std::ranges::range<decltype(utf8_text.char_indices())>);
	static_assert(std::ranges::view<decltype(utf8_text.split(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.split(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::bidirectional_range<decltype(utf8_text.split(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::common_range<decltype(utf8_text.split(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.rsplit(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.rsplit(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::bidirectional_range<decltype(utf8_text.rsplit(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::common_range<decltype(utf8_text.rsplit(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(std::views::reverse(utf8_text.split(u8"\u00E9"_u8c)))>);
	static_assert(std::ranges::range<decltype(std::views::reverse(utf8_text.split(u8"\u00E9"_u8c)))>);
	static_assert(std::ranges::bidirectional_range<decltype(std::views::reverse(utf8_text.split(u8"\u00E9"_u8c)))>);
	static_assert(std::ranges::view<decltype(utf8_text.split_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.split_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::bidirectional_range<decltype(utf8_text.split_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::common_range<decltype(utf8_text.split_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.rsplit_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.rsplit_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.splitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.splitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.rsplitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.rsplitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::same_as<
		decltype(utf8_text.split_once(u8"\u00E9"_u8c)),
		std::optional<std::pair<utf8_string_view, utf8_string_view>>>);
	static_assert(std::same_as<
		decltype(utf8_text.rsplit_once(u8"\u00E9"_u8c)),
		std::optional<std::pair<utf8_string_view, utf8_string_view>>>);
	static_assert(std::same_as<
		decltype(utf8_text.strip_prefix(u8"\u00E9"_u8c)),
		std::optional<utf8_string_view>>);
	static_assert(std::same_as<
		decltype(utf8_text.strip_circumfix(u8"\u00E9"_u8c, u8"\u00E9"_u8c)),
		std::optional<utf8_string_view>>);
	static_assert(std::same_as<
		decltype(utf8_text.trim_prefix(u8"\u00E9"_u8c)),
		utf8_string_view>);
	static_assert(std::same_as<
		decltype(utf8_text.trim_matches(u8"\u00E9"_u8c)),
		utf8_string_view>);
	static_assert(std::same_as<
		decltype(utf8_text.trim()),
		utf8_string_view>);
	static_assert(std::ranges::view<decltype(utf8_text.split_whitespace())>);
	static_assert(std::ranges::range<decltype(utf8_text.split_whitespace())>);
	static_assert(std::ranges::view<decltype(utf8_text.split_ascii_whitespace())>);
	static_assert(std::ranges::range<decltype(utf8_text.split_ascii_whitespace())>);
	static_assert(std::same_as<
		decltype(utf8_text.split_once_at(1)),
		std::optional<std::pair<utf8_string_view, utf8_string_view>>>);
	static_assert(std::ranges::view<decltype(utf8_text.graphemes())>);
	static_assert(std::ranges::range<decltype(utf8_text.graphemes())>);
	static_assert(std::ranges::view<decltype(utf8_text.reversed_graphemes())>);
	static_assert(std::ranges::range<decltype(utf8_text.reversed_graphemes())>);
	static_assert(std::ranges::view<decltype(utf8_text.grapheme_indices())>);
	static_assert(std::ranges::range<decltype(utf8_text.grapheme_indices())>);

	static_assert(std::ranges::view<decltype(utf16_text.chars())>);
	static_assert(std::ranges::range<decltype(utf16_text.chars())>);
	static_assert(std::ranges::view<decltype(utf16_text.reversed_chars())>);
	static_assert(std::ranges::range<decltype(utf16_text.reversed_chars())>);
	static_assert(std::ranges::view<decltype(utf16_text.char_indices())>);
	static_assert(std::ranges::range<decltype(utf16_text.char_indices())>);
	static_assert(std::ranges::view<decltype(utf16_text.split(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.split(u"\u00E9"_u16c))>);
	static_assert(std::ranges::bidirectional_range<decltype(utf16_text.split(u"\u00E9"_u16c))>);
	static_assert(std::ranges::common_range<decltype(utf16_text.split(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.rsplit(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.rsplit(u"\u00E9"_u16c))>);
	static_assert(std::ranges::bidirectional_range<decltype(utf16_text.rsplit(u"\u00E9"_u16c))>);
	static_assert(std::ranges::common_range<decltype(utf16_text.rsplit(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(std::views::reverse(utf16_text.split(u"\u00E9"_u16c)))>);
	static_assert(std::ranges::range<decltype(std::views::reverse(utf16_text.split(u"\u00E9"_u16c)))>);
	static_assert(std::ranges::bidirectional_range<decltype(std::views::reverse(utf16_text.split(u"\u00E9"_u16c)))>);
	static_assert(std::ranges::view<decltype(utf16_text.split_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.split_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::bidirectional_range<decltype(utf16_text.split_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::common_range<decltype(utf16_text.split_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.rsplit_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.rsplit_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.splitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.splitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.rsplitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.rsplitn(2, u"\u00E9"_u16c))>);
	static_assert(std::same_as<
		decltype(utf16_text.split_once(u"\u00E9"_u16c)),
		std::optional<std::pair<utf16_string_view, utf16_string_view>>>);
	static_assert(std::same_as<
		decltype(utf16_text.rsplit_once(u"\u00E9"_u16c)),
		std::optional<std::pair<utf16_string_view, utf16_string_view>>>);
	static_assert(std::same_as<
		decltype(utf16_text.strip_prefix(u"\u00E9"_u16c)),
		std::optional<utf16_string_view>>);
	static_assert(std::same_as<
		decltype(utf16_text.strip_circumfix(u"\u00E9"_u16c, u"\u00E9"_u16c)),
		std::optional<utf16_string_view>>);
	static_assert(std::same_as<
		decltype(utf16_text.trim_prefix(u"\u00E9"_u16c)),
		utf16_string_view>);
	static_assert(std::same_as<
		decltype(utf16_text.trim_matches(u"\u00E9"_u16c)),
		utf16_string_view>);
	static_assert(std::same_as<
		decltype(utf16_text.trim()),
		utf16_string_view>);
	static_assert(std::ranges::view<decltype(utf16_text.split_whitespace())>);
	static_assert(std::ranges::range<decltype(utf16_text.split_whitespace())>);
	static_assert(std::ranges::view<decltype(utf16_text.split_ascii_whitespace())>);
	static_assert(std::ranges::range<decltype(utf16_text.split_ascii_whitespace())>);
	static_assert(std::same_as<
		decltype(utf16_text.split_once_at(1)),
		std::optional<std::pair<utf16_string_view, utf16_string_view>>>);
	static_assert(std::ranges::view<decltype(utf16_text.graphemes())>);
	static_assert(std::ranges::range<decltype(utf16_text.graphemes())>);
	static_assert(std::ranges::view<decltype(utf16_text.reversed_graphemes())>);
	static_assert(std::ranges::range<decltype(utf16_text.reversed_graphemes())>);
	static_assert(std::ranges::view<decltype(utf16_text.grapheme_indices())>);
	static_assert(std::ranges::range<decltype(utf16_text.grapheme_indices())>);

	// utf8_char compile-time API coverage.
	static_assert("A"_u8c.ascii_lowercase() == "a"_u8c);
	static_assert("z"_u8c.ascii_uppercase() == "Z"_u8c);
	static_assert(latin1_ch.ascii_lowercase() == latin1_ch);
	static_assert("A"_u8c.eq_ignore_ascii_case("a"_u8c));
	static_assert(!latin1_ch.eq_ignore_ascii_case("e"_u8c));

	static_assert("A"_u8c.is_ascii_alphabetic());
	static_assert(!latin1_ch.is_ascii_alphabetic());
	static_assert("7"_u8c.is_ascii_digit());
	static_assert("7"_u8c.is_ascii_alphanumeric());
	static_assert("Q"_u8c.is_ascii_alphanumeric());
	static_assert(!"-"_u8c.is_ascii_alphanumeric());
	static_assert(" "_u8c.is_ascii_whitespace());
	static_assert("\n"_u8c.is_ascii_whitespace());
	static_assert(!"A"_u8c.is_ascii_whitespace());

	static_assert("Ω"_u8c.is_alphabetic());
	static_assert("Ω"_u8c.is_alphanumeric());
	static_assert("Ω"_u8c.is_uppercase());
	static_assert(!"Ω"_u8c.is_lowercase());
	static_assert("ω"_u8c.is_lowercase());
	static_assert(!"ω"_u8c.is_uppercase());
	static_assert("5"_u8c.is_digit());
	static_assert("5"_u8c.is_numeric());
	static_assert("Ⅷ"_u8c.is_numeric());
	static_assert(!"Ⅷ"_u8c.is_digit());
	static_assert(" "_u8c.is_whitespace());
	static_assert(utf8_char::from_scalar_unchecked(0x0085u).is_control());
	static_assert("F"_u8c.is_ascii_hexdigit());
	static_assert("7"_u8c.is_ascii_octdigit());
	static_assert("!"_u8c.is_ascii_punctuation());
	static_assert("A"_u8c.is_ascii_graphic());
	static_assert(!" "_u8c.is_ascii_graphic());
	static_assert("\n"_u8c.is_ascii_control());

	static_assert(std::get<0>(unicode_version) == 17);
	static_assert(std::get<1>(unicode_version) == 0);
	static_assert(std::get<2>(unicode_version) == 0);

	static_assert([] {
		utf8_char lhs = "A"_u8c;
		utf8_char rhs = "z"_u8c;
		lhs.swap(rhs);
		return lhs == "z"_u8c && rhs == "A"_u8c;
	}());

	// utf8_string_view compile-time API coverage.
	static_assert(utf8_text.size() == 6);
	static_assert(utf8_text == "Aé€"_utf8_sv);
	static_assert(utf8_text.is_char_boundary(0));
	static_assert(utf8_text.is_char_boundary(1));
	static_assert(!utf8_text.is_char_boundary(2));
	static_assert(utf8_text.ceil_char_boundary(0) == 0);
	static_assert(utf8_text.ceil_char_boundary(2) == 3);
	static_assert(utf8_text.ceil_char_boundary(utf8_string_view::npos) == utf8_text.size());
	static_assert(utf8_text.floor_char_boundary(0) == 0);
	static_assert(utf8_text.floor_char_boundary(2) == 1);
	static_assert(utf8_text.floor_char_boundary(utf8_string_view::npos) == utf8_text.size());
	static_assert(utf8_text.char_at(0).has_value());
	static_assert(utf8_text.char_at(0).value() == "A"_u8c);
	static_assert(utf8_text.char_at(1).has_value());
	static_assert(utf8_text.char_at(1).value() == "é"_u8c);
	static_assert(!utf8_text.char_at(2).has_value());
	static_assert(!utf8_text.char_at(utf8_text.size()).has_value());
	static_assert(utf8_text.char_at_unchecked(3) == "€"_u8c);
	static_assert(utf8_text.front().has_value());
	static_assert(utf8_text.front().value() == "A"_u8c);
	static_assert(utf8_text.front_unchecked() == "A"_u8c);
	static_assert(utf8_text.back().has_value());
	static_assert(utf8_text.back().value() == "€"_u8c);
	static_assert(utf8_text.back_unchecked() == "€"_u8c);
	static_assert(!utf8_string_view{}.front().has_value());
	static_assert(!utf8_string_view{}.back().has_value());
	static_assert(utf8_text.find(static_cast<char8_t>('A')) == 0);
	static_assert(utf8_text.find(static_cast<char8_t>(0xA9), 2) == 2);
	static_assert(utf8_text.find(static_cast<char8_t>('A'), utf8_string_view::npos) == utf8_string_view::npos);
	static_assert(utf8_text.find("é€"_utf8_sv) == 1);
	static_assert(utf8_text.find("é€"_utf8_sv, 2) == utf8_string_view::npos);
	static_assert(utf8_text.find("€"_utf8_sv, 2) == 3);
	static_assert(utf8_text.find("€"_utf8_sv, utf8_string_view::npos) == utf8_string_view::npos);
	static_assert(utf8_text.find_first_of(static_cast<char8_t>('A')) == 0);
	static_assert(utf8_text.find_first_of("€A"_utf8_sv) == 0);
	static_assert(utf8_text.find_first_of("€A"_utf8_sv, 1) == 3);
	static_assert(utf8_text.find_first_of(u8""_utf8_sv) == utf8_string_view::npos);
	static_assert(utf8_text.find("é"_u8c, 2) == utf8_string_view::npos);
	static_assert(utf8_text.find("€"_u8c, 2) == 3);
	static_assert(utf8_text.find("€"_u8c, utf8_string_view::npos) == utf8_string_view::npos);
	static_assert(utf8_text.find("é"_u8c) == 1);
	static_assert(utf8_text.find("€"_u8c) == 3);
	static_assert(utf8_text.find("Z"_u8c) == utf8_string_view::npos);
	static_assert(utf8_text.find_first_not_of(static_cast<char8_t>('A')) == 1);
	static_assert(utf8_text.find_first_not_of("A"_u8c) == 1);
	static_assert(utf8_text.find_first_not_of("Aé"_utf8_sv) == 3);
	static_assert(utf8_text.find_first_not_of(u8""_utf8_sv, 2) == 3);
	static_assert(utf8_text.rfind(static_cast<char8_t>('A')) == 0);
	static_assert(utf8_text.rfind(static_cast<char8_t>(0xA9), 2) == 2);
	static_assert(utf8_text.rfind(static_cast<char8_t>('A'), 0) == 0);
	static_assert(utf8_text.find_last_of(static_cast<char8_t>('A')) == 0);
	static_assert(utf8_text.find_last_of("€A"_utf8_sv) == 3);
	static_assert(utf8_text.find_last_of("€A"_utf8_sv, 1) == 0);
	static_assert(utf8_text.find_last_of(u8""_utf8_sv) == utf8_string_view::npos);
	static_assert(utf8_text.rfind("é€"_utf8_sv) == 1);
	static_assert(utf8_text.rfind("é€"_utf8_sv, 2) == 1);
	static_assert(utf8_text.rfind("€"_utf8_sv, 2) == utf8_string_view::npos);
	static_assert(utf8_text.rfind("€"_utf8_sv, utf8_string_view::npos) == 3);
	static_assert(utf8_text.rfind("é"_u8c, 2) == 1);
	static_assert(utf8_text.rfind("€"_u8c, 2) == utf8_string_view::npos);
	static_assert(utf8_text.rfind("€"_u8c) == 3);
	static_assert(utf8_text.rfind("Z"_u8c) == utf8_string_view::npos);
	static_assert(utf8_text.find_last_not_of(static_cast<char8_t>('A')) == 5);
	static_assert(utf8_text.find_last_not_of("€"_u8c) == 1);
	static_assert(utf8_text.find_last_not_of("Aé"_utf8_sv) == 3);
	static_assert(utf8_text.find_last_not_of(u8""_utf8_sv) == 3);
	static_assert([] {
		constexpr auto text = u8"e\u0301🇷🇴!"_utf8_sv;
		return text.find_grapheme(u8"e\u0301"_grapheme_utf8) == 0
			&& text.find_grapheme(u8"🇷🇴"_grapheme_utf8, 1) == 3
			&& text.find_grapheme(u8"\u0301"_u8c) == utf8_string_view::npos
			&& text.contains_grapheme(u8"🇷🇴"_grapheme_utf8)
			&& !text.contains_grapheme(u8"\u0301"_u8c)
			&& text.rfind_grapheme(u8"!"_grapheme_utf8) == 11
			&& text.rfind_grapheme(u8"🇷🇴"_grapheme_utf8, 10) == 3;
	}());
	static_assert(utf8_text.substr(1).has_value());
	static_assert(utf8_text.substr(1).value() == "é€"_utf8_sv);
	static_assert(utf8_text.substr(1, 2).value() == "é"_utf8_sv);
	static_assert(!utf8_text.substr(2, 1).has_value());
	static_assert(utf8_text.starts_with('A'));
	static_assert(utf8_text.starts_with(static_cast<char8_t>('A')));
	static_assert(utf8_text.starts_with("A"_u8c));
	static_assert(utf8_text.starts_with("A"_utf8_sv));
	static_assert(!utf8_text.starts_with("é"_u8c));
	static_assert(!utf8_text.ends_with('A'));
	static_assert(!utf8_text.ends_with(static_cast<char8_t>('A')));
	static_assert(utf8_text.ends_with("€"_u8c));
	static_assert(utf8_text.ends_with("€"_utf8_sv));
	static_assert(!utf8_text.ends_with("é"_u8c));
	static_assert(utf8_string_view::from_bytes("Aé€"_utf8_sv.as_view()).has_value());
	static_assert(utf8_text == "Aé€"_utf8_sv);
	static_assert(std::same_as<decltype(utf8_text.to_utf8_owned()), utf8_string>);
	static_assert(utf8_text < "Z"_utf8_sv);
	static_assert([] {
		constexpr auto text = u8"e\u0301X"_utf8_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u8"e\u0301"_grapheme_utf8,
			u8"X"_grapheme_utf8
		});
	}());
	static_assert([] {
		constexpr auto text = u8"\r\nX"_utf8_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u8"\r\n"_grapheme_utf8,
			u8"X"_grapheme_utf8
		});
	}());
	static_assert([] {
		constexpr auto text = u8"🇷🇴!"_utf8_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u8"🇷🇴"_grapheme_utf8,
			u8"!"_grapheme_utf8
		});
	}());
	static_assert([] {
		constexpr auto text = u8"👩‍💻!"_utf8_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u8"👩‍💻"_grapheme_utf8,
			u8"!"_grapheme_utf8
		});
	}());
	static_assert([] {
		constexpr auto text = u8"e\u0301🇷🇴!"_utf8_sv;
		return std::ranges::equal(text.reversed_graphemes(), std::array{
			u8"!"_grapheme_utf8,
			u8"🇷🇴"_grapheme_utf8,
			u8"e\u0301"_grapheme_utf8
		});
	}());
	static_assert(u8"e\u0301"_grapheme_utf8 == u8"e\u0301"_utf8_sv);
	static_assert(u8"Aé😀"_utf8_sv.to_utf16() == u"Aé😀"_utf16_sv);
	static_assert([] {
		constexpr auto text = u8"Aé€"_utf8_sv;
		auto it = text.char_indices().begin();
		if (it == text.char_indices().end()) return false;
		const auto [i0, c0] = *it++;
		if (i0 != 0 || c0 != "A"_u8c) return false;
		const auto [i1, c1] = *it++;
		if (i1 != 1 || c1 != "é"_u8c) return false;
		const auto [i2, c2] = *it++;
		return i2 == 3 && c2 == "€"_u8c && it == text.char_indices().end();
	}());
	static_assert([] {
		constexpr auto text = u8"e\u0301🇷🇴!"_utf8_sv;
		auto it = text.grapheme_indices().begin();
		if (it == text.grapheme_indices().end()) return false;
		const auto [i0, g0] = *it++;
		if (i0 != 0 || g0 != u8"e\u0301"_grapheme_utf8) return false;
		const auto [i1, g1] = *it++;
		if (i1 != 3 || g1 != u8"🇷🇴"_grapheme_utf8) return false;
		const auto [i2, g2] = *it++;
		return i2 == 11 && g2 == u8"!"_grapheme_utf8 && it == text.grapheme_indices().end();
	}());
	static_assert([] {
		constexpr auto text = u8"abra--cadabra--"_utf8_sv;
		auto parts = text.split(u8"--"_utf8_sv);
		auto it = parts.begin();
		if (it == parts.end() || *it != u8"abra"_utf8_sv) return false;
		++it;
		if (it == parts.end() || *it != u8"cadabra"_utf8_sv) return false;
		++it;
		if (it == parts.end() || *it != u8""_utf8_sv) return false;
		auto rit = parts.end();
		--rit;
		if (*rit != u8""_utf8_sv) return false;
		--rit;
		if (*rit != u8"cadabra"_utf8_sv) return false;
		--rit;
		return *rit == u8"abra"_utf8_sv;
	}());
	static_assert([] {
		constexpr auto text = u8"--abra--cadabra--"_utf8_sv;
		return std::ranges::equal(text.rsplit(u8"--"_utf8_sv), std::array{
			u8""_utf8_sv,
			u8"cadabra"_utf8_sv,
			u8"abra"_utf8_sv,
			u8""_utf8_sv
		});
	}());
	static_assert([] {
		constexpr auto text = u8"--abra--cadabra--"_utf8_sv;
		return std::ranges::equal(text.split_terminator(u8"--"_utf8_sv), std::array{
			u8""_utf8_sv,
			u8"abra"_utf8_sv,
			u8"cadabra"_utf8_sv
		});
	}());
	static_assert([] {
		constexpr auto text = u8"--abra--cadabra--"_utf8_sv;
		return std::ranges::equal(text.rsplit_terminator(u8"--"_utf8_sv), std::array{
			u8"cadabra"_utf8_sv,
			u8"abra"_utf8_sv,
			u8""_utf8_sv
		});
	}());
	static_assert([] {
		constexpr auto text = u8"abra--cadabra--!"_utf8_sv;
		return std::ranges::equal(text.splitn(0, u8"--"_utf8_sv), std::array<utf8_string_view, 0>{})
			&& std::ranges::equal(text.splitn(1, u8"--"_utf8_sv), std::array{
				u8"abra--cadabra--!"_utf8_sv
			})
			&& std::ranges::equal(text.splitn(2, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv,
				u8"cadabra--!"_utf8_sv
			})
			&& std::ranges::equal(text.splitn(4, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv,
				u8"cadabra"_utf8_sv,
				u8"!"_utf8_sv
			});
	}());
	static_assert([] {
		constexpr auto text = u8"abra--cadabra--!"_utf8_sv;
		return std::ranges::equal(text.rsplitn(0, u8"--"_utf8_sv), std::array<utf8_string_view, 0>{})
			&& std::ranges::equal(text.rsplitn(1, u8"--"_utf8_sv), std::array{
				u8"abra--cadabra--!"_utf8_sv
			})
			&& std::ranges::equal(text.rsplitn(2, u8"--"_utf8_sv), std::array{
				u8"!"_utf8_sv,
				u8"abra--cadabra"_utf8_sv
			})
			&& std::ranges::equal(text.rsplitn(4, u8"--"_utf8_sv), std::array{
				u8"!"_utf8_sv,
				u8"cadabra"_utf8_sv,
				u8"abra"_utf8_sv
			});
	}());
	static_assert([] {
		constexpr auto text = u8"abra--cadabra--!"_utf8_sv;
		const auto first = text.split_once(u8"--"_utf8_sv);
		const auto last = text.rsplit_once(u8"--"_utf8_sv);
		return first.has_value()
			&& first->first == u8"abra"_utf8_sv
			&& first->second == u8"cadabra--!"_utf8_sv
			&& last.has_value()
			&& last->first == u8"abra--cadabra"_utf8_sv
			&& last->second == u8"!"_utf8_sv
			&& !text.split_once(u8""_utf8_sv).has_value()
			&& !text.rsplit_once(u8""_utf8_sv).has_value()
			&& !u8"abra"_utf8_sv.split_once(u8"--"_utf8_sv).has_value();
	}());
	static_assert([] {
		constexpr auto text = u8"<<<\u00E9A>>>"_utf8_sv;
		const auto stripped_prefix = text.strip_prefix(u8"<<<"_utf8_sv);
		const auto stripped_suffix = text.strip_suffix(u8">>>"_utf8_sv);
		const auto stripped_circ = text.strip_circumfix(u8"<<<"_utf8_sv, u8">>>"_utf8_sv);
		const auto stripped_chars = u8"[\u00E9]"_utf8_sv.strip_circumfix(u8"["_u8c, u8"]"_u8c);
		return stripped_prefix.has_value()
			&& stripped_prefix.value() == u8"\u00E9A>>>"_utf8_sv
			&& stripped_suffix.has_value()
			&& stripped_suffix.value() == u8"<<<\u00E9A"_utf8_sv
			&& stripped_circ.has_value()
			&& stripped_circ.value() == u8"\u00E9A"_utf8_sv
			&& stripped_chars.has_value()
			&& stripped_chars.value() == u8"\u00E9"_utf8_sv
			&& !text.strip_prefix(u8">>>"_utf8_sv).has_value()
			&& !text.strip_circumfix(u8"<<<"_utf8_sv, u8"]"_utf8_sv).has_value()
			&& text.trim_prefix(u8">>>"_utf8_sv) == text
			&& text.trim_prefix(u8"<<<"_utf8_sv) == u8"\u00E9A>>>"_utf8_sv
			&& text.trim_suffix(u8">>>"_utf8_sv) == u8"<<<\u00E9A"_utf8_sv
			&& u8"\u00E9A\u00E9"_utf8_sv.trim_prefix(u8"\u00E9"_u8c) == u8"A\u00E9"_utf8_sv
			&& u8"\u00E9A\u00E9"_utf8_sv.trim_suffix(u8"\u00E9"_u8c) == u8"\u00E9A"_utf8_sv;
	}());
	static_assert([] {
		constexpr auto repeated = u8"----abra----"_utf8_sv;
		constexpr auto accented = u8"\u00E9\u00E9A\u00E9"_utf8_sv;
		return repeated.trim_start_matches(u8"--"_utf8_sv) == u8"abra----"_utf8_sv
			&& repeated.trim_end_matches(u8"--"_utf8_sv) == u8"----abra"_utf8_sv
			&& repeated.trim_matches(u8"--"_utf8_sv) == u8"abra"_utf8_sv
			&& repeated.trim_matches(u8""_utf8_sv) == repeated
			&& u8"***abra***"_utf8_sv.trim_matches(u8"*"_u8c) == u8"abra"_utf8_sv
			&& accented.trim_start_matches(u8"\u00E9"_u8c) == u8"A\u00E9"_utf8_sv
			&& accented.trim_end_matches(u8"\u00E9"_u8c) == u8"\u00E9\u00E9A"_utf8_sv
			&& accented.trim_matches(u8"\u00E9"_u8c) == u8"A"_utf8_sv;
	}());
	static_assert([] {
		constexpr auto unicode_trimmed = u8"\u00A0\tA\u00A0 "_utf8_sv;
		constexpr auto unicode_split = u8"\u00A0A\u2003B C"_utf8_sv;
		return unicode_trimmed.trim() == u8"A"_utf8_sv
			&& unicode_trimmed.trim_start() == u8"A\u00A0 "_utf8_sv
			&& unicode_trimmed.trim_end() == u8"\u00A0\tA"_utf8_sv
			&& unicode_trimmed.trim_ascii() == u8"\u00A0\tA\u00A0"_utf8_sv
			&& unicode_trimmed.trim_ascii_start() == unicode_trimmed
			&& unicode_trimmed.trim_ascii_end() == u8"\u00A0\tA\u00A0"_utf8_sv
			&& std::ranges::equal(u8""_utf8_sv.split_whitespace(), std::array<utf8_string_view, 0>{})
			&& std::ranges::equal(u8" \t\r\n"_utf8_sv.split_ascii_whitespace(), std::array<utf8_string_view, 0>{})
			&& std::ranges::equal(u8" \tA  B\n"_utf8_sv.split_whitespace(), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			})
			&& std::ranges::equal(u8" \tA  B\n"_utf8_sv.split_ascii_whitespace(), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			})
			&& std::ranges::equal(unicode_split.split_whitespace(), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv,
				u8"C"_utf8_sv
			})
			&& std::ranges::equal(unicode_split.split_ascii_whitespace(), std::array{
				u8"\u00A0A\u2003B"_utf8_sv,
				u8"C"_utf8_sv
			});
	}());
	static_assert([] {
		constexpr auto empty = u8""_utf8_sv;
		constexpr auto exact = u8"--"_utf8_sv;
		constexpr auto repeated = u8"a----b"_utf8_sv;
		constexpr auto missing = u8"abra"_utf8_sv;
		constexpr auto unicode = u8"A\u00E9B\u00E9"_utf8_sv;
		const auto exact_first = exact.split_once(u8"--"_utf8_sv);
		const auto exact_last = exact.rsplit_once(u8"--"_utf8_sv);
		const auto unicode_first = unicode.split_once(u8"\u00E9"_u8c);
		const auto unicode_last = unicode.rsplit_once(u8"\u00E9"_u8c);
		return std::ranges::equal(empty.split(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			})
			&& std::ranges::equal(empty.rsplit(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			})
			&& std::ranges::equal(empty.splitn(2, u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			})
			&& std::ranges::equal(empty.rsplitn(2, u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			})
			&& !empty.split_once(u8"--"_utf8_sv).has_value()
			&& !empty.rsplit_once(u8"--"_utf8_sv).has_value()
			&& std::ranges::equal(exact.split(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv,
				u8""_utf8_sv
			})
			&& std::ranges::equal(exact.split_terminator(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			})
			&& exact_first.has_value()
			&& exact_first->first == u8""_utf8_sv
			&& exact_first->second == u8""_utf8_sv
			&& exact_last.has_value()
			&& exact_last->first == u8""_utf8_sv
			&& exact_last->second == u8""_utf8_sv
			&& std::ranges::equal(repeated.split(u8"--"_utf8_sv), std::array{
				u8"a"_utf8_sv,
				u8""_utf8_sv,
				u8"b"_utf8_sv
			})
			&& std::ranges::equal(missing.split(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(missing.rsplit(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(std::views::reverse(missing.split(u8"--"_utf8_sv)), missing.rsplit(u8"--"_utf8_sv))
			&& std::ranges::equal(missing.split(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(missing.rsplit(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(missing.split_terminator(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(missing.rsplit_terminator(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(missing.splitn(2, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(missing.rsplitn(2, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(unicode.split(u8"\u00E9"_u8c), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv,
				u8""_utf8_sv
			})
			&& unicode_first.has_value()
			&& unicode_first->first == u8"A"_utf8_sv
			&& unicode_first->second == u8"B\u00E9"_utf8_sv
			&& unicode_last.has_value()
			&& unicode_last->first == u8"A\u00E9B"_utf8_sv
			&& unicode_last->second == u8""_utf8_sv;
	}());
	static_assert([] {
		constexpr auto text = u8"A\u00E9\u20AC"_utf8_sv;
		const auto parts = text.split_once_at(1);
		if (!parts.has_value()) return false;
		return parts->first == u8"A"_utf8_sv && parts->second == u8"\u00E9\u20AC"_utf8_sv;
	}());
	static_assert([] {
		constexpr auto text = u8"A\u00E9\u20AC"_utf8_sv;
		return !text.split_once_at(2).has_value();
	}());
	static_assert([] {
		constexpr auto text = u8"A\u00E9\u20AC"_utf8_sv;
		const auto parts = text.split_once_at_unchecked(1);
		return parts.first == u8"A"_utf8_sv && parts.second == u8"\u00E9\u20AC"_utf8_sv;
	}());
	static_assert([] {
		constexpr auto text = u8"e\u0301🇷🇴!"_utf8_sv;
		return text.grapheme_count() == 3
			&& text.is_grapheme_boundary(0)
			&& !text.is_grapheme_boundary(1)
			&& text.is_grapheme_boundary(3)
			&& !text.is_grapheme_boundary(7)
			&& text.is_grapheme_boundary(11)
			&& text.ceil_grapheme_boundary(1) == 3
			&& text.floor_grapheme_boundary(1) == 0
			&& text.ceil_grapheme_boundary(7) == 11
			&& text.floor_grapheme_boundary(7) == 3
			&& text.grapheme_at(0).has_value()
			&& text.grapheme_at(0).value() == u8"e\u0301"_grapheme_utf8
			&& text.grapheme_at(3).has_value()
			&& text.grapheme_at(3).value() == u8"🇷🇴"_grapheme_utf8
			&& !text.grapheme_at(1).has_value()
			&& text.grapheme_substr(3, 8).has_value()
			&& text.grapheme_substr(3, 8).value() == u8"🇷🇴"_utf8_sv
			&& text.grapheme_substr(3).has_value()
			&& text.grapheme_substr(3).value() == u8"🇷🇴!"_utf8_sv
			&& !text.grapheme_substr(1, 2).has_value();
	}());

	// utf16_string_view mirrors the utf8_string_view API, but with UTF-16 code-unit semantics.
	static_assert(utf16_text.size() == 4);
	static_assert(utf16_text == u"Aé😀"_utf16_sv);
	static_assert(utf16_text.is_char_boundary(0));
	static_assert(utf16_text.is_char_boundary(1));
	static_assert(utf16_text.is_char_boundary(2));
	static_assert(!utf16_text.is_char_boundary(3));
	static_assert(utf16_text.ceil_char_boundary(0) == 0);
	static_assert(utf16_text.ceil_char_boundary(3) == 4);
	static_assert(utf16_text.ceil_char_boundary(utf16_string_view::npos) == utf16_text.size());
	static_assert(utf16_text.floor_char_boundary(0) == 0);
	static_assert(utf16_text.floor_char_boundary(3) == 2);
	static_assert(utf16_text.floor_char_boundary(utf16_string_view::npos) == utf16_text.size());
	static_assert(utf16_text.char_at(0).has_value());
	static_assert(utf16_text.char_at(0).value() == u"A"_u16c);
	static_assert(utf16_text.char_at(2).has_value());
	static_assert(utf16_text.char_at(2).value() == u"😀"_u16c);
	static_assert(!utf16_text.char_at(3).has_value());
	static_assert(!utf16_text.char_at(utf16_text.size()).has_value());
	static_assert(utf16_text.char_at_unchecked(1) == u"é"_u16c);
	static_assert(utf16_text.front().has_value());
	static_assert(utf16_text.front().value() == u"A"_u16c);
	static_assert(utf16_text.front_unchecked() == u"A"_u16c);
	static_assert(utf16_text.back().has_value());
	static_assert(utf16_text.back().value() == u"😀"_u16c);
	static_assert(utf16_text.back_unchecked() == u"😀"_u16c);
	static_assert(!utf16_string_view{}.front().has_value());
	static_assert(!utf16_string_view{}.back().has_value());
	static_assert(utf16_text.find(static_cast<char16_t>(u'A')) == 0);
	static_assert(utf16_text.find(static_cast<char16_t>(0xDE00u), 3) == 3);
	static_assert(utf16_text.find(static_cast<char16_t>(u'A'), utf16_string_view::npos) == utf16_string_view::npos);
	static_assert(utf16_text.find_first_of(static_cast<char16_t>(u'A')) == 0);
	static_assert(utf16_text.find_first_of(u"😀A"_utf16_sv) == 0);
	static_assert(utf16_text.find_first_of(u"😀A"_utf16_sv, 1) == 2);
	static_assert(utf16_text.find_first_of(u""_utf16_sv) == utf16_string_view::npos);
	static_assert(utf16_text.find(u"é😀"_utf16_sv) == 1);
	static_assert(utf16_text.find(u"é😀"_utf16_sv, 2) == utf16_string_view::npos);
	static_assert(utf16_text.find(u"😀"_utf16_sv, 3) == utf16_string_view::npos);
	static_assert(utf16_text.find(u"é"_u16c, 2) == utf16_string_view::npos);
	static_assert(utf16_text.find(u"😀"_u16c, 3) == utf16_string_view::npos);
	static_assert(utf16_text.find(u"é"_u16c) == 1);
	static_assert(utf16_text.find(u"😀"_u16c) == 2);
	static_assert(utf16_text.find(u"Z"_u16c) == utf16_string_view::npos);
	static_assert(utf16_text.find_first_not_of(static_cast<char16_t>(u'A')) == 1);
	static_assert(utf16_text.find_first_not_of(u"A"_u16c) == 1);
	static_assert(utf16_text.find_first_not_of(u"Aé"_utf16_sv) == 2);
	static_assert(utf16_text.find_first_not_of(u""_utf16_sv, 2) == 2);
	static_assert(utf16_text.rfind(static_cast<char16_t>(u'A')) == 0);
	static_assert(utf16_text.rfind(static_cast<char16_t>(0xDE00u), 3) == 3);
	static_assert(utf16_text.find_last_of(static_cast<char16_t>(u'A')) == 0);
	static_assert(utf16_text.find_last_of(u"😀A"_utf16_sv) == 2);
	static_assert(utf16_text.find_last_of(u"😀A"_utf16_sv, 1) == 0);
	static_assert(utf16_text.find_last_of(u""_utf16_sv) == utf16_string_view::npos);
	static_assert(utf16_text.rfind(u"é😀"_utf16_sv) == 1);
	static_assert(utf16_text.rfind(u"é😀"_utf16_sv, 2) == 1);
	static_assert(utf16_text.rfind(u"😀"_utf16_sv, 1) == utf16_string_view::npos);
	static_assert(utf16_text.rfind(u"é"_u16c, 2) == 1);
	static_assert(utf16_text.rfind(u"😀"_u16c, 1) == utf16_string_view::npos);
	static_assert(utf16_text.rfind(u"😀"_u16c) == 2);
	static_assert(utf16_text.rfind(u"Z"_u16c) == utf16_string_view::npos);
	static_assert(utf16_text.find_last_not_of(static_cast<char16_t>(u'A')) == 3);
	static_assert(utf16_text.find_last_not_of(u"😀"_u16c) == 1);
	static_assert(utf16_text.find_last_not_of(u"Aé"_utf16_sv) == 2);
	static_assert(utf16_text.find_last_not_of(u""_utf16_sv) == 2);
	static_assert([] {
		constexpr auto text = u"e\u0301🇷🇴!"_utf16_sv;
		return text.find_grapheme(u"e\u0301"_grapheme_utf16) == 0
			&& text.find_grapheme(u"🇷🇴"_grapheme_utf16, 1) == 2
			&& text.find_grapheme(u"\u0301"_u16c) == utf16_string_view::npos
			&& text.contains_grapheme(u"🇷🇴"_grapheme_utf16)
			&& !text.contains_grapheme(u"\u0301"_u16c)
			&& text.rfind_grapheme(u"!"_grapheme_utf16) == 6
			&& text.rfind_grapheme(u"🇷🇴"_grapheme_utf16, 5) == 2;
	}());
	static_assert(utf16_text.substr(1).has_value());
	static_assert(utf16_text.substr(1).value() == u"é😀"_utf16_sv);
	static_assert(utf16_text.substr(2, 2).value() == u"😀"_utf16_sv);
	static_assert(!utf16_text.substr(3, 1).has_value());
	static_assert(utf16_text.starts_with(static_cast<char16_t>(u'A')));
	static_assert(utf16_text.starts_with(u"A"_u16c));
	static_assert(utf16_text.starts_with(u"A"_utf16_sv));
	static_assert(!utf16_text.starts_with(u"é"_u16c));
	static_assert(!utf16_text.ends_with(static_cast<char16_t>(u'A')));
	static_assert(utf16_text.ends_with(u"😀"_u16c));
	static_assert(utf16_text.ends_with(u"😀"_utf16_sv));
	static_assert(!utf16_text.ends_with(u"é"_u16c));
	static_assert(utf16_string_view::from_code_units(u"Aé😀"_utf16_sv.as_view()).has_value());
	static_assert(utf16_text == u"Aé😀"_utf16_sv);
	static_assert(std::same_as<decltype(utf16_text.to_utf16_owned()), utf16_string>);
	static_assert(utf16_text < u"Z"_utf16_sv);
	static_assert([] {
		constexpr auto text = u"e\u0301X"_utf16_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u"e\u0301"_grapheme_utf16,
			u"X"_grapheme_utf16
		});
	}());
	static_assert([] {
		constexpr auto text = u"\r\nX"_utf16_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u"\r\n"_grapheme_utf16,
			u"X"_grapheme_utf16
		});
	}());
	static_assert([] {
		constexpr auto text = u"🇷🇴!"_utf16_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u"🇷🇴"_grapheme_utf16,
			u"!"_grapheme_utf16
		});
	}());
	static_assert([] {
		constexpr auto text = u"👩‍💻!"_utf16_sv;
		return std::ranges::equal(text.graphemes(), std::array{
			u"👩‍💻"_grapheme_utf16,
			u"!"_grapheme_utf16
		});
	}());
	static_assert([] {
		constexpr auto text = u"e\u0301🇷🇴!"_utf16_sv;
		return std::ranges::equal(text.reversed_graphemes(), std::array{
			u"!"_grapheme_utf16,
			u"🇷🇴"_grapheme_utf16,
			u"e\u0301"_grapheme_utf16
		});
	}());
	static_assert(u"e\u0301"_grapheme_utf16 == u"e\u0301"_utf16_sv);
	static_assert(u"Aé😀"_utf16_sv.to_utf8() == u8"Aé😀"_utf8_sv);
	static_assert([] {
		constexpr auto text = u"Aé😀"_utf16_sv;
		auto it = text.char_indices().begin();
		if (it == text.char_indices().end()) return false;
		const auto [i0, c0] = *it++;
		if (i0 != 0 || c0 != u"A"_u16c) return false;
		const auto [i1, c1] = *it++;
		if (i1 != 1 || c1 != u"é"_u16c) return false;
		const auto [i2, c2] = *it++;
		return i2 == 2 && c2 == u"😀"_u16c && it == text.char_indices().end();
	}());
	static_assert([] {
		constexpr auto text = u"e\u0301🇷🇴!"_utf16_sv;
		auto it = text.grapheme_indices().begin();
		if (it == text.grapheme_indices().end()) return false;
		const auto [i0, g0] = *it++;
		if (i0 != 0 || g0 != u"e\u0301"_grapheme_utf16) return false;
		const auto [i1, g1] = *it++;
		if (i1 != 2 || g1 != u"🇷🇴"_grapheme_utf16) return false;
		const auto [i2, g2] = *it++;
		return i2 == 6 && g2 == u"!"_grapheme_utf16 && it == text.grapheme_indices().end();
	}());
	static_assert([] {
		constexpr auto text = u"abra--cadabra--"_utf16_sv;
		auto parts = text.split(u"--"_utf16_sv);
		auto it = parts.begin();
		if (it == parts.end() || *it != u"abra"_utf16_sv) return false;
		++it;
		if (it == parts.end() || *it != u"cadabra"_utf16_sv) return false;
		++it;
		if (it == parts.end() || *it != u""_utf16_sv) return false;
		auto rit = parts.end();
		--rit;
		if (*rit != u""_utf16_sv) return false;
		--rit;
		if (*rit != u"cadabra"_utf16_sv) return false;
		--rit;
		return *rit == u"abra"_utf16_sv;
	}());
	static_assert([] {
		constexpr auto text = u"--abra--cadabra--"_utf16_sv;
		return std::ranges::equal(text.rsplit(u"--"_utf16_sv), std::array{
			u""_utf16_sv,
			u"cadabra"_utf16_sv,
			u"abra"_utf16_sv,
			u""_utf16_sv
		});
	}());
	static_assert([] {
		constexpr auto text = u"--abra--cadabra--"_utf16_sv;
		return std::ranges::equal(text.split_terminator(u"--"_utf16_sv), std::array{
			u""_utf16_sv,
			u"abra"_utf16_sv,
			u"cadabra"_utf16_sv
		});
	}());
	static_assert([] {
		constexpr auto text = u"--abra--cadabra--"_utf16_sv;
		return std::ranges::equal(text.rsplit_terminator(u"--"_utf16_sv), std::array{
			u"cadabra"_utf16_sv,
			u"abra"_utf16_sv,
			u""_utf16_sv
		});
	}());
	static_assert([] {
		constexpr auto text = u"abra--cadabra--!"_utf16_sv;
		return std::ranges::equal(text.splitn(0, u"--"_utf16_sv), std::array<utf16_string_view, 0>{})
			&& std::ranges::equal(text.splitn(1, u"--"_utf16_sv), std::array{
				u"abra--cadabra--!"_utf16_sv
			})
			&& std::ranges::equal(text.splitn(2, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv,
				u"cadabra--!"_utf16_sv
			})
			&& std::ranges::equal(text.splitn(4, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv,
				u"cadabra"_utf16_sv,
				u"!"_utf16_sv
			});
	}());
	static_assert([] {
		constexpr auto text = u"abra--cadabra--!"_utf16_sv;
		return std::ranges::equal(text.rsplitn(0, u"--"_utf16_sv), std::array<utf16_string_view, 0>{})
			&& std::ranges::equal(text.rsplitn(1, u"--"_utf16_sv), std::array{
				u"abra--cadabra--!"_utf16_sv
			})
			&& std::ranges::equal(text.rsplitn(2, u"--"_utf16_sv), std::array{
				u"!"_utf16_sv,
				u"abra--cadabra"_utf16_sv
			})
			&& std::ranges::equal(text.rsplitn(4, u"--"_utf16_sv), std::array{
				u"!"_utf16_sv,
				u"cadabra"_utf16_sv,
				u"abra"_utf16_sv
			});
	}());
	static_assert([] {
		constexpr auto text = u"abra--cadabra--!"_utf16_sv;
		const auto first = text.split_once(u"--"_utf16_sv);
		const auto last = text.rsplit_once(u"--"_utf16_sv);
		return first.has_value()
			&& first->first == u"abra"_utf16_sv
			&& first->second == u"cadabra--!"_utf16_sv
			&& last.has_value()
			&& last->first == u"abra--cadabra"_utf16_sv
			&& last->second == u"!"_utf16_sv
			&& !text.split_once(u""_utf16_sv).has_value()
			&& !text.rsplit_once(u""_utf16_sv).has_value()
			&& !u"abra"_utf16_sv.split_once(u"--"_utf16_sv).has_value();
	}());
	static_assert([] {
		constexpr auto text = u"<<<\u00E9A>>>"_utf16_sv;
		const auto stripped_prefix = text.strip_prefix(u"<<<"_utf16_sv);
		const auto stripped_suffix = text.strip_suffix(u">>>"_utf16_sv);
		const auto stripped_circ = text.strip_circumfix(u"<<<"_utf16_sv, u">>>"_utf16_sv);
		const auto stripped_chars = u"[\u00E9]"_utf16_sv.strip_circumfix(u"["_u16c, u"]"_u16c);
		return stripped_prefix.has_value()
			&& stripped_prefix.value() == u"\u00E9A>>>"_utf16_sv
			&& stripped_suffix.has_value()
			&& stripped_suffix.value() == u"<<<\u00E9A"_utf16_sv
			&& stripped_circ.has_value()
			&& stripped_circ.value() == u"\u00E9A"_utf16_sv
			&& stripped_chars.has_value()
			&& stripped_chars.value() == u"\u00E9"_utf16_sv
			&& !text.strip_prefix(u">>>"_utf16_sv).has_value()
			&& !text.strip_circumfix(u"<<<"_utf16_sv, u"]"_utf16_sv).has_value()
			&& text.trim_prefix(u">>>"_utf16_sv) == text
			&& text.trim_prefix(u"<<<"_utf16_sv) == u"\u00E9A>>>"_utf16_sv
			&& text.trim_suffix(u">>>"_utf16_sv) == u"<<<\u00E9A"_utf16_sv
			&& u"\u00E9A\u00E9"_utf16_sv.trim_prefix(u"\u00E9"_u16c) == u"A\u00E9"_utf16_sv
			&& u"\u00E9A\u00E9"_utf16_sv.trim_suffix(u"\u00E9"_u16c) == u"\u00E9A"_utf16_sv;
	}());
	static_assert([] {
		constexpr auto repeated = u"----abra----"_utf16_sv;
		constexpr auto accented = u"\u00E9\u00E9A\u00E9"_utf16_sv;
		return repeated.trim_start_matches(u"--"_utf16_sv) == u"abra----"_utf16_sv
			&& repeated.trim_end_matches(u"--"_utf16_sv) == u"----abra"_utf16_sv
			&& repeated.trim_matches(u"--"_utf16_sv) == u"abra"_utf16_sv
			&& repeated.trim_matches(u""_utf16_sv) == repeated
			&& u"***abra***"_utf16_sv.trim_matches(u"*"_u16c) == u"abra"_utf16_sv
			&& accented.trim_start_matches(u"\u00E9"_u16c) == u"A\u00E9"_utf16_sv
			&& accented.trim_end_matches(u"\u00E9"_u16c) == u"\u00E9\u00E9A"_utf16_sv
			&& accented.trim_matches(u"\u00E9"_u16c) == u"A"_utf16_sv;
	}());
	static_assert([] {
		constexpr auto unicode_trimmed = u"\u00A0\tA\u00A0 "_utf16_sv;
		constexpr auto unicode_split = u"\u00A0A\u2003B C"_utf16_sv;
		return unicode_trimmed.trim() == u"A"_utf16_sv
			&& unicode_trimmed.trim_start() == u"A\u00A0 "_utf16_sv
			&& unicode_trimmed.trim_end() == u"\u00A0\tA"_utf16_sv
			&& unicode_trimmed.trim_ascii() == u"\u00A0\tA\u00A0"_utf16_sv
			&& unicode_trimmed.trim_ascii_start() == unicode_trimmed
			&& unicode_trimmed.trim_ascii_end() == u"\u00A0\tA\u00A0"_utf16_sv
			&& std::ranges::equal(u""_utf16_sv.split_whitespace(), std::array<utf16_string_view, 0>{})
			&& std::ranges::equal(u" \t\r\n"_utf16_sv.split_ascii_whitespace(), std::array<utf16_string_view, 0>{})
			&& std::ranges::equal(u" \tA  B\n"_utf16_sv.split_whitespace(), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			})
			&& std::ranges::equal(u" \tA  B\n"_utf16_sv.split_ascii_whitespace(), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			})
			&& std::ranges::equal(unicode_split.split_whitespace(), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv,
				u"C"_utf16_sv
			})
			&& std::ranges::equal(unicode_split.split_ascii_whitespace(), std::array{
				u"\u00A0A\u2003B"_utf16_sv,
				u"C"_utf16_sv
			});
	}());
	static_assert([] {
		constexpr auto empty = u""_utf16_sv;
		constexpr auto exact = u"--"_utf16_sv;
		constexpr auto repeated = u"a----b"_utf16_sv;
		constexpr auto missing = u"abra"_utf16_sv;
		constexpr auto unicode = u"A\u00E9B\u00E9"_utf16_sv;
		const auto exact_first = exact.split_once(u"--"_utf16_sv);
		const auto exact_last = exact.rsplit_once(u"--"_utf16_sv);
		const auto unicode_first = unicode.split_once(u"\u00E9"_u16c);
		const auto unicode_last = unicode.rsplit_once(u"\u00E9"_u16c);
		return std::ranges::equal(empty.split(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			})
			&& std::ranges::equal(empty.rsplit(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			})
			&& std::ranges::equal(empty.splitn(2, u"--"_utf16_sv), std::array{
				u""_utf16_sv
			})
			&& std::ranges::equal(empty.rsplitn(2, u"--"_utf16_sv), std::array{
				u""_utf16_sv
			})
			&& !empty.split_once(u"--"_utf16_sv).has_value()
			&& !empty.rsplit_once(u"--"_utf16_sv).has_value()
			&& std::ranges::equal(exact.split(u"--"_utf16_sv), std::array{
				u""_utf16_sv,
				u""_utf16_sv
			})
			&& std::ranges::equal(exact.split_terminator(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			})
			&& exact_first.has_value()
			&& exact_first->first == u""_utf16_sv
			&& exact_first->second == u""_utf16_sv
			&& exact_last.has_value()
			&& exact_last->first == u""_utf16_sv
			&& exact_last->second == u""_utf16_sv
			&& std::ranges::equal(repeated.split(u"--"_utf16_sv), std::array{
				u"a"_utf16_sv,
				u""_utf16_sv,
				u"b"_utf16_sv
			})
			&& std::ranges::equal(missing.split(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(missing.rsplit(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(std::views::reverse(missing.split(u"--"_utf16_sv)), missing.rsplit(u"--"_utf16_sv))
			&& std::ranges::equal(missing.split(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(missing.rsplit(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(missing.split_terminator(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(missing.rsplit_terminator(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(missing.splitn(2, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(missing.rsplitn(2, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(unicode.split(u"\u00E9"_u16c), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv,
				u""_utf16_sv
			})
			&& unicode_first.has_value()
			&& unicode_first->first == u"A"_utf16_sv
			&& unicode_first->second == u"B\u00E9"_utf16_sv
			&& unicode_last.has_value()
			&& unicode_last->first == u"A\u00E9B"_utf16_sv
			&& unicode_last->second == u""_utf16_sv;
	}());
	static_assert([] {
		constexpr auto text = u"A\u00E9\U0001F600"_utf16_sv;
		const auto parts = text.split_once_at(1);
		if (!parts.has_value()) return false;
		return parts->first == u"A"_utf16_sv && parts->second == u"\u00E9\U0001F600"_utf16_sv;
	}());
	static_assert([] {
		constexpr auto text = u"A\u00E9\U0001F600"_utf16_sv;
		return !text.split_once_at(3).has_value();
	}());
	static_assert([] {
		constexpr auto text = u"A\u00E9\U0001F600"_utf16_sv;
		const auto parts = text.split_once_at_unchecked(1);
		return parts.first == u"A"_utf16_sv && parts.second == u"\u00E9\U0001F600"_utf16_sv;
	}());
	static_assert([] {
		constexpr auto text = u"e\u0301🇷🇴!"_utf16_sv;
		return text.grapheme_count() == 3
			&& text.is_grapheme_boundary(0)
			&& !text.is_grapheme_boundary(1)
			&& text.is_grapheme_boundary(2)
			&& !text.is_grapheme_boundary(5)
			&& text.is_grapheme_boundary(6)
			&& text.ceil_grapheme_boundary(1) == 2
			&& text.floor_grapheme_boundary(1) == 0
			&& text.ceil_grapheme_boundary(5) == 6
			&& text.floor_grapheme_boundary(5) == 2
			&& text.grapheme_at(0).has_value()
			&& text.grapheme_at(0).value() == u"e\u0301"_grapheme_utf16
			&& text.grapheme_at(2).has_value()
			&& text.grapheme_at(2).value() == u"🇷🇴"_grapheme_utf16
			&& !text.grapheme_at(1).has_value()
			&& text.grapheme_substr(2, 4).has_value()
			&& text.grapheme_substr(2, 4).value() == u"🇷🇴"_utf16_sv
			&& text.grapheme_substr(2).has_value()
			&& text.grapheme_substr(2).value() == u"🇷🇴!"_utf16_sv
			&& !text.grapheme_substr(1, 2).has_value();
	}());

	// utf8_char scalar stepping and UTF-16 encoding edge cases.
	static_assert([] {
		utf8_char ch = utf8_char::from_scalar_unchecked(0x7Fu);
		const utf8_char old = ch++;
		return old.as_scalar() == 0x7Fu && ch.as_scalar() == 0x80u;
	}());
	static_assert([] {
		utf8_char ch = utf8_char::from_scalar_unchecked(0xD7FFu);
		++ch;
		return ch.as_scalar() == 0xE000u;
	}());
	static_assert([] {
		return !utf8_char::from_scalar(0xD800u).has_value();
	}());
	static_assert([] {
		utf8_char ch = utf8_char::from_scalar_unchecked(0x10FFFFu);
		++ch;
		return ch.as_scalar() == 0u;
	}());
	static_assert([] {
		return !utf8_char::from_scalar(0x110000u).has_value();
	}());
	static_assert([] {
		utf8_char ch = utf8_char::from_scalar_unchecked(0x80u);
		const utf8_char old = ch--;
		return old.as_scalar() == 0x80u && ch.as_scalar() == 0x7Fu;
	}());
	static_assert([] {
		utf8_char ch = utf8_char::from_scalar_unchecked(0xE000u);
		--ch;
		return ch.as_scalar() == 0xD7FFu;
	}());
	static_assert([] {
		return !utf8_char::from_scalar(0xD800u).has_value();
	}());
	static_assert([] {
		utf8_char ch = utf8_char::from_scalar_unchecked(0u);
		--ch;
		return ch.as_scalar() == 0x10FFFFu;
	}());
	static_assert([] {
		return !utf8_char::from_scalar(0x110000u).has_value();
	}());
	static_assert(utf8_char::from_scalar(0x20ACu).has_value());
	static_assert([] {
		std::array<char16_t, 2> buffer{};
		const auto n = "€"_u8c.encode_utf16<char16_t>(buffer.begin());
		return n == 1 && buffer[0] == static_cast<char16_t>(0x20ACu);
	}());
	static_assert([] {
		std::array<char16_t, 2> buffer{};
		const auto n = "😀"_u8c.encode_utf16<char16_t>(buffer.begin());
		return n == 2
			&& buffer[0] == static_cast<char16_t>(0xD83Du)
			&& buffer[1] == static_cast<char16_t>(0xDE00u);
	}());

	// utf16_char parity with utf8_char.
	static_assert(utf16_char::from_scalar(0x20ACu).has_value());
	static_assert(!utf16_char::from_scalar(0x110000u).has_value());
	static_assert(utf16_char::from_utf16_code_units(u"€", 1).has_value());
	static_assert(!utf16_char::from_utf16_code_units(u"\xD800", 1).has_value());
	static_assert(utf16_char::from_scalar_unchecked(0x20ACu).as_scalar() == 0x20ACu);
	static_assert(utf16_char::from_scalar_unchecked(0x1F600u).code_unit_count() == 2);
	static_assert(u"€"_u16c.as_scalar() == 0x20ACu);
	static_assert(u"😀"_u16c.code_unit_count() == 2);
	static_assert([] {
		utf16_char ch = utf16_char::from_scalar_unchecked(0x7Fu);
		const auto old = ch++;
		return old.as_scalar() == 0x7Fu && ch.as_scalar() == 0x80u;
	}());
	static_assert([] {
		utf16_char ch = utf16_char::from_scalar_unchecked(0xE000u);
		--ch;
		return ch.as_scalar() == 0xD7FFu;
	}());
	static_assert(u"A"_u16c.is_ascii());
	static_assert(u"A"_u16c.is_ascii_alphabetic());
	static_assert(u"7"_u16c.is_ascii_digit());
	static_assert(!u"Ω"_u16c.is_ascii());
	static_assert(u"Ω"_u16c.is_alphabetic());
	static_assert(u"Ω"_u16c.is_uppercase());
	static_assert(u"ω"_u16c.is_lowercase());
	static_assert(u" "_u16c.is_ascii_whitespace());
	static_assert(u"F"_u16c.is_ascii_hexdigit());
	static_assert(u"!"_u16c.is_ascii_punctuation());
	static_assert(details::non_narrowing_convertible<char16_t, char16_t>);
	static_assert(details::non_narrowing_convertible<char16_t, std::uint32_t>);
	static_assert(!details::non_narrowing_convertible<char16_t, char8_t>);
	static_assert(u"A"_u16c.ascii_lowercase() == u"a"_u16c);
	static_assert(u"z"_u16c.ascii_uppercase() == u"Z"_u16c);
	static_assert(u"A"_u16c.eq_ignore_ascii_case(u"a"_u16c));
	static_assert([] {
		utf16_char lhs = u"A"_u16c;
		utf16_char rhs = u"z"_u16c;
		lhs.swap(rhs);
		return lhs == u"z"_u16c && rhs == u"A"_u16c;
	}());
	static_assert([] {
		utf16_char utf16 = "€"_u8c;
		utf8_char utf8 = utf16;
		return utf16 == u"€"_u16c && utf8 == "€"_u8c;
	}());
	static_assert([] {
		std::array<char, 4> buffer{};
		const auto n = u"€"_u16c.encode_utf8<char>(buffer.begin());
		return n == 3
			&& static_cast<unsigned char>(buffer[0]) == 0xE2u
			&& static_cast<unsigned char>(buffer[1]) == 0x82u
				&& static_cast<unsigned char>(buffer[2]) == 0xACu;
	}());

	// Runtime mirrors for compile-time API coverage.
	assert((std::same_as<
		pmr::utf8_string,
		basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>>));
	assert((std::same_as<
		pmr::utf16_string,
		basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>>));
	assert((unicode_character<utf8_char>));
	assert((unicode_character<const utf8_char&>));
	assert((unicode_character<utf16_char>));
	assert((unicode_character<utf16_char&&>));
	assert((!unicode_character<char8_t>));
	assert((!unicode_character<char16_t>));

	assert((std::ranges::view<views::utf8_view>));
	assert((std::ranges::range<views::utf8_view>));
	assert((std::ranges::view<views::reversed_utf8_view>));
	assert((std::ranges::range<views::reversed_utf8_view>));
	assert((std::ranges::view<views::utf16_view>));
	assert((std::ranges::range<views::utf16_view>));
	assert((std::ranges::view<views::reversed_utf16_view>));
	assert((std::ranges::range<views::reversed_utf16_view>));
	assert((std::ranges::view<views::grapheme_cluster_view<char8_t>>));
	assert((std::ranges::range<views::grapheme_cluster_view<char8_t>>));
	assert((std::ranges::view<views::grapheme_cluster_view<char16_t>>));
	assert((std::ranges::range<views::grapheme_cluster_view<char16_t>>));
	assert((std::ranges::view<views::reversed_grapheme_cluster_view<char8_t>>));
	assert((std::ranges::range<views::reversed_grapheme_cluster_view<char8_t>>));
	assert((std::ranges::view<views::reversed_grapheme_cluster_view<char16_t>>));
	assert((std::ranges::range<views::reversed_grapheme_cluster_view<char16_t>>));
	assert((std::ranges::view<views::lossy_utf8_view<char>>));
	assert((std::ranges::range<views::lossy_utf8_view<char>>));
	assert((std::ranges::view<views::lossy_utf8_view<char8_t>>));
	assert((std::ranges::range<views::lossy_utf8_view<char8_t>>));
	assert((std::ranges::view<views::lossy_utf16_view<char16_t>>));
	assert((std::ranges::range<views::lossy_utf16_view<char16_t>>));
	assert((std::ranges::view<views::lossy_utf16_view<wchar_t>>));
	assert((std::ranges::range<views::lossy_utf16_view<wchar_t>>));

	{
		[[maybe_unused]] const auto runtime_latin1_ch = utf8_char::from_scalar_unchecked(0x00E9u);
		const std::u8string runtime_utf8_storage = u8"A\u00E9\u20AC";
		const auto runtime_utf8_text = unwrap_utf8_view(runtime_utf8_storage);
		const std::u16string runtime_utf16_storage = u"A\u00E9\U0001F600";
		const auto runtime_utf16_text = unwrap_utf16_view(runtime_utf16_storage);

		assert((std::ranges::view<decltype(runtime_utf8_text.chars())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.chars())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.reversed_chars())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.reversed_chars())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.char_indices())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.char_indices())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.graphemes())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.graphemes())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.reversed_graphemes())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.reversed_graphemes())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.grapheme_indices())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.grapheme_indices())>));

		assert((std::ranges::view<decltype(runtime_utf16_text.chars())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.chars())>));
		assert((std::ranges::view<decltype(runtime_utf16_text.reversed_chars())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.reversed_chars())>));
		assert((std::ranges::view<decltype(runtime_utf16_text.char_indices())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.char_indices())>));
		assert((std::ranges::view<decltype(runtime_utf16_text.graphemes())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.graphemes())>));
		assert((std::ranges::view<decltype(runtime_utf16_text.reversed_graphemes())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.reversed_graphemes())>));
		assert((std::ranges::view<decltype(runtime_utf16_text.grapheme_indices())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.grapheme_indices())>));

		assert(u8"A"_u8c.ascii_lowercase() == u8"a"_u8c);
		assert(u8"z"_u8c.ascii_uppercase() == u8"Z"_u8c);
		assert(runtime_latin1_ch.ascii_lowercase() == runtime_latin1_ch);
		assert(u8"A"_u8c.eq_ignore_ascii_case(u8"a"_u8c));
		assert(!runtime_latin1_ch.eq_ignore_ascii_case(u8"e"_u8c));

		assert(u8"A"_u8c.is_ascii_alphabetic());
		assert(!runtime_latin1_ch.is_ascii_alphabetic());
		assert(u8"7"_u8c.is_ascii_digit());
		assert(u8"7"_u8c.is_ascii_alphanumeric());
		assert(u8"Q"_u8c.is_ascii_alphanumeric());
		assert(!u8"-"_u8c.is_ascii_alphanumeric());
		assert(u8" "_u8c.is_ascii_whitespace());
		assert(u8"\n"_u8c.is_ascii_whitespace());
		assert(!u8"A"_u8c.is_ascii_whitespace());

		assert(u8"\u03A9"_u8c.is_alphabetic());
		assert(u8"\u03A9"_u8c.is_alphanumeric());
		assert(u8"\u03A9"_u8c.is_uppercase());
		assert(!u8"\u03A9"_u8c.is_lowercase());
		assert(u8"\u03C9"_u8c.is_lowercase());
		assert(!u8"\u03C9"_u8c.is_uppercase());
		assert(u8"5"_u8c.is_digit());
		assert(u8"5"_u8c.is_numeric());
		assert(u8"\u2167"_u8c.is_numeric());
		assert(!u8"\u2167"_u8c.is_digit());
		assert(u8"\u2003"_u8c.is_whitespace());
		assert(utf8_char::from_scalar_unchecked(0x0085u).is_control());
		assert(u8"F"_u8c.is_ascii_hexdigit());
		assert(u8"7"_u8c.is_ascii_octdigit());
		assert(u8"!"_u8c.is_ascii_punctuation());
		assert(u8"A"_u8c.is_ascii_graphic());
		assert(!u8" "_u8c.is_ascii_graphic());
		assert(u8"\n"_u8c.is_ascii_control());

		assert(std::get<0>(unicode_version) == 17);
		assert(std::get<1>(unicode_version) == 0);
		assert(std::get<2>(unicode_version) == 0);

		{
			utf8_char lhs = u8"A"_u8c;
			utf8_char rhs = u8"z"_u8c;
			lhs.swap(rhs);
			assert(lhs == u8"z"_u8c);
			assert(rhs == u8"A"_u8c);
		}

		assert(runtime_utf8_text.size() == 6);
		assert(runtime_utf8_text == u8"A\u00E9\u20AC"_utf8_sv);
		assert(runtime_utf8_text.is_char_boundary(0));
		assert(runtime_utf8_text.is_char_boundary(1));
		assert(!runtime_utf8_text.is_char_boundary(2));
		assert(runtime_utf8_text.ceil_char_boundary(0) == 0);
		assert(runtime_utf8_text.ceil_char_boundary(2) == 3);
		assert(runtime_utf8_text.ceil_char_boundary(utf8_string_view::npos) == runtime_utf8_text.size());
		assert(runtime_utf8_text.floor_char_boundary(0) == 0);
		assert(runtime_utf8_text.floor_char_boundary(2) == 1);
		assert(runtime_utf8_text.floor_char_boundary(utf8_string_view::npos) == runtime_utf8_text.size());
		assert(runtime_utf8_text.char_at(0).has_value());
		assert(runtime_utf8_text.char_at(0).value() == u8"A"_u8c);
		assert(runtime_utf8_text.char_at(1).has_value());
		assert(runtime_utf8_text.char_at(1).value() == u8"\u00E9"_u8c);
		assert(!runtime_utf8_text.char_at(2).has_value());
		assert(!runtime_utf8_text.char_at(runtime_utf8_text.size()).has_value());
		assert(runtime_utf8_text.char_at_unchecked(3) == u8"\u20AC"_u8c);
		assert(runtime_utf8_text.front().has_value());
		assert(runtime_utf8_text.front().value() == u8"A"_u8c);
		assert(runtime_utf8_text.front_unchecked() == u8"A"_u8c);
		assert(runtime_utf8_text.back().has_value());
		assert(runtime_utf8_text.back().value() == u8"\u20AC"_u8c);
		assert(runtime_utf8_text.back_unchecked() == u8"\u20AC"_u8c);
		{
			[[maybe_unused]] const utf8_string_view empty_text{};
			assert(!empty_text.front().has_value());
			assert(!empty_text.back().has_value());
		}
		assert(runtime_utf8_text.find(static_cast<char8_t>('A')) == 0);
		assert(runtime_utf8_text.find(static_cast<char8_t>(0xA9), 2) == 2);
		assert(runtime_utf8_text.find(static_cast<char8_t>('A'), utf8_string_view::npos) == utf8_string_view::npos);
		assert(runtime_utf8_text.find(u8"\u00E9\u20AC"_utf8_sv) == 1);
		assert(runtime_utf8_text.find(u8"\u00E9\u20AC"_utf8_sv, 2) == utf8_string_view::npos);
		assert(runtime_utf8_text.find(u8"\u20AC"_utf8_sv, 2) == 3);
		assert(runtime_utf8_text.find(u8"\u20AC"_utf8_sv, utf8_string_view::npos) == utf8_string_view::npos);
		assert(runtime_utf8_text.find_first_of(static_cast<char8_t>('A')) == 0);
		assert(runtime_utf8_text.find_first_of(u8"\u20ACA"_utf8_sv) == 0);
		assert(runtime_utf8_text.find_first_of(u8"\u20ACA"_utf8_sv, 1) == 3);
		assert(runtime_utf8_text.find_first_of(u8""_utf8_sv) == utf8_string_view::npos);
		assert(runtime_utf8_text.find(u8"\u00E9"_u8c, 2) == utf8_string_view::npos);
		assert(runtime_utf8_text.find(u8"\u20AC"_u8c, 2) == 3);
		assert(runtime_utf8_text.find(u8"\u20AC"_u8c, utf8_string_view::npos) == utf8_string_view::npos);
		assert(runtime_utf8_text.find(u8"\u00E9"_u8c) == 1);
		assert(runtime_utf8_text.find(u8"\u20AC"_u8c) == 3);
		assert(runtime_utf8_text.find(u8"Z"_u8c) == utf8_string_view::npos);
		assert(runtime_utf8_text.find_first_not_of(static_cast<char8_t>('A')) == 1);
		assert(runtime_utf8_text.find_first_not_of(u8"A"_u8c) == 1);
		assert(runtime_utf8_text.find_first_not_of(u8"A\u00E9"_utf8_sv) == 3);
		assert(runtime_utf8_text.find_first_not_of(u8""_utf8_sv, 2) == 3);
		assert(runtime_utf8_text.rfind(static_cast<char8_t>('A')) == 0);
		assert(runtime_utf8_text.rfind(static_cast<char8_t>(0xA9), 2) == 2);
		assert(runtime_utf8_text.rfind(static_cast<char8_t>('A'), 0) == 0);
		assert(runtime_utf8_text.find_last_of(static_cast<char8_t>('A')) == 0);
		assert(runtime_utf8_text.find_last_of(u8"\u20ACA"_utf8_sv) == 3);
		assert(runtime_utf8_text.find_last_of(u8"\u20ACA"_utf8_sv, 1) == 0);
		assert(runtime_utf8_text.find_last_of(u8""_utf8_sv) == utf8_string_view::npos);
		assert(runtime_utf8_text.rfind(u8"\u00E9\u20AC"_utf8_sv) == 1);
		assert(runtime_utf8_text.rfind(u8"\u00E9\u20AC"_utf8_sv, 2) == 1);
		assert(runtime_utf8_text.rfind(u8"\u20AC"_utf8_sv, 2) == utf8_string_view::npos);
		assert(runtime_utf8_text.rfind(u8"\u20AC"_utf8_sv, utf8_string_view::npos) == 3);
		assert(runtime_utf8_text.rfind(u8"\u00E9"_u8c, 2) == 1);
		assert(runtime_utf8_text.rfind(u8"\u20AC"_u8c, 2) == utf8_string_view::npos);
		assert(runtime_utf8_text.rfind(u8"\u20AC"_u8c) == 3);
		assert(runtime_utf8_text.rfind(u8"Z"_u8c) == utf8_string_view::npos);
		assert(runtime_utf8_text.find_last_not_of(static_cast<char8_t>('A')) == 5);
		assert(runtime_utf8_text.find_last_not_of(u8"\u20AC"_u8c) == 1);
		assert(runtime_utf8_text.find_last_not_of(u8"A\u00E9"_utf8_sv) == 3);
		assert(runtime_utf8_text.find_last_not_of(u8""_utf8_sv) == 3);
		{
			const std::u8string grapheme_storage = u8"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto grapheme_text = unwrap_utf8_view(grapheme_storage);

			assert(grapheme_text.find_grapheme(u8"e\u0301"_grapheme_utf8) == 0);
			assert(grapheme_text.find_grapheme(u8"\U0001F1F7\U0001F1F4"_grapheme_utf8, 1) == 3);
			assert(grapheme_text.find_grapheme(u8"\u0301"_u8c) == utf8_string_view::npos);
			assert(grapheme_text.contains_grapheme(u8"\U0001F1F7\U0001F1F4"_grapheme_utf8));
			assert(!grapheme_text.contains_grapheme(u8"\u0301"_u8c));
			assert(grapheme_text.rfind_grapheme(u8"!"_grapheme_utf8) == 11);
			assert(grapheme_text.rfind_grapheme(u8"\U0001F1F7\U0001F1F4"_grapheme_utf8, 10) == 3);
		}
		assert(runtime_utf8_text.substr(1).has_value());
		assert(runtime_utf8_text.substr(1).value() == u8"\u00E9\u20AC"_utf8_sv);
		assert(runtime_utf8_text.substr(1, 2).value() == u8"\u00E9"_utf8_sv);
		assert(!runtime_utf8_text.substr(2, 1).has_value());
		assert(runtime_utf8_text.starts_with('A'));
		assert(runtime_utf8_text.starts_with(static_cast<char8_t>('A')));
		assert(runtime_utf8_text.starts_with(u8"A"_u8c));
		assert(runtime_utf8_text.starts_with(u8"A"_utf8_sv));
		assert(!runtime_utf8_text.starts_with(u8"\u00E9"_u8c));
		assert(!runtime_utf8_text.ends_with('A'));
		assert(!runtime_utf8_text.ends_with(static_cast<char8_t>('A')));
		assert(runtime_utf8_text.ends_with(u8"\u20AC"_u8c));
		assert(runtime_utf8_text.ends_with(u8"\u20AC"_utf8_sv));
		assert(!runtime_utf8_text.ends_with(u8"\u00E9"_u8c));
		assert(utf8_string_view::from_bytes(runtime_utf8_text.as_view()).has_value());
		assert(runtime_utf8_text == u8"A\u00E9\u20AC"_utf8_sv);
		assert(runtime_utf8_text < u8"Z"_utf8_sv);
		{
			const std::u8string text_storage = u8"e\u0301X";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u8"e\u0301"_grapheme_utf8,
				u8"X"_grapheme_utf8
			}));
		}
		{
			const std::u8string text_storage = u8"\r\nX";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u8"\r\n"_grapheme_utf8,
				u8"X"_grapheme_utf8
			}));
		}
		{
			const std::u8string text_storage = u8"\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u8"\U0001F1F7\U0001F1F4"_grapheme_utf8,
				u8"!"_grapheme_utf8
			}));
		}
		{
			const std::u8string text_storage = u8"\U0001F469\u200D\U0001F4BB!";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u8"\U0001F469\u200D\U0001F4BB"_grapheme_utf8,
				u8"!"_grapheme_utf8
			}));
		}
		{
			const std::u8string text_storage = u8"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(std::ranges::equal(text.reversed_graphemes(), std::array{
				u8"!"_grapheme_utf8,
				u8"\U0001F1F7\U0001F1F4"_grapheme_utf8,
				u8"e\u0301"_grapheme_utf8
			}));
		}
		assert(u8"e\u0301"_grapheme_utf8 == u8"e\u0301"_utf8_sv);
		{
			const std::u8string text_storage = u8"A\u00E9\U0001F600";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(text.to_utf16() == u"A\u00E9\U0001F600"_utf16_sv);
			assert(text.to_utf8_owned() == u8"A\u00E9\U0001F600"_utf8_sv);
			assert(u8"\u00E9"_u8c.to_utf8_owned() == u8"\u00E9"_utf8_sv);
		}
		{
			auto it = runtime_utf8_text.char_indices().begin();
			assert(it != runtime_utf8_text.char_indices().end());
			[[maybe_unused]] const auto [i0, c0] = *it++;
			assert(i0 == 0);
			assert(c0 == u8"A"_u8c);
			[[maybe_unused]] const auto [i1, c1] = *it++;
			assert(i1 == 1);
			assert(c1 == u8"\u00E9"_u8c);
			[[maybe_unused]] const auto [i2, c2] = *it++;
			assert(i2 == 3);
			assert(c2 == u8"\u20AC"_u8c);
			assert(it == runtime_utf8_text.char_indices().end());
		}
		{
			const std::u8string text_storage = u8"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			auto it = text.grapheme_indices().begin();
			assert(it != text.grapheme_indices().end());
			[[maybe_unused]] const auto [i0, g0] = *it++;
			assert(i0 == 0);
			assert(g0 == u8"e\u0301"_grapheme_utf8);
			[[maybe_unused]] const auto [i1, g1] = *it++;
			assert(i1 == 3);
			assert(g1 == u8"\U0001F1F7\U0001F1F4"_grapheme_utf8);
			[[maybe_unused]] const auto [i2, g2] = *it++;
			assert(i2 == 11);
			assert(g2 == u8"!"_grapheme_utf8);
			assert(it == text.grapheme_indices().end());
		}
		{
			const auto text = u8"abra--cadabra--"_utf8_sv;
			auto parts = text.split(u8"--"_utf8_sv);
			auto it = parts.begin();
			assert(it != parts.end());
			assert(*it == u8"abra"_utf8_sv);
			++it;
			assert(it != parts.end());
			assert(*it == u8"cadabra"_utf8_sv);
			++it;
			assert(it != parts.end());
			assert(*it == u8""_utf8_sv);
			auto rit = parts.end();
			--rit;
			assert(*rit == u8""_utf8_sv);
			--rit;
			assert(*rit == u8"cadabra"_utf8_sv);
			--rit;
			assert(*rit == u8"abra"_utf8_sv);
		}
		{
			[[maybe_unused]] const auto text = u8"--abra--cadabra--"_utf8_sv;
			assert(std::ranges::equal(text.rsplit(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv,
				u8"cadabra"_utf8_sv,
				u8"abra"_utf8_sv,
				u8""_utf8_sv
			}));
			assert(std::ranges::equal(text.split_terminator(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv,
				u8"abra"_utf8_sv,
				u8"cadabra"_utf8_sv
			}));
			assert(std::ranges::equal(text.rsplit_terminator(u8"--"_utf8_sv), std::array{
				u8"cadabra"_utf8_sv,
				u8"abra"_utf8_sv,
				u8""_utf8_sv
			}));
		}
		{
			const auto text = u8"abra--cadabra--!"_utf8_sv;
			assert(std::ranges::equal(text.splitn(0, u8"--"_utf8_sv), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(text.splitn(1, u8"--"_utf8_sv), std::array{
				u8"abra--cadabra--!"_utf8_sv
			}));
			assert(std::ranges::equal(text.splitn(2, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv,
				u8"cadabra--!"_utf8_sv
			}));
			assert(std::ranges::equal(text.splitn(4, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv,
				u8"cadabra"_utf8_sv,
				u8"!"_utf8_sv
			}));
			assert(std::ranges::equal(text.rsplitn(0, u8"--"_utf8_sv), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(text.rsplitn(1, u8"--"_utf8_sv), std::array{
				u8"abra--cadabra--!"_utf8_sv
			}));
			assert(std::ranges::equal(text.rsplitn(2, u8"--"_utf8_sv), std::array{
				u8"!"_utf8_sv,
				u8"abra--cadabra"_utf8_sv
			}));
			assert(std::ranges::equal(text.rsplitn(4, u8"--"_utf8_sv), std::array{
				u8"!"_utf8_sv,
				u8"cadabra"_utf8_sv,
				u8"abra"_utf8_sv
			}));
			[[maybe_unused]] const auto first = text.split_once(u8"--"_utf8_sv);
			assert(first.has_value());
			assert(first->first == u8"abra"_utf8_sv);
			assert(first->second == u8"cadabra--!"_utf8_sv);
			[[maybe_unused]] const auto last = text.rsplit_once(u8"--"_utf8_sv);
			assert(last.has_value());
			assert(last->first == u8"abra--cadabra"_utf8_sv);
			assert(last->second == u8"!"_utf8_sv);
			assert(!text.split_once(u8""_utf8_sv).has_value());
			assert(!text.rsplit_once(u8""_utf8_sv).has_value());
			assert(!u8"abra"_utf8_sv.split_once(u8"--"_utf8_sv).has_value());
			assert(std::ranges::equal(text.splitn(3, u8""_utf8_sv), std::array{
				u8"abra--cadabra--!"_utf8_sv
			}));
		}
		{
			const auto text = u8"<<<\u00E9A>>>"_utf8_sv;
			[[maybe_unused]] const auto stripped_prefix = text.strip_prefix(u8"<<<"_utf8_sv);
			assert(stripped_prefix.has_value());
			assert(stripped_prefix.value() == u8"\u00E9A>>>"_utf8_sv);
			[[maybe_unused]] const auto stripped_suffix = text.strip_suffix(u8">>>"_utf8_sv);
			assert(stripped_suffix.has_value());
			assert(stripped_suffix.value() == u8"<<<\u00E9A"_utf8_sv);
			[[maybe_unused]] const auto stripped_circ = text.strip_circumfix(u8"<<<"_utf8_sv, u8">>>"_utf8_sv);
			assert(stripped_circ.has_value());
			assert(stripped_circ.value() == u8"\u00E9A"_utf8_sv);
			[[maybe_unused]] const auto stripped_chars = u8"[\u00E9]"_utf8_sv.strip_circumfix(u8"["_u8c, u8"]"_u8c);
			assert(stripped_chars.has_value());
			assert(stripped_chars.value() == u8"\u00E9"_utf8_sv);
			assert(!text.strip_prefix(u8">>>"_utf8_sv).has_value());
			assert(!text.strip_circumfix(u8"<<<"_utf8_sv, u8"]"_utf8_sv).has_value());
			assert(text.trim_prefix(u8">>>"_utf8_sv) == text);
			assert(text.trim_prefix(u8"<<<"_utf8_sv) == u8"\u00E9A>>>"_utf8_sv);
			assert(text.trim_suffix(u8">>>"_utf8_sv) == u8"<<<\u00E9A"_utf8_sv);
			assert(u8"\u00E9A\u00E9"_utf8_sv.trim_prefix(u8"\u00E9"_u8c) == u8"A\u00E9"_utf8_sv);
			assert(u8"\u00E9A\u00E9"_utf8_sv.trim_suffix(u8"\u00E9"_u8c) == u8"\u00E9A"_utf8_sv);
		}
		{
			[[maybe_unused]] const auto repeated = u8"----abra----"_utf8_sv;
			[[maybe_unused]] const auto accented = u8"\u00E9\u00E9A\u00E9"_utf8_sv;
			assert(repeated.trim_start_matches(u8"--"_utf8_sv) == u8"abra----"_utf8_sv);
			assert(repeated.trim_end_matches(u8"--"_utf8_sv) == u8"----abra"_utf8_sv);
			assert(repeated.trim_matches(u8"--"_utf8_sv) == u8"abra"_utf8_sv);
			assert(repeated.trim_matches(u8""_utf8_sv) == repeated);
			assert(u8"***abra***"_utf8_sv.trim_matches(u8"*"_u8c) == u8"abra"_utf8_sv);
			assert(accented.trim_start_matches(u8"\u00E9"_u8c) == u8"A\u00E9"_utf8_sv);
			assert(accented.trim_end_matches(u8"\u00E9"_u8c) == u8"\u00E9\u00E9A"_utf8_sv);
			assert(accented.trim_matches(u8"\u00E9"_u8c) == u8"A"_utf8_sv);
		}
		{
			[[maybe_unused]] const auto unicode_trimmed = u8"\u00A0\tA\u00A0 "_utf8_sv;
			[[maybe_unused]] const auto unicode_split = u8"\u00A0A\u2003B C"_utf8_sv;
			assert(unicode_trimmed.trim() == u8"A"_utf8_sv);
			assert(unicode_trimmed.trim_start() == u8"A\u00A0 "_utf8_sv);
			assert(unicode_trimmed.trim_end() == u8"\u00A0\tA"_utf8_sv);
			assert(unicode_trimmed.trim_ascii() == u8"\u00A0\tA\u00A0"_utf8_sv);
			assert(unicode_trimmed.trim_ascii_start() == unicode_trimmed);
			assert(unicode_trimmed.trim_ascii_end() == u8"\u00A0\tA\u00A0"_utf8_sv);
			assert(std::ranges::equal(u8""_utf8_sv.split_whitespace(), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(u8" \t\r\n"_utf8_sv.split_ascii_whitespace(), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(u8" \tA  B\n"_utf8_sv.split_whitespace(), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			}));
			assert(std::ranges::equal(u8" \tA  B\n"_utf8_sv.split_ascii_whitespace(), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			}));
			assert(std::ranges::equal(unicode_split.split_whitespace(), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv,
				u8"C"_utf8_sv
			}));
			assert(std::ranges::equal(unicode_split.split_ascii_whitespace(), std::array{
				u8"\u00A0A\u2003B"_utf8_sv,
				u8"C"_utf8_sv
			}));
		}
		{
			[[maybe_unused]] const auto empty = u8""_utf8_sv;
			const auto exact = u8"--"_utf8_sv;
			[[maybe_unused]] const auto repeated = u8"a----b"_utf8_sv;
			[[maybe_unused]] const auto missing = u8"abra"_utf8_sv;
			const auto unicode = u8"A\u00E9B\u00E9"_utf8_sv;
			assert(std::ranges::equal(empty.split(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			}));
			assert(std::ranges::equal(empty.rsplit(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			}));
			assert(std::ranges::equal(empty.splitn(2, u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			}));
			assert(std::ranges::equal(empty.rsplitn(2, u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			}));
			assert(!empty.split_once(u8"--"_utf8_sv).has_value());
			assert(!empty.rsplit_once(u8"--"_utf8_sv).has_value());
			assert(std::ranges::equal(exact.split(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv,
				u8""_utf8_sv
			}));
			assert(std::ranges::equal(exact.split_terminator(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			}));
			[[maybe_unused]] const auto exact_first = exact.split_once(u8"--"_utf8_sv);
			assert(exact_first.has_value());
			assert(exact_first->first == u8""_utf8_sv);
			assert(exact_first->second == u8""_utf8_sv);
			[[maybe_unused]] const auto exact_last = exact.rsplit_once(u8"--"_utf8_sv);
			assert(exact_last.has_value());
			assert(exact_last->first == u8""_utf8_sv);
			assert(exact_last->second == u8""_utf8_sv);
			assert(std::ranges::equal(repeated.split(u8"--"_utf8_sv), std::array{
				u8"a"_utf8_sv,
				u8""_utf8_sv,
				u8"b"_utf8_sv
			}));
			assert(std::ranges::equal(missing.split(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(missing.rsplit(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(std::views::reverse(missing.split(u8"--"_utf8_sv)), missing.rsplit(u8"--"_utf8_sv)));
			assert(std::ranges::equal(missing.split(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(missing.rsplit(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(missing.split_terminator(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(missing.rsplit_terminator(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(missing.splitn(2, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(missing.rsplitn(2, u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(unicode.split(u8"\u00E9"_u8c), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv,
				u8""_utf8_sv
			}));
			[[maybe_unused]] const auto unicode_first = unicode.split_once(u8"\u00E9"_u8c);
			assert(unicode_first.has_value());
			assert(unicode_first->first == u8"A"_utf8_sv);
			assert(unicode_first->second == u8"B\u00E9"_utf8_sv);
			[[maybe_unused]] const auto unicode_last = unicode.rsplit_once(u8"\u00E9"_u8c);
			assert(unicode_last.has_value());
			assert(unicode_last->first == u8"A\u00E9B"_utf8_sv);
			assert(unicode_last->second == u8""_utf8_sv);
		}
		{
			const auto text = u8"A\u00E9\u20AC"_utf8_sv;
			[[maybe_unused]] const auto split = text.split_once_at(1);
			assert(split.has_value());
			assert(split->first == u8"A"_utf8_sv);
			assert(split->second == u8"\u00E9\u20AC"_utf8_sv);
			assert(!text.split_once_at(2).has_value());
			[[maybe_unused]] const auto unchecked = text.split_once_at_unchecked(1);
			assert(unchecked.first == u8"A"_utf8_sv);
			assert(unchecked.second == u8"\u00E9\u20AC"_utf8_sv);
		}
		{
			const std::u8string text_storage = u8"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(text.grapheme_count() == 3);
			assert(text.is_grapheme_boundary(0));
			assert(!text.is_grapheme_boundary(1));
			assert(text.is_grapheme_boundary(3));
			assert(!text.is_grapheme_boundary(7));
			assert(text.is_grapheme_boundary(11));
			assert(text.ceil_grapheme_boundary(1) == 3);
			assert(text.floor_grapheme_boundary(1) == 0);
			assert(text.ceil_grapheme_boundary(7) == 11);
			assert(text.floor_grapheme_boundary(7) == 3);
			assert(text.grapheme_at(0).has_value());
			assert(text.grapheme_at(0).value() == u8"e\u0301"_grapheme_utf8);
			assert(text.grapheme_at(3).has_value());
			assert(text.grapheme_at(3).value() == u8"\U0001F1F7\U0001F1F4"_grapheme_utf8);
			assert(!text.grapheme_at(1).has_value());
			assert(text.grapheme_substr(3, 8).has_value());
			assert(text.grapheme_substr(3, 8).value() == u8"\U0001F1F7\U0001F1F4"_utf8_sv);
			assert(text.grapheme_substr(3).has_value());
			assert(text.grapheme_substr(3).value() == u8"\U0001F1F7\U0001F1F4!"_utf8_sv);
			assert(!text.grapheme_substr(1, 2).has_value());
		}

		assert(runtime_utf16_text.size() == 4);
		assert(runtime_utf16_text == u"A\u00E9\U0001F600"_utf16_sv);
		assert(runtime_utf16_text.is_char_boundary(0));
		assert(runtime_utf16_text.is_char_boundary(1));
		assert(runtime_utf16_text.is_char_boundary(2));
		assert(!runtime_utf16_text.is_char_boundary(3));
		assert(runtime_utf16_text.ceil_char_boundary(0) == 0);
		assert(runtime_utf16_text.ceil_char_boundary(3) == 4);
		assert(runtime_utf16_text.ceil_char_boundary(utf16_string_view::npos) == runtime_utf16_text.size());
		assert(runtime_utf16_text.floor_char_boundary(0) == 0);
		assert(runtime_utf16_text.floor_char_boundary(3) == 2);
		assert(runtime_utf16_text.floor_char_boundary(utf16_string_view::npos) == runtime_utf16_text.size());
		assert(runtime_utf16_text.char_at(0).has_value());
		assert(runtime_utf16_text.char_at(0).value() == u"A"_u16c);
		assert(runtime_utf16_text.char_at(2).has_value());
		assert(runtime_utf16_text.char_at(2).value() == u"\U0001F600"_u16c);
		assert(!runtime_utf16_text.char_at(3).has_value());
		assert(!runtime_utf16_text.char_at(runtime_utf16_text.size()).has_value());
		assert(runtime_utf16_text.char_at_unchecked(1) == u"\u00E9"_u16c);
		assert(runtime_utf16_text.front().has_value());
		assert(runtime_utf16_text.front().value() == u"A"_u16c);
		assert(runtime_utf16_text.front_unchecked() == u"A"_u16c);
		assert(runtime_utf16_text.back().has_value());
		assert(runtime_utf16_text.back().value() == u"\U0001F600"_u16c);
		assert(runtime_utf16_text.back_unchecked() == u"\U0001F600"_u16c);
		{
			[[maybe_unused]] const utf16_string_view empty_text{};
			assert(!empty_text.front().has_value());
			assert(!empty_text.back().has_value());
		}
		assert(runtime_utf16_text.find(static_cast<char16_t>(u'A')) == 0);
		assert(runtime_utf16_text.find(static_cast<char16_t>(0xDE00u), 3) == 3);
		assert(runtime_utf16_text.find(static_cast<char16_t>(u'A'), utf16_string_view::npos) == utf16_string_view::npos);
		assert(runtime_utf16_text.find_first_of(static_cast<char16_t>(u'A')) == 0);
		assert(runtime_utf16_text.find_first_of(u"\U0001F600A"_utf16_sv) == 0);
		assert(runtime_utf16_text.find_first_of(u"\U0001F600A"_utf16_sv, 1) == 2);
		assert(runtime_utf16_text.find_first_of(u""_utf16_sv) == utf16_string_view::npos);
		assert(runtime_utf16_text.find(u"\u00E9\U0001F600"_utf16_sv) == 1);
		assert(runtime_utf16_text.find(u"\u00E9\U0001F600"_utf16_sv, 2) == utf16_string_view::npos);
		assert(runtime_utf16_text.find(u"\U0001F600"_utf16_sv, 3) == utf16_string_view::npos);
		assert(runtime_utf16_text.find(u"\u00E9"_u16c, 2) == utf16_string_view::npos);
		assert(runtime_utf16_text.find(u"\U0001F600"_u16c, 3) == utf16_string_view::npos);
		assert(runtime_utf16_text.find(u"\u00E9"_u16c) == 1);
		assert(runtime_utf16_text.find(u"\U0001F600"_u16c) == 2);
		assert(runtime_utf16_text.find(u"Z"_u16c) == utf16_string_view::npos);
		assert(runtime_utf16_text.find_first_not_of(static_cast<char16_t>(u'A')) == 1);
		assert(runtime_utf16_text.find_first_not_of(u"A"_u16c) == 1);
		assert(runtime_utf16_text.find_first_not_of(u"A\u00E9"_utf16_sv) == 2);
		assert(runtime_utf16_text.find_first_not_of(u""_utf16_sv, 2) == 2);
		assert(runtime_utf16_text.rfind(static_cast<char16_t>(u'A')) == 0);
		assert(runtime_utf16_text.rfind(static_cast<char16_t>(0xDE00u), 3) == 3);
		assert(runtime_utf16_text.find_last_of(static_cast<char16_t>(u'A')) == 0);
		assert(runtime_utf16_text.find_last_of(u"\U0001F600A"_utf16_sv) == 2);
		assert(runtime_utf16_text.find_last_of(u"\U0001F600A"_utf16_sv, 1) == 0);
		assert(runtime_utf16_text.find_last_of(u""_utf16_sv) == utf16_string_view::npos);
		assert(runtime_utf16_text.rfind(u"\u00E9\U0001F600"_utf16_sv) == 1);
		assert(runtime_utf16_text.rfind(u"\u00E9\U0001F600"_utf16_sv, 2) == 1);
		assert(runtime_utf16_text.rfind(u"\U0001F600"_utf16_sv, 1) == utf16_string_view::npos);
		assert(runtime_utf16_text.rfind(u"\u00E9"_u16c, 2) == 1);
		assert(runtime_utf16_text.rfind(u"\U0001F600"_u16c, 1) == utf16_string_view::npos);
		assert(runtime_utf16_text.rfind(u"\U0001F600"_u16c) == 2);
		assert(runtime_utf16_text.rfind(u"Z"_u16c) == utf16_string_view::npos);
		assert(runtime_utf16_text.find_last_not_of(static_cast<char16_t>(u'A')) == 3);
		assert(runtime_utf16_text.find_last_not_of(u"\U0001F600"_u16c) == 1);
		assert(runtime_utf16_text.find_last_not_of(u"A\u00E9"_utf16_sv) == 2);
		assert(runtime_utf16_text.find_last_not_of(u""_utf16_sv) == 2);
		{
			const std::u16string grapheme_storage = u"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto grapheme_text = unwrap_utf16_view(grapheme_storage);

			assert(grapheme_text.find_grapheme(u"e\u0301"_grapheme_utf16) == 0);
			assert(grapheme_text.find_grapheme(u"\U0001F1F7\U0001F1F4"_grapheme_utf16, 1) == 2);
			assert(grapheme_text.find_grapheme(u"\u0301"_u16c) == utf16_string_view::npos);
			assert(grapheme_text.contains_grapheme(u"\U0001F1F7\U0001F1F4"_grapheme_utf16));
			assert(!grapheme_text.contains_grapheme(u"\u0301"_u16c));
			assert(grapheme_text.rfind_grapheme(u"!"_grapheme_utf16) == 6);
			assert(grapheme_text.rfind_grapheme(u"\U0001F1F7\U0001F1F4"_grapheme_utf16, 5) == 2);
		}
		assert(runtime_utf16_text.substr(1).has_value());
		assert(runtime_utf16_text.substr(1).value() == u"\u00E9\U0001F600"_utf16_sv);
		assert(runtime_utf16_text.substr(2, 2).value() == u"\U0001F600"_utf16_sv);
		assert(!runtime_utf16_text.substr(3, 1).has_value());
		assert(runtime_utf16_text.starts_with(static_cast<char16_t>(u'A')));
		assert(runtime_utf16_text.starts_with(u"A"_u16c));
		assert(runtime_utf16_text.starts_with(u"A"_utf16_sv));
		assert(!runtime_utf16_text.starts_with(u"\u00E9"_u16c));
		assert(!runtime_utf16_text.ends_with(static_cast<char16_t>(u'A')));
		assert(runtime_utf16_text.ends_with(u"\U0001F600"_u16c));
		assert(runtime_utf16_text.ends_with(u"\U0001F600"_utf16_sv));
		assert(!runtime_utf16_text.ends_with(u"\u00E9"_u16c));
		assert(utf16_string_view::from_code_units(runtime_utf16_text.as_view()).has_value());
		assert(runtime_utf16_text == u"A\u00E9\U0001F600"_utf16_sv);
		assert(runtime_utf16_text < u"Z"_utf16_sv);
		{
			const std::u16string text_storage = u"e\u0301X";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u"e\u0301"_grapheme_utf16,
				u"X"_grapheme_utf16
			}));
		}
		{
			const std::u16string text_storage = u"\r\nX";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u"\r\n"_grapheme_utf16,
				u"X"_grapheme_utf16
			}));
		}
		{
			const std::u16string text_storage = u"\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u"\U0001F1F7\U0001F1F4"_grapheme_utf16,
				u"!"_grapheme_utf16
			}));
		}
		{
			const std::u16string text_storage = u"\U0001F469\u200D\U0001F4BB!";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				u"\U0001F469\u200D\U0001F4BB"_grapheme_utf16,
				u"!"_grapheme_utf16
			}));
		}
		{
			const std::u16string text_storage = u"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(std::ranges::equal(text.reversed_graphemes(), std::array{
				u"!"_grapheme_utf16,
				u"\U0001F1F7\U0001F1F4"_grapheme_utf16,
				u"e\u0301"_grapheme_utf16
			}));
		}
		assert(u"e\u0301"_grapheme_utf16 == u"e\u0301"_utf16_sv);
		{
			const std::u16string text_storage = u"A\u00E9\U0001F600";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(text.to_utf8() == u8"A\u00E9\U0001F600"_utf8_sv);
			assert(text.to_utf16_owned() == u"A\u00E9\U0001F600"_utf16_sv);
			assert(u"\U0001F600"_u16c.to_utf16_owned() == u"\U0001F600"_utf16_sv);
		}
		{
			auto it = runtime_utf16_text.char_indices().begin();
			assert(it != runtime_utf16_text.char_indices().end());
			[[maybe_unused]] const auto [i0, c0] = *it++;
			assert(i0 == 0);
			assert(c0 == u"A"_u16c);
			[[maybe_unused]] const auto [i1, c1] = *it++;
			assert(i1 == 1);
			assert(c1 == u"\u00E9"_u16c);
			[[maybe_unused]] const auto [i2, c2] = *it++;
			assert(i2 == 2);
			assert(c2 == u"\U0001F600"_u16c);
			assert(it == runtime_utf16_text.char_indices().end());
		}
		{
			const std::u16string text_storage = u"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			auto it = text.grapheme_indices().begin();
			assert(it != text.grapheme_indices().end());
			[[maybe_unused]] const auto [i0, g0] = *it++;
			assert(i0 == 0);
			assert(g0 == u"e\u0301"_grapheme_utf16);
			[[maybe_unused]] const auto [i1, g1] = *it++;
			assert(i1 == 2);
			assert(g1 == u"\U0001F1F7\U0001F1F4"_grapheme_utf16);
			[[maybe_unused]] const auto [i2, g2] = *it++;
			assert(i2 == 6);
			assert(g2 == u"!"_grapheme_utf16);
			assert(it == text.grapheme_indices().end());
		}
		{
			const auto text = u"abra--cadabra--"_utf16_sv;
			auto parts = text.split(u"--"_utf16_sv);
			auto it = parts.begin();
			assert(it != parts.end());
			assert(*it == u"abra"_utf16_sv);
			++it;
			assert(it != parts.end());
			assert(*it == u"cadabra"_utf16_sv);
			++it;
			assert(it != parts.end());
			assert(*it == u""_utf16_sv);
			auto rit = parts.end();
			--rit;
			assert(*rit == u""_utf16_sv);
			--rit;
			assert(*rit == u"cadabra"_utf16_sv);
			--rit;
			assert(*rit == u"abra"_utf16_sv);
		}
		{
			[[maybe_unused]] const auto text = u"--abra--cadabra--"_utf16_sv;
			assert(std::ranges::equal(text.rsplit(u"--"_utf16_sv), std::array{
				u""_utf16_sv,
				u"cadabra"_utf16_sv,
				u"abra"_utf16_sv,
				u""_utf16_sv
			}));
			assert(std::ranges::equal(text.split_terminator(u"--"_utf16_sv), std::array{
				u""_utf16_sv,
				u"abra"_utf16_sv,
				u"cadabra"_utf16_sv
			}));
			assert(std::ranges::equal(text.rsplit_terminator(u"--"_utf16_sv), std::array{
				u"cadabra"_utf16_sv,
				u"abra"_utf16_sv,
				u""_utf16_sv
			}));
		}
		{
			const auto text = u"abra--cadabra--!"_utf16_sv;
			assert(std::ranges::equal(text.splitn(0, u"--"_utf16_sv), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(text.splitn(1, u"--"_utf16_sv), std::array{
				u"abra--cadabra--!"_utf16_sv
			}));
			assert(std::ranges::equal(text.splitn(2, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv,
				u"cadabra--!"_utf16_sv
			}));
			assert(std::ranges::equal(text.splitn(4, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv,
				u"cadabra"_utf16_sv,
				u"!"_utf16_sv
			}));
			assert(std::ranges::equal(text.rsplitn(0, u"--"_utf16_sv), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(text.rsplitn(1, u"--"_utf16_sv), std::array{
				u"abra--cadabra--!"_utf16_sv
			}));
			assert(std::ranges::equal(text.rsplitn(2, u"--"_utf16_sv), std::array{
				u"!"_utf16_sv,
				u"abra--cadabra"_utf16_sv
			}));
			assert(std::ranges::equal(text.rsplitn(4, u"--"_utf16_sv), std::array{
				u"!"_utf16_sv,
				u"cadabra"_utf16_sv,
				u"abra"_utf16_sv
			}));
			[[maybe_unused]] const auto first = text.split_once(u"--"_utf16_sv);
			assert(first.has_value());
			assert(first->first == u"abra"_utf16_sv);
			assert(first->second == u"cadabra--!"_utf16_sv);
			[[maybe_unused]] const auto last = text.rsplit_once(u"--"_utf16_sv);
			assert(last.has_value());
			assert(last->first == u"abra--cadabra"_utf16_sv);
			assert(last->second == u"!"_utf16_sv);
			assert(!text.split_once(u""_utf16_sv).has_value());
			assert(!text.rsplit_once(u""_utf16_sv).has_value());
			assert(!u"abra"_utf16_sv.split_once(u"--"_utf16_sv).has_value());
			assert(std::ranges::equal(text.splitn(3, u""_utf16_sv), std::array{
				u"abra--cadabra--!"_utf16_sv
			}));
		}
		{
			const auto text = u"<<<\u00E9A>>>"_utf16_sv;
			[[maybe_unused]] const auto stripped_prefix = text.strip_prefix(u"<<<"_utf16_sv);
			assert(stripped_prefix.has_value());
			assert(stripped_prefix.value() == u"\u00E9A>>>"_utf16_sv);
			[[maybe_unused]] const auto stripped_suffix = text.strip_suffix(u">>>"_utf16_sv);
			assert(stripped_suffix.has_value());
			assert(stripped_suffix.value() == u"<<<\u00E9A"_utf16_sv);
			[[maybe_unused]] const auto stripped_circ = text.strip_circumfix(u"<<<"_utf16_sv, u">>>"_utf16_sv);
			assert(stripped_circ.has_value());
			assert(stripped_circ.value() == u"\u00E9A"_utf16_sv);
			[[maybe_unused]] const auto stripped_chars = u"[\u00E9]"_utf16_sv.strip_circumfix(u"["_u16c, u"]"_u16c);
			assert(stripped_chars.has_value());
			assert(stripped_chars.value() == u"\u00E9"_utf16_sv);
			assert(!text.strip_prefix(u">>>"_utf16_sv).has_value());
			assert(!text.strip_circumfix(u"<<<"_utf16_sv, u"]"_utf16_sv).has_value());
			assert(text.trim_prefix(u">>>"_utf16_sv) == text);
			assert(text.trim_prefix(u"<<<"_utf16_sv) == u"\u00E9A>>>"_utf16_sv);
			assert(text.trim_suffix(u">>>"_utf16_sv) == u"<<<\u00E9A"_utf16_sv);
			assert(u"\u00E9A\u00E9"_utf16_sv.trim_prefix(u"\u00E9"_u16c) == u"A\u00E9"_utf16_sv);
			assert(u"\u00E9A\u00E9"_utf16_sv.trim_suffix(u"\u00E9"_u16c) == u"\u00E9A"_utf16_sv);
		}
		{
			[[maybe_unused]] const auto repeated = u"----abra----"_utf16_sv;
			[[maybe_unused]] const auto accented = u"\u00E9\u00E9A\u00E9"_utf16_sv;
			assert(repeated.trim_start_matches(u"--"_utf16_sv) == u"abra----"_utf16_sv);
			assert(repeated.trim_end_matches(u"--"_utf16_sv) == u"----abra"_utf16_sv);
			assert(repeated.trim_matches(u"--"_utf16_sv) == u"abra"_utf16_sv);
			assert(repeated.trim_matches(u""_utf16_sv) == repeated);
			assert(u"***abra***"_utf16_sv.trim_matches(u"*"_u16c) == u"abra"_utf16_sv);
			assert(accented.trim_start_matches(u"\u00E9"_u16c) == u"A\u00E9"_utf16_sv);
			assert(accented.trim_end_matches(u"\u00E9"_u16c) == u"\u00E9\u00E9A"_utf16_sv);
			assert(accented.trim_matches(u"\u00E9"_u16c) == u"A"_utf16_sv);
		}
		{
			[[maybe_unused]] const auto unicode_trimmed = u"\u00A0\tA\u00A0 "_utf16_sv;
			[[maybe_unused]] const auto unicode_split = u"\u00A0A\u2003B C"_utf16_sv;
			assert(unicode_trimmed.trim() == u"A"_utf16_sv);
			assert(unicode_trimmed.trim_start() == u"A\u00A0 "_utf16_sv);
			assert(unicode_trimmed.trim_end() == u"\u00A0\tA"_utf16_sv);
			assert(unicode_trimmed.trim_ascii() == u"\u00A0\tA\u00A0"_utf16_sv);
			assert(unicode_trimmed.trim_ascii_start() == unicode_trimmed);
			assert(unicode_trimmed.trim_ascii_end() == u"\u00A0\tA\u00A0"_utf16_sv);
			assert(std::ranges::equal(u""_utf16_sv.split_whitespace(), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(u" \t\r\n"_utf16_sv.split_ascii_whitespace(), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(u" \tA  B\n"_utf16_sv.split_whitespace(), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			}));
			assert(std::ranges::equal(u" \tA  B\n"_utf16_sv.split_ascii_whitespace(), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			}));
			assert(std::ranges::equal(unicode_split.split_whitespace(), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv,
				u"C"_utf16_sv
			}));
			assert(std::ranges::equal(unicode_split.split_ascii_whitespace(), std::array{
				u"\u00A0A\u2003B"_utf16_sv,
				u"C"_utf16_sv
			}));
		}
		{
			[[maybe_unused]] const auto empty = u""_utf16_sv;
			const auto exact = u"--"_utf16_sv;
			[[maybe_unused]] const auto repeated = u"a----b"_utf16_sv;
			[[maybe_unused]] const auto missing = u"abra"_utf16_sv;
			const auto unicode = u"A\u00E9B\u00E9"_utf16_sv;
			assert(std::ranges::equal(empty.split(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			}));
			assert(std::ranges::equal(empty.rsplit(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			}));
			assert(std::ranges::equal(empty.splitn(2, u"--"_utf16_sv), std::array{
				u""_utf16_sv
			}));
			assert(std::ranges::equal(empty.rsplitn(2, u"--"_utf16_sv), std::array{
				u""_utf16_sv
			}));
			assert(!empty.split_once(u"--"_utf16_sv).has_value());
			assert(!empty.rsplit_once(u"--"_utf16_sv).has_value());
			assert(std::ranges::equal(exact.split(u"--"_utf16_sv), std::array{
				u""_utf16_sv,
				u""_utf16_sv
			}));
			assert(std::ranges::equal(exact.split_terminator(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			}));
			[[maybe_unused]] const auto exact_first = exact.split_once(u"--"_utf16_sv);
			assert(exact_first.has_value());
			assert(exact_first->first == u""_utf16_sv);
			assert(exact_first->second == u""_utf16_sv);
			[[maybe_unused]] const auto exact_last = exact.rsplit_once(u"--"_utf16_sv);
			assert(exact_last.has_value());
			assert(exact_last->first == u""_utf16_sv);
			assert(exact_last->second == u""_utf16_sv);
			assert(std::ranges::equal(repeated.split(u"--"_utf16_sv), std::array{
				u"a"_utf16_sv,
				u""_utf16_sv,
				u"b"_utf16_sv
			}));
			assert(std::ranges::equal(missing.split(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(missing.rsplit(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(std::views::reverse(missing.split(u"--"_utf16_sv)), missing.rsplit(u"--"_utf16_sv)));
			assert(std::ranges::equal(missing.split(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(missing.rsplit(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(missing.split_terminator(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(missing.rsplit_terminator(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(missing.splitn(2, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(missing.rsplitn(2, u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(unicode.split(u"\u00E9"_u16c), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv,
				u""_utf16_sv
			}));
			[[maybe_unused]] const auto unicode_first = unicode.split_once(u"\u00E9"_u16c);
			assert(unicode_first.has_value());
			assert(unicode_first->first == u"A"_utf16_sv);
			assert(unicode_first->second == u"B\u00E9"_utf16_sv);
			[[maybe_unused]] const auto unicode_last = unicode.rsplit_once(u"\u00E9"_u16c);
			assert(unicode_last.has_value());
			assert(unicode_last->first == u"A\u00E9B"_utf16_sv);
			assert(unicode_last->second == u""_utf16_sv);
		}
		{
			const auto text = u"A\u00E9\U0001F600"_utf16_sv;
			[[maybe_unused]] const auto split = text.split_once_at(1);
			assert(split.has_value());
			assert(split->first == u"A"_utf16_sv);
			assert(split->second == u"\u00E9\U0001F600"_utf16_sv);
			assert(!text.split_once_at(3).has_value());
			[[maybe_unused]] const auto unchecked = text.split_once_at_unchecked(1);
			assert(unchecked.first == u"A"_utf16_sv);
			assert(unchecked.second == u"\u00E9\U0001F600"_utf16_sv);
		}
		{
			const std::u16string text_storage = u"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(text.grapheme_count() == 3);
			assert(text.is_grapheme_boundary(0));
			assert(!text.is_grapheme_boundary(1));
			assert(text.is_grapheme_boundary(2));
			assert(!text.is_grapheme_boundary(5));
			assert(text.is_grapheme_boundary(6));
			assert(text.ceil_grapheme_boundary(1) == 2);
			assert(text.floor_grapheme_boundary(1) == 0);
			assert(text.ceil_grapheme_boundary(5) == 6);
			assert(text.floor_grapheme_boundary(5) == 2);
			assert(text.grapheme_at(0).has_value());
			assert(text.grapheme_at(0).value() == u"e\u0301"_grapheme_utf16);
			assert(text.grapheme_at(2).has_value());
			assert(text.grapheme_at(2).value() == u"\U0001F1F7\U0001F1F4"_grapheme_utf16);
			assert(!text.grapheme_at(1).has_value());
			assert(text.grapheme_substr(2, 4).has_value());
			assert(text.grapheme_substr(2, 4).value() == u"\U0001F1F7\U0001F1F4"_utf16_sv);
			assert(text.grapheme_substr(2).has_value());
			assert(text.grapheme_substr(2).value() == u"\U0001F1F7\U0001F1F4!"_utf16_sv);
			assert(!text.grapheme_substr(1, 2).has_value());
		}
	}

	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0x7Fu);
		[[maybe_unused]] const utf8_char old = ch++;
		assert(old.as_scalar() == 0x7Fu);
		assert(ch.as_scalar() == 0x80u);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0xD7FFu);
		++ch;
		assert(ch.as_scalar() == 0xE000u);
	}
	assert(!utf8_char::from_scalar(0xD800u).has_value());
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0x10FFFFu);
		++ch;
		assert(ch.as_scalar() == 0u);
	}
	assert(!utf8_char::from_scalar(0x110000u).has_value());
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0x80u);
		[[maybe_unused]] const utf8_char old = ch--;
		assert(old.as_scalar() == 0x80u);
		assert(ch.as_scalar() == 0x7Fu);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0xE000u);
		--ch;
		assert(ch.as_scalar() == 0xD7FFu);
	}
	assert(!utf8_char::from_scalar(0xD800u).has_value());
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0u);
		--ch;
		assert(ch.as_scalar() == 0x10FFFFu);
	}
	assert(!utf8_char::from_scalar(0x110000u).has_value());
	assert(utf8_char::from_scalar(0x20ACu).has_value());
	{
		std::array<char16_t, 2> buffer{};
		[[maybe_unused]] const auto n = u8"\u20AC"_u8c.encode_utf16<char16_t>(buffer.begin());
		assert(n == 1);
		assert(buffer[0] == static_cast<char16_t>(0x20ACu));
	}
	{
		std::array<char16_t, 2> buffer{};
		[[maybe_unused]] const auto n = u8"\U0001F600"_u8c.encode_utf16<char16_t>(buffer.begin());
		assert(n == 2);
		assert(buffer[0] == static_cast<char16_t>(0xD83Du));
		assert(buffer[1] == static_cast<char16_t>(0xDE00u));
	}

	assert(utf16_char::from_scalar(0x20ACu).has_value());
	assert(!utf16_char::from_scalar(0x110000u).has_value());
	assert(utf16_char::from_utf16_code_units(u"\u20AC", 1).has_value());
	assert(!utf16_char::from_utf16_code_units(u"\xD800", 1).has_value());
	assert(utf16_char::from_scalar_unchecked(0x20ACu).as_scalar() == 0x20ACu);
	assert(utf16_char::from_scalar_unchecked(0x1F600u).code_unit_count() == 2);
	assert(u"\u20AC"_u16c.as_scalar() == 0x20ACu);
	assert(u"\U0001F600"_u16c.code_unit_count() == 2);
	{
		utf16_char ch = utf16_char::from_scalar_unchecked(0x7Fu);
		[[maybe_unused]] const auto old = ch++;
		assert(old.as_scalar() == 0x7Fu);
		assert(ch.as_scalar() == 0x80u);
	}
	{
		utf16_char ch = utf16_char::from_scalar_unchecked(0xE000u);
		--ch;
		assert(ch.as_scalar() == 0xD7FFu);
	}
	assert(u"A"_u16c.is_ascii());
	assert(u"A"_u16c.is_ascii_alphabetic());
	assert(u"7"_u16c.is_ascii_digit());
	assert(!u"\u03A9"_u16c.is_ascii());
	assert(u"\u03A9"_u16c.is_alphabetic());
	assert(u"\u03A9"_u16c.is_uppercase());
	assert(u"\u03C9"_u16c.is_lowercase());
	assert(u" "_u16c.is_ascii_whitespace());
	assert(u"F"_u16c.is_ascii_hexdigit());
	assert(u"!"_u16c.is_ascii_punctuation());
	assert((details::non_narrowing_convertible<char16_t, char16_t>));
	assert((details::non_narrowing_convertible<char16_t, std::uint32_t>));
	assert((!details::non_narrowing_convertible<char16_t, char8_t>));
	assert(u"A"_u16c.ascii_lowercase() == u"a"_u16c);
	assert(u"z"_u16c.ascii_uppercase() == u"Z"_u16c);
	assert(u"A"_u16c.eq_ignore_ascii_case(u"a"_u16c));
	{
		utf16_char lhs = u"A"_u16c;
		utf16_char rhs = u"z"_u16c;
		lhs.swap(rhs);
		assert(lhs == u"z"_u16c);
		assert(rhs == u"A"_u16c);
	}
	{
		utf16_char utf16 = u8"\u20AC"_u8c;
		[[maybe_unused]] utf8_char utf8 = utf16;
		assert(utf16 == u"\u20AC"_u16c);
		assert(utf8 == u8"\u20AC"_u8c);
	}
	{
		std::array<char, 4> buffer{};
		[[maybe_unused]] const auto n = u"\u20AC"_u16c.encode_utf8<char>(buffer.begin());
		assert(n == 3);
		assert(static_cast<unsigned char>(buffer[0]) == 0xE2u);
		assert(static_cast<unsigned char>(buffer[1]) == 0x82u);
		assert(static_cast<unsigned char>(buffer[2]) == 0xACu);
	}

	// Runtime formatting and transcoding checks.
	assert(std::format("{}", "A"_u8c) == "A");
	assert(std::format("{:c}", latin1_ch) == "é");
	assert(std::format("{:c}", "€"_u8c) == "€");
	assert(std::format("{:c}", "😀"_u8c) == "😀");

	{
		std::u16string encoded;
		[[maybe_unused]] const auto n = "😀"_u8c.encode_utf16<char16_t>(std::back_inserter(encoded));
		assert(n == 2);
		assert(encoded.size() == 2);
		assert(encoded[0] == static_cast<char16_t>(0xD83Du));
		assert(encoded[1] == static_cast<char16_t>(0xDE00u));
	}
	assert(std::format("{}", u"€"_u16c) == "€");
	assert(std::format("{:x}", u"€"_u16c) == "20ac");
	{
		utf16_char ch = u"A"_u16c;
		++ch;
		assert(ch == u"B"_u16c);
		--ch;
		assert(ch == u"A"_u16c);
	}
	assert(std::format(L"{}", "€"_u8c) == wide_from_scalar(0x20ACu));
	assert(std::format(L"{}", u"€"_u16c) == wide_from_scalar(0x20ACu));
	assert(std::format(L"{}", u"😀"_u16c) == wide_from_scalar(0x1F600u));
	assert(std::format(L"{:x}", "€"_u8c) == L"20ac");
	assert(std::format(L"{:x}", u"€"_u16c) == L"20ac");
	{
		std::array<char16_t, 2> encoded{};
		[[maybe_unused]] const auto n = u"😀"_u16c.encode_utf16<char16_t>(encoded.begin());
		assert(n == 2);
		assert(encoded[0] == static_cast<char16_t>(0xD83Du));
		assert(encoded[1] == static_cast<char16_t>(0xDE00u));
	}

	// Direct UTF-16 view iteration, both forward and reverse.
	{
		constexpr std::u16string_view text = u"Aé😀";
		[[maybe_unused]] const auto view = views::utf16_view::from_code_units_unchecked(text);
		assert(std::ranges::equal(view, std::array{ u"A"_u16c, u"é"_u16c, u"😀"_u16c }));
	}
	{
		const std::u16string text = u"Aé😀";
		std::string decoded;
		for (const utf16_char ch : views::utf16_view::from_code_units_unchecked(text))
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "Aé😀");
	}
		{
			constexpr std::u16string_view text = u"Aé😀";
			[[maybe_unused]] const auto view = views::reversed_utf16_view::from_code_units_unchecked(text);
			assert(std::ranges::equal(view, std::array{ u"😀"_u16c, u"é"_u16c, u"A"_u16c }));
		}
		{
			const std::u16string text = u"Aé😀";
			std::string decoded;
			for (const utf16_char ch : views::reversed_utf16_view::from_code_units_unchecked(text))
			{
				ch.encode_utf8<char>(std::back_inserter(decoded));
			}
			assert(decoded == "😀éA");
		}
	{
		// Invalid UTF-16 must report the first failing code-unit index.
		const std::array<char16_t, 1> invalid{ static_cast<char16_t>(0xD800u) };
		const auto result = utf16_string_view::from_code_units({ invalid.data(), invalid.size() });
		if (result.has_value())
		{
			assert(false);
		}
		else
		{
			assert(result.error().code == utf16_error_code::truncated_surrogate_pair);
			assert(result.error().first_invalid_code_unit_index == 0);
		}
	}

	// Formatting ranges by first materializing them into owning strings.
	assert(std::format("{:d}", "A"_u8c) == "65");
	assert(std::format("{:x}", latin1_ch) == "e9");
	assert(std::format("{:X}", latin1_ch) == "E9");
	assert(std::format("{:o}", latin1_ch) == "351");
	assert(std::format("{:b}", "A"_u8c) == "1000001");
	assert(std::format("{:>4c}", "A"_u8c) == "   A");
	assert(std::format("{:*^5c}", "A"_u8c) == "**A**");
	assert(std::format("{:_<4c}", "A"_u8c) == "A___");
	assert(std::format("{:#06x}", "A"_u8c) == "0x0041");
	assert(std::format("{:#010b}", "A"_u8c) == "0b01000001");
	assert(std::format("{:>6d}", latin1_ch) == "   233");
	assert(std::format("{:s}", utf8_text.chars() | std::ranges::to<utf8_string>()) == "Aé€");
	assert(std::format("{:>6s}", utf8_text.chars() | std::ranges::to<utf8_string>()) == "   Aé€");
	assert(std::format("{:_<6s}", utf8_text.reversed_chars() | std::ranges::to<utf8_string>()) == "€éA___");

	// Owning UTF-8 string construction, concatenation, and mutation coverage.
	assert(utf8_string{}.base().empty());
	static_assert(std::same_as<utf8_string::value_type, utf8_char>);
	static_assert(std::same_as<decltype(utf8_string{}.get_allocator()), std::allocator<char8_t>>);
	static_assert(std::same_as<decltype(utf8_string{}.pop_back()), std::optional<utf8_char>>);
	{
		auto bytes = std::u8string{ u8"A\u00E9\U0001F600" };
		const auto result = utf8_string::from_bytes(std::move(bytes));
		assert(result.has_value());
		assert(result.value() == u8"A\u00E9\U0001F600"_utf8_sv);
	}
	{
		auto bytes = std::u8string{ u8"A\u00E9\U0001F600" };
		const auto result = utf8_string::from_bytes_unchecked(std::move(bytes));
		assert(result == u8"A\u00E9\U0001F600"_utf8_sv);
	}
	{
		const auto result = utf8_string::from_bytes_unchecked(std::string_view{ "A\xC3\xA9\xF0\x9F\x98\x80", 7 });
		assert(result == u8"A\u00E9\U0001F600"_utf8_sv);
	}
	{
		const auto result = utf8_string::from_bytes_unchecked(std::wstring_view{ L"A\u00E9\U0001F600" });
		assert(result == u8"A\u00E9\U0001F600"_utf8_sv);
	}
	assert("Aé€"_utf8_s == utf8_text);
	assert(utf8_string{ utf8_text } == "Aé€"_utf8_s);
	assert((utf8_string{ u"Aé😀"_utf16_sv } == u8"Aé😀"_utf8_sv));
	assert((utf8_string{ utf16_string{ u"Aé😀"_utf16_sv } } == u8"Aé😀"_utf8_sv));
	assert(std::format("{}", utf8_string{ utf8_text }) == "Aé€");
	{
		const auto result = utf8_string::from_bytes("Aé😀");
		assert(result.has_value());
		assert(result.value() == u8"Aé😀"_utf8_sv);
	}
	{
		const auto result = utf8_string::from_bytes(std::wstring_view{ L"Aé😀" });
		assert(result.has_value());
		assert(result.value() == u8"Aé😀"_utf8_sv);
	}
	assert(utf8_string{ u8"Aé😀"_utf8_sv }.to_utf16() == u"Aé😀"_utf16_sv);
	assert(utf8_string{ u8"Aé😀"_utf8_sv }.to_utf8_owned() == u8"Aé😀"_utf8_sv);
	{
		std::pmr::monotonic_buffer_resource resource;
		const auto transcoded = utf8_string{ u8"Aé😀"_utf8_sv }.to_utf16(std::pmr::polymorphic_allocator<char16_t>{ &resource });
		static_assert(std::same_as<
			std::remove_cvref_t<decltype(transcoded)>,
			pmr::utf16_string>);
		assert(transcoded == u"Aé😀"_utf16_sv);
	}
	{
		const auto e_acute = utf8_char::from_scalar_unchecked(0x00E9u);
		const auto lhs = "A"_utf8_s;
		const utf8_string rhs(std::from_range, std::array{ e_acute });
		const utf8_string expected(std::from_range, std::array{ "A"_u8c, e_acute });

		assert(lhs + rhs == expected);
		assert(rhs + lhs == utf8_string(std::from_range, std::array{ e_acute, "A"_u8c }));
		assert(lhs + e_acute == expected);
		assert(e_acute + lhs == utf8_string(std::from_range, std::array{ e_acute, "A"_u8c }));
		auto moved_lhs = "A"_utf8_s;
		assert(std::move(moved_lhs) + rhs == expected);

		auto moved_rhs = "A"_utf8_s;
		assert(rhs + std::move(moved_rhs) == utf8_string(std::from_range, std::array{ e_acute, "A"_u8c }));
	}
	{
		auto appended = "A"_utf8_s;
		appended += "é"_utf8_sv;
		appended += u"😀"_utf16_sv;
		appended += "!"_u8c;
		appended += u"?"_u16c;
		assert(appended == u8"Aé😀!?"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.append(s.as_view());
		assert(s == u8"AéAé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s += s.as_view();
		assert(s == u8"AéAé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.append_range(s.chars());
		assert(s == u8"AéAé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		const auto chars = s.chars();
		s.append(chars.begin(), chars.end());
		assert(s == u8"AéAé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.insert(1, s.as_view());
		assert(s == u8"AAéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.insert_range(1, s.chars());
		assert(s == u8"AAéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		const auto chars = s.chars();
		s.insert(1, chars.begin(), chars.end());
		assert(s == u8"AAéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace(0, 1, s.as_view());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace(0, s.as_view());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace_with_range(0, 1, s.chars());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace_with_range(0, s.chars());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		const auto suffix = s.as_view().substr(1).value();
		s.append(suffix);
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		const auto reversed = utf8_text.reversed_chars() | std::ranges::to<utf8_string>();
		assert(reversed == "€éA"_utf8_sv);
	}
	{
		assert(std::ranges::equal(u8"e\u0301🇷🇴!"_utf8_sv.graphemes(), std::array{
			u8"e\u0301"_grapheme_utf8,
			u8"🇷🇴"_grapheme_utf8,
			u8"!"_grapheme_utf8
		}));
	}
	{
		assert(std::ranges::equal(u8"e\u0301🇷🇴!"_utf8_sv.reversed_graphemes(), std::array{
			u8"!"_grapheme_utf8,
			u8"🇷🇴"_grapheme_utf8,
			u8"e\u0301"_grapheme_utf8
		}));
	}

	{
		const auto result = utf8_string::from_bytes("Aé😀", std::allocator<char8_t>{});
		assert(result.has_value());
		assert(result.value() == u8"Aé😀"_utf8_sv);
	}
	{
		const auto result = utf8_string::from_bytes(std::wstring_view{ L"Aé😀" }, std::allocator<char8_t>{});
		assert(result.has_value());
		assert(result.value() == u8"Aé😀"_utf8_sv);
	}
	{
		const auto result = utf8_string::from_bytes_unchecked(std::string_view{ "A\xC3\xA9\xF0\x9F\x98\x80", 7 }, std::allocator<char8_t>{});
		assert(result == u8"A\u00E9\U0001F600"_utf8_sv);
	}
	{
		const auto result = utf8_string::from_bytes_unchecked(std::wstring_view{ L"A\u00E9\U0001F600" }, std::allocator<char8_t>{});
		assert(result == u8"A\u00E9\U0001F600"_utf8_sv);
	}

	// Owning UTF-16 string construction, concatenation, and mutation coverage.
	{
		auto bytes = std::u16string{ u"A\u00E9\U0001F600" };
		const auto result = utf16_string::from_bytes(std::move(bytes));
		assert(result.has_value());
		assert(result.value() == u"A\u00E9\U0001F600"_utf16_sv);
	}
	{
		auto bytes = std::u16string{ u"A\u00E9\U0001F600" };
		const auto result = utf16_string::from_bytes_unchecked(std::move(bytes));
		assert(result == u"A\u00E9\U0001F600"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes_unchecked(std::string_view{ "A\xC3\xA9\xF0\x9F\x98\x80", 7 });
		assert(result == u"A\u00E9\U0001F600"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes_unchecked(std::wstring_view{ L"A\u00E9\U0001F600" });
		assert(result == u"A\u00E9\U0001F600"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes("Aé😀", std::allocator<char16_t>{});
		assert(result.has_value());
		assert(result.value() == u"Aé😀"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes(std::wstring_view{ L"Aé😀" }, std::allocator<char16_t>{});
		assert(result.has_value());
		assert(result.value() == u"Aé😀"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes_unchecked(std::string_view{ "A\xC3\xA9\xF0\x9F\x98\x80", 7 }, std::allocator<char16_t>{});
		assert(result == u"A\u00E9\U0001F600"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes_unchecked(std::wstring_view{ L"A\u00E9\U0001F600" }, std::allocator<char16_t>{});
		assert(result == u"A\u00E9\U0001F600"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_code_units_unchecked(
			std::u16string{ u"Aé😀" },
			std::allocator<char16_t>{});
		assert(result == u"Aé😀"_utf16_sv);
	}
	assert(utf16_string{}.base().empty());
	static_assert(std::same_as<utf16_string::value_type, utf16_char>);
	static_assert(std::same_as<decltype(utf16_string{}.get_allocator()), std::allocator<char16_t>>);
	static_assert(std::same_as<decltype(utf16_string{}.pop_back()), std::optional<utf16_char>>);
	assert(u"Aé😀"_utf16_s == utf16_text);
	assert(utf16_string{ utf16_text } == u"Aé😀"_utf16_s);
	assert((utf16_string{ u8"Aé😀"_utf8_sv } == u"Aé😀"_utf16_sv));
	assert((utf16_string{ utf8_string{ u8"Aé😀"_utf8_sv } } == u"Aé😀"_utf16_sv));
	assert(std::format("{}", utf16_string{ utf16_text }) == "Aé😀");
	{
		const auto result = utf16_string::from_bytes("Aé😀");
		assert(result.has_value());
		assert(result.value() == u"Aé😀"_utf16_sv);
	}
	{
		const auto result = utf16_string::from_bytes(std::wstring_view{ L"Aé😀" });
		assert(result.has_value());
		assert(result.value() == u"Aé😀"_utf16_sv);
	}
	assert(utf16_string{ u"Aé😀"_utf16_sv }.to_utf8() == u8"Aé😀"_utf8_sv);
	assert(utf16_string{ u"Aé😀"_utf16_sv }.to_utf16_owned() == u"Aé😀"_utf16_sv);
	{
		std::pmr::monotonic_buffer_resource resource;
		const auto transcoded = utf16_string{ u"Aé😀"_utf16_sv }.to_utf8(std::pmr::polymorphic_allocator<char8_t>{ &resource });
		static_assert(std::same_as<
			std::remove_cvref_t<decltype(transcoded)>,
			pmr::utf8_string>);
		assert(transcoded == u8"Aé😀"_utf8_sv);
	}
	{
		const auto e_acute = utf16_char::from_scalar_unchecked(0x00E9u);
		const auto lhs = u"A"_utf16_s;
		const utf16_string rhs(std::from_range, std::array{ e_acute });
		const utf16_string expected(std::from_range, std::array{ u"A"_u16c, e_acute });

		assert(lhs + rhs == expected);
		assert(rhs + lhs == utf16_string(std::from_range, std::array{ e_acute, u"A"_u16c }));
		assert(lhs + e_acute == expected);
		assert(e_acute + lhs == utf16_string(std::from_range, std::array{ e_acute, u"A"_u16c }));
		auto moved_lhs = u"A"_utf16_s;
		assert(std::move(moved_lhs) + rhs == expected);

		auto moved_rhs = u"A"_utf16_s;
		assert(rhs + std::move(moved_rhs) == utf16_string(std::from_range, std::array{ e_acute, u"A"_u16c }));
	}
	{
		auto appended = u"A"_utf16_s;
		appended += u"é"_utf16_sv;
		appended += u8"😀"_utf8_sv;
		appended += u"!"_u16c;
		appended += "?"_u8c;
		assert(appended == u"Aé😀!?"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.append(s.as_view());
		assert(s == u"AéAé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s += s.as_view();
		assert(s == u"AéAé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.append_range(s.chars());
		assert(s == u"AéAé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		const auto chars = s.chars();
		s.append(chars.begin(), chars.end());
		assert(s == u"AéAé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.insert(1, s.as_view());
		assert(s == u"AAéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.insert_range(1, s.chars());
		assert(s == u"AAéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		const auto chars = s.chars();
		s.insert(1, chars.begin(), chars.end());
		assert(s == u"AAéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace(0, 1, s.as_view());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace(0, s.as_view());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace_with_range(0, 1, s.chars());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace_with_range(0, s.chars());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		const auto suffix = s.as_view().substr(1).value();
		s.append(suffix);
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		utf16_string s;
		s.assign(u"Aé😀"_utf16_sv);
		assert(s == u"Aé😀"_utf16_sv);
	}
	{
		utf16_string s;
		s.assign(u"Ω"_u16c);
		assert(s == u"Ω"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.assign(3, u"!"_u16c);
		assert(s == u"!!!"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.assign_range(std::array{ u"Ω"_u16c, u"!"_u16c });
		assert(s == u"Ω!"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		const std::array data{ u"β"_u16c, u"γ"_u16c };
		s.assign(data.begin(), data.end());
		assert(s == u"βγ"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.assign({ u"x"_u16c, u"y"_u16c, u"z"_u16c });
		assert(s == u"xyz"_utf16_sv);
	}
	{
		auto s = u"A\u00E9\U0001F600"_utf16_s;
		[[maybe_unused]] const auto removed = s.pop_back();
		assert(removed.has_value());
		assert(*removed == u"\U0001F600"_u16c);
		assert(s == u"A\u00E9"_utf16_sv);
	}
	{
		utf16_string s;
		[[maybe_unused]] const auto removed = s.pop_back();
		assert(!removed.has_value());
		assert(s.empty());
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.erase(1, 1);
		assert(s == u"A😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.erase(2);
		assert(s == u"Aé"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace(1, 1, u"Ω"_utf16_sv);
		assert(s == u"AΩ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace(1, 1, u"Ω"_u16c);
		assert(s == u"AΩ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace(1, u"XYZ"_utf16_sv);
		assert(s == u"AXYZ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace(1, u"Ω"_u16c);
		assert(s == u"AΩ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_with_range(1, 1, std::array{ u"Ω"_u16c, u"!"_u16c });
		assert(s == u"AΩ!😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_with_range(1, std::array{ u"Ω"_u16c, u"!"_u16c });
		assert(s == u"AΩ!😀"_utf16_sv);
	}
	{
		assert(std::ranges::equal(u"e\u0301🇷🇴!"_utf16_sv.graphemes(), std::array{
			u"e\u0301"_grapheme_utf16,
			u"🇷🇴"_grapheme_utf16,
			u"!"_grapheme_utf16
		}));
	}
	{
		assert(std::ranges::equal(u"e\u0301🇷🇴!"_utf16_sv.reversed_graphemes(), std::array{
			u"!"_grapheme_utf16,
			u"🇷🇴"_grapheme_utf16,
			u"e\u0301"_grapheme_utf16
		}));
	}

	// UTF-8 mutation failure cases should throw rather than silently splitting characters.
	{
		utf8_string s;
		s.assign("Aé€"_utf8_sv);
		assert(s == "Aé€"_utf8_sv);
	}
	{
		utf8_string s;
		s.assign("Ω"_u8c);
		assert(s == "Ω"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.assign(3, "!"_u8c);
		assert(s == "!!!"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.assign_range(std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "Ω!"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		const std::array data{ "β"_u8c, "γ"_u8c };
		s.assign(data.begin(), data.end());
		assert(s == "βγ"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.assign({ "x"_u8c, "y"_u8c, "z"_u8c });
		assert(s == "xyz"_utf8_sv);
	}
	{
		auto s = u8"A\u00E9\u20AC"_utf8_s;
		[[maybe_unused]] const auto removed = s.pop_back();
		assert(removed.has_value());
		assert(*removed == u8"\u20AC"_u8c);
		assert(s == u8"A\u00E9"_utf8_sv);
	}
	{
		utf8_string s;
		[[maybe_unused]] const auto removed = s.pop_back();
		assert(!removed.has_value());
		assert(s.empty());
	}
	{
		auto s = "Aé€"_utf8_s;
		s.erase(1, 2);
		assert(s == "A€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.erase(1);
		assert(s == "A"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace(1, 2, "Ω"_utf8_sv);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace(1, 2, "Ω"_u8c);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace(1, "XYZ"_utf8_sv);
		assert(s == "AXYZ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace(1, "Ω"_u8c);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_with_range(1, 2, std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "AΩ!€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_with_range(1, std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "AΩ!€"_utf8_sv);
	}
	{
		utf8_string s{ utf8_text };
		s.erase(s.size(), 1);
		assert(s == utf8_text);
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.erase(decltype(s)::npos); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.erase(2, 1); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.erase(1, 1); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace(decltype(s)::npos, 1, "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace(2, 1, "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace(1, 1, "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace(s.size(), "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace(2, "Ω"_u8c); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_with_range(decltype(s)::npos, 1, std::array{ "Ω"_u8c }); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_with_range(2, std::array{ "Ω"_u8c }); }))
		{
			assert(false);
		}
	}
	{
		const auto result = utf8_string::from_bytes(std::string_view{ "A\xFF", 2 });
		assert(!result.has_value());
		assert(result.error().code == utf8_error_code::invalid_lead_byte);
		assert(result.error().first_invalid_byte_index == 1);
	}
	{
		const auto result = utf16_string::from_bytes(std::string_view{ "A\xFF", 2 });
		assert(!result.has_value());
		assert(result.error().code == utf8_error_code::invalid_lead_byte);
		assert(result.error().first_invalid_byte_index == 1);
	}
	// Formatting, hashing, and stream insertion for borrowed and owning strings.
	assert(std::format("{}", utf8_text) == "Aé€");
	assert(std::format("{}", utf16_text) == "Aé😀");
	assert(std::hash<utf8_string_view>{}(utf8_text) == std::hash<utf8_string_view>{}("Aé€"_utf8_sv));
	assert(std::hash<utf16_string_view>{}(utf16_text) == std::hash<utf16_string_view>{}(u"Aé😀"_utf16_sv));
	{
		std::ostringstream oss;
		oss << utf8_text;
		assert(oss.str() == "Aé€");
	}
	{
		std::ostringstream oss;
		oss << utf16_text;
		assert(oss.str() == "Aé😀");
	}
		{
			std::ostringstream oss;
			oss << utf8_string{ utf8_text };
			assert(oss.str() == "Aé€");
		}

	// Lossy views replace malformed input with the Unicode replacement character.
	assert(!utf8_char::from_scalar(0x110000u).has_value());
	assert(std::format("{}", utf8_char::replacement_character) == "�");
	assert(std::format("{:x}", utf8_char::replacement_character) == "fffd");
	assert(std::format("{:#08x}", utf8_char::replacement_character) == "0x00fffd");

	{
		const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
		std::string decoded;
		for (const utf8_char ch : views::lossy_utf8_view<char>{ input })
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "A��(�");
	}
	{
		const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
		std::string decoded;
		for (const utf8_char ch : input | views::lossy_utf8)
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "A��(�");
	}
	{
		const std::u16string input{
			static_cast<char16_t>(u'A'),
			static_cast<char16_t>(0xD800),
			static_cast<char16_t>(u'B'),
			static_cast<char16_t>(0xDC00),
			static_cast<char16_t>(0xD83D),
			static_cast<char16_t>(0xDE00)
		};
		std::string decoded;
		for (const utf16_char ch : views::lossy_utf16_view<char16_t>{ input })
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "A�B�😀");
	}
	{
		const std::u16string input{
			static_cast<char16_t>(u'A'),
			static_cast<char16_t>(0xD800),
			static_cast<char16_t>(u'B'),
			static_cast<char16_t>(0xDC00),
			static_cast<char16_t>(0xD83D),
			static_cast<char16_t>(0xDE00)
		};
		std::string decoded;
		for (const utf16_char ch : input | views::lossy_utf16)
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "A�B�😀");
	}
}

#endif // UTF8_RANGES_TESTS_HPP
