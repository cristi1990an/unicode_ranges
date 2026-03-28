#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto title = "café noir"_utf8_s;

	std::println("whole string: {}", title.to_uppercase());                       // CAFÉ NOIR
	std::println("tail only: {}", title.to_uppercase(6, utf8_string::npos));      // café NOIR
}
