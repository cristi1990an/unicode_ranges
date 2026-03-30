#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto lhs = "é"_utf8_sv.to_nfd();
	const auto rhs = "é"_utf8_sv.to_nfd();

	std::println("{}", lhs == rhs);  // true
}
