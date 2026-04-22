#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto left = "A"_u8c;
	auto right = "Z"_u8c;
	left.swap(right);

	std::println("{}", "x"_u8c.ascii_uppercase());                 // X
	std::println("{}", "É"_u8c.ascii_lowercase());                 // É
	std::println("{}", "x"_u8c.eq_ignore_ascii_case("X"_u8c));     // true
	std::println("{}", left);                                      // Z
	std::println("{}", right);                                     // A
}
