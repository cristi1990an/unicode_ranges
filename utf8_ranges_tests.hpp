#ifndef UTF8_RANGES_TESTS_HPP
#define UTF8_RANGES_TESTS_HPP

#include "utf8_ranges.hpp"

#include <array>
#include <cassert>
#include <format>
#include <functional>
#include <sstream>
#include <string>

using namespace utf8_ranges;
using namespace utf8_ranges::literals;

inline void run_utf8_ranges_tests()
{
	constexpr utf8_char latin1_ch = "é"_u8c;
	constexpr auto utf8_text = "Aé€"_utf8_sv;

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
	static_assert(utf8_text.find(static_cast<char8_t>('A')) == 0);
	static_assert(utf8_text.find(static_cast<char8_t>(0xA9), 2) == 2);
	static_assert(utf8_text.find(static_cast<char8_t>('A'), utf8_string_view::npos) == utf8_string_view::npos);
	static_assert(utf8_text.find("é€"_utf8_sv) == 1);
	static_assert(utf8_text.find("é€"_utf8_sv, 2) == utf8_string_view::npos);
	static_assert(utf8_text.find("€"_utf8_sv, 2) == 3);
	static_assert(utf8_text.find("€"_utf8_sv, utf8_string_view::npos) == utf8_string_view::npos);
	static_assert(utf8_text.find("é"_u8c, 2) == utf8_string_view::npos);
	static_assert(utf8_text.find("€"_u8c, 2) == 3);
	static_assert(utf8_text.find("€"_u8c, utf8_string_view::npos) == utf8_string_view::npos);
	static_assert(utf8_text.find("é"_u8c) == 1);
	static_assert(utf8_text.find("€"_u8c) == 3);
	static_assert(utf8_text.find("Z"_u8c) == utf8_string_view::npos);
	static_assert(utf8_text.rfind(static_cast<char8_t>('A')) == 0);
	static_assert(utf8_text.rfind(static_cast<char8_t>(0xA9), 2) == 2);
	static_assert(utf8_text.rfind(static_cast<char8_t>('A'), 0) == 0);
	static_assert(utf8_text.rfind("é€"_utf8_sv) == 1);
	static_assert(utf8_text.rfind("é€"_utf8_sv, 2) == 1);
	static_assert(utf8_text.rfind("€"_utf8_sv, 2) == utf8_string_view::npos);
	static_assert(utf8_text.rfind("€"_utf8_sv, utf8_string_view::npos) == 3);
	static_assert(utf8_text.rfind("é"_u8c, 2) == 1);
	static_assert(utf8_text.rfind("€"_u8c, 2) == utf8_string_view::npos);
	static_assert(utf8_text.rfind("€"_u8c) == 3);
	static_assert(utf8_text.rfind("Z"_u8c) == utf8_string_view::npos);
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

	assert(std::format("{}", "A"_u8c) == "A");
	assert(std::format("{:c}", latin1_ch) == "\xC3\xA9");
	assert(std::format("{:c}", "€"_u8c) == "\xE2\x82\xAC");
	assert(std::format("{:c}", "😀"_u8c) == "\xF0\x9F\x98\x80");

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
	assert(std::format("{:s}", utf8_text.chars() | std::ranges::to<utf8_string>()) == "A" "\xC3\xA9" "\xE2\x82\xAC");
	assert(std::format("{:>6s}", utf8_text.chars() | std::ranges::to<utf8_string>()) == "   A" "\xC3\xA9" "\xE2\x82\xAC");
	assert(std::format("{:_<6s}", utf8_text.reversed_chars() | std::ranges::to<utf8_string>()) == "\xE2\x82\xAC" "\xC3\xA9" "A___");

	assert(utf8_string{}.base().empty());
	static_assert(std::same_as<decltype(utf8_string{}.get_allocator()), std::allocator<char8_t>>);
	assert(utf8_string{ utf8_text }.as_view() == "Aé€"_utf8_sv);
	assert(std::format("{}", utf8_string{ utf8_text }) == "A\xC3\xA9\xE2\x82\xAC");
	{
		const auto e_acute = utf8_char::from_scalar_unchecked(0x00E9u);
		const utf8_string lhs{ "A"_utf8_sv };
		const utf8_string rhs(std::from_range, std::array{ e_acute });
		const utf8_string expected(std::from_range, std::array{ "A"_u8c, e_acute });

		assert((lhs + rhs.as_view()).as_view() == expected.as_view());
		assert((rhs.as_view() + lhs).as_view() == utf8_string(std::from_range, std::array{ e_acute, "A"_u8c }).as_view());
		assert((lhs + e_acute).as_view() == expected.as_view());
		assert((e_acute + lhs).as_view() == utf8_string(std::from_range, std::array{ e_acute, "A"_u8c }).as_view());
		assert((std::move(utf8_string{ "A"_utf8_sv }) + rhs.as_view()).as_view() == expected.as_view());
		assert((rhs.as_view() + std::move(utf8_string{ "A"_utf8_sv })).as_view() == utf8_string(std::from_range, std::array{ e_acute, "A"_u8c }).as_view());
	}
	{
		const auto reversed = utf8_text.reversed_chars() | std::ranges::to<utf8_string>();
		assert(reversed == "€éA"_utf8_sv);
	}
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
		utf8_string s{ "Aé€"_utf8_sv };
		s.assign(3, "!"_u8c);
		assert(s == "!!!"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.assign_range(std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "Ω!"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		const std::array data{ "β"_u8c, "γ"_u8c };
		s.assign(data.begin(), data.end());
		assert(s == "βγ"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.assign({ "x"_u8c, "y"_u8c, "z"_u8c });
		assert(s == "xyz"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.erase(1, 2);
		assert(s == "A€"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.erase(1);
		assert(s == "A"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.replace(1, 2, "Ω"_utf8_sv);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.replace(1, 2, "Ω"_u8c);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.replace(1, "XYZ"_utf8_sv);
		assert(s == "AXYZ€"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.replace(1, "Ω"_u8c);
		assert(s == "AΩ€"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
		s.replace_with_range(1, 2, std::array{ "Ω"_u8c, "!"_u8c });
		assert(s == "AΩ!€"_utf8_sv);
	}
	{
		utf8_string s{ "Aé€"_utf8_sv };
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
		bool threw = false;
		try
		{
			s.erase(decltype(s)::npos);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.erase(2, 1);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.erase(1, 1);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace(decltype(s)::npos, 1, "Ω"_utf8_sv);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace(2, 1, "Ω"_utf8_sv);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace(1, 1, "Ω"_utf8_sv);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace(s.size(), "Ω"_utf8_sv);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace(2, "Ω"_u8c);
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace_with_range(decltype(s)::npos, 1, std::array{ "Ω"_u8c });
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}
	{
		utf8_string s{ utf8_text };
		bool threw = false;
		try
		{
			s.replace_with_range(2, std::array{ "Ω"_u8c });
		}
		catch (const std::out_of_range&)
		{
			threw = true;
		}
		assert(threw);
	}

	assert(std::format("{}", utf8_text) == "A\xC3\xA9\xE2\x82\xAC");
	assert(std::hash<utf8_string_view>{}(utf8_text) == std::hash<utf8_string_view>{}("Aé€"_utf8_sv));
	{
		std::ostringstream oss;
		oss << utf8_text;
		assert(oss.str() == "A\xC3\xA9\xE2\x82\xAC");
	}
	{
		std::ostringstream oss;
		oss << utf8_string{ utf8_text };
		assert(oss.str() == "A\xC3\xA9\xE2\x82\xAC");
	}

	assert(!utf8_char::from_scalar(0x110000u).has_value());
	assert(std::format("{}", utf8_char::replacement_character) == "\xEF\xBF\xBD");
	assert(std::format("{:x}", utf8_char::replacement_character) == "fffd");
	assert(std::format("{:#08x}", utf8_char::replacement_character) == "0x00fffd");

	{
		const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
		std::string decoded;
		for (const utf8_char ch : views::lossy_utf8_view<char>{ input })
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "A\xEF\xBF\xBD\xEF\xBF\xBD(\xEF\xBF\xBD");
	}
	{
		const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
		std::string decoded;
		for (const utf8_char ch : input | views::lossy_utf8)
		{
			ch.encode_utf8<char>(std::back_inserter(decoded));
		}
		assert(decoded == "A\xEF\xBF\xBD\xEF\xBF\xBD(\xEF\xBF\xBD");
	}
}

#endif // UTF8_RANGES_TESTS_HPP
