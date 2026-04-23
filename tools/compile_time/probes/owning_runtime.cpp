#include "unicode_ranges_all.hpp"

#include <string_view>

using namespace unicode_ranges;

auto owning_runtime_probe() -> std::size_t
{
  auto text = utf8_string::from_bytes(std::string_view{ "\x53\x74\x72\x61\xC3\x9F\x65 \xF0\x9F\x98\x84" }).value();
  auto utf16 = text.to_utf16();
  auto utf32 = utf16.to_utf32();
  auto folded = text.case_fold();
  auto upper = text.to_uppercase();
  auto normalized = text.to_nfc();
  auto rebuilt = utf32.to_utf8();

  return utf32.char_count()
    + folded.char_count()
    + upper.char_count()
    + normalized.char_count()
    + rebuilt.char_count();
}
