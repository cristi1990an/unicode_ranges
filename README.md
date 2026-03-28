# unicode_ranges

`unicode_ranges` is a header-only C++23 library for representing, validating, iterating, transforming, and formatting UTF-8 and UTF-16 text.

It provides validated character types, borrowed string views, owning strings, grapheme-aware iteration, Unicode casing, normalization, and conversion between UTF-8 and UTF-16.

## Documentation

- Docs site: `https://cristi1990an.github.io/unicode_ranges/`
- Docs in repo: [docs/](docs/)
- Changelog: [CHANGELOG.md](CHANGELOG.md)

The large monolithic README has been replaced with a dedicated docs site so the library can document semantics, examples, and reference material on separate pages instead of one long file.

## At a glance

| Category | UTF-8 | UTF-16 |
| --- | --- | --- |
| Character | `utf8_char` | `utf16_char` |
| Borrowed text | `utf8_string_view` | `utf16_string_view` |
| Owning text | `utf8_string` | `utf16_string` |
| Forward scalar iteration | `views::utf8_view` | `views::utf16_view` |
| Reverse scalar iteration | `views::reversed_utf8_view` | `views::reversed_utf16_view` |
| Grapheme iteration | `views::grapheme_cluster_view<char8_t>` | `views::grapheme_cluster_view<char16_t>` |
| Lossy iteration | `views::lossy_utf8_view<CharT>` | `views::lossy_utf16_view<CharT>` |

## Requirements

This library requires a compiler and standard library with strong C++23 support.

Minimum toolchains currently covered by CI:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: the `ClangCL` toolset from current Visual Studio 2022 builds
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

Unicode tables currently track Unicode `17.0.0`.

## Quick start

```cpp
#include "unicode_ranges.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto text = u8"caf\u00E9 \u20AC"_utf8_sv;

static_assert(text.size() == 9);       // UTF-8 code units
static_assert(text.char_count() == 6); // Unicode scalar values
static_assert(text.grapheme_count() == 6);
static_assert(text.find(u8"\u20AC"_u8c) == 6); // byte offset
```

For runtime validation of raw input:

```cpp
#include "unicode_ranges.hpp"

#include <print>
#include <string_view>

using namespace unicode_ranges;

int main()
{
    constexpr std::string_view raw = "caf\xC3\xA9";

    auto text = utf8_string::from_bytes(raw);
    if (!text)
    {
        std::println(stderr,
                     "Invalid UTF-8 at byte {}",
                     text.error().first_invalid_byte_index);
        return 1;
    }

    std::println("Characters: {}", text->char_count());
}
```

## Highlights

- Validated UTF-8 and UTF-16 character types
- Borrowed views and owning strings
- Scalar and grapheme iteration
- Search, split, trim, and replace APIs
- Unicode and ASCII casing
- Unicode normalization and full case folding
- `constexpr`-friendly literals and core operations
- Formatting, streaming, and hashing support for library types

## Build docs locally

```bash
python -m pip install -r docs/requirements.txt
python -m mkdocs serve
```

Then open `http://127.0.0.1:8000/`.
