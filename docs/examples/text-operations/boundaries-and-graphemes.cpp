#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "é🇷🇴!"_utf8_sv;

	std::println("is_char_boundary(1): {}", text.is_char_boundary(1));           // true
	std::println("is_grapheme_boundary(1): {}", text.is_grapheme_boundary(1));   // false
	std::println("ceil_grapheme_boundary(7): {}", text.ceil_grapheme_boundary(7));   // 11
	std::println("floor_grapheme_boundary(7): {}", text.floor_grapheme_boundary(7)); // 3

	std::println("chars(): {}", text.chars());             // [e, ́, 🇷, 🇴, !]
	std::println("graphemes(): {::s}", text.graphemes());  // [é, 🇷🇴, !]
}
