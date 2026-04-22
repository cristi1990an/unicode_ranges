#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "é🇷🇴!"_utf8_sv;

	std::println("{}", text.is_char_boundary(1));         // true
	std::println("{}", text.is_grapheme_boundary(1));     // false
	std::println("{}", text.ceil_grapheme_boundary(7));   // 11
	std::println("{}", text.floor_grapheme_boundary(7));  // 3

	std::println("{}", text.chars());          // [e, ́, 🇷, 🇴, !]
	std::println("{::s}", text.graphemes());   // [é, 🇷🇴, !]
}
