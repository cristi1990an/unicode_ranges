#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "é🇷🇴!"_utf8_sv;

	std::println("text: {}", text);                               // é🇷🇴!
	std::println("size(): {}", text.size());                      // 12 UTF-8 code units
	std::println("char_count(): {}", text.char_count());          // 5 Unicode scalars
	std::println("grapheme_count(): {}", text.grapheme_count());  // 3 graphemes
	std::println("find('!'): {}", text.find("!"_u8c));           // 11
	std::println("find('🇷'): {}", text.find("🇷"_u8c));          // 3

	std::println("chars(): {}", text.chars());             // [e, ́, 🇷, 🇴, !]
	std::println("graphemes(): {::s}", text.graphemes());  // [é, 🇷🇴, !]
}
