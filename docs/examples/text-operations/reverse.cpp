#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto chars = "é🇷🇴!"_utf8_s;
	auto graphemes = chars;

	chars.reverse();
	graphemes.reverse_graphemes();

	std::println("{}", chars);      // !🇴🇷́e
	std::println("{}", graphemes);  // !🇷🇴é
}
