#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	std::println("{}", "Straße"_utf8_sv.case_fold()); // strasse
}
