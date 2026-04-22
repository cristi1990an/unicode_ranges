#include "unicode_ranges_full.hpp"

#include <format>
#include <print>
#include <string>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const auto sparkle = "✨"_u8c;
	const utf16_char sparkle16 = sparkle;

	std::string encoded8;
	sparkle.encode_utf8<char>(std::back_inserter(encoded8));

	std::u16string encoded16;
	sparkle.encode_utf16<char16_t>(std::back_inserter(encoded16));

	std::println("{}", std::format("{:X}", sparkle.as_scalar()));              // 2728
	std::println("{}", sparkle16);                                             // ✨
	std::println("{}", sparkle.to_utf8_owned());                               // ✨
	std::println("{}", sparkle16.to_utf16_owned());                            // ✨
	std::println("{}", sparkle.code_unit_count());                             // 3
	std::println("{}", encoded8);                                              // ✨
	std::println("{}", utf16_string_view::from_code_units(encoded16).value()); // ✨
}
