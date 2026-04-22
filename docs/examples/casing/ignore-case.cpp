#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = u8"Stra\u00DFe"_utf8_sv;
	constexpr auto text32 = U"Stra\u00DFe"_utf32_sv;

	std::println("{}", text.eq_ignore_case(u8"STRASSE"_utf8_sv)); // true
	std::println("{}", text.starts_with_ignore_case(u8"stras"_utf8_sv)); // true
	std::println("{}", text.ends_with_ignore_case(u8"SSE"_utf8_sv)); // true
	std::println("{}", text.compare_ignore_case(u8"strasse"_utf8_sv) == std::weak_ordering::equivalent); // true
	std::println("{}", !u8"\u00E9"_utf8_sv.eq_ignore_case(u8"e\u0301"_utf8_sv)); // true
	std::println("{}", text32.eq_ignore_case(U"STRASSE"_utf32_sv)); // true
}
