#include <print>

#include "utf8_ranges_tests.hpp"

using namespace utf8_ranges;
using namespace utf8_ranges::literals;

int main()
{
	run_utf8_ranges_tests();
	

	auto sv = "Hello, 世界!"_utf8_sv;

	std::println("{}", sv.reversed_chars() | std::ranges::to<utf8_string>());
}
