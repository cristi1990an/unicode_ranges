#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	std::println("{}", "A"_u8c.is_ascii_alphabetic());  // true
	std::println("{}", "9"_u8c.is_ascii_hexdigit());    // true
	std::println("{}", "é"_u8c.is_ascii_alphabetic());  // false
}
