#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	utf8_string built;
	built.append_range(u8"\U0001F604\U0001F1F7\U0001F1F4"_utf8_sv.chars());
	built.push_back(u8"\u2728"_u8c);
	std::println("{}", built); // 😄🇷🇴✨

	auto transcoded = built;
	transcoded.append_range(u"\U0001F389"_utf16_sv.chars());
	std::println("{}", transcoded); // 😄🇷🇴✨🎉

	const auto widened = built.to_utf32();
	std::println("{}", widened); // 😄🇷🇴✨

	auto inserted = built;
	inserted.insert(4, u8"\U0001F389"_utf8_sv);
	std::println("{}", inserted); // 😄🎉🇷🇴✨

	auto reversed = built;
	reversed.reverse();
	std::println("{}", reversed); // ✨🇴🇷😄

	auto replaced = built.replace_all(u8"\u2728"_u8c, u8"\U0001F525"_u8c);
	std::println("{}", replaced); // 😄🇷🇴🔥

	utf8_string_view borrowed = built.as_view();
	const std::u8string& raw = built.base();
	std::println("{}", borrowed);   // 😄🇷🇴✨
	std::println("{}", raw.size()); // 15
}
