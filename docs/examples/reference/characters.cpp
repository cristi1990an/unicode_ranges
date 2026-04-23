#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto sparkle = "✨"_u8c;
	const auto smile = u"😄"_u16c;
	const auto rocket = U"🚀"_u32c;

	std::println("{}", sparkle);                    // ✨
	std::println("{}", sparkle.code_unit_count());  // 3
	std::println("{}", smile);                      // 😄
	std::println("{}", smile.code_unit_count());    // 2
	std::println("{}", rocket);                     // 🚀
	std::println("{}", rocket.code_unit_count());   // 1
	std::println("{}", "x"_u8c.ascii_uppercase());  // X
}
