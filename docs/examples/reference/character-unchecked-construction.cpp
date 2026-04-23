#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;

int main()
{
	const auto trusted_utf8 = utf8_char::from_utf8_bytes_unchecked(u8"✨", 3);
	const auto trusted_scalar = utf16_char::from_scalar_unchecked(U'😄');
	const auto checked_utf16 = utf16_char::from_utf16_code_units(u"😄", 2).value();

	std::println("{}", trusted_utf8);    // ✨
	std::println("{}", trusted_scalar);  // 😄
	std::println("{}", checked_utf16);   // 😄
}
