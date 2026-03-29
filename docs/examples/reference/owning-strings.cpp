#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	utf8_string built;
	built.append_range("😄🇷🇴"_utf8_sv.chars());
	built.push_back("✨"_u8c);
	std::println("{}", built); // 😄🇷🇴✨

	auto transcoded = built;
	transcoded.append_range(u"🎉"_utf16_sv.chars());
	std::println("{}", transcoded); // 😄🇷🇴✨🎉

	auto inserted = built;
	inserted.insert(4, "🎉"_utf8_sv);
	std::println("{}", inserted); // 😄🎉🇷🇴✨

	auto reversed = built;
	reversed.reverse();
	std::println("{}", reversed); // ✨🇷🇴😄

	auto replaced = built.replace_all("✨"_u8c, "🔥"_u8c);
	std::println("{}", replaced); // 😄🇷🇴🔥

	utf8_string_view borrowed = built.as_view();
	const std::u8string& raw = built.base();
	std::println("{}", borrowed);   // 😄🇷🇴✨
	std::println("{}", raw.size()); // 15
}
