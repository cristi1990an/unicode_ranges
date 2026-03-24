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

	// Owning UTF-16 string construction, concatenation, and mutation coverage.
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
		const auto result = utf16_string::from_code_units_unchecked(
			std::u16string{ u"Aé😀" },
			std::allocator<char16_t>{});
		assert(result == u"Aé😀"_utf16_sv);
	}
	assert(utf16_string{}.base().empty());
	static_assert(std::same_as<utf16_string::value_type, utf16_char>);
	static_assert(std::same_as<decltype(utf16_string{}.get_allocator()), std::allocator<char16_t>>);
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
