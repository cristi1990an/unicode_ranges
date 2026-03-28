# unicode_ranges

`unicode_ranges` is a header-only C++23 library for representing, validating, iterating, transforming, and formatting UTF-8 and UTF-16 text.

It provides validated character types, borrowed string views, owning strings, grapheme-aware iteration, Unicode casing, normalization, and conversion between UTF-8 and UTF-16.

## Documentation

- Docs site: [https://cristi1990an.github.io/unicode_ranges/](https://cristi1990an.github.io/unicode_ranges/)
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

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
    constexpr auto text = "é🇷🇴!"_utf8_sv;

    std::println("text: {}", text);                               // é🇷🇴!
    std::println("size(): {}", text.size());                      // 12 UTF-8 code units
    std::println("char_count(): {}", text.char_count());          // 5 Unicode scalars
    std::println("grapheme_count(): {}", text.grapheme_count());  // 3 graphemes
    std::println("find('!'): {}", text.find("!"_u8c));           // 11
    std::println("find('🇷'): {}", text.find("🇷"_u8c));          // 3
    std::println("chars(): {}", text.chars());                    // [e, ́, 🇷, 🇴, !]
    std::println("graphemes(): {::s}", text.graphemes());         // [é, 🇷🇴, !]
}
```

For runtime validation of raw input:

```cpp
#include "unicode_ranges.hpp"

#include <print>
#include <string>

using namespace unicode_ranges;

int main()
{
    std::string raw = "Grüße din România 👋";

    auto text = utf8_string::from_bytes(raw);
    if (!text)
    {
        std::println(stderr,
                     "Invalid UTF-8 at byte {}",
                     text.error().first_invalid_byte_index);
        return 1;
    }

    std::println("validated: {}", *text);                  // Grüße din România 👋
    std::println("characters: {}", text->char_count());    // 18
    std::println("first char: {}", text->front().value()); // G
    std::println("last char: {}", text->back().value());   // 👋
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
- Docs examples under `docs/examples/` are compiled in CI for sanity

## Build docs locally

```bash
python -m pip install -r docs/requirements.txt
python -m mkdocs serve
```

Then open `http://127.0.0.1:8000/`.
