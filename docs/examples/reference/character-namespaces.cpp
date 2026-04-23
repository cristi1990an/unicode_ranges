#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;

int main()
{
	std::println("{}", characters::utf8::currency::euro_sign);        // €
	std::println("{}", characters::utf8::math::approximately_equal_to); // ≈
	std::println("{}", characters::utf8::emojis::clown_face);         // 🤡
	std::println("{}", characters::utf8::emojis::party_popper);       // 🎉

	std::println("{}", characters::utf16::currency::euro_sign);       // €
	std::println("{}", characters::utf16::arrows::right_arrow);       // →
	std::println("{}", characters::utf16::emojis::red_heart);         // ❤
	std::println("{}", characters::utf16::emojis::rocket);            // 🚀
	std::println("{}", characters::utf32::currency::euro_sign);       // €
	std::println("{}", characters::utf32::symbols::section_sign);     // §
	std::println("{}", characters::utf32::emojis::sparkles);          // ✨
	std::println("{}", characters::utf32::emojis::rocket);            // 🚀
}
