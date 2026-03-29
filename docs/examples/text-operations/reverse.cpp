#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto text = "café noir"_utf8_s;

	text.reverse();
	std::println("{}", text); // rion éfac
}
