#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto composed = "é"_utf8_sv;
	constexpr auto decomposed = "é"_utf8_sv;

	std::println("{}", composed.is_nfc());    // true
	std::println("{}", decomposed.is_nfc());  // false
	std::println("{}", decomposed.is_nfd());  // true
}
