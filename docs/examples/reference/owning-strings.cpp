#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	utf8_string built;
	built.append_range("😄🇷🇴"_utf8_sv.chars());
	built.push_back("✨"_u8c);
	std::println("append_range(\"😄🇷🇴\"_utf8_sv.chars()) + push_back('✨'): {}", built); // 😄🇷🇴✨

	auto transcoded = built;
	transcoded.append_range(u"🎉"_utf16_sv.chars());
	std::println("append_range(u\"🎉\"_utf16_sv.chars()): {}", transcoded); // 😄🇷🇴✨🎉

	auto inserted = built;
	inserted.insert(4, "🎉"_utf8_sv);
	std::println("insert(4, \"🎉\"_utf8_sv): {}", inserted);    // 😄🎉🇷🇴✨

	auto reversed = built;
	reversed.reverse();
	std::println("reverse(): {}", reversed);                     // ✨🇷🇴😄

	auto replaced = built.replace_all("✨"_u8c, "🔥"_u8c);
	std::println("replace_all('✨', '🔥'): {}", replaced);        // 😄🇷🇴🔥

	utf8_string_view borrowed = built.as_view();
	const std::u8string& raw = built.base();
	std::println("borrowed utf8_string_view: {}", borrowed);     // 😄🇷🇴✨
	std::println("underlying std::u8string size: {}", raw.size()); // 15
}
