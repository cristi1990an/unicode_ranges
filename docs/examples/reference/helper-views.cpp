#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr std::u8string_view utf8_bytes = u8"😄🇷🇴✨";
	constexpr std::u16string_view utf16_code_units = u"😄🇷🇴✨";

	const auto utf8_chars = views::utf8_view::from_bytes_unchecked(utf8_bytes);
	const auto utf8_reversed = views::reversed_utf8_view::from_bytes_unchecked(utf8_bytes);
	const auto utf8_graphemes = views::grapheme_cluster_view<char8_t>::from_code_units_unchecked(utf8_bytes);

	const auto utf16_chars = views::utf16_view::from_code_units_unchecked(utf16_code_units);
	const auto utf16_reversed = views::reversed_utf16_view::from_code_units_unchecked(utf16_code_units);
	const auto utf16_graphemes = views::grapheme_cluster_view<char16_t>::from_code_units_unchecked(utf16_code_units);

	std::println("utf8_view: {}", utf8_chars);                    // [😄, 🇷, 🇴, ✨]
	std::println("reversed_utf8_view: {}", utf8_reversed);        // [✨, 🇴, 🇷, 😄]
	std::println("utf8 graphemes: {::s}", utf8_graphemes);        // [😄, 🇷🇴, ✨]

	std::println("utf16_view: {}", utf16_chars);                  // [😄, 🇷, 🇴, ✨]
	std::println("reversed_utf16_view: {}", utf16_reversed);      // [✨, 🇴, 🇷, 😄]
	std::println("utf16 graphemes: {::s}", utf16_graphemes);      // [😄, 🇷🇴, ✨]

	std::println("literal char: {}", "✨"_u8c);                  // ✨
	std::println("literal grapheme: {}", "🇷🇴"_grapheme_utf8);    // 🇷🇴
}
