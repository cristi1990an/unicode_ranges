#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;

int main()
{
	utf8_char zero8{};
	utf16_char zero16{};
	utf32_char zero32{};

	std::println("{}", zero8.as_scalar());                  // 0
	std::println("{}", zero16.as_scalar());                 // 0
	std::println("{}", zero32.as_scalar());                 // 0
	std::println("{}", utf8_char::replacement_character);   // �
	std::println("{}", utf16_char::null_terminator.as_scalar()); // 0
	std::println("{}", utf32_char::null_terminator.as_scalar()); // 0
}
