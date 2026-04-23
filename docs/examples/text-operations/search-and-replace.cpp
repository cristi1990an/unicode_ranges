#include "unicode_ranges_all.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto view = "café café"_utf8_sv;
	auto owned = "été en été"_utf8_s;

	std::println("{}", view);                  // café café
	std::println("{}", view.find("é"_u8c));    // 3
	std::println("{}", view.rfind("é"_u8c));   // 9

	std::println("{}",
		owned.replace_all("é"_u8c, "e"_u8c)); // ete en ete
}
