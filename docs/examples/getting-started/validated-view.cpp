#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "é🇷🇴!"_utf8_sv;

	std::println("{}", text);                   // é🇷🇴!
	std::println("{}", text.size());            // 12 UTF-8 code units
	std::println("{}", text.char_count());      // 5 Unicode scalars
	std::println("{}", text.grapheme_count());  // 3 graphemes
	std::println("{}", text.find("!"_u8c));     // 11
	std::println("{}", text.find("🇷"_u8c));    // 3

	std::println("{}", text.chars());          // [e, ́, 🇷, 🇴, !]
	std::println("{::s}", text.graphemes());   // [é, 🇷🇴, !]
}
