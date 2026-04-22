#include "unicode_ranges_full.hpp"

#include <array>
#include <print>
#include <span>
#include <string_view>

using namespace unicode_ranges;

int main()
{
	constexpr std::array<char, 3> broken_utf8{ 'A', static_cast<char>(0xFF), 'B' };
	constexpr std::array<char16_t, 3> broken_utf16{ u'A', static_cast<char16_t>(0xD800u), u'B' };

	const auto utf8_lossy = std::span{ broken_utf8 } | views::lossy_utf8;
	const auto utf16_lossy = std::span{ broken_utf16 } | views::lossy_utf16;

	std::println("{}", utf8_lossy);   // [A, �, B]
	std::println("{}", utf16_lossy);  // [A, �, B]
}
