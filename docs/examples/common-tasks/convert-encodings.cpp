#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto utf8 = "Grüße 👋"_utf8_sv;
	const auto utf16 = utf8.to_utf16();

	std::println("{}", utf16);           // Grüße 👋
	std::println("{}", utf16.to_utf8()); // Grüße 👋
}
