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

	std::println("text: {}", text);                                // 😄🇷🇴✨
	std::println("find('✨'): {}", text.find(sparkle));            // 12
	std::println("chars(): {}", text.chars());                     // [😄, 🇷, 🇴, ✨]
	std::println("graphemes(): {::s}", text.graphemes());          // [😄, 🇷🇴, ✨]
	std::println("grapheme literal: {}", flag);                    // 🇷🇴
	std::println("replace('✨', '🔥'): {}", owned.replace_all("✨"_u8c, "🔥"_u8c)); // 😄🔥
}
