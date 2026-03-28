#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto sparkle = utf8_char::from_scalar(U'✨').value();
	const auto smile = utf16_char::from_scalar(U'😄').value();

	std::println("utf8_char: {}", sparkle);                          // ✨
	std::println("utf8_char code units: {}", sparkle.code_unit_count()); // 3
	std::println("utf16_char: {}", smile);                          // 😄
	std::println("utf16_char code units: {}", smile.code_unit_count());  // 2
	std::println("ASCII upper('x'): {}", "x"_u8c.ascii_uppercase());   // X
}
