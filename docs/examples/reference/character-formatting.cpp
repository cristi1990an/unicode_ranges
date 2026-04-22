#include "unicode_ranges_full.hpp"

#include <format>
#include <functional>
#include <print>
#include <sstream>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto euro = "€"_u8c;
	const auto grin = u"😄"_u16c;

	std::ostringstream out;
	out << euro;

	std::println("{}", euro == "€"_u8c);                                      // true
	std::println("{}", std::hash<utf8_char>{}(euro) == std::hash<utf8_char>{}("€"_u8c)); // true
	std::println("{}", out.str());                                             // €
	std::println("{}", std::format("{:X}", euro));                             // 20AC
	std::println("{}", std::format("{}", grin));                               // 😄
}
