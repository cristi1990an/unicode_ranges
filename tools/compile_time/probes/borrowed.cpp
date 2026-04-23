#include "unicode_ranges_borrowed.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto borrowed_probe_text = u8"e\u0301\U0001F1F7\U0001F1F4!"_utf8_sv;

static_assert(borrowed_probe_text.char_count() == 5);
static_assert(borrowed_probe_text.starts_with(u8"e\u0301"_utf8_sv));
static_assert(borrowed_probe_text.ends_with(u8"!"_utf8_sv));
static_assert(borrowed_probe_text.trim_suffix(u8"!"_utf8_sv) == u8"e\u0301\U0001F1F7\U0001F1F4"_utf8_sv);
static_assert(borrowed_probe_text.back().value() == u8"!"_u8c);

auto borrowed_probe() -> bool
{
  auto trimmed = borrowed_probe_text.trim_suffix(u8"!"_utf8_sv);
  return trimmed.starts_with(u8"e\u0301"_utf8_sv)
    && trimmed.ends_with(u8"\U0001F1F4"_utf8_sv);
}
