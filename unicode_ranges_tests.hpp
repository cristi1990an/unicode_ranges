#ifndef UTF8_RANGES_TESTS_HPP
#define UTF8_RANGES_TESTS_HPP

#include "unicode_ranges.hpp"

#include <array>
#include <cassert>
#include <format>
#include <functional>
#include <memory_resource>
#include <span>
#include <sstream>
#include <string>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6262) // monolithic test aggregator; /analyze stack estimate is not actionable here
#endif
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
	const auto unwrap_utf32_view = [](std::u32string_view code_points)
	{
		const auto result = utf32_string_view::from_code_points(code_points);
		assert(result.has_value());
		return result.value();
	};

	static_assert(std::same_as<
		pmr::utf8_string,
		basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>>);
	static_assert(std::same_as<
		pmr::utf16_string,
		basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>>);
	static_assert(std::same_as<
		pmr::utf32_string,
		basic_utf32_string<std::pmr::polymorphic_allocator<char32_t>>>);
	constexpr utf8_char latin1_ch = "é"_u8c;
	constexpr auto utf8_text = "Aé€"_utf8_sv;
	constexpr auto utf16_text = u"Aé😀"_utf16_sv;
	constexpr auto utf32_text = U"A\u00E9\U0001F600"_utf32_sv;

	static_assert(unicode_character<utf8_char>);
	static_assert(std::same_as<decltype("A"_u8c.to_utf8_owned()), utf8_string>);
	static_assert(std::same_as<decltype(u"A"_u16c.to_utf16_owned()), utf16_string>);
	static_assert(std::same_as<decltype(U"A"_u32c.to_utf32_owned()), utf32_string>);
	static_assert(unicode_character<const utf8_char&>);
	static_assert(unicode_character<utf16_char>);
	static_assert(unicode_character<utf16_char&&>);
	static_assert(unicode_character<utf32_char>);
	static_assert(unicode_character<utf32_char&&>);
	static_assert(!unicode_character<char8_t>);
	static_assert(!unicode_character<char16_t>);
	static_assert(!unicode_character<char32_t>);
	static_assert(std::same_as<decltype(characters::utf8::emojis::clown_face), const utf8_char>);
	static_assert(std::same_as<decltype(characters::utf16::emojis::clown_face), const utf16_char>);
	static_assert(std::same_as<decltype(characters::utf32::emojis::clown_face), const utf32_char>);
	static_assert(characters::utf32::punctuation::ellipsis == U"\u2026"_u32c);
	static_assert(characters::utf32::currency::euro_sign == U"\u20AC"_u32c);
	static_assert(characters::utf32::arrows::right_arrow == U"\u2192"_u32c);
	static_assert(characters::utf32::emojis::clown_face == U"\U0001F921"_u32c);
	static_assert(characters::utf32::emojis::red_heart == U"\u2764"_u32c);
	static_assert(characters::utf8::punctuation::ellipsis == "…"_u8c);
	static_assert(characters::utf16::punctuation::ellipsis == u"…"_u16c);
	static_assert(characters::utf8::currency::euro_sign == "€"_u8c);
	static_assert(characters::utf16::currency::euro_sign == u"€"_u16c);
	static_assert(characters::utf8::arrows::right_arrow == "→"_u8c);
	static_assert(characters::utf16::arrows::right_arrow == u"→"_u16c);
	static_assert(characters::utf8::emojis::clown_face == "🤡"_u8c);
	static_assert(characters::utf16::emojis::clown_face == u"🤡"_u16c);
	static_assert(characters::utf8::emojis::red_heart == "❤"_u8c);
	static_assert(characters::utf16::emojis::red_heart == u"❤"_u16c);

	// Standard ranges/view concept coverage for the library view surface.
	static_assert(std::ranges::view<views::utf8_view>);
	static_assert(std::ranges::range<views::utf8_view>);
	static_assert(std::ranges::view<views::reversed_utf8_view>);
	static_assert(std::ranges::range<views::reversed_utf8_view>);
	static_assert(std::ranges::view<views::utf16_view>);
	static_assert(std::ranges::range<views::utf16_view>);
	static_assert(std::ranges::view<views::reversed_utf16_view>);
	static_assert(std::ranges::range<views::reversed_utf16_view>);
	static_assert(std::ranges::view<views::utf32_view>);
	static_assert(std::ranges::range<views::utf32_view>);
	static_assert(std::ranges::sized_range<views::utf32_view>);
	static_assert(std::ranges::common_range<views::utf32_view>);
	static_assert(std::ranges::bidirectional_range<views::utf32_view>);
	static_assert(std::ranges::random_access_range<views::utf32_view>);
	static_assert(std::ranges::view<views::reversed_utf32_view>);
	static_assert(std::ranges::range<views::reversed_utf32_view>);
	static_assert(std::ranges::sized_range<views::reversed_utf32_view>);
	static_assert(std::ranges::common_range<views::reversed_utf32_view>);
	static_assert(std::ranges::bidirectional_range<views::reversed_utf32_view>);
	static_assert(std::ranges::random_access_range<views::reversed_utf32_view>);
	static_assert(std::ranges::view<views::grapheme_cluster_view<char8_t>>);
	static_assert(std::ranges::range<views::grapheme_cluster_view<char8_t>>);
	static_assert(std::ranges::view<views::grapheme_cluster_view<char16_t>>);
	static_assert(std::ranges::range<views::grapheme_cluster_view<char16_t>>);
	static_assert(std::ranges::view<views::grapheme_cluster_view<char32_t>>);
	static_assert(std::ranges::range<views::grapheme_cluster_view<char32_t>>);
	static_assert(std::ranges::view<views::lossy_utf8_view<char>>);
	static_assert(std::ranges::range<views::lossy_utf8_view<char>>);
	static_assert(std::ranges::view<views::lossy_utf8_view<char8_t>>);
	static_assert(std::ranges::range<views::lossy_utf8_view<char8_t>>);
	static_assert(std::ranges::view<views::lossy_utf16_view<char16_t>>);
	static_assert(std::ranges::range<views::lossy_utf16_view<char16_t>>);
	static_assert(std::ranges::view<views::lossy_utf16_view<wchar_t>>);
	static_assert(std::ranges::range<views::lossy_utf16_view<wchar_t>>);
	static_assert(std::ranges::view<views::lossy_utf32_view<char32_t>>);
	static_assert(std::ranges::range<views::lossy_utf32_view<char32_t>>);
	static_assert(std::ranges::sized_range<views::lossy_utf32_view<char32_t>>);
	static_assert(std::ranges::common_range<views::lossy_utf32_view<char32_t>>);
	static_assert(std::ranges::bidirectional_range<views::lossy_utf32_view<char32_t>>);
	static_assert(std::ranges::random_access_range<views::lossy_utf32_view<char32_t>>);
	static_assert(std::ranges::view<decltype(utf32_text.chars())>);
	static_assert(std::ranges::range<decltype(utf32_text.chars())>);
	static_assert(std::ranges::sized_range<decltype(utf32_text.chars())>);
	static_assert(std::ranges::common_range<decltype(utf32_text.chars())>);
	static_assert(std::ranges::bidirectional_range<decltype(utf32_text.chars())>);
	static_assert(std::ranges::random_access_range<decltype(utf32_text.chars())>);
	static_assert(std::ranges::view<decltype(utf32_text.reversed_chars())>);
	static_assert(std::ranges::range<decltype(utf32_text.reversed_chars())>);
	static_assert(std::ranges::sized_range<decltype(utf32_text.reversed_chars())>);
	static_assert(std::ranges::common_range<decltype(utf32_text.reversed_chars())>);
	static_assert(std::ranges::bidirectional_range<decltype(utf32_text.reversed_chars())>);
	static_assert(std::ranges::random_access_range<decltype(utf32_text.reversed_chars())>);
	static_assert(std::ranges::view<decltype(utf32_text.char_indices())>);
	static_assert(std::ranges::range<decltype(utf32_text.char_indices())>);
	static_assert(std::ranges::sized_range<decltype(utf32_text.char_indices())>);
	static_assert(std::ranges::common_range<decltype(utf32_text.char_indices())>);
	static_assert(std::ranges::bidirectional_range<decltype(utf32_text.char_indices())>);
	static_assert(std::ranges::random_access_range<decltype(utf32_text.char_indices())>);
	static_assert(std::ranges::view<decltype(utf32_text.graphemes())>);
	static_assert(std::ranges::range<decltype(utf32_text.graphemes())>);
	static_assert(std::ranges::view<decltype(utf32_text.grapheme_indices())>);
	static_assert(std::ranges::range<decltype(utf32_text.grapheme_indices())>);
	static_assert(utf32_text.size() == 3);
	static_assert(utf32_text.char_count() == 3);
	static_assert(utf32_text.grapheme_count() == 3);
	static_assert(utf32_text.char_at(1).has_value());
	static_assert(utf32_text.char_at(1).value() == U"\u00E9"_u32c);
	static_assert(utf32_text.chars().size() == utf32_text.size());
	static_assert(utf32_text.reversed_chars().size() == utf32_text.size());
	static_assert(utf32_text.char_indices().size() == utf32_text.size());
	static_assert(utf32_text.chars().begin()[1] == U"\u00E9"_u32c);
	static_assert(utf32_text.reversed_chars().begin()[0] == U"\U0001F600"_u32c);
	static_assert(utf32_text.char_indices().begin()[2].first == 2);
	static_assert(utf32_text.char_indices().begin()[2].second == U"\U0001F600"_u32c);
	static_assert(utf32_text.substr(1, 1).value() == U"\u00E9"_utf32_sv);
	static_assert(utf32_text.starts_with(U"A"_u32c));
	static_assert(utf32_text.ends_with(U"\U0001F600"_u32c));
	static_assert(utf32_text.contains(U"\u00E9"_u32c));
	static_assert(utf32_text.find(U"\u00E9"_u32c) == 1);
	static_assert(utf32_text.rfind(U"\U0001F600"_u32c) == 2);
	static_assert(utf32_text.replace_all(U"\u00E9"_u32c, U"!"_u32c) == U"A!\U0001F600"_utf32_sv);
	static_assert(utf32_text.to_utf32_owned() == utf32_text);
	static_assert(utf32_text.to_utf8() == u8"A\u00E9\U0001F600"_utf8_sv);
	static_assert(utf32_text.to_utf16() == u"A\u00E9\U0001F600"_utf16_sv);
	static_assert(std::same_as<decltype(utf32_text.to_utf32_owned()), utf32_string>);
	static_assert(std::same_as<decltype(utf32_text.to_utf8()), utf8_string>);
	static_assert(std::same_as<decltype(utf32_text.to_utf16()), utf16_string>);
	static_assert(std::same_as<decltype(utf32_text.to_lowercase()), utf32_string>);
	static_assert(std::same_as<decltype(utf32_text.to_uppercase()), utf32_string>);
	static_assert(std::same_as<decltype(utf32_text.case_fold()), utf32_string>);
	static_assert(std::same_as<decltype(utf32_text.normalize(normalization_form::nfc)), utf32_string>);
	static_assert(std::same_as<decltype(utf32_text.replace_all(U"\u00E9"_u32c, U"!"_u32c)), utf32_string>);
	static_assert(std::same_as<decltype(utf32_text.replace_n(1, U"\u00E9"_u32c, U"!"_u32c)), utf32_string>);
	static_assert([] {
		constexpr std::array any_of{ U"a"_u32c, U"b"_u32c };
		return std::same_as<
			decltype(utf32_text.replace_all(std::span{ any_of }, U"!"_u32c)),
			utf32_string>
			&& std::same_as<
				decltype(utf32_text.replace_all(std::span{ any_of }, U"!"_u32c, std::pmr::polymorphic_allocator<char32_t>{})),
				pmr::utf32_string>
			&& std::same_as<
				decltype(utf32_text.replace_n(2, std::span{ any_of }, U"!"_u32c)),
				utf32_string>
			&& std::same_as<
				decltype(utf32_text.replace_n(2, std::span{ any_of }, U"!"_u32c, std::pmr::polymorphic_allocator<char32_t>{})),
				pmr::utf32_string>
			&& std::same_as<
				decltype(utf32_string{}.replace_all(std::span{ any_of }, U"!"_u32c)),
				utf32_string>
			&& std::same_as<
				decltype(utf32_string{}.replace_n(2, std::span{ any_of }, U"!"_u32c)),
				utf32_string>;
	}());
	static_assert(U"Stra\u00DFe"_utf32_sv.eq_ignore_case(U"STRASSE"_utf32_sv));
	static_assert(U"Stra\u00DFe"_utf32_sv.starts_with_ignore_case(U"str"_utf32_sv));
	static_assert(U"Stra\u00DFe"_utf32_sv.ends_with_ignore_case(U"SSE"_utf32_sv));
	static_assert(U"Stra\u00DFe"_utf32_sv.compare_ignore_case(U"STRASSE"_utf32_sv) == std::weak_ordering::equivalent);
	static_assert(!U"\u00E9"_utf32_sv.eq_ignore_case(U"e\u0301"_utf32_sv));

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
	static_assert(std::ranges::view<decltype(utf8_text.split_trimmed(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.split_trimmed(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.rsplit_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.rsplit_terminator(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.splitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.splitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.rsplitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.rsplitn(2, u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.split_inclusive(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.split_inclusive(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::view<decltype(utf8_text.matches(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.matches(u8"\u00E9"_u8c))>);
	static_assert(std::same_as<
		std::ranges::range_value_t<decltype(utf8_text.matches(u8"\u00E9"_u8c))>,
		utf8_string_view>);
	static_assert(std::ranges::view<decltype(utf8_text.rmatch_indices(u8"\u00E9"_u8c))>);
	static_assert(std::ranges::range<decltype(utf8_text.rmatch_indices(u8"\u00E9"_u8c))>);
	static_assert(std::same_as<
		std::ranges::range_value_t<decltype(utf8_text.rmatch_indices(u8"\u00E9"_u8c))>,
		std::pair<std::size_t, utf8_string_view>>);
	static_assert(std::same_as<
		decltype(utf8_text.split_once(u8"\u00E9"_u8c)),
		std::optional<std::pair<utf8_string_view, utf8_string_view>>>);
	static_assert(std::same_as<
		decltype(utf8_text.replace_all(u8"\u00E9"_u8c, u8"!"_u8c)),
		utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.replace_all(u8"\u00E9"_u8c, u8"!"_u8c, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.replace_n(2, u8"\u00E9"_u8c, u8"!"_u8c)),
		utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.replace_n(2, u8"\u00E9"_u8c, u8"!"_u8c, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert([] {
		constexpr std::array any_of{ u8"a"_u8c, u8"b"_u8c };
		return std::same_as<
			decltype(utf8_text.replace_all(std::span{ any_of }, u8"!"_u8c)),
			utf8_string>
			&& std::same_as<
				decltype(utf8_text.replace_all(std::span{ any_of }, u8"!"_u8c, std::pmr::polymorphic_allocator<char8_t>{})),
				pmr::utf8_string>
			&& std::same_as<
				decltype(utf8_text.replace_n(2, std::span{ any_of }, u8"!"_u8c)),
				utf8_string>
			&& std::same_as<
				decltype(utf8_text.replace_n(2, std::span{ any_of }, u8"!"_u8c, std::pmr::polymorphic_allocator<char8_t>{})),
				pmr::utf8_string>
			&& std::same_as<
				decltype(utf8_string{}.replace_all(std::span{ any_of }, u8"!"_u8c)),
				utf8_string>
			&& std::same_as<
				decltype(utf8_string{}.replace_n(2, std::span{ any_of }, u8"!"_u8c)),
				utf8_string>;
	}());
	static_assert([] {
		constexpr std::array any_of{ u8"\u00E9"_u8c, u8"\u20AC"_u8c };
		constexpr auto text = u8"\u00E9A\u20AC"_utf8_sv;
		const auto first = text.split_once(std::span{ any_of });
		const auto last = text.rsplit_once(std::span{ any_of });
		return text.contains(std::span{ any_of })
			&& text.find(std::span{ any_of }) == 0
			&& text.rfind(std::span{ any_of }) == 3
			&& text.starts_with(std::span{ any_of })
			&& text.ends_with(std::span{ any_of })
			&& text.trim_start_matches(std::span{ any_of }) == u8"A\u20AC"_utf8_sv
			&& text.trim_end_matches(std::span{ any_of }) == u8"\u00E9A"_utf8_sv
			&& text.trim_matches(std::span{ any_of }) == u8"A"_utf8_sv
			&& first.has_value()
			&& first->first == u8""_utf8_sv
			&& first->second == u8"A\u20AC"_utf8_sv
			&& last.has_value()
			&& last->first == u8"\u00E9A"_utf8_sv
			&& last->second == u8""_utf8_sv;
	}());
	static_assert(std::same_as<
		decltype(utf8_string{}.replace_all(u8"\u00E9"_u8c, u8"!"_u8c, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_string{}.replace_n(2, u8"\u00E9"_u8c, u8"!"_u8c, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_ascii_lowercase()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_ascii_lowercase(0, 1)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_ascii_lowercase(std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_ascii_lowercase(0, 1, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_ascii_uppercase()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_ascii_uppercase(0, 1)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_ascii_uppercase(std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_ascii_uppercase(0, 1, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_lowercase()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_lowercase(0, 1)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_lowercase(std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_lowercase(0, 1, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
#if UTF8_RANGES_HAS_ICU
	static_assert(std::string_view{ "tr"_locale.name } == std::string_view{ "tr" });
	static_assert(std::same_as<decltype(is_available_locale("tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf8_text.to_lowercase("tr"_locale)), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_lowercase(0, 1, "tr"_locale)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_lowercase("tr"_locale, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_uppercase("tr"_locale)), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_uppercase(0, 1, "tr"_locale)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_uppercase("tr"_locale, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.case_fold("tr"_locale)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.case_fold("tr"_locale, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.eq_ignore_case(utf8_text, "tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf8_text.starts_with_ignore_case(utf8_text, "tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf8_text.ends_with_ignore_case(utf8_text, "tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf8_text.compare_ignore_case(utf8_text, "tr"_locale)), std::weak_ordering>);
#endif
	static_assert(std::same_as<decltype(utf8_text.to_uppercase()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_uppercase(0, 1)), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_uppercase(std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.to_uppercase(0, 1, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.normalize(normalization_form::nfc)), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_nfc()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_nfd()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_nfkc()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.to_nfkd()), utf8_string>);
	static_assert(std::same_as<decltype(utf8_text.case_fold()), utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.normalize(normalization_form::nfc, std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<
		decltype(utf8_text.case_fold(std::pmr::polymorphic_allocator<char8_t>{})),
		pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_ascii_lowercase()), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_ascii_lowercase(0, 0)), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_ascii_uppercase()), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_ascii_uppercase(0, 0)), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_lowercase()), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_lowercase(0, 0)), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_uppercase()), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_uppercase(0, 0)), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.to_nfc()), pmr::utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.case_fold()), pmr::utf8_string>);
#if UTF8_RANGES_HAS_ICU
	static_assert(std::same_as<decltype(utf8_string{}.case_fold("tr"_locale)), utf8_string>);
	static_assert(std::same_as<decltype(pmr::utf8_string{}.case_fold("tr"_locale)), pmr::utf8_string>);
#endif
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
	static_assert(std::ranges::view<decltype(utf16_text.split_trimmed(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.split_trimmed(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.rsplit_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.rsplit_terminator(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.splitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.splitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.rsplitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.rsplitn(2, u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.split_inclusive(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.split_inclusive(u"\u00E9"_u16c))>);
	static_assert(std::ranges::view<decltype(utf16_text.matches(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.matches(u"\u00E9"_u16c))>);
	static_assert(std::same_as<
		std::ranges::range_value_t<decltype(utf16_text.matches(u"\u00E9"_u16c))>,
		utf16_string_view>);
	static_assert(std::ranges::view<decltype(utf16_text.rmatch_indices(u"\u00E9"_u16c))>);
	static_assert(std::ranges::range<decltype(utf16_text.rmatch_indices(u"\u00E9"_u16c))>);
	static_assert(std::same_as<
		std::ranges::range_value_t<decltype(utf16_text.rmatch_indices(u"\u00E9"_u16c))>,
		std::pair<std::size_t, utf16_string_view>>);
	static_assert(std::same_as<
		decltype(utf16_text.split_once(u"\u00E9"_u16c)),
		std::optional<std::pair<utf16_string_view, utf16_string_view>>>);
	static_assert(std::same_as<
		decltype(utf16_text.replace_all(u"\u00E9"_u16c, u"!"_u16c)),
		utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.replace_all(u"\u00E9"_u16c, u"!"_u16c, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.replace_n(2, u"\u00E9"_u16c, u"!"_u16c)),
		utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.replace_n(2, u"\u00E9"_u16c, u"!"_u16c, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert([] {
		constexpr std::array any_of{ u"a"_u16c, u"b"_u16c };
		return std::same_as<
			decltype(utf16_text.replace_all(std::span{ any_of }, u"!"_u16c)),
			utf16_string>
			&& std::same_as<
				decltype(utf16_text.replace_all(std::span{ any_of }, u"!"_u16c, std::pmr::polymorphic_allocator<char16_t>{})),
				pmr::utf16_string>
			&& std::same_as<
				decltype(utf16_text.replace_n(2, std::span{ any_of }, u"!"_u16c)),
				utf16_string>
			&& std::same_as<
				decltype(utf16_text.replace_n(2, std::span{ any_of }, u"!"_u16c, std::pmr::polymorphic_allocator<char16_t>{})),
				pmr::utf16_string>
			&& std::same_as<
				decltype(utf16_string{}.replace_all(std::span{ any_of }, u"!"_u16c)),
				utf16_string>
			&& std::same_as<
				decltype(utf16_string{}.replace_n(2, std::span{ any_of }, u"!"_u16c)),
				utf16_string>;
	}());
	static_assert([] {
		constexpr std::array any_of{ u"\u00E9"_u16c, u"\U0001F600"_u16c };
		constexpr auto text = u"\u00E9A\U0001F600"_utf16_sv;
		const auto first = text.split_once(std::span{ any_of });
		const auto last = text.rsplit_once(std::span{ any_of });
		return text.contains(std::span{ any_of })
			&& text.find(std::span{ any_of }) == 0
			&& text.rfind(std::span{ any_of }) == 2
			&& text.starts_with(std::span{ any_of })
			&& text.ends_with(std::span{ any_of })
			&& text.trim_start_matches(std::span{ any_of }) == u"A\U0001F600"_utf16_sv
			&& text.trim_end_matches(std::span{ any_of }) == u"\u00E9A"_utf16_sv
			&& text.trim_matches(std::span{ any_of }) == u"A"_utf16_sv
			&& first.has_value()
			&& first->first == u""_utf16_sv
			&& first->second == u"A\U0001F600"_utf16_sv
			&& last.has_value()
			&& last->first == u"\u00E9A"_utf16_sv
			&& last->second == u""_utf16_sv;
	}());
	static_assert(std::same_as<
		decltype(utf16_string{}.replace_all(u"\u00E9"_u16c, u"!"_u16c, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_string{}.replace_n(2, u"\u00E9"_u16c, u"!"_u16c, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_ascii_lowercase()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_ascii_lowercase(0, 1)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_ascii_lowercase(std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_ascii_lowercase(0, 1, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_ascii_uppercase()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_ascii_uppercase(0, 1)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_ascii_uppercase(std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_ascii_uppercase(0, 1, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_lowercase()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_lowercase(0, 1)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_lowercase(std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_lowercase(0, 1, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
#if UTF8_RANGES_HAS_ICU
	static_assert(std::same_as<decltype(utf16_text.to_lowercase("tr"_locale)), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_lowercase(0, 1, "tr"_locale)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_lowercase("tr"_locale, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_uppercase("tr"_locale)), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_uppercase(0, 1, "tr"_locale)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_uppercase("tr"_locale, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.case_fold("tr"_locale)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.case_fold("tr"_locale, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.eq_ignore_case(utf16_text, "tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf16_text.starts_with_ignore_case(utf16_text, "tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf16_text.ends_with_ignore_case(utf16_text, "tr"_locale)), bool>);
	static_assert(std::same_as<decltype(utf16_text.compare_ignore_case(utf16_text, "tr"_locale)), std::weak_ordering>);
#endif
	static_assert(std::same_as<decltype(utf16_text.to_uppercase()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_uppercase(0, 1)), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_uppercase(std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.to_uppercase(0, 1, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.normalize(normalization_form::nfc)), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_nfc()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_nfd()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_nfkc()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.to_nfkd()), utf16_string>);
	static_assert(std::same_as<decltype(utf16_text.case_fold()), utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.normalize(normalization_form::nfc, std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<
		decltype(utf16_text.case_fold(std::pmr::polymorphic_allocator<char16_t>{})),
		pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_ascii_lowercase()), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_ascii_lowercase(0, 0)), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_ascii_uppercase()), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_ascii_uppercase(0, 0)), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_lowercase()), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_lowercase(0, 0)), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_uppercase()), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_uppercase(0, 0)), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.to_nfc()), pmr::utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.case_fold()), pmr::utf16_string>);
#if UTF8_RANGES_HAS_ICU
	static_assert(std::same_as<decltype(utf16_string{}.case_fold("tr"_locale)), utf16_string>);
	static_assert(std::same_as<decltype(pmr::utf16_string{}.case_fold("tr"_locale)), pmr::utf16_string>);
#endif
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
	static_assert(std::ranges::view<decltype(utf16_text.grapheme_indices())>);
	static_assert(std::ranges::range<decltype(utf16_text.grapheme_indices())>);

	static_assert([] {
		constexpr auto ascii_utf8 = std::u8string_view{ u8"AbCdEfGhIjKlMnOp" };
		constexpr auto mixed_utf8 = std::u8string_view{ u8"ASCII\u00E9" };
		constexpr auto ascii_utf16 = std::u16string_view{ u"AbCdEfGhIjKlMnOp" };
		constexpr auto mixed_utf16 = std::u16string_view{ u"ASCII\u00E9" };
		std::array<char8_t, ascii_utf8.size()> lower_utf8{};
		std::array<char8_t, ascii_utf8.size()> upper_utf8{};
		std::array<char16_t, ascii_utf8.size()> widened_utf8{};
		std::array<char16_t, ascii_utf16.size()> lower_utf16{};
		std::array<char16_t, ascii_utf16.size()> upper_utf16{};
		std::array<char8_t, ascii_utf16.size()> narrowed_utf16{};

		return details::ascii_prefix_length_scalar(ascii_utf8) == ascii_utf8.size()
			&& details::ascii_prefix_length(mixed_utf8) == 5
			&& details::ascii_prefix_length_scalar(ascii_utf16) == ascii_utf16.size()
			&& details::ascii_prefix_length(mixed_utf16) == 5
			&& details::ascii_lowercase_copy(lower_utf8.data(), ascii_utf8)
			&& details::ascii_uppercase_copy(upper_utf8.data(), ascii_utf8)
			&& details::ascii_lowercase_copy(lower_utf16.data(), ascii_utf16)
			&& details::ascii_uppercase_copy(upper_utf16.data(), ascii_utf16)
			&& (std::u8string_view{ lower_utf8.data(), lower_utf8.size() } == u8"abcdefghijklmnop")
			&& (std::u8string_view{ upper_utf8.data(), upper_utf8.size() } == u8"ABCDEFGHIJKLMNOP")
			&& (std::u16string_view{ lower_utf16.data(), lower_utf16.size() } == u"abcdefghijklmnop")
			&& (std::u16string_view{ upper_utf16.data(), upper_utf16.size() } == u"ABCDEFGHIJKLMNOP")
			&& (details::copy_ascii_utf8_to_utf16(widened_utf8.data(), ascii_utf8), std::u16string_view{ widened_utf8.data(), widened_utf8.size() } == u"AbCdEfGhIjKlMnOp")
			&& (details::copy_ascii_utf16_to_utf8(narrowed_utf16.data(), ascii_utf16), std::u8string_view{ narrowed_utf16.data(), narrowed_utf16.size() } == u8"AbCdEfGhIjKlMnOp");
	}());

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
	static_assert(u8"A"_u8c.general_category() == unicode_general_category::uppercase_letter);
	static_assert(u8" "_u8c.general_category() == unicode_general_category::space_separator);
	static_assert(u8"\u20AC"_u8c.general_category() == unicode_general_category::currency_symbol);
	static_assert(utf8_char::from_scalar_unchecked(0x0378u).general_category() == unicode_general_category::unassigned);
	static_assert(u8"\u0301"_u8c.canonical_combining_class() == 230u);
	static_assert(u8"\u0301"_u8c.grapheme_break_property() == unicode_grapheme_break_property::extend);
	static_assert(u8"\u03A9"_u8c.script() == unicode_script::greek);
	static_assert(u8"!"_u8c.script() == unicode_script::common);
	static_assert(utf8_char::from_scalar_unchecked(0x0378u).script() == unicode_script::unknown);
	static_assert(u8"A"_u8c.east_asian_width() == unicode_east_asian_width::narrow);
	static_assert(u8"\u754C"_u8c.east_asian_width() == unicode_east_asian_width::wide);
	static_assert(u8"\u03A9"_u8c.east_asian_width() == unicode_east_asian_width::ambiguous);
	static_assert(u8"A"_u8c.line_break_class() == unicode_line_break_class::alphabetic);
	static_assert(u8" "_u8c.line_break_class() == unicode_line_break_class::space);
	static_assert(u8"A"_u8c.bidi_class() == unicode_bidi_class::left_to_right);
	static_assert(u8"\u0634"_u8c.bidi_class() == unicode_bidi_class::arabic_letter);
	static_assert(u8"A"_u8c.word_break_property() == unicode_word_break_property::a_letter);
	static_assert(u8"7"_u8c.word_break_property() == unicode_word_break_property::numeric);
	static_assert(u8"A"_u8c.sentence_break_property() == unicode_sentence_break_property::upper);
	static_assert(u8"."_u8c.sentence_break_property() == unicode_sentence_break_property::a_term);
	static_assert(u8"\U0001F600"_u8c.is_emoji());
	static_assert(!u8"A"_u8c.is_emoji());
	static_assert(u8"\U0001F600"_u8c.is_emoji_presentation());
	static_assert(!u8"A"_u8c.is_emoji_presentation());
	static_assert(u8"\U0001F600"_u8c.is_extended_pictographic());
	static_assert(!u8"A"_u8c.is_extended_pictographic());
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
	static_assert(utf8_text.char_count() == 3);
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
	static_assert(utf8_text.starts_with([](utf8_char ch) constexpr noexcept { return ch == u8"A"_u8c; }));
	static_assert(!utf8_text.starts_with("é"_u8c));
	static_assert(!utf8_text.starts_with([](utf8_char ch) constexpr noexcept { return ch == u8"\u00E9"_u8c; }));
	static_assert(!u8""_utf8_sv.starts_with([](utf8_char) constexpr noexcept { return true; }));
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
		constexpr auto empty = u8""_utf8_sv;
		constexpr auto text = u8"a--b--"_utf8_sv;
		constexpr auto leading = u8"--abra"_utf8_sv;
		constexpr auto unicode = u8"A\u00E9B\u00E9"_utf8_sv;
		return std::ranges::equal(empty.split_inclusive(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			})
			&& std::ranges::equal(text.split_inclusive(u8"--"_utf8_sv), std::array{
				u8"a--"_utf8_sv,
				u8"b--"_utf8_sv
			})
			&& std::ranges::equal(leading.split_inclusive(u8"--"_utf8_sv), std::array{
				u8"--"_utf8_sv,
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(u8"abra"_utf8_sv.split_inclusive(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			})
			&& std::ranges::equal(unicode.matches(u8"\u00E9"_u8c), std::array{
				u8"\u00E9"_utf8_sv,
				u8"\u00E9"_utf8_sv
			})
			&& std::ranges::equal(u8"aaaa"_utf8_sv.matches(u8"aa"_utf8_sv), std::array{
				u8"aa"_utf8_sv,
				u8"aa"_utf8_sv
			})
			&& std::ranges::equal(u8"aaaa"_utf8_sv.matches(u8""_utf8_sv), std::array<utf8_string_view, 0>{})
			&& std::ranges::equal(u8"aaaa"_utf8_sv.rmatch_indices(u8"aa"_utf8_sv), std::array{
				std::pair<std::size_t, utf8_string_view>{ 2, u8"aa"_utf8_sv },
				std::pair<std::size_t, utf8_string_view>{ 0, u8"aa"_utf8_sv }
			})
			&& std::ranges::equal(u8"abra"_utf8_sv.rmatch_indices(u8""_utf8_sv), std::array<std::pair<std::size_t, utf8_string_view>, 0>{});
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
	static_assert(utf16_text.char_count() == 3);
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
	static_assert(utf16_text.starts_with([](utf16_char ch) constexpr noexcept { return ch == u"A"_u16c; }));
	static_assert(!utf16_text.starts_with(u"é"_u16c));
	static_assert(!utf16_text.starts_with([](utf16_char ch) constexpr noexcept { return ch == u"\u00E9"_u16c; }));
	static_assert(!u""_utf16_sv.starts_with([](utf16_char) constexpr noexcept { return true; }));
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
		constexpr auto empty = u""_utf16_sv;
		constexpr auto text = u"a--b--"_utf16_sv;
		constexpr auto leading = u"--abra"_utf16_sv;
		constexpr auto unicode = u"A\u00E9B\u00E9"_utf16_sv;
		return std::ranges::equal(empty.split_inclusive(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			})
			&& std::ranges::equal(text.split_inclusive(u"--"_utf16_sv), std::array{
				u"a--"_utf16_sv,
				u"b--"_utf16_sv
			})
			&& std::ranges::equal(leading.split_inclusive(u"--"_utf16_sv), std::array{
				u"--"_utf16_sv,
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(u"abra"_utf16_sv.split_inclusive(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			})
			&& std::ranges::equal(unicode.matches(u"\u00E9"_u16c), std::array{
				u"\u00E9"_utf16_sv,
				u"\u00E9"_utf16_sv
			})
			&& std::ranges::equal(u"aaaa"_utf16_sv.matches(u"aa"_utf16_sv), std::array{
				u"aa"_utf16_sv,
				u"aa"_utf16_sv
			})
			&& std::ranges::equal(u"aaaa"_utf16_sv.matches(u""_utf16_sv), std::array<utf16_string_view, 0>{})
			&& std::ranges::equal(u"aaaa"_utf16_sv.rmatch_indices(u"aa"_utf16_sv), std::array{
				std::pair<std::size_t, utf16_string_view>{ 2, u"aa"_utf16_sv },
				std::pair<std::size_t, utf16_string_view>{ 0, u"aa"_utf16_sv }
			})
			&& std::ranges::equal(u"abra"_utf16_sv.rmatch_indices(u""_utf16_sv), std::array<std::pair<std::size_t, utf16_string_view>, 0>{});
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
	static_assert(u8"\u00E9"_utf8_sv.to_nfd() == u8"e\u0301"_utf8_sv);
	static_assert(u8"e\u0301"_utf8_sv.to_nfc() == u8"\u00E9"_utf8_sv);
	static_assert(u8"\uFF21"_utf8_sv.to_nfkc() == u8"A"_utf8_sv);
	static_assert(u8"\u00E9"_utf8_sv.is_nfc());
	static_assert(u8"e\u0301"_utf8_sv.is_nfd());
	static_assert(u8"Straße"_utf8_sv.case_fold() == u8"strasse"_utf8_sv);
	static_assert(std::same_as<decltype(u8""_utf8_sv.compare_ignore_case(u8""_utf8_sv)), std::weak_ordering>);
	static_assert(u8"Straße"_utf8_sv.eq_ignore_case(u8"STRASSE"_utf8_sv));
	static_assert(u8"Straße"_utf8_sv.starts_with_ignore_case(u8"stras"_utf8_sv));
	static_assert(u8"Straße"_utf8_sv.ends_with_ignore_case(u8"SSE"_utf8_sv));
	static_assert(u8"Straße"_utf8_sv.compare_ignore_case(u8"strasse"_utf8_sv) == std::weak_ordering::equivalent);
	static_assert(u8"Cafe"_utf8_sv.compare_ignore_case(u8"cafg"_utf8_sv) == std::weak_ordering::less);
	static_assert(!u8"\u00E9"_utf8_sv.eq_ignore_case(u8"e\u0301"_utf8_sv));
	static_assert(u8"\u00E9"_utf8_sv.compare_ignore_case(u8"e\u0301"_utf8_sv) == std::weak_ordering::greater);
	static_assert(u"\u00E9"_utf16_sv.to_nfd() == u"e\u0301"_utf16_sv);
	static_assert(u"e\u0301"_utf16_sv.to_nfc() == u"\u00E9"_utf16_sv);
	static_assert(u"\uFF21"_utf16_sv.to_nfkc() == u"A"_utf16_sv);
	static_assert(u"\u00E9"_utf16_sv.is_nfc());
	static_assert(u"e\u0301"_utf16_sv.is_nfd());
	static_assert(u"Straße"_utf16_sv.case_fold() == u"strasse"_utf16_sv);
	static_assert(std::same_as<decltype(u""_utf16_sv.compare_ignore_case(u""_utf16_sv)), std::weak_ordering>);
	static_assert(u"Straße"_utf16_sv.eq_ignore_case(u"STRASSE"_utf16_sv));
	static_assert(u"Straße"_utf16_sv.starts_with_ignore_case(u"stras"_utf16_sv));
	static_assert(u"Straße"_utf16_sv.ends_with_ignore_case(u"SSE"_utf16_sv));
	static_assert(u"Straße"_utf16_sv.compare_ignore_case(u"strasse"_utf16_sv) == std::weak_ordering::equivalent);
	static_assert(u"Cafe"_utf16_sv.compare_ignore_case(u"cafg"_utf16_sv) == std::weak_ordering::less);
	static_assert(!u"\u00E9"_utf16_sv.eq_ignore_case(u"e\u0301"_utf16_sv));
	static_assert(u"\u00E9"_utf16_sv.compare_ignore_case(u"e\u0301"_utf16_sv) == std::weak_ordering::greater);

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
	static_assert(utf16_char::from_scalar_unchecked(0x1F600u).as_scalar() == 0x1F600u);
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
	static_assert(u"A"_u16c.general_category() == unicode_general_category::uppercase_letter);
	static_assert(u" "_u16c.general_category() == unicode_general_category::space_separator);
	static_assert(u"\u20AC"_u16c.general_category() == unicode_general_category::currency_symbol);
	static_assert(utf16_char::from_scalar_unchecked(0x0378u).general_category() == unicode_general_category::unassigned);
	static_assert(u"\u0301"_u16c.canonical_combining_class() == 230u);
	static_assert(u"\u0301"_u16c.grapheme_break_property() == unicode_grapheme_break_property::extend);
	static_assert(u"\u03A9"_u16c.script() == unicode_script::greek);
	static_assert(u"!"_u16c.script() == unicode_script::common);
	static_assert(utf16_char::from_scalar_unchecked(0x0378u).script() == unicode_script::unknown);
	static_assert(u"A"_u16c.east_asian_width() == unicode_east_asian_width::narrow);
	static_assert(u"\u754C"_u16c.east_asian_width() == unicode_east_asian_width::wide);
	static_assert(u"\u03A9"_u16c.east_asian_width() == unicode_east_asian_width::ambiguous);
	static_assert(u"A"_u16c.line_break_class() == unicode_line_break_class::alphabetic);
	static_assert(u" "_u16c.line_break_class() == unicode_line_break_class::space);
	static_assert(u"A"_u16c.bidi_class() == unicode_bidi_class::left_to_right);
	static_assert(u"\u0634"_u16c.bidi_class() == unicode_bidi_class::arabic_letter);
	static_assert(u"A"_u16c.word_break_property() == unicode_word_break_property::a_letter);
	static_assert(u"7"_u16c.word_break_property() == unicode_word_break_property::numeric);
	static_assert(u"A"_u16c.sentence_break_property() == unicode_sentence_break_property::upper);
	static_assert(u"."_u16c.sentence_break_property() == unicode_sentence_break_property::a_term);
	static_assert(u"\U0001F600"_u16c.is_emoji());
	static_assert(!u"A"_u16c.is_emoji());
	static_assert(u"\U0001F600"_u16c.is_emoji_presentation());
	static_assert(!u"A"_u16c.is_emoji_presentation());
	static_assert(u"\U0001F600"_u16c.is_extended_pictographic());
	static_assert(!u"A"_u16c.is_extended_pictographic());
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
	assert((std::same_as<
		pmr::utf32_string,
		basic_utf32_string<std::pmr::polymorphic_allocator<char32_t>>>));
	assert((unicode_character<utf8_char>));
	assert((unicode_character<const utf8_char&>));
	assert((unicode_character<utf16_char>));
	assert((unicode_character<utf16_char&&>));
	assert((unicode_character<utf32_char>));
	assert((unicode_character<utf32_char&&>));
	assert((!unicode_character<char8_t>));
	assert((!unicode_character<char16_t>));
	assert((!unicode_character<char32_t>));

	assert((std::ranges::view<views::utf8_view>));
	assert((std::ranges::range<views::utf8_view>));
	assert((std::ranges::view<views::reversed_utf8_view>));
	assert((std::ranges::range<views::reversed_utf8_view>));
	assert((std::ranges::view<views::utf16_view>));
	assert((std::ranges::range<views::utf16_view>));
	assert((std::ranges::view<views::reversed_utf16_view>));
	assert((std::ranges::range<views::reversed_utf16_view>));
	assert((std::ranges::view<views::utf32_view>));
	assert((std::ranges::range<views::utf32_view>));
	assert((std::ranges::sized_range<views::utf32_view>));
	assert((std::ranges::common_range<views::utf32_view>));
	assert((std::ranges::bidirectional_range<views::utf32_view>));
	assert((std::ranges::random_access_range<views::utf32_view>));
	assert((std::ranges::view<views::reversed_utf32_view>));
	assert((std::ranges::range<views::reversed_utf32_view>));
	assert((std::ranges::sized_range<views::reversed_utf32_view>));
	assert((std::ranges::common_range<views::reversed_utf32_view>));
	assert((std::ranges::bidirectional_range<views::reversed_utf32_view>));
	assert((std::ranges::random_access_range<views::reversed_utf32_view>));
	assert((std::ranges::view<views::grapheme_cluster_view<char8_t>>));
	assert((std::ranges::range<views::grapheme_cluster_view<char8_t>>));
	assert((std::ranges::view<views::grapheme_cluster_view<char16_t>>));
	assert((std::ranges::range<views::grapheme_cluster_view<char16_t>>));
	assert((std::ranges::view<views::grapheme_cluster_view<char32_t>>));
	assert((std::ranges::range<views::grapheme_cluster_view<char32_t>>));
	assert((std::ranges::view<views::lossy_utf8_view<char>>));
	assert((std::ranges::range<views::lossy_utf8_view<char>>));
	assert((std::ranges::view<views::lossy_utf8_view<char8_t>>));
	assert((std::ranges::range<views::lossy_utf8_view<char8_t>>));
	assert((std::ranges::view<views::lossy_utf16_view<char16_t>>));
	assert((std::ranges::range<views::lossy_utf16_view<char16_t>>));
	assert((std::ranges::view<views::lossy_utf16_view<wchar_t>>));
	assert((std::ranges::range<views::lossy_utf16_view<wchar_t>>));
	assert((std::ranges::view<views::lossy_utf32_view<char32_t>>));
	assert((std::ranges::range<views::lossy_utf32_view<char32_t>>));
	assert((std::ranges::sized_range<views::lossy_utf32_view<char32_t>>));
	assert((std::ranges::common_range<views::lossy_utf32_view<char32_t>>));
	assert((std::ranges::bidirectional_range<views::lossy_utf32_view<char32_t>>));
	assert((std::ranges::random_access_range<views::lossy_utf32_view<char32_t>>));

	{
		[[maybe_unused]] const auto runtime_latin1_ch = utf8_char::from_scalar_unchecked(0x00E9u);
		const std::u8string runtime_utf8_storage = u8"A\u00E9\u20AC";
		const auto runtime_utf8_text = unwrap_utf8_view(runtime_utf8_storage);
		const std::u16string runtime_utf16_storage = u"A\u00E9\U0001F600";
		const auto runtime_utf16_text = unwrap_utf16_view(runtime_utf16_storage);
		const std::u32string runtime_utf32_storage = U"A\u00E9\U0001F600";
		const auto runtime_utf32_text = unwrap_utf32_view(runtime_utf32_storage);

		assert((std::ranges::view<decltype(runtime_utf8_text.chars())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.chars())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.reversed_chars())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.reversed_chars())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.char_indices())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.char_indices())>));
		assert((std::ranges::view<decltype(runtime_utf8_text.graphemes())>));
		assert((std::ranges::range<decltype(runtime_utf8_text.graphemes())>));
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
		assert((std::ranges::view<decltype(runtime_utf16_text.grapheme_indices())>));
		assert((std::ranges::range<decltype(runtime_utf16_text.grapheme_indices())>));

		assert((std::ranges::view<decltype(runtime_utf32_text.chars())>));
		assert((std::ranges::range<decltype(runtime_utf32_text.chars())>));
		assert((std::ranges::sized_range<decltype(runtime_utf32_text.chars())>));
		assert((std::ranges::common_range<decltype(runtime_utf32_text.chars())>));
		assert((std::ranges::bidirectional_range<decltype(runtime_utf32_text.chars())>));
		assert((std::ranges::random_access_range<decltype(runtime_utf32_text.chars())>));
		assert((std::ranges::view<decltype(runtime_utf32_text.reversed_chars())>));
		assert((std::ranges::range<decltype(runtime_utf32_text.reversed_chars())>));
		assert((std::ranges::sized_range<decltype(runtime_utf32_text.reversed_chars())>));
		assert((std::ranges::common_range<decltype(runtime_utf32_text.reversed_chars())>));
		assert((std::ranges::bidirectional_range<decltype(runtime_utf32_text.reversed_chars())>));
		assert((std::ranges::random_access_range<decltype(runtime_utf32_text.reversed_chars())>));
		assert((std::ranges::view<decltype(runtime_utf32_text.char_indices())>));
		assert((std::ranges::range<decltype(runtime_utf32_text.char_indices())>));
		assert((std::ranges::sized_range<decltype(runtime_utf32_text.char_indices())>));
		assert((std::ranges::common_range<decltype(runtime_utf32_text.char_indices())>));
		assert((std::ranges::bidirectional_range<decltype(runtime_utf32_text.char_indices())>));
		assert((std::ranges::random_access_range<decltype(runtime_utf32_text.char_indices())>));
		assert((std::ranges::view<decltype(runtime_utf32_text.graphemes())>));
		assert((std::ranges::range<decltype(runtime_utf32_text.graphemes())>));
		assert((std::ranges::view<decltype(runtime_utf32_text.grapheme_indices())>));
		assert((std::ranges::range<decltype(runtime_utf32_text.grapheme_indices())>));
		assert(runtime_utf32_text.chars().size() == runtime_utf32_text.size());
		assert(runtime_utf32_text.reversed_chars().size() == runtime_utf32_text.size());
		assert(runtime_utf32_text.char_indices().size() == runtime_utf32_text.size());
		assert(runtime_utf32_text.chars().begin()[1] == U"\u00E9"_u32c);
		assert(runtime_utf32_text.reversed_chars().begin()[0] == U"\U0001F600"_u32c);
		assert(runtime_utf32_text.char_indices().begin()[2].first == 2);
		assert(runtime_utf32_text.char_indices().begin()[2].second == U"\U0001F600"_u32c);

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
		assert(u8"A"_u8c.general_category() == unicode_general_category::uppercase_letter);
		assert(u8" "_u8c.general_category() == unicode_general_category::space_separator);
		assert(u8"\u20AC"_u8c.general_category() == unicode_general_category::currency_symbol);
		assert(utf8_char::from_scalar_unchecked(0x0378u).general_category() == unicode_general_category::unassigned);
		assert(u8"\u0301"_u8c.canonical_combining_class() == 230u);
		assert(u8"\u0301"_u8c.grapheme_break_property() == unicode_grapheme_break_property::extend);
		assert(u8"\u03A9"_u8c.script() == unicode_script::greek);
		assert(u8"!"_u8c.script() == unicode_script::common);
		assert(utf8_char::from_scalar_unchecked(0x0378u).script() == unicode_script::unknown);
		assert(u8"A"_u8c.east_asian_width() == unicode_east_asian_width::narrow);
		assert(u8"\u754C"_u8c.east_asian_width() == unicode_east_asian_width::wide);
		assert(u8"\u03A9"_u8c.east_asian_width() == unicode_east_asian_width::ambiguous);
		assert(u8"A"_u8c.line_break_class() == unicode_line_break_class::alphabetic);
		assert(u8" "_u8c.line_break_class() == unicode_line_break_class::space);
		assert(u8"A"_u8c.bidi_class() == unicode_bidi_class::left_to_right);
		assert(u8"\u0634"_u8c.bidi_class() == unicode_bidi_class::arabic_letter);
		assert(u8"A"_u8c.word_break_property() == unicode_word_break_property::a_letter);
		assert(u8"7"_u8c.word_break_property() == unicode_word_break_property::numeric);
		assert(u8"A"_u8c.sentence_break_property() == unicode_sentence_break_property::upper);
		assert(u8"."_u8c.sentence_break_property() == unicode_sentence_break_property::a_term);
		assert(u8"\U0001F600"_u8c.is_emoji());
		assert(!u8"A"_u8c.is_emoji());
		assert(u8"\U0001F600"_u8c.is_emoji_presentation());
		assert(!u8"A"_u8c.is_emoji_presentation());
		assert(u8"\U0001F600"_u8c.is_extended_pictographic());
		assert(!u8"A"_u8c.is_extended_pictographic());
		assert(utf8_char::from_scalar_unchecked(0x0085u).is_control());
		assert(u8"F"_u8c.is_ascii_hexdigit());
		assert(u8"7"_u8c.is_ascii_octdigit());
		assert(u8"!"_u8c.is_ascii_punctuation());
		assert(u8"A"_u8c.is_ascii_graphic());
		assert(!u8" "_u8c.is_ascii_graphic());
		assert(u8"\n"_u8c.is_ascii_control());

		assert(U"A"_u32c.ascii_lowercase() == U"a"_u32c);
		assert(U"z"_u32c.ascii_uppercase() == U"Z"_u32c);
		assert(U"A"_u32c.eq_ignore_ascii_case(U"a"_u32c));
		assert(utf32_char::from_scalar(0x20ACu).has_value());
		assert(!utf32_char::from_scalar(0x110000u).has_value());
		{
			const auto euro = utf32_char::from_scalar(0x20ACu);
			assert(euro.has_value());
			assert(euro->as_scalar() == 0x20ACu);
			assert(std::hash<utf32_char>{}(*euro) == std::hash<std::u32string_view>{}(U"\u20AC"_utf32_sv.base()));

			const std::array<char32_t, 1> euro_code_points{ U'\u20AC' };
			assert(utf32_char::from_utf32_code_points(euro_code_points.data(), euro_code_points.size()).value() == U"\u20AC"_u32c);
			assert(utf32_char::from_utf32_code_points_unchecked(euro_code_points.data(), euro_code_points.size()) == U"\u20AC"_u32c);

			const std::array<char32_t, 1> invalid_code_points{ static_cast<char32_t>(0xD800u) };
			assert(!utf32_char::from_utf32_code_points(invalid_code_points.data(), invalid_code_points.size()).has_value());
		}
		assert(U"\u03A9"_u32c.is_alphabetic());
		assert(U"\u03A9"_u32c.is_alphanumeric());
		assert(U"\u03A9"_u32c.is_uppercase());
		assert(U"\u03C9"_u32c.is_lowercase());
		assert(U"5"_u32c.is_digit());
		assert(U"\u2167"_u32c.is_numeric());
		assert(U"\u2003"_u32c.is_whitespace());
		assert(U"A"_u32c.is_ascii_alphabetic());
		assert(U"7"_u32c.is_ascii_digit());
		assert(U"7"_u32c.is_ascii_alphanumeric());
		assert(U"Q"_u32c.is_ascii_alphanumeric());
		assert(!U"-"_u32c.is_ascii_alphanumeric());
		assert(U" "_u32c.is_ascii_whitespace());
		assert(U"\n"_u32c.is_ascii_whitespace());
		assert(!U"A"_u32c.is_ascii_whitespace());
		assert(U"A"_u32c.general_category() == unicode_general_category::uppercase_letter);
		assert(U" "_u32c.general_category() == unicode_general_category::space_separator);
		assert(U"\u20AC"_u32c.general_category() == unicode_general_category::currency_symbol);
		assert(utf32_char::from_scalar_unchecked(0x0378u).general_category() == unicode_general_category::unassigned);
		assert(U"\u0301"_u32c.canonical_combining_class() == 230u);
		assert(U"\u0301"_u32c.grapheme_break_property() == unicode_grapheme_break_property::extend);
		assert(U"\u03A9"_u32c.script() == unicode_script::greek);
		assert(U"!"_u32c.script() == unicode_script::common);
		assert(utf32_char::from_scalar_unchecked(0x0378u).script() == unicode_script::unknown);
		assert(U"A"_u32c.east_asian_width() == unicode_east_asian_width::narrow);
		assert(U"\u754C"_u32c.east_asian_width() == unicode_east_asian_width::wide);
		assert(U"\u03A9"_u32c.east_asian_width() == unicode_east_asian_width::ambiguous);
		assert(U"A"_u32c.line_break_class() == unicode_line_break_class::alphabetic);
		assert(U" "_u32c.line_break_class() == unicode_line_break_class::space);
		assert(U"A"_u32c.bidi_class() == unicode_bidi_class::left_to_right);
		assert(U"\u0634"_u32c.bidi_class() == unicode_bidi_class::arabic_letter);
		assert(U"A"_u32c.word_break_property() == unicode_word_break_property::a_letter);
		assert(U"7"_u32c.word_break_property() == unicode_word_break_property::numeric);
		assert(U"A"_u32c.sentence_break_property() == unicode_sentence_break_property::upper);
		assert(U"."_u32c.sentence_break_property() == unicode_sentence_break_property::a_term);
		assert(U"\U0001F600"_u32c.is_emoji());
		assert(!U"A"_u32c.is_emoji());
		assert(U"\U0001F600"_u32c.is_emoji_presentation());
		assert(!U"A"_u32c.is_emoji_presentation());
		assert(U"\U0001F600"_u32c.is_extended_pictographic());
		assert(!U"A"_u32c.is_extended_pictographic());
		assert(utf32_char::from_scalar_unchecked(0x0085u).is_control());
		assert(U"F"_u32c.is_ascii_hexdigit());
		assert(U"7"_u32c.is_ascii_octdigit());
		assert(U"!"_u32c.is_ascii_punctuation());
		assert(U"A"_u32c.is_ascii_graphic());
		assert(!U" "_u32c.is_ascii_graphic());
		assert(U"\n"_u32c.is_ascii_control());
		{
			utf32_char lhs = U"A"_u32c;
			utf32_char rhs = U"z"_u32c;
			lhs.swap(rhs);
			assert(lhs == U"z"_u32c);
			assert(rhs == U"A"_u32c);

			auto old = lhs++;
			assert(old == U"z"_u32c);
			assert(lhs == U"{"_u32c);

			old = lhs--;
			assert(old == U"{"_u32c);
			assert(lhs == U"z"_u32c);

			auto wrapped = utf32_char::from_scalar_unchecked(details::encoding_constants::max_unicode_scalar);
			++wrapped;
			assert(wrapped == utf32_char::null_terminator);

			auto rewound = utf32_char::null_terminator;
			--rewound;
			assert(rewound == utf32_char::from_scalar_unchecked(details::encoding_constants::max_unicode_scalar));
		}

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
		assert(runtime_utf8_text.char_count() == 3);
		assert(runtime_utf8_text == u8"A\u00E9\u20AC"_utf8_sv);
		assert(runtime_utf8_text.as_view() == runtime_utf8_text.base());
		assert(static_cast<std::u8string_view>(runtime_utf8_text) == runtime_utf8_text.base());
		assert((runtime_utf8_text <=> u8"B"_utf8_sv) == std::strong_ordering::less);
		{
			const std::array<char8_t, 2> invalid_bytes{ static_cast<char8_t>('A'), static_cast<char8_t>(0xFFu) };
			assert(!utf8_string_view::from_bytes(std::u8string_view{ invalid_bytes.data(), invalid_bytes.size() }).has_value());
		}
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
		assert(runtime_utf8_text.starts_with([](utf8_char ch) constexpr noexcept { return ch == u8"A"_u8c; }));
		assert(!runtime_utf8_text.starts_with(u8"\u00E9"_u8c));
		assert(!runtime_utf8_text.starts_with([](utf8_char ch) constexpr noexcept { return ch == u8"\u00E9"_u8c; }));
		assert(!u8""_utf8_sv.starts_with([](utf8_char) constexpr noexcept { return true; }));
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
		assert(u8"e\u0301"_grapheme_utf8 == u8"e\u0301"_utf8_sv);
		{
			const std::u8string text_storage = u8"A\u00E9\U0001F600";
			[[maybe_unused]] const auto text = unwrap_utf8_view(text_storage);
			assert(text.to_utf16() == u"A\u00E9\U0001F600"_utf16_sv);
			assert(text.to_utf8_owned() == u8"A\u00E9\U0001F600"_utf8_sv);
			assert(u8"\u00E9"_u8c.to_utf8_owned() == u8"\u00E9"_utf8_sv);
		}
		{
			const std::u8string ascii_storage = u8"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789";
			const std::string ascii_bytes = "AbCdEfGhIjKlMnOpQrStUvWxYz0123456789";
			const std::string mixed_bytes = "ASCII\xC3\xA9";
			assert(details::ascii_prefix_length(std::u8string_view{ ascii_storage }) == ascii_storage.size());
			assert(details::ascii_prefix_length(std::string_view{ ascii_bytes }) == ascii_bytes.size());
			assert(details::ascii_prefix_length(std::string_view{ mixed_bytes }) == 5);

			std::u8string lowered(ascii_storage.size(), u8'\0');
			std::u8string uppered(ascii_storage.size(), u8'\0');
			std::u16string widened(ascii_storage.size(), u'\0');
			assert(details::ascii_lowercase_copy(lowered.data(), std::u8string_view{ ascii_storage }));
			assert(lowered == u8"abcdefghijklmnopqrstuvwxyz0123456789");
			assert(details::ascii_uppercase_copy(uppered.data(), std::u8string_view{ ascii_storage }));
			assert(uppered == u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
			details::copy_ascii_utf8_to_utf16(widened.data(), std::u8string_view{ ascii_storage });
			assert(widened == u"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789");

			[[maybe_unused]] const auto validated = details::validate_utf8(std::string_view{ ascii_bytes });
			assert(validated.has_value());
			const auto copied = details::copy_validated_utf8_bytes(ascii_bytes, std::allocator<char8_t>{});
			assert(copied.has_value());
			const auto copied_view = std::u8string_view{ copied->data(), copied->size() };
			assert(copied_view == std::u8string_view{ ascii_storage });
			const auto transcoded = details::transcode_utf8_to_utf16_checked(ascii_bytes, std::allocator<char16_t>{});
			assert(transcoded.has_value());
			const auto transcoded_view = std::u16string_view{ transcoded->data(), transcoded->size() };
			assert(transcoded_view == u"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789");
		}
		{
			assert(u8"AbC-\u00E9\u00DF"_utf8_sv.to_ascii_lowercase() == u8"abc-\u00E9\u00DF"_utf8_sv);
			assert(u8"AbC-\u00E9\u00DF"_utf8_sv.to_ascii_lowercase(0, 3) == u8"abc-\u00E9\u00DF"_utf8_sv);
			assert(u8"aBc-\u00E9\u00DF"_utf8_sv.to_ascii_uppercase() == u8"ABC-\u00E9\u00DF"_utf8_sv);
			assert(u8"aBc-\u00E9\u00DF"_utf8_sv.to_ascii_uppercase(0, 3) == u8"ABC-\u00E9\u00DF"_utf8_sv);
			assert(u8"\u00C4\u03A9\u0130"_utf8_sv.to_lowercase() == u8"\u00E4\u03C9i\u0307"_utf8_sv);
			assert(u8"XX\u00C4\u03A9YY"_utf8_sv.to_lowercase(2, 4) == u8"XX\u00E4\u03C9YY"_utf8_sv);
			assert(u8"\u00E4\u00DF\u03C9"_utf8_sv.to_uppercase() == u8"\u00C4SS\u03A9"_utf8_sv);
			assert(u8"ab\u00E4\u00DFcd"_utf8_sv.to_uppercase(2, 4) == u8"ab\u00C4SScd"_utf8_sv);
			assert(u8"\u00E9"_utf8_sv.to_nfd() == u8"e\u0301"_utf8_sv);
			assert(u8"e\u0301"_utf8_sv.to_nfc() == u8"\u00E9"_utf8_sv);
			assert(u8"\uFF21"_utf8_sv.to_nfkc() == u8"A"_utf8_sv);
			assert(u8"A"_utf8_sv.to_nfkd() == u8"A"_utf8_sv);
			assert(u8"\u00E9"_utf8_sv.is_nfc());
			assert(!u8"\u00E9"_utf8_sv.is_nfd());
			assert(u8"e\u0301"_utf8_sv.is_nfd());
			assert(u8"Straße"_utf8_sv.case_fold() == u8"strasse"_utf8_sv);
			assert(u8"Straße"_utf8_sv.eq_ignore_case(u8"STRASSE"_utf8_sv));
			assert(u8"Straße"_utf8_sv.starts_with_ignore_case(u8"stras"_utf8_sv));
			assert(u8"Straße"_utf8_sv.ends_with_ignore_case(u8"SSE"_utf8_sv));
			assert(u8"Straße"_utf8_sv.compare_ignore_case(u8"strasse"_utf8_sv) == std::weak_ordering::equivalent);
			assert(u8"Cafe"_utf8_sv.compare_ignore_case(u8"cafg"_utf8_sv) == std::weak_ordering::less);
			assert(!u8"\u00E9"_utf8_sv.eq_ignore_case(u8"e\u0301"_utf8_sv));
			[[maybe_unused]] auto ascii_lowered_owned = utf8_string{ u8"AbC-\u00E9\u00DF"_utf8_sv }.to_ascii_lowercase();
			assert(ascii_lowered_owned == u8"abc-\u00E9\u00DF"_utf8_sv);
			auto partial_ascii_lower_owned = u8"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789-\u00E9\u00DF"_utf8_s;
			[[maybe_unused]] auto partial_ascii_lowered = std::move(partial_ascii_lower_owned).to_ascii_lowercase(0, 26);
			assert(partial_ascii_lowered == u8"abcdefghijklmnopqrstuvwxyz0123456789-\u00E9\u00DF"_utf8_sv);
			auto ascii_lower_owned = u8"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789AbCdEfGhIjKlMnOpQrSt"_utf8_s;
			[[maybe_unused]] const auto* ascii_lower_original = ascii_lower_owned.base().data();
			[[maybe_unused]] auto lowered_in_place = std::move(ascii_lower_owned).to_lowercase();
			assert(lowered_in_place == u8"abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrst"_utf8_sv);
			assert(lowered_in_place.base().data() == ascii_lower_original);
			auto partial_lower_owned = u8"XX\u00C4\u03A9YY"_utf8_s;
			[[maybe_unused]] auto partial_lowered = std::move(partial_lower_owned).to_lowercase(2, 4);
			assert(partial_lowered == u8"XX\u00E4\u03C9YY"_utf8_sv);
			auto ascii_upper_owned = u8"aBcDeFgHiJkLmNoPqRsTuVwXyZ0123456789aBcDeFgHiJkLmNoPqRsT"_utf8_s;
			[[maybe_unused]] const auto* ascii_upper_original = ascii_upper_owned.base().data();
			[[maybe_unused]] auto uppered_in_place = std::move(ascii_upper_owned).to_uppercase();
			assert(uppered_in_place == u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRST"_utf8_sv);
			assert(uppered_in_place.base().data() == ascii_upper_original);
			auto partial_upper_owned = u8"ab\u00E4\u00DFcd"_utf8_s;
			[[maybe_unused]] auto partial_uppered = std::move(partial_upper_owned).to_uppercase(2, 4);
			assert(partial_uppered == u8"ab\u00C4SScd"_utf8_sv);
			std::pmr::monotonic_buffer_resource resource;
			const auto alloc = std::pmr::polymorphic_allocator<char8_t>{ &resource };
			[[maybe_unused]] const auto lowered_alloc = u8"\u00C4\u03A9\u0130"_utf8_sv.to_lowercase(alloc);
			assert(lowered_alloc == u8"\u00E4\u03C9i\u0307"_utf8_sv);
			assert(lowered_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto partial_uppered_alloc = u8"ab\u00E4\u00DFcd"_utf8_sv.to_uppercase(2, 4, alloc);
			assert(partial_uppered_alloc == u8"ab\u00C4SScd"_utf8_sv);
			assert(partial_uppered_alloc.get_allocator().resource() == &resource);
#if UTF8_RANGES_HAS_ICU
			assert(u8"I\u0130"_utf8_sv.to_lowercase("tr"_locale) == u8"\u0131i"_utf8_sv);
			assert(u8"XXI\u0130YY"_utf8_sv.to_lowercase(2, 3, "tr"_locale) == u8"XX\u0131iYY"_utf8_sv);
			[[maybe_unused]] const auto lowered_locale_alloc = u8"I\u0130"_utf8_sv.to_lowercase("tr"_locale, alloc);
			assert(lowered_locale_alloc == u8"\u0131i"_utf8_sv);
			assert(lowered_locale_alloc.get_allocator().resource() == &resource);
			assert(u8"i\u0131"_utf8_sv.to_uppercase("tr"_locale) == u8"\u0130I"_utf8_sv);
			assert(u8"XXi\u0131YY"_utf8_sv.to_uppercase(2, 3, "tr"_locale) == u8"XX\u0130IYY"_utf8_sv);
			[[maybe_unused]] const auto uppered_locale_alloc = u8"i\u0131"_utf8_sv.to_uppercase("tr"_locale, alloc);
			assert(uppered_locale_alloc == u8"\u0130I"_utf8_sv);
			assert(uppered_locale_alloc.get_allocator().resource() == &resource);
			assert(u8"istanbul izmir"_utf8_sv.to_titlecase("tr"_locale) == u8"\u0130stanbul \u0130zmir"_utf8_sv);
			[[maybe_unused]] const auto titled_locale_alloc = u8"istanbul izmir"_utf8_sv.to_titlecase("tr"_locale, alloc);
			assert(titled_locale_alloc == u8"\u0130stanbul \u0130zmir"_utf8_sv);
			assert(titled_locale_alloc.get_allocator().resource() == &resource);
			assert(u8"I\u0130"_utf8_sv.case_fold("tr"_locale) == u8"\u0131i"_utf8_sv);
			[[maybe_unused]] const auto folded_locale_alloc = u8"I\u0130"_utf8_sv.case_fold("tr"_locale, alloc);
			assert(folded_locale_alloc == u8"\u0131i"_utf8_sv);
			assert(folded_locale_alloc.get_allocator().resource() == &resource);
			assert(u8"I"_utf8_sv.eq_ignore_case(u8"\u0131"_utf8_sv, "tr"_locale));
			assert(u8"i"_utf8_sv.eq_ignore_case(u8"\u0130"_utf8_sv, "tr"_locale));
			assert(u8"Istanbul"_utf8_sv.starts_with_ignore_case(u8"\u0131s"_utf8_sv, "tr"_locale));
			assert(u8"kap\u0131I"_utf8_sv.ends_with_ignore_case(u8"\u0131"_utf8_sv, "tr"_locale));
			assert(u8"I"_utf8_sv.compare_ignore_case(u8"\u0131"_utf8_sv, "tr"_locale) == std::weak_ordering::equivalent);
			assert(!u8"I"_utf8_sv.eq_ignore_case(u8"\u0131"_utf8_sv));
			assert(is_available_locale("tr"_locale));
			assert(is_available_locale(locale_id{ "tr" }));
			assert(!is_available_locale("definitely_not_a_real_locale"_locale));
			assert(!is_available_locale(locale_id{ nullptr }));
#endif
			[[maybe_unused]] const auto normalized_alloc = u8"e\u0301"_utf8_sv.to_nfc(alloc);
			assert(normalized_alloc == u8"\u00E9"_utf8_sv);
			assert(normalized_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto folded_alloc = u8"Straße"_utf8_sv.case_fold(alloc);
			assert(folded_alloc == u8"strasse"_utf8_sv);
			assert(folded_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto pmr_ascii_lowered = pmr::utf8_string{ u8"AbC-\u00E9\u00DF"_utf8_sv, alloc }.to_ascii_lowercase();
			assert(pmr_ascii_lowered == u8"abc-\u00E9\u00DF"_utf8_sv);
			assert(pmr_ascii_lowered.get_allocator().resource() == &resource);
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
			auto chars = runtime_utf8_text.chars();
			assert(chars.base() == runtime_utf8_storage);
			assert(chars.reserve_hint() == runtime_utf8_storage.size());

			auto it = chars.begin();
			const auto copy = it;
			assert(it == copy);
			assert(!(it == chars.end()));
			assert(!(chars.end() == it));
			assert(*it == u8"A"_u8c);
			const auto post = it++;
			assert(*post == u8"A"_u8c);
			assert(*it == u8"\u00E9"_u8c);
			++it;
			assert(*it == u8"\u20AC"_u8c);
			++it;
			assert(it == chars.end());
			assert(chars.end() == it);
			assert(utf8_string_view{}.chars().begin() == utf8_string_view{}.chars().end());
		}
		{
			auto reversed = runtime_utf8_text.reversed_chars();
			assert(reversed.reserve_hint() == runtime_utf8_storage.size());

			auto it = reversed.begin();
			const auto copy = it;
			assert(it == copy);
			assert(!(it == reversed.end()));
			assert(!(reversed.end() == it));
			assert(*it == u8"\u20AC"_u8c);
			const auto post = it++;
			assert(*post == u8"\u20AC"_u8c);
			assert(*it == u8"\u00E9"_u8c);
			++it;
			assert(*it == u8"A"_u8c);
			++it;
			assert(it == reversed.end());
			assert(reversed.end() == it);
			assert(utf8_string_view{}.reversed_chars().begin() == utf8_string_view{}.reversed_chars().end());
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
			[[maybe_unused]] const auto empty = u8""_utf8_sv;
			[[maybe_unused]] const auto trimmed = u8"--a----b--"_utf8_sv;
			[[maybe_unused]] const auto text = u8"a--b--"_utf8_sv;
			[[maybe_unused]] const auto leading = u8"--abra"_utf8_sv;
			[[maybe_unused]] const auto unicode = u8"A\u00E9B\u00E9"_utf8_sv;
			[[maybe_unused]] constexpr auto ascii_space = [](utf8_char ch) constexpr noexcept
			{
				return ch.is_ascii_whitespace();
			};
			[[maybe_unused]] constexpr auto split_on_accent = [](utf8_char ch) constexpr noexcept
			{
				return ch == u8"\u00E9"_u8c;
			};
			assert(std::ranges::equal(empty.split_trimmed(u8"--"_utf8_sv), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(u8"abra"_utf8_sv.split_trimmed(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(leading.split_trimmed(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(u8"abra--"_utf8_sv.split_trimmed(u8"--"_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(trimmed.split_trimmed(u8"--"_utf8_sv), std::array{
				u8"a"_utf8_sv,
				u8"b"_utf8_sv
			}));
			assert(std::ranges::equal(u8"----"_utf8_sv.split_trimmed(u8"--"_utf8_sv), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(empty.split_trimmed(u8""_utf8_sv), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(u8"abra"_utf8_sv.split_trimmed(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(unicode.split_trimmed(u8"\u00E9"_u8c), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			}));
			assert(std::ranges::equal(u8" \tA \t B\t"_utf8_sv.split_trimmed(ascii_space), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			}));
			assert(std::ranges::equal(unicode.split_trimmed(split_on_accent), std::array{
				u8"A"_utf8_sv,
				u8"B"_utf8_sv
			}));
			assert(std::ranges::equal(empty.split_inclusive(u8"--"_utf8_sv), std::array{
				u8""_utf8_sv
			}));
			assert(std::ranges::equal(text.split_inclusive(u8"--"_utf8_sv), std::array{
				u8"a--"_utf8_sv,
				u8"b--"_utf8_sv
			}));
			assert(std::ranges::equal(leading.split_inclusive(u8"--"_utf8_sv), std::array{
				u8"--"_utf8_sv,
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(u8"abra"_utf8_sv.split_inclusive(u8""_utf8_sv), std::array{
				u8"abra"_utf8_sv
			}));
			assert(std::ranges::equal(unicode.matches(u8"\u00E9"_u8c), std::array{
				u8"\u00E9"_utf8_sv,
				u8"\u00E9"_utf8_sv
			}));
			assert(std::ranges::equal(u8"aaaa"_utf8_sv.matches(u8"aa"_utf8_sv), std::array{
				u8"aa"_utf8_sv,
				u8"aa"_utf8_sv
			}));
			assert(std::ranges::equal(u8"aaaa"_utf8_sv.matches(u8""_utf8_sv), std::array<utf8_string_view, 0>{}));
			assert(std::ranges::equal(u8"aaaa"_utf8_sv.rmatch_indices(u8"aa"_utf8_sv), std::array{
				std::pair<std::size_t, utf8_string_view>{ 2, u8"aa"_utf8_sv },
				std::pair<std::size_t, utf8_string_view>{ 0, u8"aa"_utf8_sv }
			}));
			assert(std::ranges::equal(u8"abra"_utf8_sv.rmatch_indices(u8""_utf8_sv), std::array<std::pair<std::size_t, utf8_string_view>, 0>{}));
			assert(u8"aaaa"_utf8_sv.replace_all(u8"aa"_utf8_sv, u8"x"_utf8_sv) == u8"xx"_utf8_sv);
			assert(u8"aaaa"_utf8_sv.replace_n(1, u8"aa"_utf8_sv, u8"x"_utf8_sv) == u8"xaa"_utf8_sv);
			assert(unicode.replace_all(u8"\u00E9"_u8c, u8"!"_u8c) == u8"A!B!"_utf8_sv);
			assert(unicode.replace_n(1, u8"\u00E9"_u8c, u8"!"_u8c) == u8"A!B\u00E9"_utf8_sv);
			assert(unicode.replace_all(u8""_utf8_sv, u8"!"_utf8_sv) == unicode);
			[[maybe_unused]] const auto long_needle_text = u8"prefixabcdefghijmiddleabcdefghijsuffix"_utf8_sv;
			assert(long_needle_text.find(u8"abcdefghij"_utf8_sv) == 6);
			assert(long_needle_text.rfind(u8"abcdefghij"_utf8_sv) == 22);
			assert(long_needle_text.rfind(u8"abcdefghij"_utf8_sv, 21) == 6);
			assert(long_needle_text.replace_all(u8"abcdefghij"_utf8_sv, u8"ABCDEFGHIJ"_utf8_sv)
				== u8"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix"_utf8_sv);
			assert(long_needle_text.replace_n(1, u8"abcdefghij"_utf8_sv, u8"ABCDEFGHIJ"_utf8_sv)
				== u8"prefixABCDEFGHIJmiddleabcdefghijsuffix"_utf8_sv);
			std::pmr::monotonic_buffer_resource resource;
			const auto replaced_alloc = unicode.replace_all(
				u8"\u00E9"_u8c,
				u8"!"_u8c,
				std::pmr::polymorphic_allocator<char8_t>{ &resource });
			assert(replaced_alloc == u8"A!B!"_utf8_sv);
			assert(replaced_alloc.get_allocator().resource() == &resource);
			const auto replaced_n_alloc = utf8_string{ unicode }.replace_n(
				1,
				u8"\u00E9"_u8c,
				u8"!"_u8c,
				std::pmr::polymorphic_allocator<char8_t>{ &resource });
			assert(replaced_n_alloc == u8"A!B\u00E9"_utf8_sv);
			assert(replaced_n_alloc.get_allocator().resource() == &resource);
		}
		{
			const std::array any_of{ u8"\u00E9"_u8c, u8"\u20AC"_u8c };
			const auto any = std::span{ any_of };
			const auto text = u8"\u00E9A\u20AC"_utf8_sv;
			assert(text.contains(any));
			assert(!u8"ABC"_utf8_sv.contains(any));
			assert(text.find(any) == 0);
			assert(text.rfind(any) == 3);
			assert(text.starts_with(any));
			assert(text.ends_with(any));
			assert(!u8"A"_utf8_sv.starts_with(any));
			assert(!u8"A"_utf8_sv.ends_with(any));
			assert(text.trim_start_matches(any) == u8"A\u20AC"_utf8_sv);
			assert(text.trim_end_matches(any) == u8"\u00E9A"_utf8_sv);
			assert(text.trim_matches(any) == u8"A"_utf8_sv);
			[[maybe_unused]] const auto first = text.split_once(any);
			assert(first.has_value());
			assert(first->first == u8""_utf8_sv);
			assert(first->second == u8"A\u20AC"_utf8_sv);
			[[maybe_unused]] const auto last = text.rsplit_once(any);
			assert(last.has_value());
			assert(last->first == u8"\u00E9A"_utf8_sv);
			assert(last->second == u8""_utf8_sv);
			assert(text.replace_all(any, u8"!"_u8c) == u8"!A!"_utf8_sv);
			assert(text.replace_n(1, any, u8"!"_u8c) == u8"!A\u20AC"_utf8_sv);
			[[maybe_unused]] constexpr auto is_ascii_digit = [](utf8_char ch) constexpr noexcept
			{
				return ch.is_ascii_digit();
			};
			assert(u8"123-456"_utf8_sv.replace_all(is_ascii_digit, u8"x"_u8c) == u8"xxx-xxx"_utf8_sv);
			assert(u8"123-456"_utf8_sv.replace_n(2, is_ascii_digit, u8"x"_u8c) == u8"xx3-456"_utf8_sv);
		}
		{
			const std::array any_of{
				u8"\u00E9"_u8c,
				u8"\u00DF"_u8c,
				u8"\u0103"_u8c,
				u8"\u0111"_u8c,
				u8"\u03C9"_u8c,
				u8"\u0416"_u8c,
				u8"\u05D0"_u8c,
				u8"\u20AC"_u8c
			};
			const auto any = std::span{ any_of };
			[[maybe_unused]] const auto text = u8"plain ascii words and \u03B1\u03B2\u03B3 \u20AC"_utf8_sv;
			assert(text.find(any) == text.size() - 3);
			assert(text.rfind(any) == text.size() - 3);
			assert(text.contains(any));
			assert(!u8"plain ascii words and \u03B1\u03B2\u03B3"_utf8_sv.contains(any));
		}
		{
			std::array<utf8_char, 17> overflow_any_of{};
			for (std::size_t i = 0; i != overflow_any_of.size(); ++i)
			{
				overflow_any_of[i] = utf8_char::from_scalar_unchecked(0x0100u + static_cast<std::uint32_t>(i));
			}
			const auto any = std::span{ overflow_any_of };
			[[maybe_unused]] const auto text = u8"\u0110A\u0100"_utf8_sv;
			assert(text.contains(any));
			assert(text.find(any) == 0);
			assert(text.rfind(any) == 3);
			assert(text.starts_with(any));
			assert(text.ends_with(any));
			assert(text.trim_matches(any) == u8"A"_utf8_sv);
			assert(text.replace_all(any, u8"!"_u8c) == u8"!A!"_utf8_sv);
			assert(!u8"ABC"_utf8_sv.contains(any));
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
		assert(runtime_utf16_text.char_count() == 3);
		assert(runtime_utf16_text == u"A\u00E9\U0001F600"_utf16_sv);
		assert(runtime_utf16_text.as_view() == runtime_utf16_text.base());
		assert(static_cast<std::u16string_view>(runtime_utf16_text) == runtime_utf16_text.base());
		assert((runtime_utf16_text <=> u"B"_utf16_sv) == std::strong_ordering::less);
		{
			const std::array<char16_t, 1> invalid_code_units{ static_cast<char16_t>(0xD800u) };
			assert(!utf16_string_view::from_code_units(std::u16string_view{ invalid_code_units.data(), invalid_code_units.size() }).has_value());
		}
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
		assert(runtime_utf16_text.starts_with([](utf16_char ch) constexpr noexcept { return ch == u"A"_u16c; }));
		assert(!runtime_utf16_text.starts_with(u"\u00E9"_u16c));
		assert(!runtime_utf16_text.starts_with([](utf16_char ch) constexpr noexcept { return ch == u"\u00E9"_u16c; }));
		assert(!u""_utf16_sv.starts_with([](utf16_char) constexpr noexcept { return true; }));
		assert(!runtime_utf16_text.ends_with(static_cast<char16_t>(u'A')));
		assert(runtime_utf16_text.ends_with(u"\U0001F600"_u16c));
		assert(runtime_utf16_text.ends_with(u"\U0001F600"_utf16_sv));
		assert(!runtime_utf16_text.ends_with(u"\u00E9"_u16c));
		assert(utf16_string_view::from_code_units(runtime_utf16_text.as_view()).has_value());
		assert(runtime_utf16_text == u"A\u00E9\U0001F600"_utf16_sv);
		assert(runtime_utf16_text < u"Z"_utf16_sv);

		assert(runtime_utf32_text.size() == 3);
		assert(runtime_utf32_text.char_count() == 3);
		assert(runtime_utf32_text.grapheme_count() == 3);
		assert(runtime_utf32_text == U"A\u00E9\U0001F600"_utf32_sv);
		assert(runtime_utf32_text.as_view() == runtime_utf32_text.base());
		assert(static_cast<std::u32string_view>(runtime_utf32_text) == runtime_utf32_text.base());
		assert((runtime_utf32_text <=> U"B"_utf32_sv) == std::strong_ordering::less);
		assert(utf32_string_view::from_code_points(runtime_utf32_text.base()).has_value());
		{
			const std::array<char32_t, 1> invalid_code_points{ static_cast<char32_t>(0xD800u) };
			assert(!utf32_string_view::from_code_points(std::u32string_view{ invalid_code_points.data(), invalid_code_points.size() }).has_value());
		}
		assert(runtime_utf32_text.is_char_boundary(0));
		assert(runtime_utf32_text.is_char_boundary(1));
		assert(runtime_utf32_text.is_char_boundary(2));
		assert(runtime_utf32_text.is_char_boundary(3));
		assert(runtime_utf32_text.ceil_char_boundary(0) == 0);
		assert(runtime_utf32_text.ceil_char_boundary(2) == 2);
		assert(runtime_utf32_text.ceil_char_boundary(utf32_string_view::npos) == runtime_utf32_text.size());
		assert(runtime_utf32_text.floor_char_boundary(0) == 0);
		assert(runtime_utf32_text.floor_char_boundary(2) == 2);
		assert(runtime_utf32_text.floor_char_boundary(utf32_string_view::npos) == runtime_utf32_text.size());
		assert(runtime_utf32_text.char_at(1).has_value());
		assert(runtime_utf32_text.char_at(1).value() == U"\u00E9"_u32c);
		assert(!runtime_utf32_text.char_at(runtime_utf32_text.size()).has_value());
		assert(runtime_utf32_text.char_at_unchecked(2) == U"\U0001F600"_u32c);
		assert(runtime_utf32_text.front().has_value());
		assert(runtime_utf32_text.front().value() == U"A"_u32c);
		assert(runtime_utf32_text.front_unchecked() == U"A"_u32c);
		assert(runtime_utf32_text.back().has_value());
		assert(runtime_utf32_text.back().value() == U"\U0001F600"_u32c);
		assert(runtime_utf32_text.back_unchecked() == U"\U0001F600"_u32c);
		{
			[[maybe_unused]] const utf32_string_view empty_text{};
			assert(!empty_text.front().has_value());
			assert(!empty_text.back().has_value());
		}
		assert(runtime_utf32_text.contains(U"\u00E9"_u32c));
		assert(runtime_utf32_text.find(U"\u00E9"_u32c) == 1);
		assert(runtime_utf32_text.rfind(U"\U0001F600"_u32c) == 2);
		assert(runtime_utf32_text.find_first_not_of(U"A"_u32c) == 1);
		assert(runtime_utf32_text.find_last_not_of(U"\U0001F600"_u32c) == 1);
		assert(runtime_utf32_text.substr(1).value() == U"\u00E9\U0001F600"_utf32_sv);
		assert(runtime_utf32_text.substr(2, 1).value() == U"\U0001F600"_utf32_sv);
		assert(runtime_utf32_text.starts_with(U"A"_u32c));
		assert(runtime_utf32_text.ends_with(U"\U0001F600"_u32c));
		assert(runtime_utf32_text.to_utf8() == u8"A\u00E9\U0001F600"_utf8_sv);
		assert(runtime_utf32_text.to_utf16() == u"A\u00E9\U0001F600"_utf16_sv);
		assert(runtime_utf32_text.to_utf32_owned() == runtime_utf32_text);
		assert(runtime_utf32_text.replace_all(U"\u00E9"_u32c, U"!"_u32c) == U"A!\U0001F600"_utf32_sv);
		assert(U"Stra\u00DFe"_utf32_sv.eq_ignore_case(U"STRASSE"_utf32_sv));
		assert(U"Stra\u00DFe"_utf32_sv.starts_with_ignore_case(U"str"_utf32_sv));
		assert(U"Stra\u00DFe"_utf32_sv.ends_with_ignore_case(U"SSE"_utf32_sv));
		assert(U"Stra\u00DFe"_utf32_sv.compare_ignore_case(U"STRASSE"_utf32_sv) == std::weak_ordering::equivalent);
		assert(!U"\u00E9"_utf32_sv.eq_ignore_case(U"e\u0301"_utf32_sv));
#if UTF8_RANGES_HAS_ICU
		assert(U"I"_utf32_sv.to_lowercase("tr"_locale) == U"\u0131"_utf32_sv);
		assert(U"istanbul"_utf32_sv.to_titlecase("tr"_locale) == U"İstanbul"_utf32_sv);
#endif
		{
			const std::u32string text_storage = U"e\u0301X";
			[[maybe_unused]] const auto text = unwrap_utf32_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				U"e\u0301"_grapheme_utf32,
				U"X"_grapheme_utf32
			}));
		}
		{
			const std::u32string text_storage = U"\r\nX";
			[[maybe_unused]] const auto text = unwrap_utf32_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				U"\r\n"_grapheme_utf32,
				U"X"_grapheme_utf32
			}));
		}
		{
			const std::u32string text_storage = U"\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf32_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				U"\U0001F1F7\U0001F1F4"_grapheme_utf32,
				U"!"_grapheme_utf32
			}));
		}
		{
			const std::u32string text_storage = U"\U0001F469\u200D\U0001F4BB!";
			[[maybe_unused]] const auto text = unwrap_utf32_view(text_storage);
			assert(std::ranges::equal(text.graphemes(), std::array{
				U"\U0001F469\u200D\U0001F4BB"_grapheme_utf32,
				U"!"_grapheme_utf32
			}));
		}
		{
			const std::u32string grapheme_storage = U"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto grapheme_text = unwrap_utf32_view(grapheme_storage);

			assert(grapheme_text.find_grapheme(U"e\u0301"_grapheme_utf32) == 0);
			assert(grapheme_text.find_grapheme(U"\U0001F1F7\U0001F1F4"_grapheme_utf32, 1) == 2);
			assert(grapheme_text.find_grapheme(U"\u0301"_u32c) == utf32_string_view::npos);
			assert(grapheme_text.contains_grapheme(U"\U0001F1F7\U0001F1F4"_grapheme_utf32));
			assert(!grapheme_text.contains_grapheme(U"\u0301"_u32c));
			assert(grapheme_text.rfind_grapheme(U"!"_grapheme_utf32) == 4);
			assert(grapheme_text.rfind_grapheme(U"\U0001F1F7\U0001F1F4"_grapheme_utf32, 3) == 2);
		}
		assert(U"e\u0301"_grapheme_utf32 == U"e\u0301"_utf32_sv);
		{
			assert(U"AbC-\u00E9\u00DF"_utf32_sv.to_ascii_lowercase() == U"abc-\u00E9\u00DF"_utf32_sv);
			assert(U"AbC-\u00E9\u00DF"_utf32_sv.to_ascii_lowercase(0, 3) == U"abc-\u00E9\u00DF"_utf32_sv);
			assert(U"aBc-\u00E9\u00DF"_utf32_sv.to_ascii_uppercase() == U"ABC-\u00E9\u00DF"_utf32_sv);
			assert(U"aBc-\u00E9\u00DF"_utf32_sv.to_ascii_uppercase(0, 3) == U"ABC-\u00E9\u00DF"_utf32_sv);
			assert(U"\u00C4\u03A9\u0130"_utf32_sv.to_lowercase() == U"\u00E4\u03C9i\u0307"_utf32_sv);
			assert(U"XX\u00C4\u03A9YY"_utf32_sv.to_lowercase(2, 2) == U"XX\u00E4\u03C9YY"_utf32_sv);
			assert(U"\u00E4\u00DF\u03C9"_utf32_sv.to_uppercase() == U"\u00C4SS\u03A9"_utf32_sv);
			assert(U"ab\u00E4\u00DFcd"_utf32_sv.to_uppercase(2, 2) == U"ab\u00C4SScd"_utf32_sv);
			assert(U"\u00E9"_utf32_sv.to_nfd() == U"e\u0301"_utf32_sv);
			assert(U"e\u0301"_utf32_sv.to_nfc() == U"\u00E9"_utf32_sv);
			assert(U"\uFF21"_utf32_sv.to_nfkc() == U"A"_utf32_sv);
			assert(U"A"_utf32_sv.to_nfkd() == U"A"_utf32_sv);
			assert(U"\u00E9"_utf32_sv.is_nfc());
			assert(!U"\u00E9"_utf32_sv.is_nfd());
			assert(U"e\u0301"_utf32_sv.is_nfd());
			assert(U"Stra\u00DFe"_utf32_sv.case_fold() == U"strasse"_utf32_sv);
			assert(U"Stra\u00DFe"_utf32_sv.eq_ignore_case(U"STRASSE"_utf32_sv));
			assert(U"Stra\u00DFe"_utf32_sv.starts_with_ignore_case(U"stras"_utf32_sv));
			assert(U"Stra\u00DFe"_utf32_sv.ends_with_ignore_case(U"SSE"_utf32_sv));
			assert(U"Stra\u00DFe"_utf32_sv.compare_ignore_case(U"strasse"_utf32_sv) == std::weak_ordering::equivalent);
			assert(U"Cafe"_utf32_sv.compare_ignore_case(U"cafg"_utf32_sv) == std::weak_ordering::less);
			assert(!U"\u00E9"_utf32_sv.eq_ignore_case(U"e\u0301"_utf32_sv));
			[[maybe_unused]] auto ascii_lowered_owned = utf32_string{ U"AbC-\u00E9\u00DF"_utf32_sv }.to_ascii_lowercase();
			assert(ascii_lowered_owned == U"abc-\u00E9\u00DF"_utf32_sv);
			auto partial_ascii_lower_owned = U"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789-\u00E9\u00DF"_utf32_s;
			[[maybe_unused]] auto partial_ascii_lowered = std::move(partial_ascii_lower_owned).to_ascii_lowercase(0, 26);
			assert(partial_ascii_lowered == U"abcdefghijklmnopqrstuvwxyz0123456789-\u00E9\u00DF"_utf32_sv);
			auto ascii_lower_owned = U"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789AbCdEfGhIjKlMnOpQrSt"_utf32_s;
			[[maybe_unused]] const auto* ascii_lower_original = ascii_lower_owned.base().data();
			[[maybe_unused]] auto lowered_in_place = std::move(ascii_lower_owned).to_lowercase();
			assert(lowered_in_place == U"abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrst"_utf32_sv);
			assert(lowered_in_place.base().data() == ascii_lower_original);
			auto partial_lower_owned = U"XX\u00C4\u03A9YY"_utf32_s;
			[[maybe_unused]] auto partial_lowered = std::move(partial_lower_owned).to_lowercase(2, 2);
			assert(partial_lowered == U"XX\u00E4\u03C9YY"_utf32_sv);
			auto ascii_upper_owned = U"aBcDeFgHiJkLmNoPqRsTuVwXyZ0123456789aBcDeFgHiJkLmNoPqRsT"_utf32_s;
			[[maybe_unused]] const auto* ascii_upper_original = ascii_upper_owned.base().data();
			[[maybe_unused]] auto uppered_in_place = std::move(ascii_upper_owned).to_uppercase();
			assert(uppered_in_place == U"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRST"_utf32_sv);
			assert(uppered_in_place.base().data() == ascii_upper_original);
			auto partial_upper_owned = U"ab\u00E4\u00DFcd"_utf32_s;
			[[maybe_unused]] auto partial_uppered = std::move(partial_upper_owned).to_uppercase(2, 2);
			assert(partial_uppered == U"ab\u00C4SScd"_utf32_sv);
			std::pmr::monotonic_buffer_resource resource;
			const auto alloc = std::pmr::polymorphic_allocator<char32_t>{ &resource };
			[[maybe_unused]] const auto lowered_alloc = U"\u00C4\u03A9\u0130"_utf32_sv.to_lowercase(alloc);
			assert(lowered_alloc == U"\u00E4\u03C9i\u0307"_utf32_sv);
			assert(lowered_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto partial_uppered_alloc = U"ab\u00E4\u00DFcd"_utf32_sv.to_uppercase(2, 2, alloc);
			assert(partial_uppered_alloc == U"ab\u00C4SScd"_utf32_sv);
			assert(partial_uppered_alloc.get_allocator().resource() == &resource);
#if UTF8_RANGES_HAS_ICU
			assert(U"I\u0130"_utf32_sv.to_lowercase("tr"_locale) == U"\u0131i"_utf32_sv);
			assert(U"XXI\u0130YY"_utf32_sv.to_lowercase(2, 2, "tr"_locale) == U"XX\u0131iYY"_utf32_sv);
			[[maybe_unused]] const auto lowered_locale_alloc = U"I\u0130"_utf32_sv.to_lowercase("tr"_locale, alloc);
			assert(lowered_locale_alloc == U"\u0131i"_utf32_sv);
			assert(lowered_locale_alloc.get_allocator().resource() == &resource);
			assert(U"i\u0131"_utf32_sv.to_uppercase("tr"_locale) == U"\u0130I"_utf32_sv);
			assert(U"XXi\u0131YY"_utf32_sv.to_uppercase(2, 2, "tr"_locale) == U"XX\u0130IYY"_utf32_sv);
			[[maybe_unused]] const auto uppered_locale_alloc = U"i\u0131"_utf32_sv.to_uppercase("tr"_locale, alloc);
			assert(uppered_locale_alloc == U"\u0130I"_utf32_sv);
			assert(uppered_locale_alloc.get_allocator().resource() == &resource);
			assert(U"istanbul izmir"_utf32_sv.to_titlecase("tr"_locale) == U"\u0130stanbul \u0130zmir"_utf32_sv);
			[[maybe_unused]] const auto titled_locale_alloc = U"istanbul izmir"_utf32_sv.to_titlecase("tr"_locale, alloc);
			assert(titled_locale_alloc == U"\u0130stanbul \u0130zmir"_utf32_sv);
			assert(titled_locale_alloc.get_allocator().resource() == &resource);
			assert(U"I\u0130"_utf32_sv.case_fold("tr"_locale) == U"\u0131i"_utf32_sv);
			[[maybe_unused]] const auto folded_locale_alloc = U"I\u0130"_utf32_sv.case_fold("tr"_locale, alloc);
			assert(folded_locale_alloc == U"\u0131i"_utf32_sv);
			assert(folded_locale_alloc.get_allocator().resource() == &resource);
			assert(U"I"_utf32_sv.eq_ignore_case(U"\u0131"_utf32_sv, "tr"_locale));
			assert(U"i"_utf32_sv.eq_ignore_case(U"\u0130"_utf32_sv, "tr"_locale));
			assert(U"Istanbul"_utf32_sv.starts_with_ignore_case(U"\u0131s"_utf32_sv, "tr"_locale));
			assert(U"kap\u0131I"_utf32_sv.ends_with_ignore_case(U"\u0131"_utf32_sv, "tr"_locale));
			assert(U"I"_utf32_sv.compare_ignore_case(U"\u0131"_utf32_sv, "tr"_locale) == std::weak_ordering::equivalent);
			assert(!U"I"_utf32_sv.eq_ignore_case(U"\u0131"_utf32_sv));
#endif
			[[maybe_unused]] const auto normalized_alloc = U"e\u0301"_utf32_sv.to_nfc(alloc);
			assert(normalized_alloc == U"\u00E9"_utf32_sv);
			assert(normalized_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto folded_alloc = U"Stra\u00DFe"_utf32_sv.case_fold(alloc);
			assert(folded_alloc == U"strasse"_utf32_sv);
			assert(folded_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto pmr_ascii_lowered = pmr::utf32_string{ U"AbC-\u00E9\u00DF"_utf32_sv, alloc }.to_ascii_lowercase();
			assert(pmr_ascii_lowered == U"abc-\u00E9\u00DF"_utf32_sv);
			assert(pmr_ascii_lowered.get_allocator().resource() == &resource);
		}
		{
			auto it = runtime_utf32_text.char_indices().begin();
			assert(it != runtime_utf32_text.char_indices().end());
			[[maybe_unused]] const auto [i0, c0] = *it++;
			assert(i0 == 0);
			assert(c0 == U"A"_u32c);
			[[maybe_unused]] const auto [i1, c1] = *it++;
			assert(i1 == 1);
			assert(c1 == U"\u00E9"_u32c);
			[[maybe_unused]] const auto [i2, c2] = *it++;
			assert(i2 == 2);
			assert(c2 == U"\U0001F600"_u32c);
			assert(it == runtime_utf32_text.char_indices().end());
		}
		{
			auto chars = runtime_utf32_text.chars();
			assert(chars.base() == runtime_utf32_storage);
			assert(chars.size() == runtime_utf32_storage.size());
			assert(chars.reserve_hint() == runtime_utf32_storage.size());

			auto it = chars.begin();
			assert(*it == U"A"_u32c);
			const auto post = it++;
			assert(*post == U"A"_u32c);
			assert(*it == U"\u00E9"_u32c);
			assert(it[1] == U"\U0001F600"_u32c);

			auto shifted = 1 + it;
			assert(*shifted == U"\U0001F600"_u32c);
			auto advanced = it + 1;
			assert(*advanced == U"\U0001F600"_u32c);
			assert(advanced - it == 1);
			const auto postdec = advanced--;
			assert(*postdec == U"\U0001F600"_u32c);
			assert(*advanced == U"\u00E9"_u32c);
			--advanced;
			assert(*advanced == U"A"_u32c);
			advanced += 2;
			assert(*advanced == U"\U0001F600"_u32c);
			++advanced;
			assert(advanced - chars.begin() == static_cast<std::ptrdiff_t>(chars.size()));
			--advanced;
			assert(*advanced == U"\U0001F600"_u32c);
			assert(chars.end() - chars.begin() == static_cast<std::ptrdiff_t>(chars.size()));
		}
		{
			const std::array<char32_t, 4> invalid_code_points{
				U'A',
				static_cast<char32_t>(0xD800u),
				U'B',
				static_cast<char32_t>(0x110000u)
			};
			const auto lossy = views::lossy_utf32(invalid_code_points);
			assert(lossy.size() == invalid_code_points.size());

			auto it = lossy.begin();
			assert(*it == U"A"_u32c);
			const auto post = it++;
			assert(*post == U"A"_u32c);
			assert(*it == utf32_char::replacement_character);
			assert(it[1] == U"B"_u32c);

			auto shifted = 1 + it;
			assert(*shifted == U"B"_u32c);
			auto advanced = it + 2;
			assert(*advanced == utf32_char::replacement_character);
			assert(advanced - it == 2);
			const auto postdec = advanced--;
			assert(*postdec == utf32_char::replacement_character);
			assert(*advanced == U"B"_u32c);
			--advanced;
			assert(*advanced == utf32_char::replacement_character);
			advanced += 1;
			assert(*advanced == U"B"_u32c);
			advanced += 1;
			assert(*advanced == utf32_char::replacement_character);
		}
		{
			const std::u32string text_storage = U"e\u0301\U0001F1F7\U0001F1F4!";
			[[maybe_unused]] const auto text = unwrap_utf32_view(text_storage);
			auto it = text.grapheme_indices().begin();
			assert(it != text.grapheme_indices().end());
			[[maybe_unused]] const auto [i0, g0] = *it++;
			assert(i0 == 0);
			assert(g0 == U"e\u0301"_grapheme_utf32);
			[[maybe_unused]] const auto [i1, g1] = *it++;
			assert(i1 == 2);
			assert(g1 == U"\U0001F1F7\U0001F1F4"_grapheme_utf32);
			[[maybe_unused]] const auto [i2, g2] = *it++;
			assert(i2 == 4);
			assert(g2 == U"!"_grapheme_utf32);
			assert(it == text.grapheme_indices().end());
		}
		{
			const auto text = U"abra--cadabra--!"_utf32_sv;
			assert(std::ranges::equal(text.splitn(0, U"--"_utf32_sv), std::array<utf32_string_view, 0>{}));
			assert(std::ranges::equal(text.splitn(1, U"--"_utf32_sv), std::array{
				U"abra--cadabra--!"_utf32_sv
			}));
			assert(std::ranges::equal(text.splitn(2, U"--"_utf32_sv), std::array{
				U"abra"_utf32_sv,
				U"cadabra--!"_utf32_sv
			}));
			assert(std::ranges::equal(text.rsplitn(2, U"--"_utf32_sv), std::array{
				U"!"_utf32_sv,
				U"abra--cadabra"_utf32_sv
			}));
			[[maybe_unused]] const auto first = text.split_once(U"--"_utf32_sv);
			assert(first.has_value());
			assert(first->first == U"abra"_utf32_sv);
			assert(first->second == U"cadabra--!"_utf32_sv);
			[[maybe_unused]] const auto last = text.rsplit_once(U"--"_utf32_sv);
			assert(last.has_value());
			assert(last->first == U"abra--cadabra"_utf32_sv);
			assert(last->second == U"!"_utf32_sv);
			assert(!text.split_once(U""_utf32_sv).has_value());
			assert(!text.rsplit_once(U""_utf32_sv).has_value());
		}
		{
			const auto text = U"<<<\u00E9A>>>"_utf32_sv;
			[[maybe_unused]] const auto stripped_prefix = text.strip_prefix(U"<<<"_utf32_sv);
			assert(stripped_prefix.has_value());
			assert(stripped_prefix.value() == U"\u00E9A>>>"_utf32_sv);
			[[maybe_unused]] const auto stripped_suffix = text.strip_suffix(U">>>"_utf32_sv);
			assert(stripped_suffix.has_value());
			assert(stripped_suffix.value() == U"<<<\u00E9A"_utf32_sv);
			[[maybe_unused]] const auto stripped_circ = text.strip_circumfix(U"<<<"_utf32_sv, U">>>"_utf32_sv);
			assert(stripped_circ.has_value());
			assert(stripped_circ.value() == U"\u00E9A"_utf32_sv);
			assert(text.trim_prefix(U">>>"_utf32_sv) == text);
			assert(text.trim_prefix(U"<<<"_utf32_sv) == U"\u00E9A>>>"_utf32_sv);
			assert(text.trim_suffix(U">>>"_utf32_sv) == U"<<<\u00E9A"_utf32_sv);
			assert(U"\u00E9A\u00E9"_utf32_sv.trim_prefix(U"\u00E9"_u32c) == U"A\u00E9"_utf32_sv);
			assert(U"\u00E9A\u00E9"_utf32_sv.trim_suffix(U"\u00E9"_u32c) == U"\u00E9A"_utf32_sv);
		}
		{
			[[maybe_unused]] const auto repeated = U"----abra----"_utf32_sv;
			[[maybe_unused]] const auto accented = U"\u00E9\u00E9A\u00E9"_utf32_sv;
			assert(repeated.trim_start_matches(U"--"_utf32_sv) == U"abra----"_utf32_sv);
			assert(repeated.trim_end_matches(U"--"_utf32_sv) == U"----abra"_utf32_sv);
			assert(repeated.trim_matches(U"--"_utf32_sv) == U"abra"_utf32_sv);
			assert(repeated.trim_matches(U""_utf32_sv) == repeated);
			assert(U"***abra***"_utf32_sv.trim_matches(U"*"_u32c) == U"abra"_utf32_sv);
			assert(accented.trim_start_matches(U"\u00E9"_u32c) == U"A\u00E9"_utf32_sv);
			assert(accented.trim_end_matches(U"\u00E9"_u32c) == U"\u00E9\u00E9A"_utf32_sv);
			assert(accented.trim_matches(U"\u00E9"_u32c) == U"A"_utf32_sv);
		}
		{
			const std::u32string repeated_storage = U"----abra----";
			const auto repeated = unwrap_utf32_view(repeated_storage);
			const std::u32string accented_storage = U"\u00E9\u00E9A\u00E9";
			const auto accented = unwrap_utf32_view(accented_storage);
			assert(repeated.trim_prefix(U"++"_utf32_sv) == repeated);
			assert(repeated.trim_suffix(U"++"_utf32_sv) == repeated);
			assert(repeated.trim_start_matches(U"--"_utf32_sv) == U"abra----"_utf32_sv);
			assert(repeated.trim_end_matches(U"--"_utf32_sv) == U"----abra"_utf32_sv);
			assert(repeated.trim_matches(U"--"_utf32_sv) == U"abra"_utf32_sv);
			assert(accented.trim_start_matches(U"\u00E9"_u32c) == U"A\u00E9"_utf32_sv);
			assert(accented.trim_end_matches(U"\u00E9"_u32c) == U"\u00E9\u00E9A"_utf32_sv);
			assert(accented.trim_matches(U"\u00E9"_u32c) == U"A"_utf32_sv);
		}
		{
			[[maybe_unused]] const auto unicode_trimmed = U"\u00A0\tA\u00A0 "_utf32_sv;
			[[maybe_unused]] const auto unicode_split = U"\u00A0A\u2003B C"_utf32_sv;
			assert(unicode_trimmed.trim() == U"A"_utf32_sv);
			assert(unicode_trimmed.trim_start() == U"A\u00A0 "_utf32_sv);
			assert(unicode_trimmed.trim_end() == U"\u00A0\tA"_utf32_sv);
			assert(unicode_trimmed.trim_ascii() == U"\u00A0\tA\u00A0"_utf32_sv);
			assert(unicode_trimmed.trim_ascii_start() == unicode_trimmed);
			assert(unicode_trimmed.trim_ascii_end() == U"\u00A0\tA\u00A0"_utf32_sv);
			assert(std::ranges::equal(U""_utf32_sv.split_whitespace(), std::array<utf32_string_view, 0>{}));
			assert(std::ranges::equal(U" \t\r\n"_utf32_sv.split_ascii_whitespace(), std::array<utf32_string_view, 0>{}));
			assert(std::ranges::equal(U" \tA  B\n"_utf32_sv.split_whitespace(), std::array{
				U"A"_utf32_sv,
				U"B"_utf32_sv
			}));
			assert(std::ranges::equal(U" \tA  B\n"_utf32_sv.split_ascii_whitespace(), std::array{
				U"A"_utf32_sv,
				U"B"_utf32_sv
			}));
			assert(std::ranges::equal(unicode_split.split_whitespace(), std::array{
				U"A"_utf32_sv,
				U"B"_utf32_sv,
				U"C"_utf32_sv
			}));
			assert(std::ranges::equal(unicode_split.split_ascii_whitespace(), std::array{
				U"\u00A0A\u2003B"_utf32_sv,
				U"C"_utf32_sv
			}));
		}
		{
			const std::u32string unicode_trimmed_storage = U"\u00A0\tA\u00A0 ";
			const auto unicode_trimmed = unwrap_utf32_view(unicode_trimmed_storage);
			const std::u32string unicode_split_storage = U"\u00A0A\u2003B C";
			const auto unicode_split = unwrap_utf32_view(unicode_split_storage);
			const std::u32string empty_storage;
			const auto empty = unwrap_utf32_view(empty_storage);
			assert(unicode_trimmed.trim() == U"A"_utf32_sv);
			assert(unicode_trimmed.trim_start() == U"A\u00A0 "_utf32_sv);
			assert(unicode_trimmed.trim_end() == U"\u00A0\tA"_utf32_sv);
			assert(unicode_trimmed.trim_ascii_start() == unicode_trimmed);
			assert(unicode_trimmed.trim_ascii_end() == U"\u00A0\tA\u00A0"_utf32_sv);
			assert(std::ranges::equal(empty.split_whitespace(), std::array<utf32_string_view, 0>{}));
			assert(std::ranges::equal(U" \t\r\n"_utf32_sv.split_ascii_whitespace(), std::array<utf32_string_view, 0>{}));
			assert(std::ranges::equal(unicode_split.split_whitespace(), std::array{
				U"A"_utf32_sv,
				U"B"_utf32_sv,
				U"C"_utf32_sv
			}));
			assert(std::ranges::equal(unicode_split.split_ascii_whitespace(), std::array{
				U"\u00A0A\u2003B"_utf32_sv,
				U"C"_utf32_sv
			}));
		}
		{
			std::ostringstream oss;
			oss << runtime_utf32_text;
			assert(oss.str() == "A\xC3\xA9\xF0\x9F\x98\x80");
			assert(std::hash<utf32_string_view>{}(runtime_utf32_text) == std::hash<std::u32string_view>{}(runtime_utf32_text.base()));
		}
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
		assert(u"e\u0301"_grapheme_utf16 == u"e\u0301"_utf16_sv);
		{
			const std::u16string text_storage = u"A\u00E9\U0001F600";
			[[maybe_unused]] const auto text = unwrap_utf16_view(text_storage);
			assert(text.to_utf8() == u8"A\u00E9\U0001F600"_utf8_sv);
			assert(text.to_utf16_owned() == u"A\u00E9\U0001F600"_utf16_sv);
			assert(u"\U0001F600"_u16c.to_utf16_owned() == u"\U0001F600"_utf16_sv);
		}
		{
			const std::u16string ascii_storage = u"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789";
			assert(details::ascii_prefix_length(std::u16string_view{ ascii_storage }) == ascii_storage.size());

			std::u16string lowered(ascii_storage.size(), u'\0');
			std::u16string uppered(ascii_storage.size(), u'\0');
			std::u8string narrowed(ascii_storage.size(), u8'\0');
			assert(details::ascii_lowercase_copy(lowered.data(), std::u16string_view{ ascii_storage }));
			assert(lowered == u"abcdefghijklmnopqrstuvwxyz0123456789");
			assert(details::ascii_uppercase_copy(uppered.data(), std::u16string_view{ ascii_storage }));
			assert(uppered == u"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
			details::copy_ascii_utf16_to_utf8(narrowed.data(), std::u16string_view{ ascii_storage });
			assert(narrowed == u8"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789");

			[&]<typename WideChar = wchar_t>()
			{
				if constexpr (sizeof(WideChar) == 2)
				{
					const std::basic_string_view<WideChar> ascii_wide = L"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789";
					[[maybe_unused]] const auto validated = details::validate_utf16(std::basic_string_view<WideChar>{ ascii_wide });
					assert(validated.has_value());
					[[maybe_unused]] const auto copied = details::copy_validated_utf16_code_units(std::basic_string_view<WideChar>{ ascii_wide }, std::allocator<char16_t>{});
					assert(copied.has_value());
					[[maybe_unused]] const auto copied_view = std::u16string_view{ copied->data(), copied->size() };
					assert(copied_view == std::u16string_view{ ascii_storage });
					[[maybe_unused]] const auto transcoded = details::transcode_utf16_to_utf8_checked(std::basic_string_view<WideChar>{ ascii_wide }, std::allocator<char8_t>{});
					assert(transcoded.has_value());
					[[maybe_unused]] const auto transcoded_view = std::u8string_view{ transcoded->data(), transcoded->size() };
					assert(transcoded_view == u8"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789");
				}
			}();
		}
		{
			assert(u"AbC-\u00E9\u00DF"_utf16_sv.to_ascii_lowercase() == u"abc-\u00E9\u00DF"_utf16_sv);
			assert(u"AbC-\u00E9\u00DF"_utf16_sv.to_ascii_lowercase(0, 3) == u"abc-\u00E9\u00DF"_utf16_sv);
			assert(u"aBc-\u00E9\u00DF"_utf16_sv.to_ascii_uppercase() == u"ABC-\u00E9\u00DF"_utf16_sv);
			assert(u"aBc-\u00E9\u00DF"_utf16_sv.to_ascii_uppercase(0, 3) == u"ABC-\u00E9\u00DF"_utf16_sv);
			assert(u"\u00C4\u03A9\u0130"_utf16_sv.to_lowercase() == u"\u00E4\u03C9i\u0307"_utf16_sv);
			assert(u"XX\u00C4\u03A9YY"_utf16_sv.to_lowercase(2, 2) == u"XX\u00E4\u03C9YY"_utf16_sv);
			assert(u"\u00E4\u00DF\u03C9"_utf16_sv.to_uppercase() == u"\u00C4SS\u03A9"_utf16_sv);
			assert(u"ab\u00E4\u00DFcd"_utf16_sv.to_uppercase(2, 2) == u"ab\u00C4SScd"_utf16_sv);
			assert(u"\u00E9"_utf16_sv.to_nfd() == u"e\u0301"_utf16_sv);
			assert(u"e\u0301"_utf16_sv.to_nfc() == u"\u00E9"_utf16_sv);
			assert(u"\uFF21"_utf16_sv.to_nfkc() == u"A"_utf16_sv);
			assert(u"A"_utf16_sv.to_nfkd() == u"A"_utf16_sv);
			assert(u"\u00E9"_utf16_sv.is_nfc());
			assert(!u"\u00E9"_utf16_sv.is_nfd());
			assert(u"e\u0301"_utf16_sv.is_nfd());
			assert(u"Straße"_utf16_sv.case_fold() == u"strasse"_utf16_sv);
			assert(u"Straße"_utf16_sv.eq_ignore_case(u"STRASSE"_utf16_sv));
			assert(u"Straße"_utf16_sv.starts_with_ignore_case(u"stras"_utf16_sv));
			assert(u"Straße"_utf16_sv.ends_with_ignore_case(u"SSE"_utf16_sv));
			assert(u"Straße"_utf16_sv.compare_ignore_case(u"strasse"_utf16_sv) == std::weak_ordering::equivalent);
			assert(u"Cafe"_utf16_sv.compare_ignore_case(u"cafg"_utf16_sv) == std::weak_ordering::less);
			assert(!u"\u00E9"_utf16_sv.eq_ignore_case(u"e\u0301"_utf16_sv));
			[[maybe_unused]] auto ascii_lowered_owned = utf16_string{ u"AbC-\u00E9\u00DF"_utf16_sv }.to_ascii_lowercase();
			assert(ascii_lowered_owned == u"abc-\u00E9\u00DF"_utf16_sv);
			auto partial_ascii_lower_owned = u"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789-\u00E9\u00DF"_utf16_s;
			[[maybe_unused]] auto partial_ascii_lowered = std::move(partial_ascii_lower_owned).to_ascii_lowercase(0, 26);
			assert(partial_ascii_lowered == u"abcdefghijklmnopqrstuvwxyz0123456789-\u00E9\u00DF"_utf16_sv);
			auto ascii_lower_owned = u"AbCdEfGhIjKlMnOpQrStUvWxYz0123456789AbCdEfGhIjKlMnOpQrSt"_utf16_s;
			[[maybe_unused]] const auto* ascii_lower_original = ascii_lower_owned.base().data();
			[[maybe_unused]] auto lowered_in_place = std::move(ascii_lower_owned).to_lowercase();
			assert(lowered_in_place == u"abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrst"_utf16_sv);
			assert(lowered_in_place.base().data() == ascii_lower_original);
			auto partial_lower_owned = u"XX\u00C4\u03A9YY"_utf16_s;
			[[maybe_unused]] auto partial_lowered = std::move(partial_lower_owned).to_lowercase(2, 2);
			assert(partial_lowered == u"XX\u00E4\u03C9YY"_utf16_sv);
			auto ascii_upper_owned = u"aBcDeFgHiJkLmNoPqRsTuVwXyZ0123456789aBcDeFgHiJkLmNoPqRsT"_utf16_s;
			[[maybe_unused]] const auto* ascii_upper_original = ascii_upper_owned.base().data();
			[[maybe_unused]] auto uppered_in_place = std::move(ascii_upper_owned).to_uppercase();
			assert(uppered_in_place == u"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRST"_utf16_sv);
			assert(uppered_in_place.base().data() == ascii_upper_original);
			auto partial_upper_owned = u"ab\u00E4\u00DFcd"_utf16_s;
			[[maybe_unused]] auto partial_uppered = std::move(partial_upper_owned).to_uppercase(2, 2);
			assert(partial_uppered == u"ab\u00C4SScd"_utf16_sv);
			std::pmr::monotonic_buffer_resource resource;
			const auto alloc = std::pmr::polymorphic_allocator<char16_t>{ &resource };
			[[maybe_unused]] const auto lowered_alloc = u"\u00C4\u03A9\u0130"_utf16_sv.to_lowercase(alloc);
			assert(lowered_alloc == u"\u00E4\u03C9i\u0307"_utf16_sv);
			assert(lowered_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto partial_uppered_alloc = u"ab\u00E4\u00DFcd"_utf16_sv.to_uppercase(2, 2, alloc);
			assert(partial_uppered_alloc == u"ab\u00C4SScd"_utf16_sv);
			assert(partial_uppered_alloc.get_allocator().resource() == &resource);
#if UTF8_RANGES_HAS_ICU
			assert(u"I\u0130"_utf16_sv.to_lowercase("tr"_locale) == u"\u0131i"_utf16_sv);
			assert(u"XXI\u0130YY"_utf16_sv.to_lowercase(2, 2, "tr"_locale) == u"XX\u0131iYY"_utf16_sv);
			[[maybe_unused]] const auto lowered_locale_alloc = u"I\u0130"_utf16_sv.to_lowercase("tr"_locale, alloc);
			assert(lowered_locale_alloc == u"\u0131i"_utf16_sv);
			assert(lowered_locale_alloc.get_allocator().resource() == &resource);
			assert(u"i\u0131"_utf16_sv.to_uppercase("tr"_locale) == u"\u0130I"_utf16_sv);
			assert(u"XXi\u0131YY"_utf16_sv.to_uppercase(2, 2, "tr"_locale) == u"XX\u0130IYY"_utf16_sv);
			[[maybe_unused]] const auto uppered_locale_alloc = u"i\u0131"_utf16_sv.to_uppercase("tr"_locale, alloc);
			assert(uppered_locale_alloc == u"\u0130I"_utf16_sv);
			assert(uppered_locale_alloc.get_allocator().resource() == &resource);
			assert(u"istanbul izmir"_utf16_sv.to_titlecase("tr"_locale) == u"\u0130stanbul \u0130zmir"_utf16_sv);
			[[maybe_unused]] const auto titled_locale_alloc = u"istanbul izmir"_utf16_sv.to_titlecase("tr"_locale, alloc);
			assert(titled_locale_alloc == u"\u0130stanbul \u0130zmir"_utf16_sv);
			assert(titled_locale_alloc.get_allocator().resource() == &resource);
			assert(u"I\u0130"_utf16_sv.case_fold("tr"_locale) == u"\u0131i"_utf16_sv);
			[[maybe_unused]] const auto folded_locale_alloc = u"I\u0130"_utf16_sv.case_fold("tr"_locale, alloc);
			assert(folded_locale_alloc == u"\u0131i"_utf16_sv);
			assert(folded_locale_alloc.get_allocator().resource() == &resource);
			assert(u"I"_utf16_sv.eq_ignore_case(u"\u0131"_utf16_sv, "tr"_locale));
			assert(u"i"_utf16_sv.eq_ignore_case(u"\u0130"_utf16_sv, "tr"_locale));
			assert(u"Istanbul"_utf16_sv.starts_with_ignore_case(u"\u0131s"_utf16_sv, "tr"_locale));
			assert(u"kap\u0131I"_utf16_sv.ends_with_ignore_case(u"\u0131"_utf16_sv, "tr"_locale));
			assert(u"I"_utf16_sv.compare_ignore_case(u"\u0131"_utf16_sv, "tr"_locale) == std::weak_ordering::equivalent);
			assert(!u"I"_utf16_sv.eq_ignore_case(u"\u0131"_utf16_sv));
			assert(is_available_locale("tr"_locale));
			assert(is_available_locale(locale_id{ "tr" }));
			assert(!is_available_locale("definitely_not_a_real_locale"_locale));
			assert(!is_available_locale(locale_id{ nullptr }));
#endif
			[[maybe_unused]] const auto normalized_alloc = u"e\u0301"_utf16_sv.to_nfc(alloc);
			assert(normalized_alloc == u"\u00E9"_utf16_sv);
			assert(normalized_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto folded_alloc = u"Straße"_utf16_sv.case_fold(alloc);
			assert(folded_alloc == u"strasse"_utf16_sv);
			assert(folded_alloc.get_allocator().resource() == &resource);
			[[maybe_unused]] const auto pmr_ascii_lowered = pmr::utf16_string{ u"AbC-\u00E9\u00DF"_utf16_sv, alloc }.to_ascii_lowercase();
			assert(pmr_ascii_lowered == u"abc-\u00E9\u00DF"_utf16_sv);
			assert(pmr_ascii_lowered.get_allocator().resource() == &resource);
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
			auto chars = runtime_utf16_text.chars();
			assert(chars.base() == runtime_utf16_storage);
			assert(chars.reserve_hint() == runtime_utf16_storage.size());

			auto it = chars.begin();
			const auto copy = it;
			assert(it == copy);
			assert(!(it == chars.end()));
			assert(!(chars.end() == it));
			assert(*it == u"A"_u16c);
			const auto post = it++;
			assert(*post == u"A"_u16c);
			assert(*it == u"\u00E9"_u16c);
			++it;
			assert(*it == u"\U0001F600"_u16c);
			++it;
			assert(it == chars.end());
			assert(chars.end() == it);
			assert(utf16_string_view{}.chars().begin() == utf16_string_view{}.chars().end());
		}
		{
			auto reversed = runtime_utf16_text.reversed_chars();
			assert(reversed.reserve_hint() == runtime_utf16_storage.size());

			auto it = reversed.begin();
			const auto copy = it;
			assert(it == copy);
			assert(!(it == reversed.end()));
			assert(!(reversed.end() == it));
			assert(*it == u"\U0001F600"_u16c);
			const auto post = it++;
			assert(*post == u"\U0001F600"_u16c);
			assert(*it == u"\u00E9"_u16c);
			++it;
			assert(*it == u"A"_u16c);
			++it;
			assert(it == reversed.end());
			assert(reversed.end() == it);
			assert(utf16_string_view{}.reversed_chars().begin() == utf16_string_view{}.reversed_chars().end());
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
			[[maybe_unused]] const auto empty = u""_utf16_sv;
			[[maybe_unused]] const auto trimmed = u"--a----b--"_utf16_sv;
			[[maybe_unused]] const auto text = u"a--b--"_utf16_sv;
			[[maybe_unused]] const auto leading = u"--abra"_utf16_sv;
			[[maybe_unused]] const auto unicode = u"A\u00E9B\u00E9"_utf16_sv;
			[[maybe_unused]] constexpr auto ascii_space = [](utf16_char ch) constexpr noexcept
			{
				return ch.is_ascii_whitespace();
			};
			[[maybe_unused]] constexpr auto split_on_accent = [](utf16_char ch) constexpr noexcept
			{
				return ch == u"\u00E9"_u16c;
			};
			assert(std::ranges::equal(empty.split_trimmed(u"--"_utf16_sv), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(u"abra"_utf16_sv.split_trimmed(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(leading.split_trimmed(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(u"abra--"_utf16_sv.split_trimmed(u"--"_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(trimmed.split_trimmed(u"--"_utf16_sv), std::array{
				u"a"_utf16_sv,
				u"b"_utf16_sv
			}));
			assert(std::ranges::equal(u"----"_utf16_sv.split_trimmed(u"--"_utf16_sv), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(empty.split_trimmed(u""_utf16_sv), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(u"abra"_utf16_sv.split_trimmed(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(unicode.split_trimmed(u"\u00E9"_u16c), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			}));
			assert(std::ranges::equal(u" \tA \t B\t"_utf16_sv.split_trimmed(ascii_space), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			}));
			assert(std::ranges::equal(unicode.split_trimmed(split_on_accent), std::array{
				u"A"_utf16_sv,
				u"B"_utf16_sv
			}));
			assert(std::ranges::equal(empty.split_inclusive(u"--"_utf16_sv), std::array{
				u""_utf16_sv
			}));
			assert(std::ranges::equal(text.split_inclusive(u"--"_utf16_sv), std::array{
				u"a--"_utf16_sv,
				u"b--"_utf16_sv
			}));
			assert(std::ranges::equal(leading.split_inclusive(u"--"_utf16_sv), std::array{
				u"--"_utf16_sv,
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(u"abra"_utf16_sv.split_inclusive(u""_utf16_sv), std::array{
				u"abra"_utf16_sv
			}));
			assert(std::ranges::equal(unicode.matches(u"\u00E9"_u16c), std::array{
				u"\u00E9"_utf16_sv,
				u"\u00E9"_utf16_sv
			}));
			assert(std::ranges::equal(u"aaaa"_utf16_sv.matches(u"aa"_utf16_sv), std::array{
				u"aa"_utf16_sv,
				u"aa"_utf16_sv
			}));
			assert(std::ranges::equal(u"aaaa"_utf16_sv.matches(u""_utf16_sv), std::array<utf16_string_view, 0>{}));
			assert(std::ranges::equal(u"aaaa"_utf16_sv.rmatch_indices(u"aa"_utf16_sv), std::array{
				std::pair<std::size_t, utf16_string_view>{ 2, u"aa"_utf16_sv },
				std::pair<std::size_t, utf16_string_view>{ 0, u"aa"_utf16_sv }
			}));
			assert(std::ranges::equal(u"abra"_utf16_sv.rmatch_indices(u""_utf16_sv), std::array<std::pair<std::size_t, utf16_string_view>, 0>{}));
			assert(u"aaaa"_utf16_sv.replace_all(u"aa"_utf16_sv, u"x"_utf16_sv) == u"xx"_utf16_sv);
			assert(u"aaaa"_utf16_sv.replace_n(1, u"aa"_utf16_sv, u"x"_utf16_sv) == u"xaa"_utf16_sv);
			assert(unicode.replace_all(u"\u00E9"_u16c, u"!"_u16c) == u"A!B!"_utf16_sv);
			assert(unicode.replace_n(1, u"\u00E9"_u16c, u"!"_u16c) == u"A!B\u00E9"_utf16_sv);
			assert(unicode.replace_all(u""_utf16_sv, u"!"_utf16_sv) == unicode);
			[[maybe_unused]] const auto long_needle_text = u"prefixabcdefghijmiddleabcdefghijsuffix"_utf16_sv;
			assert(long_needle_text.find(u"abcdefghij"_utf16_sv) == 6);
			assert(long_needle_text.rfind(u"abcdefghij"_utf16_sv) == 22);
			assert(long_needle_text.rfind(u"abcdefghij"_utf16_sv, 21) == 6);
			assert(long_needle_text.replace_all(u"abcdefghij"_utf16_sv, u"ABCDEFGHIJ"_utf16_sv)
				== u"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix"_utf16_sv);
			assert(long_needle_text.replace_n(1, u"abcdefghij"_utf16_sv, u"ABCDEFGHIJ"_utf16_sv)
				== u"prefixABCDEFGHIJmiddleabcdefghijsuffix"_utf16_sv);
			std::pmr::monotonic_buffer_resource resource;
			const auto replaced_alloc = unicode.replace_all(
				u"\u00E9"_u16c,
				u"!"_u16c,
				std::pmr::polymorphic_allocator<char16_t>{ &resource });
			assert(replaced_alloc == u"A!B!"_utf16_sv);
			assert(replaced_alloc.get_allocator().resource() == &resource);
			const auto replaced_n_alloc = utf16_string{ unicode }.replace_n(
				1,
				u"\u00E9"_u16c,
				u"!"_u16c,
				std::pmr::polymorphic_allocator<char16_t>{ &resource });
			assert(replaced_n_alloc == u"A!B\u00E9"_utf16_sv);
			assert(replaced_n_alloc.get_allocator().resource() == &resource);
		}
		{
			const std::array any_of{ u"\u00E9"_u16c, u"\U0001F600"_u16c };
			const auto any = std::span{ any_of };
			const auto text = u"\u00E9A\U0001F600"_utf16_sv;
			assert(text.contains(any));
			assert(!u"ABC"_utf16_sv.contains(any));
			assert(text.find(any) == 0);
			assert(text.rfind(any) == 2);
			assert(text.starts_with(any));
			assert(text.ends_with(any));
			assert(!u"A"_utf16_sv.starts_with(any));
			assert(!u"A"_utf16_sv.ends_with(any));
			assert(text.trim_start_matches(any) == u"A\U0001F600"_utf16_sv);
			assert(text.trim_end_matches(any) == u"\u00E9A"_utf16_sv);
			assert(text.trim_matches(any) == u"A"_utf16_sv);
			[[maybe_unused]] const auto first = text.split_once(any);
			assert(first.has_value());
			assert(first->first == u""_utf16_sv);
			assert(first->second == u"A\U0001F600"_utf16_sv);
			[[maybe_unused]] const auto last = text.rsplit_once(any);
			assert(last.has_value());
			assert(last->first == u"\u00E9A"_utf16_sv);
			assert(last->second == u""_utf16_sv);
			assert(text.replace_all(any, u"!"_u16c) == u"!A!"_utf16_sv);
			assert(text.replace_n(1, any, u"!"_u16c) == u"!A\U0001F600"_utf16_sv);
			[[maybe_unused]] constexpr auto is_ascii_digit = [](utf16_char ch) constexpr noexcept
			{
				return ch.is_ascii_digit();
			};
			assert(u"123-456"_utf16_sv.replace_all(is_ascii_digit, u"x"_u16c) == u"xxx-xxx"_utf16_sv);
			assert(u"123-456"_utf16_sv.replace_n(2, is_ascii_digit, u"x"_u16c) == u"xx3-456"_utf16_sv);
		}
		{
			const std::array any_of{
				u"\u00E9"_u16c,
				u"\u00DF"_u16c,
				u"\u0103"_u16c,
				u"\u0111"_u16c,
				u"\u03C9"_u16c,
				u"\u0416"_u16c,
				u"\u05D0"_u16c,
				u"\u20AC"_u16c
			};
			const auto any = std::span{ any_of };
			[[maybe_unused]] const auto text = u"plain ascii words and \u03B1\u03B2\u03B3 \u20AC"_utf16_sv;
			assert(text.find(any) == text.size() - 1);
			assert(text.rfind(any) == text.size() - 1);
			assert(text.contains(any));
			assert(!u"plain ascii words and \u03B1\u03B2\u03B3"_utf16_sv.contains(any));
		}
		{
			std::array<utf16_char, 17> overflow_any_of{};
			for (std::size_t i = 0; i != overflow_any_of.size(); ++i)
			{
				overflow_any_of[i] = utf16_char::from_scalar_unchecked(0x0100u + static_cast<std::uint32_t>(i));
			}
			const auto any = std::span{ overflow_any_of };
			[[maybe_unused]] const auto text = u"\u0110A\u0100"_utf16_sv;
			assert(text.contains(any));
			assert(text.find(any) == 0);
			assert(text.rfind(any) == 2);
			assert(text.starts_with(any));
			assert(text.ends_with(any));
			assert(text.trim_matches(any) == u"A"_utf16_sv);
			assert(text.replace_all(any, u"!"_u16c) == u"!A!"_utf16_sv);
			assert(!u"ABC"_utf16_sv.contains(any));
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
		const auto euro = utf8_char::from_scalar(0x20ACu);
		assert(euro.has_value());
		assert(euro->as_scalar() == 0x20ACu);
		assert(std::hash<utf8_char>{}(*euro) == std::hash<std::u8string_view>{}(details::utf8_char_view(*euro)));
		const std::array<char8_t, 1> ascii_bytes{ static_cast<char8_t>('A') };
		const std::array<char8_t, 2> latin1_bytes{ static_cast<char8_t>(0xC3u), static_cast<char8_t>(0xA9u) };
		const std::array<char8_t, 4> emoji_bytes{ static_cast<char8_t>(0xF0u), static_cast<char8_t>(0x9Fu), static_cast<char8_t>(0x98u), static_cast<char8_t>(0x80u) };
		assert(utf8_char::from_utf8_bytes_unchecked(ascii_bytes.data(), ascii_bytes.size()) == u8"A"_u8c);
		assert(utf8_char::from_utf8_bytes_unchecked(latin1_bytes.data(), latin1_bytes.size()) == u8"\u00E9"_u8c);
		assert(utf8_char::from_utf8_bytes_unchecked(emoji_bytes.data(), emoji_bytes.size()) == u8"\U0001F600"_u8c);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0x7FFu);
		++ch;
		assert(ch.as_scalar() == 0x800u);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0xFFFFu);
		++ch;
		assert(ch.as_scalar() == 0x10000u);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(details::encoding_constants::max_unicode_scalar);
		++ch;
		assert(ch == utf8_char::null_terminator);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0x800u);
		--ch;
		assert(ch.as_scalar() == 0x7FFu);
	}
	{
		utf8_char ch = utf8_char::from_scalar_unchecked(0x10000u);
		--ch;
		assert(ch.as_scalar() == 0xFFFFu);
	}
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
	assert(!utf16_char::from_scalar(0xD800u).has_value());
	assert(!utf16_char::from_scalar(0x110000u).has_value());
	assert(utf16_char::from_utf16_code_units(u"\u20AC", 1).has_value());
	assert(!utf16_char::from_utf16_code_units(u"\xD800", 1).has_value());
	{
		const auto euro = utf16_char::from_scalar(0x20ACu);
		assert(euro.has_value());
		assert(euro->as_scalar() == 0x20ACu);
		assert(std::hash<utf16_char>{}(*euro) == std::hash<std::u16string_view>{}(details::utf16_char_view(*euro)));

		const std::array<char16_t, 2> emoji_units{ static_cast<char16_t>(0xD83Du), static_cast<char16_t>(0xDE00u) };
		assert(utf16_char::from_utf16_code_units(emoji_units.data(), emoji_units.size()).value() == u"\U0001F600"_u16c);
		assert(utf16_char::from_utf16_code_units_unchecked(emoji_units.data(), emoji_units.size()) == u"\U0001F600"_u16c);

		const std::array<wchar_t, 2> emoji_wide_units{ static_cast<wchar_t>(0xD83Du), static_cast<wchar_t>(0xDE00u) };
		assert(utf16_char::from_utf16_code_units(emoji_wide_units.data(), emoji_wide_units.size()).value() == u"\U0001F600"_u16c);
		assert(utf16_char::from_utf16_code_units_unchecked(emoji_wide_units.data(), emoji_wide_units.size()) == u"\U0001F600"_u16c);
	}
	assert(utf16_char::from_scalar_unchecked(0x20ACu).as_scalar() == 0x20ACu);
	assert(utf16_char::from_scalar_unchecked(0x1F600u).as_scalar() == 0x1F600u);
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
	{
		utf16_char ch = utf16_char::from_scalar_unchecked(0xFFFFu);
		++ch;
		assert(ch.as_scalar() == 0x10000u);
	}
	{
		utf16_char ch = utf16_char::from_scalar_unchecked(details::encoding_constants::max_unicode_scalar);
		++ch;
		assert(ch == utf16_char::null_terminator);
	}
	{
		utf16_char ch = utf16_char::from_scalar_unchecked(0x10000u);
		--ch;
		assert(ch.as_scalar() == 0xFFFFu);
	}
	assert(u"A"_u16c.is_ascii());
	assert(u"A"_u16c.is_ascii_alphabetic());
	assert(u"7"_u16c.is_ascii_digit());
	assert(!u"\u03A9"_u16c.is_ascii());
	assert(u"\u03A9"_u16c.is_alphabetic());
	assert(u"\u03A9"_u16c.is_uppercase());
	assert(u"\u03C9"_u16c.is_lowercase());
	assert(u" "_u16c.is_ascii_whitespace());
	assert(u"A"_u16c.general_category() == unicode_general_category::uppercase_letter);
	assert(u" "_u16c.general_category() == unicode_general_category::space_separator);
	assert(u"\u20AC"_u16c.general_category() == unicode_general_category::currency_symbol);
	assert(utf16_char::from_scalar_unchecked(0x0378u).general_category() == unicode_general_category::unassigned);
	assert(u"\u0301"_u16c.canonical_combining_class() == 230u);
	assert(u"\u0301"_u16c.grapheme_break_property() == unicode_grapheme_break_property::extend);
	assert(u"\u03A9"_u16c.script() == unicode_script::greek);
	assert(u"!"_u16c.script() == unicode_script::common);
	assert(utf16_char::from_scalar_unchecked(0x0378u).script() == unicode_script::unknown);
	assert(u"A"_u16c.east_asian_width() == unicode_east_asian_width::narrow);
	assert(u"\u754C"_u16c.east_asian_width() == unicode_east_asian_width::wide);
	assert(u"\u03A9"_u16c.east_asian_width() == unicode_east_asian_width::ambiguous);
	assert(u"A"_u16c.line_break_class() == unicode_line_break_class::alphabetic);
	assert(u" "_u16c.line_break_class() == unicode_line_break_class::space);
	assert(u"A"_u16c.bidi_class() == unicode_bidi_class::left_to_right);
	assert(u"\u0634"_u16c.bidi_class() == unicode_bidi_class::arabic_letter);
	assert(u"A"_u16c.word_break_property() == unicode_word_break_property::a_letter);
	assert(u"7"_u16c.word_break_property() == unicode_word_break_property::numeric);
	assert(u"A"_u16c.sentence_break_property() == unicode_sentence_break_property::upper);
	assert(u"."_u16c.sentence_break_property() == unicode_sentence_break_property::a_term);
	assert(u"\U0001F600"_u16c.is_emoji());
	assert(!u"A"_u16c.is_emoji());
	assert(u"\U0001F600"_u16c.is_emoji_presentation());
	assert(!u"A"_u16c.is_emoji_presentation());
	assert(u"\U0001F600"_u16c.is_extended_pictographic());
	assert(!u"A"_u16c.is_extended_pictographic());
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

	// Direct UTF-8 view iteration, both forward and reverse.
	{
		constexpr std::u8string_view text = u8"A\u00E9\u20AC";
		[[maybe_unused]] const auto view = utf8_string_view::from_bytes(text).value().chars();
		assert(std::ranges::equal(view, std::array{ u8"A"_u8c, u8"\u00E9"_u8c, u8"\u20AC"_u8c }));
	}
	{
		const std::u8string text = u8"A\u00E9\u20AC";
		auto view = utf8_string_view::from_bytes(text).value().chars();
		assert(view.reserve_hint() == text.size());
		auto it = view.begin();
		assert(!(it == view.end()));
		assert(!(view.end() == it));
		const auto first = it++;
		assert(*first == u8"A"_u8c);
		assert(*it == u8"\u00E9"_u8c);
		++it;
		assert(*it == u8"\u20AC"_u8c);
		++it;
		assert(it == view.end());
		assert(view.end() == it);
	}
	{
		constexpr std::u8string_view text = u8"A\u00E9\u20AC";
		[[maybe_unused]] const auto view = utf8_string_view::from_bytes(text).value().reversed_chars();
		assert(std::ranges::equal(view, std::array{ u8"\u20AC"_u8c, u8"\u00E9"_u8c, u8"A"_u8c }));
	}
	{
		const std::u8string text = u8"A\u00E9\u20AC";
		auto view = utf8_string_view::from_bytes(text).value().reversed_chars();
		assert(view.reserve_hint() == text.size());
		auto it = view.begin();
		assert(!(it == view.end()));
		assert(!(view.end() == it));
		const auto first = it++;
		assert(*first == u8"\u20AC"_u8c);
		assert(*it == u8"\u00E9"_u8c);
		++it;
		assert(*it == u8"A"_u8c);
		++it;
		assert(it == view.end());
		assert(view.end() == it);
	}

	// Direct UTF-16 view iteration, both forward and reverse.
	{
		constexpr std::u16string_view text = u"Aé😀";
		[[maybe_unused]] const auto view = utf16_string_view::from_code_units(text).value().chars();
		assert(std::ranges::equal(view, std::array{ u"A"_u16c, u"é"_u16c, u"😀"_u16c }));
	}
	{
		const std::u16string text = u"Aé😀";
		std::string decoded;
		for (const utf16_char ch : utf16_string_view::from_code_units(text).value().chars())
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "Aé😀");
	}
		{
			constexpr std::u16string_view text = u"Aé😀";
			[[maybe_unused]] const auto view = utf16_string_view::from_code_units(text).value().reversed_chars();
			assert(std::ranges::equal(view, std::array{ u"😀"_u16c, u"é"_u16c, u"A"_u16c }));
		}
		{
			const std::u16string text = u"Aé😀";
			std::string decoded;
			for (const utf16_char ch : utf16_string_view::from_code_units(text).value().reversed_chars())
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
static_assert(std::same_as<decltype(utf8_string{}.reverse()), utf8_string&>);
static_assert(std::same_as<decltype(utf8_string{}.reverse_graphemes()), utf8_string&>);
static_assert(noexcept(utf8_string{}.reverse()));
static_assert(noexcept(utf8_string{}.reverse_graphemes()));
static_assert([] {
	auto s = u8"e\u0301\U0001F1F7\U0001F1F4!"_utf8_s;
	s.reverse_graphemes();
	return s == u8"!\U0001F1F7\U0001F1F4e\u0301"_utf8_sv;
}());
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
		s.replace_inplace(0, 1, s.as_view());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace_inplace(0, s.as_view());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace_with_range_inplace(0, 1, s.chars());
		assert(s == u8"Aéé"_utf8_sv);
	}
	{
		auto s = u8"Aé"_utf8_s;
		s.replace_with_range_inplace(0, s.chars());
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
static_assert(std::same_as<decltype(utf16_string{}.reverse()), utf16_string&>);
static_assert(std::same_as<decltype(utf16_string{}.reverse_graphemes()), utf16_string&>);
static_assert(noexcept(utf16_string{}.reverse()));
static_assert(noexcept(utf16_string{}.reverse_graphemes()));
static_assert([] {
	auto s = u"e\u0301\U0001F1F7\U0001F1F4!"_utf16_s;
	s.reverse_graphemes();
	return s == u"!\U0001F1F7\U0001F1F4e\u0301"_utf16_sv;
}());
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
		s.replace_inplace(0, 1, s.as_view());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace_inplace(0, s.as_view());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace_with_range_inplace(0, 1, s.chars());
		assert(s == u"Aéé"_utf16_sv);
	}
	{
		auto s = u"Aé"_utf16_s;
		s.replace_with_range_inplace(0, s.chars());
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
		auto s = u"A\u00E9\U0001F600"_utf16_s;
		s.reverse();
		assert(s == u"\U0001F600\u00E9A"_utf16_sv);
	}
	{
		auto s = u"e\u0301\U0001F1F7\U0001F1F4!"_utf16_s;
		s.reverse_graphemes();
		assert(s == u"!\U0001F1F7\U0001F1F4e\u0301"_utf16_sv);
	}
	{
		auto s = u"A\u00E9B\U0001F600C"_utf16_s;
		s.reverse(1, 4);
		assert(s == u"A\U0001F600B\u00E9C"_utf16_sv);
	}
	{
		auto s = u"Xe\u0301\U0001F1F7\U0001F1F4!Y"_utf16_s;
		s.reverse_graphemes(1, 7);
		assert(s == u"X!\U0001F1F7\U0001F1F4e\u0301Y"_utf16_sv);
	}
	{
		auto s = u"A\u00E9"_utf16_s;
		s.reverse(s.size(), 0);
		assert(s == u"A\u00E9"_utf16_sv);
	}
	{
		auto s = u"A\u00E9"_utf16_s;
		s.reverse_graphemes(s.size(), 0);
		assert(s == u"A\u00E9"_utf16_sv);
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
		s.replace_inplace(1, 1, u"Ω"_utf16_sv);
		assert(s == u"AΩ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_inplace(1, 1, u"Ω"_u16c);
		assert(s == u"AΩ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_inplace(1, u"XYZ"_utf16_sv);
		assert(s == u"AXYZ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_inplace(1, u"Ω"_u16c);
		assert(s == u"AΩ😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_with_range_inplace(1, 1, std::array{ u"Ω"_u16c, u"!"_u16c });
		assert(s == u"AΩ!😀"_utf16_sv);
	}
	{
		auto s = u"Aé😀"_utf16_s;
		s.replace_with_range_inplace(1, std::array{ u"Ω"_u16c, u"!"_u16c });
		assert(s == u"AΩ!😀"_utf16_sv);
	}
	{
		auto s = u"aaaa"_utf16_s;
		[[maybe_unused]] const auto replaced = s.replace_all(u"aa"_utf16_sv, u"x"_utf16_sv);
		assert(s == u"aaaa"_utf16_sv);
		assert(replaced == u"xx"_utf16_sv);
	}
	{
		auto s = u"aaaa"_utf16_s;
		[[maybe_unused]] const auto replaced = s.replace_n(1, u"aa"_utf16_sv, u"x"_utf16_sv);
		assert(s == u"aaaa"_utf16_sv);
		assert(replaced == u"xaa"_utf16_sv);
	}
	{
		auto s = u"A\u00E9B\u00E9"_utf16_s;
		[[maybe_unused]] const auto replaced = s.replace_all(u"\u00E9"_u16c, u"!"_u16c);
		assert(s == u"A\u00E9B\u00E9"_utf16_sv);
		assert(replaced == u"A!B!"_utf16_sv);
	}
	{
		auto s = u"cabca"_utf16_s;
		s.reserve(64);
		[[maybe_unused]] const auto* original = s.base().data();
		[[maybe_unused]] auto replaced = std::move(s).replace_all(u"a"_u16c, u"z"_u16c);
		assert(replaced == u"czbcz"_utf16_sv);
		assert(replaced.base().data() == original);
	}
	{
		auto s = u"prefixabcdefghijmiddleabcdefghijsuffix"_utf16_s;
		s.reserve(128);
		[[maybe_unused]] const auto* original = s.base().data();
		[[maybe_unused]] auto replaced = std::move(s).replace_all(u"abcdefghij"_utf16_sv, u"ABCDEFGHIJ"_utf16_sv);
		assert(replaced == u"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix"_utf16_sv);
		assert(replaced.base().data() == original);
	}
	{
		auto s = u"aaaa"_utf16_s;
		s.reserve(64);
		[[maybe_unused]] auto replaced = std::move(s).replace_n(1, u"aa"_utf16_sv, u"x"_utf16_sv);
		assert(replaced == u"xaa"_utf16_sv);
	}
	{
		auto s = u"prefixabcdefghijmiddleabcdefghijsuffix"_utf16_s;
		s.reserve(128);
		[[maybe_unused]] const auto* original = s.base().data();
		[[maybe_unused]] auto replaced = std::move(s).replace_n(1, u"abcdefghij"_utf16_sv, u"ABCDEFGHIJ"_utf16_sv);
		assert(replaced == u"prefixABCDEFGHIJmiddleabcdefghijsuffix"_utf16_sv);
		assert(replaced.base().data() == original);
	}
	{
		assert(std::ranges::equal(u"e\u0301🇷🇴!"_utf16_sv.graphemes(), std::array{
			u"e\u0301"_grapheme_utf16,
			u"🇷🇴"_grapheme_utf16,
			u"!"_grapheme_utf16
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
		auto s = u8"A\u00E9\U0001F600"_utf8_s;
		s.reverse();
		assert(s == u8"\U0001F600\u00E9A"_utf8_sv);
	}
	{
		auto s = u8"e\u0301\U0001F1F7\U0001F1F4!"_utf8_s;
		s.reverse_graphemes();
		assert(s == u8"!\U0001F1F7\U0001F1F4e\u0301"_utf8_sv);
	}
	{
		auto s = u8"A\u00E9B\U0001F600C"_utf8_s;
		s.reverse(1, 7);
		assert(s == u8"A\U0001F600B\u00E9C"_utf8_sv);
	}
	{
		auto s = u8"Xe\u0301\U0001F1F7\U0001F1F4!Y"_utf8_s;
		s.reverse_graphemes(1, 12);
		assert(s == u8"X!\U0001F1F7\U0001F1F4e\u0301Y"_utf8_sv);
	}
	{
		auto s = u8"A\u00E9"_utf8_s;
		s.reverse(s.size(), 0);
		assert(s == u8"A\u00E9"_utf8_sv);
	}
	{
		auto s = u8"A\u00E9"_utf8_s;
		s.reverse_graphemes(s.size(), 0);
		assert(s == u8"A\u00E9"_utf8_sv);
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
		s.replace_inplace(1, 2, "Ω"_utf8_sv);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_inplace(1, 2, "Ω"_u8c);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_inplace(1, "XYZ"_utf8_sv);
		assert(s == "AXYZ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_inplace(1, "Ω"_u8c);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_with_range_inplace(1, 2, std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "AΩ!€"_utf8_sv);
	}
	{
		auto s = "Aé€"_utf8_s;
		s.replace_with_range_inplace(1, std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "AΩ!€"_utf8_sv);
	}
	{
		auto s = u8"aaaa"_utf8_s;
		[[maybe_unused]] const auto replaced = s.replace_all(u8"aa"_utf8_sv, u8"x"_utf8_sv);
		assert(s == u8"aaaa"_utf8_sv);
		assert(replaced == u8"xx"_utf8_sv);
	}
	{
		auto s = u8"aaaa"_utf8_s;
		[[maybe_unused]] const auto replaced = s.replace_n(1, u8"aa"_utf8_sv, u8"x"_utf8_sv);
		assert(s == u8"aaaa"_utf8_sv);
		assert(replaced == u8"xaa"_utf8_sv);
	}
	{
		auto s = u8"A\u00E9B\u00E9"_utf8_s;
		[[maybe_unused]] const auto replaced = s.replace_all(u8"\u00E9"_u8c, u8"!"_u8c);
		assert(s == u8"A\u00E9B\u00E9"_utf8_sv);
		assert(replaced == u8"A!B!"_utf8_sv);
	}
	{
		auto s = u8"cabca"_utf8_s;
		s.reserve(64);
		[[maybe_unused]] const auto* original = s.base().data();
		[[maybe_unused]] auto replaced = std::move(s).replace_all(u8"a"_u8c, u8"z"_u8c);
		assert(replaced == u8"czbcz"_utf8_sv);
		assert(replaced.base().data() == original);
	}
	{
		auto s = u8"prefixabcdefghijmiddleabcdefghijsuffix"_utf8_s;
		s.reserve(128);
		[[maybe_unused]] const auto* original = s.base().data();
		[[maybe_unused]] auto replaced = std::move(s).replace_all(u8"abcdefghij"_utf8_sv, u8"ABCDEFGHIJ"_utf8_sv);
		assert(replaced == u8"prefixABCDEFGHIJmiddleABCDEFGHIJsuffix"_utf8_sv);
		assert(replaced.base().data() == original);
	}
	{
		auto s = u8"aaaa"_utf8_s;
		s.reserve(64);
		[[maybe_unused]] auto replaced = std::move(s).replace_n(1, u8"aa"_utf8_sv, u8"x"_utf8_sv);
		assert(replaced == u8"xaa"_utf8_sv);
	}
	{
		auto s = u8"prefixabcdefghijmiddleabcdefghijsuffix"_utf8_s;
		s.reserve(128);
		[[maybe_unused]] const auto* original = s.base().data();
		[[maybe_unused]] auto replaced = std::move(s).replace_n(1, u8"abcdefghij"_utf8_sv, u8"ABCDEFGHIJ"_utf8_sv);
		assert(replaced == u8"prefixABCDEFGHIJmiddleabcdefghijsuffix"_utf8_sv);
		assert(replaced.base().data() == original);
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
		if (!expect_out_of_range([&] { s.replace_inplace(decltype(s)::npos, 1, "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_inplace(2, 1, "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_inplace(1, 1, "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_inplace(s.size(), "Ω"_utf8_sv); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_inplace(2, "Ω"_u8c); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_with_range_inplace(decltype(s)::npos, 1, std::array{ "Ω"_u8c }); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.replace_with_range_inplace(2, std::array{ "Ω"_u8c }); }))
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
	{
		const auto result = utf8_string::from_bytes(std::string_view{ "\xE2\x82", 2 });
		assert(!result.has_value());
		assert(result.error().code == utf8_error_code::truncated_sequence);
		assert(result.error().first_invalid_byte_index == 0);
	}
	{
		const auto result = utf8_string::from_bytes(std::string_view{ "\xE0\x80\x80", 3 });
		assert(!result.has_value());
		assert(result.error().code == utf8_error_code::invalid_sequence);
		assert(result.error().first_invalid_byte_index == 0);
	}
	{
		const auto result = utf8_string::from_bytes(std::string_view{ "\xED\xA0\x80", 3 });
		assert(!result.has_value());
		assert(result.error().code == utf8_error_code::invalid_sequence);
		assert(result.error().first_invalid_byte_index == 0);
	}
	{
		const auto result = utf8_string::from_bytes(std::string_view{ "\xF4\x90\x80\x80", 4 });
		assert(!result.has_value());
		assert(result.error().code == utf8_error_code::invalid_sequence);
		assert(result.error().first_invalid_byte_index == 0);
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
	{
		const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
		auto view = views::lossy_utf8_view<char>{ input };
		auto it = view.begin();
		assert(!(it == view.end()));
		assert(!(view.end() == it));
		const auto first = it++;
		assert(*first == u8"A"_u8c);
		assert(*it == utf8_char::replacement_character);
	}
	{
		const std::array<char8_t, 5> input{
			static_cast<char8_t>('A'),
			static_cast<char8_t>(0xFFu),
			static_cast<char8_t>(0xE2u),
			static_cast<char8_t>(0x28u),
			static_cast<char8_t>(0xA1u)
		};
		std::string decoded;
		for (const utf8_char ch : views::lossy_utf8_view<char8_t>{ std::u8string_view{ input.data(), input.size() } })
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		std::string expected = "A";
		utf8_char::replacement_character.encode_utf8<char>(std::back_inserter(expected));
		utf8_char::replacement_character.encode_utf8<char>(std::back_inserter(expected));
		expected.push_back('(');
		utf8_char::replacement_character.encode_utf8<char>(std::back_inserter(expected));
		assert(decoded == expected);
	}
	{
		const std::array<char8_t, 5> input{
			static_cast<char8_t>('A'),
			static_cast<char8_t>(0xFFu),
			static_cast<char8_t>(0xE2u),
			static_cast<char8_t>(0x28u),
			static_cast<char8_t>(0xA1u)
		};
		auto view = views::lossy_utf8_view<char8_t>{ std::u8string_view{ input.data(), input.size() } };
		auto it = view.begin();
		assert(!(it == view.end()));
		assert(!(view.end() == it));
		const auto first = it++;
		assert(*first == u8"A"_u8c);
		assert(*it == utf8_char::replacement_character);
	}
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
		auto view = views::lossy_utf16_view<char16_t>{ input };
		auto it = view.begin();
		assert(!(it == view.end()));
		assert(!(view.end() == it));
		const auto first = it++;
		assert(*first == u"A"_u16c);
		assert(*it == utf16_char::replacement_character);
	}
	{
		const std::array<wchar_t, 6> input{
			static_cast<wchar_t>(L'A'),
			static_cast<wchar_t>(0xD800u),
			static_cast<wchar_t>(L'B'),
			static_cast<wchar_t>(0xDC00u),
			static_cast<wchar_t>(0xD83Du),
			static_cast<wchar_t>(0xDE00u)
		};
		std::string decoded;
		for (const utf16_char ch : views::lossy_utf16_view<wchar_t>{ std::basic_string_view<wchar_t>{ input.data(), input.size() } })
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		std::string expected = "A";
		utf16_char::replacement_character.encode_utf8<char>(std::back_inserter(expected));
		expected.push_back('B');
		utf16_char::replacement_character.encode_utf8<char>(std::back_inserter(expected));
		u"\U0001F600"_u16c.encode_utf8<char>(std::back_inserter(expected));
		assert(decoded == expected);
	}
	{
		const std::array<wchar_t, 6> input{
			static_cast<wchar_t>(L'A'),
			static_cast<wchar_t>(0xD800u),
			static_cast<wchar_t>(L'B'),
			static_cast<wchar_t>(0xDC00u),
			static_cast<wchar_t>(0xD83Du),
			static_cast<wchar_t>(0xDE00u)
		};
		auto view = views::lossy_utf16_view<wchar_t>{ std::basic_string_view<wchar_t>{ input.data(), input.size() } };
		auto it = view.begin();
		assert(!(it == view.end()));
		assert(!(view.end() == it));
		const auto first = it++;
		assert(*first == u"A"_u16c);
		assert(*it == utf16_char::replacement_character);
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
	{
		utf16_string s{ utf16_text };
		s.reverse(s.size(), 0);
		assert(s == utf16_text);
	}
	{
		utf16_string s{ utf16_text };
		if (!expect_out_of_range([&] { s.reverse(decltype(s)::npos); }))
		{
			assert(false);
		}
	}
	{
		utf16_string s{ utf16_text };
		if (!expect_out_of_range([&] { s.reverse(1, 4); }))
		{
			assert(false);
		}
	}
	{
		utf16_string s{ utf16_text };
		if (!expect_out_of_range([&] { s.reverse(3, 1); }))
		{
			assert(false);
		}
	}
	{
		utf16_string s{ utf16_text };
		if (!expect_out_of_range([&] { s.reverse(2, 1); }))
		{
			assert(false);
		}
	}
	{
		auto s = u"e\u0301\U0001F1F7\U0001F1F4!"_utf16_s;
		if (!expect_out_of_range([&] { s.reverse_graphemes(1, 6); }))
		{
			assert(false);
		}
	}
	{
		auto s = u"e\u0301\U0001F1F7\U0001F1F4!"_utf16_s;
		if (!expect_out_of_range([&] { s.reverse_graphemes(2, 2); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.reverse(decltype(s)::npos); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.reverse(1, 6); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.reverse(2, 1); }))
		{
			assert(false);
		}
	}
	{
		utf8_string s{ utf8_text };
		if (!expect_out_of_range([&] { s.reverse(1, 1); }))
		{
			assert(false);
		}
	}
	{
		auto s = u8"e\u0301\U0001F1F7\U0001F1F4!"_utf8_s;
		if (!expect_out_of_range([&] { s.reverse_graphemes(1, 11); }))
		{
			assert(false);
		}
	}
	{
		auto s = u8"e\u0301\U0001F1F7\U0001F1F4!"_utf8_s;
		if (!expect_out_of_range([&] { s.reverse_graphemes(3, 4); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u8"A\u00E9B"_utf8_sv.to_uppercase(utf8_string::npos, 0)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u8"A\u00E9B"_utf8_sv.to_lowercase(2, 1)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u8"A\u00E9B"_utf8_s.to_ascii_uppercase(2, 1)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u8"A\u00E9B"_utf8_s.to_ascii_lowercase(1, 4)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u"A\U0001F600B"_utf16_sv.to_uppercase(utf16_string::npos, 0)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u"A\U0001F600B"_utf16_sv.to_lowercase(2, 1)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u"A\U0001F600B"_utf16_s.to_ascii_uppercase(2, 1)); }))
		{
			assert(false);
		}
	}
	{
		if (!expect_out_of_range([&] { static_cast<void>(u"A\U0001F600B"_utf16_s.to_ascii_lowercase(1, 4)); }))
		{
			assert(false);
		}
	}
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // UTF8_RANGES_TESTS_HPP
