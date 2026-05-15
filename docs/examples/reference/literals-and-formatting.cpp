#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = u8"😄🇷🇴✨"_utf8_sv;
	constexpr auto sparkle = u8"✨"_u8c;
	constexpr auto flag = u8"🇷🇴"_grapheme_utf8;
	constexpr auto rocket = U"🚀"_u32c;
	constexpr auto text32 = U"😄🇷🇴✨"_utf32_sv;
	auto owned = U"😄✨"_utf32_s;

	std::println("{}", text);                              // 😄🇷🇴✨
	std::println("{}", text.find(sparkle));               // 12
	std::println("{}", text.chars());                     // [😄, 🇷, 🇴, ✨]
	std::println("{::s}", text.graphemes());              // [😄, 🇷🇴, ✨]
	std::println("{}", flag);                             // 🇷🇴
	std::println("{}", rocket);                           // 🚀
	std::println("{}", text32);                           // 😄🇷🇴✨
	std::println("{}", owned.replace_all(U"✨"_u32c, U"🔥"_u32c)); // 😄🔥
}
