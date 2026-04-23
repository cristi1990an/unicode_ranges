#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto lhs = u8"\u00E9"_utf8_sv.to_nfd();
	const auto rhs = u8"e\u0301"_utf8_sv.to_nfd();

	std::println("{}", lhs == rhs);  // true
}
