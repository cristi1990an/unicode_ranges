#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = "Straße"_utf8_sv;

	std::println("{}", text.eq_ignore_case("STRASSE"_utf8_sv)); // true
	std::println("{}", text.starts_with_ignore_case("stras"_utf8_sv)); // true
	std::println("{}", text.ends_with_ignore_case("SSE"_utf8_sv)); // true
	std::println("{}", text.compare_ignore_case("strasse"_utf8_sv) == std::weak_ordering::equivalent); // true
	std::println("{}", !"\u00E9"_utf8_sv.eq_ignore_case("e\u0301"_utf8_sv)); // true
}
