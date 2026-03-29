#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto sparkle = utf8_char::from_scalar(U'✨').value();
	const auto smile = utf16_char::from_scalar(U'😄').value();

	std::println("{}", sparkle);                    // ✨
	std::println("{}", sparkle.code_unit_count());  // 3
	std::println("{}", smile);                      // 😄
	std::println("{}", smile.code_unit_count());    // 2
	std::println("{}", "x"_u8c.ascii_uppercase());  // X
}
