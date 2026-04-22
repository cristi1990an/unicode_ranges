#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;

int main()
{
	const auto omega = utf8_char::from_scalar(U'Ω').value();
	const auto rocket = utf16_char::from_scalar(U'🚀').value();
	const auto invalid = utf8_char::from_scalar(0xD800u);

	std::println("{}", omega);              // Ω
	std::println("{}", rocket);             // 🚀
	std::println("{}", invalid.has_value()); // false
}
