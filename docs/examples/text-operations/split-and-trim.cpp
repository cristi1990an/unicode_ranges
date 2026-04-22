#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto line = " café | thé | apă "_utf8_sv;
	constexpr auto framed = "***café***"_utf8_sv;

	for (auto part : line.split_trimmed("|"_utf8_sv))
	{
		std::println("[{}]", part);
	}
	// [café]
	// [thé]
	// [apă]

	std::println("{}", framed.trim_matches("*"_u8c)); // café
}
