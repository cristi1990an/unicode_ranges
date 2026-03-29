#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "😄🇷🇴✨"_utf8_sv;
	constexpr auto sparkle = "✨"_u8c;
	constexpr auto flag = "🇷🇴"_grapheme_utf8;
	auto owned = "😄✨"_utf8_s;

	std::println("{}", text);                              // 😄🇷🇴✨
	std::println("{}", text.find(sparkle));               // 12
	std::println("{}", text.chars());                     // [😄, 🇷, 🇴, ✨]
	std::println("{::s}", text.graphemes());              // [😄, 🇷🇴, ✨]
	std::println("{}", flag);                             // 🇷🇴
	std::println("{}", owned.replace_all("✨"_u8c, "🔥"_u8c)); // 😄🔥
}
