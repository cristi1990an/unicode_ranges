#include "unicode_ranges_all.hpp"

#include <concepts>
#include <utility>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto all_probe_view = u8"Stra\u00DFe \U0001F604"_utf8_sv;

static_assert(all_probe_view.char_count() == 8);
static_assert(std::same_as<decltype(std::declval<const utf8_string&>().to_utf16()), utf16_string>);
static_assert(std::same_as<decltype(std::declval<const utf8_string&>().to_utf32()), utf32_string>);
static_assert(std::same_as<decltype(std::declval<const utf8_string&>().case_fold()), utf8_string>);
static_assert(std::same_as<decltype(std::declval<const utf8_string&>().to_uppercase()), utf8_string>);

auto all_probe() -> std::size_t
{
  auto text = u8"Stra\u00DFe \U0001F604"_utf8_s;
  auto folded = text.case_fold();
  auto utf16 = text.to_utf16();
  auto utf32 = text.to_utf32();
  auto upper = text.to_uppercase();

  return text.char_count()
    + folded.char_count()
    + utf16.char_count()
    + utf32.char_count()
    + upper.char_count();
}
