#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto utf8 = u8"Gr\u00FC\u00DFe \U0001F44B"_utf8_sv;
	const auto utf16 = utf8.to_utf16();
	const auto utf32 = utf8.to_utf32();

	std::println("{}", utf16);           // Grüße 👋
	std::println("{}", utf32);           // Grüße 👋
	std::println("{}", utf16.to_utf8()); // Grüße 👋
	std::println("{}", utf32.to_utf8()); // Grüße 👋
}
