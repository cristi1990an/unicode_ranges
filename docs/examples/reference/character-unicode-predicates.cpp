#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	std::println("{}", "Ω"_u8c.is_alphabetic());  // true
	std::println("{}", "Ω"_u8c.is_ascii());       // false
	std::println("{}", "7"_u8c.is_digit());       // true
	std::println("{}", " "_u8c.is_whitespace());  // true
}
