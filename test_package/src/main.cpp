#include "unicode_ranges_all.hpp"
#include <unicode_ranges/config.hpp>

#ifdef UTF8_RANGES_ENABLE_ICU
#error "The installed headers must encode ICU availability without a consumer macro"
#endif

#include <string_view>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

static_assert(u8"Stra\u00DFe"_utf8_sv.eq_ignore_case(u8"strasse"_utf8_sv));

int main()
{
	auto text = utf8_string::from_bytes(
		std::string_view{ "\x47\x72\xC3\xBC\xC3\x9F\x65" });
	if (!text)
	{
		return 1;
	}

	const auto utf16 = text->to_utf16();
	const auto utf32 = text->to_utf32();
	if (utf16.char_count() != text->char_count()
		|| utf32.char_count() != text->char_count())
	{
		return 2;
	}

#if UTF8_RANGES_CONFIG_HAS_ICU
	if (!is_available_locale(locale_id{"en"}))
	{
		return 3;
	}
#endif

	return 0;
}
