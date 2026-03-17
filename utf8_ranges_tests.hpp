#pragma once

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
	constexpr utf8_char latin1_ch = u8"\u00E9"_u8c;
	constexpr auto utf8_text = u8"A\u00E9\u20AC"_utf8_sv;

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

	static_assert(u8"\u03A9"_u8c.is_alphabetic());
	static_assert(u8"\u03A9"_u8c.is_alphanumeric());
	static_assert(u8"\u03A9"_u8c.is_uppercase());
	static_assert(!u8"\u03A9"_u8c.is_lowercase());
	static_assert(u8"\u03C9"_u8c.is_lowercase());
	static_assert(!u8"\u03C9"_u8c.is_uppercase());
	static_assert("5"_u8c.is_digit());
	static_assert("5"_u8c.is_numeric());
	static_assert(u8"\u2167"_u8c.is_numeric());
	static_assert(!u8"\u2167"_u8c.is_digit());
	static_assert(u8"\u2003"_u8c.is_whitespace());
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
	static_assert(utf8_text.as_view() == u8"A\u00E9\u20AC");
	static_assert(utf8_text.is_char_boundary(0));
	static_assert(utf8_text.is_char_boundary(1));
	static_assert(!utf8_text.is_char_boundary(2));
	static_assert(utf8_text.find(u8'A') == 0);
	static_assert(utf8_text.find("é"_u8c) == 1);
	static_assert(utf8_text.find("€"_u8c) == 3);
	static_assert(utf8_text.find("Z"_u8c) == utf8_string_view::npos);
	static_assert(utf8_text.substr(1).has_value());
	static_assert(utf8_text.substr(1).value() == u8"\u00E9\u20AC"_utf8_sv);
	static_assert(utf8_text.substr(1, 2).value() == u8"\u00E9"_utf8_sv);
	static_assert(!utf8_text.substr(2, 1).has_value());
	static_assert(utf8_string_view::from_bytes(u8"A\u00E9\u20AC").has_value());
	static_assert([] {
		const std::array<char8_t, 3> invalid_bytes{
			static_cast<char8_t>(0xE2),
			static_cast<char8_t>(0x28),
			static_cast<char8_t>(0xA1)
		};
		const auto invalid = utf8_string_view::from_bytes(
			std::u8string_view{ invalid_bytes.data(), invalid_bytes.size() });
		return !invalid.has_value()
			&& invalid.error().code == utf8_error_code::invalid_sequence
			&& invalid.error().first_invalid_byte_index == 0;
	}());
	static_assert(utf8_text == u8"A\u00E9\u20AC"_utf8_sv);
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
	assert(std::format("{:c}", u8"\u20AC"_u8c) == "\xE2\x82\xAC");
	assert(std::format("{:c}", u8"\U0001F600"_u8c) == "\xF0\x9F\x98\x80");

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

	assert(utf8_string<>{}.base().empty());
	assert(utf8_string<>{ utf8_text }.base() == u8"A\u00E9\u20AC");
	assert(std::format("{}", utf8_string<>{ utf8_text }) == "A\xC3\xA9\xE2\x82\xAC");

	assert(std::format("{}", utf8_text) == "A\xC3\xA9\xE2\x82\xAC");
	assert(std::hash<utf8_string_view>{}(utf8_text) == std::hash<utf8_string_view>{}(u8"A\u00E9\u20AC"_utf8_sv));
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
