#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto sparkle = "✨"_u8c;
	constexpr auto grin = u"😄"_u16c;
	constexpr auto rocket = U"🚀"_u32c;

	std::println("{}", sparkle);  // ✨
	std::println("{}", grin);     // 😄
	std::println("{}", rocket);   // 🚀
}
