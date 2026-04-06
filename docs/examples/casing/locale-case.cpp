#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
#if UTF8_RANGES_HAS_ICU
	std::println("{}", u8"I\u0130"_utf8_sv.case_fold());                      // ii̇
	std::println("{}", u8"I\u0130"_utf8_sv.case_fold("tr"_locale));           // ıi
	std::println("{}", u8"I\u0130"_utf8_sv.to_lowercase("tr"_locale));        // ıi
	std::println("{}", u8"i\u0131"_utf8_sv.to_uppercase("tr"_locale));        // İI
	std::println("{}", u8"istanbul izmir"_utf8_sv.to_titlecase("tr"_locale)); // İstanbul İzmir
	std::println("{}", U"istanbul izmir"_utf32_sv.to_titlecase("tr"_locale)); // İstanbul İzmir
	std::println("{}", u8"I"_utf8_sv.eq_ignore_case(u8"\u0131"_utf8_sv, "tr"_locale)); // true
	std::println("{}", is_available_locale("tr"_locale));                     // true
#else
	std::println("Enable ICU-backed locale casing to use _locale.");
#endif
}
