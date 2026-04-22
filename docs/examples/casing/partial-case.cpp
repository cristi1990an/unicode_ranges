#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto title = "café noir"_utf8_s;

	std::println("{}", title.to_uppercase());                                  // CAFÉ NOIR
	std::println("{}", title.to_uppercase(6, utf8_string::npos));               // café NOIR
}
