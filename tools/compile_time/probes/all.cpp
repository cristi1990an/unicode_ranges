#include "unicode_ranges_all.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto all_probe_text = u8"Stra\u00DFe \U0001F604"_utf8_s;

static_assert(all_probe_text.char_count() == 8);
static_assert(all_probe_text.case_fold() == u8"strasse \U0001F604"_utf8_sv);
static_assert(all_probe_text.to_utf16() == u"Stra\u00DFe \U0001F604"_utf16_sv);
static_assert(all_probe_text.to_utf32() == U"Stra\u00DFe \U0001F604"_utf32_sv);

auto all_probe() -> utf8_string
{
  return all_probe_text.to_uppercase();
}
