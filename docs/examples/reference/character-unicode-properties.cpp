#include "unicode_ranges_full.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	std::println("{}", "A"_u8c.general_category() == unicode_general_category::uppercase_letter);      // true
	std::println("{}", "́"_u8c.canonical_combining_class() == 230);                                      // true
	std::println("{}", "́"_u8c.grapheme_break_property() == unicode_grapheme_break_property::extend);   // true
	std::println("{}", "Ω"_u8c.script() == unicode_script::greek);                                      // true
	std::println("{}", "界"_u8c.east_asian_width() == unicode_east_asian_width::wide);                 // true
	std::println("{}", "A"_u8c.line_break_class() == unicode_line_break_class::alphabetic);            // true
	std::println("{}", "ش"_u8c.bidi_class() == unicode_bidi_class::arabic_letter);                     // true
	std::println("{}", "A"_u8c.word_break_property() == unicode_word_break_property::a_letter);        // true
	std::println("{}", "."_u8c.sentence_break_property() == unicode_sentence_break_property::a_term);  // true
	std::println("{}", "😀"_u8c.is_emoji());                                                            // true
	std::println("{}", "😀"_u8c.is_emoji_presentation());                                               // true
	std::println("{}", "😀"_u8c.is_extended_pictographic());                                            // true
	std::println("{}", U"Ω"_u32c.script() == unicode_script::greek);                                   // true
	std::println("{}", U"😀"_u32c.is_emoji());                                                          // true
}
