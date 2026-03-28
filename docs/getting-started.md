# Getting Started

## Requirements

`unicode_ranges` requires a compiler and standard library with strong C++23 support.

Minimum toolchains currently exercised in CI:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

The checked-in Unicode data currently tracks Unicode `17.0.0`.

## Include the library

```cpp
#include "unicode_ranges.hpp"
```

## Choose the right entry point

- Use `_utf8_sv` / `_utf16_sv` for validated compile-time views.
- Use `utf8_string::from_bytes(...)` / `utf16_string::from_code_units(...)` when input arrives as raw runtime data.
- Use `_utf8_s` / `_utf16_s` when you want an owning validated string immediately.

## Compile-time validated literals

```cpp
#include "unicode_ranges.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto text = u8"caf\u00E9 \u20AC"_utf8_sv;

static_assert(text.size() == 9);       // UTF-8 code units
static_assert(text.char_count() == 6); // Unicode scalar values
static_assert(text.find(u8"\u20AC"_u8c) == 6); // byte offset
```

## Runtime validation

```cpp
#include "unicode_ranges.hpp"

#include <print>
#include <string>

using namespace unicode_ranges;

int main()
{
    std::string raw = "caf\xC3\xA9";

    auto text = utf8_string::from_bytes(raw);
    if (!text)
    {
        std::println(stderr,
                     "Invalid UTF-8 at byte {}",
                     text.error().first_invalid_byte_index);
        return 1;
    }

    std::println("Characters: {}", text->char_count());
    std::println("As UTF-16: {}", text->to_utf16());
}
```

## Views versus owning strings

- `utf8_string_view` / `utf16_string_view` borrow existing storage.
- `utf8_string` / `utf16_string` own and mutate storage.
- `chars()`, `graphemes()`, `char_indices()`, and `grapheme_indices()` are borrowing range views.

Do not keep borrowed ranges alive after the source storage dies or after the owning string mutates.

## Counting and indexing

The library intentionally distinguishes:

- code units: `size()`
- Unicode scalar values: `char_count()`
- grapheme clusters: `grapheme_count()`

UTF-8 view/string search APIs generally return byte offsets. UTF-16 view/string search APIs generally return code-unit offsets. Character-oriented APIs are named explicitly, such as `char_at`, `is_char_boundary`, and `ceil_char_boundary`.

## Formatting and printing

Library-defined UTF-8 and UTF-16 types support formatting and stream insertion, unlike standard `std::u8string` itself.

```cpp
#include "unicode_ranges.hpp"

#include <cassert>
#include <format>
#include <sstream>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

const utf8_string text = u8"caf\u00E9 \u20AC"_utf8_s;

assert(std::format("{}", text) == "caf\u00E9 \u20AC");

std::ostringstream oss;
oss << text;
assert(oss.str() == "caf\u00E9 \u20AC");
```

## Where to go next

- [Design](design.md)
- [Text Operations](text-operations.md)
- [Casing and Normalization](casing-and-normalization.md)
- [Reference](reference/index.md)
