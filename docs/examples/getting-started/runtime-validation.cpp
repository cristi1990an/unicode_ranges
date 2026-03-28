#include "unicode_ranges.hpp"

#include <print>
#include <string>

using namespace unicode_ranges;

int main()
{
	std::string raw = "Grüße din România 👋";

	auto text = utf8_string::from_bytes(raw);
	if (!text)
	{
		std::println(stderr,
			"Invalid UTF-8 at byte {}",
			text.error().first_invalid_byte_index);
		return 1;
	}

	std::println("validated: {}", *text);                  // Grüße din România 👋
	std::println("characters: {}", text->char_count());    // 18
	std::println("first char: {}", text->front().value()); // G
	std::println("last char: {}", text->back().value());   // 👋
}
