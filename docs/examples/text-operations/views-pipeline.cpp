#include "unicode_ranges_all.hpp"

#include <print>
#include <ranges>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto phrase = "Be the change you want to see in the world"_utf8_s;

	phrase = phrase.split_ascii_whitespace()
		| std::views::transform(&utf8_string_view::chars)
		| std::views::join_with("_"_u8c)
		| std::ranges::to<utf8_string>();

	std::println("{}", phrase); // Be_the_change_you_want_to_see_in_the_world
}
