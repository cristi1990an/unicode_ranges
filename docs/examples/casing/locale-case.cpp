#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
#if UTF8_RANGES_HAS_ICU
	std::println("{}", "Iİ"_utf8_sv.case_fold());              // ii̇
	std::println("{}", "Iİ"_utf8_sv.case_fold("tr"_locale));   // ıi
	std::println("{}", "Iİ"_utf8_sv.to_lowercase("tr"_locale)); // ıi
	std::println("{}", "iı"_utf8_sv.to_uppercase("tr"_locale)); // İI
	std::println("{}", is_available_locale("tr"_locale));       // true
#else
	std::println("Enable ICU-backed locale casing to use _locale.");
#endif
}
