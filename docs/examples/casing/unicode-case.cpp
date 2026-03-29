#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "straße café"_utf8_sv;

	std::println("{}", text.to_ascii_uppercase());       // STRAßE CAFé
	std::println("{}", text.to_uppercase());             // STRASSE CAFÉ
	std::println("{}", "CAFÉ Ω"_utf8_sv.to_lowercase()); // café ω
}
