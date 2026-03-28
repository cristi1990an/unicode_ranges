#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const utf8_string text = "mañana 👩‍💻"_utf8_s;

	std::println("text: {}", text);                      // mañana 👩‍💻
	std::println("chars(): {}", text.chars());           // [m, a, ñ, a, n, a,  , 👩, ‍, 💻]
	std::println("graphemes(): {::s}", text.graphemes());  // [m, a, ñ, a, n, a,  , 👩‍💻]
}
