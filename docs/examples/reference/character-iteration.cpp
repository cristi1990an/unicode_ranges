#include "unicode_ranges_all.hpp"

#include <format>
#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto latin = "A"_u8c;
	++latin;

	auto before_surrogates = utf16_char::from_scalar(0xD7FFu).value();
	++before_surrogates;

	auto wrap = utf8_char::from_scalar(0x10FFFFu).value();
	++wrap;

	std::println("{}", latin);                                        // B
	std::println("{}", std::format("{:X}", before_surrogates.as_scalar())); // E000
	std::println("{}", wrap.as_scalar());                             // 0
}
