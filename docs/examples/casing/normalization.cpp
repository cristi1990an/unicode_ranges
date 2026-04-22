#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto composed = "é"_utf8_sv;
	constexpr auto decomposed = "é"_utf8_sv;

	std::println("{}", composed.to_nfd());         // é
	std::println("{}", decomposed.to_nfc());       // é
	std::println("{}", "Ａ"_utf8_sv.to_nfkc());    // A
}
