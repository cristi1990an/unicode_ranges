#include "unicode_ranges_borrowed.hpp"
#include "unicode_ranges_all.hpp"

#include <string_view>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

static_assert("Straße"_utf8_sv.eq_ignore_case(u8"strasse"_utf8_sv));

int main()
{
	auto text = utf8_string::from_bytes(std::string_view{ "\x47\x72\xC3\xBC\xC3\x9F\x65" });
	if (!text)
	{
		return 1;
	}

	auto utf16 = text->to_utf16();
	auto utf32 = text->to_utf32();

	if (utf16.char_count() != text->char_count())
	{
		return 2;
	}

	if (utf32.char_count() != text->char_count())
	{
		return 3;
	}

	return 0;
}
