# unicode_ranges

`unicode_ranges` is a header-only C++23 library for representing, validating, iterating, transforming, and formatting UTF-8, UTF-16, and UTF-32 text.

It provides validated character types, borrowed string views, owning strings, grapheme-aware iteration, Unicode casing, normalization, and conversion between UTF-8, UTF-16, and UTF-32.

## Why this library exists

Many existing C and C++ Unicode/text APIs start from raw byte buffers or raw code-unit strings and rely on documentation, preconditions, and caller discipline to keep invalid text out. Validation, boundary rules, and error handling are often separate from the type that is later passed around.

`unicode_ranges` takes the opposite approach:

- validate once, then operate with invariants
- use lightweight dedicated UTF-8, UTF-16, and UTF-32 types instead of "maybe valid" raw strings
- make invalid states unrepresentable once construction succeeds
- support both compile-time construction through validated literals and runtime construction through checked factories
- keep explicit `_unchecked` escape hatches for callers that already proved validity elsewhere

The goal is predictable Unicode handling with clear invariants and explicit failure modes. You do not pay for what you do not use: checked and unchecked paths are separate, borrowed and owning types are separate, ASCII-only and Unicode-aware operations are separate, and scalar-level and grapheme-level APIs are separate.

## Documentation

- Docs site: [https://cristi1990an.github.io/unicode_ranges/](https://cristi1990an.github.io/unicode_ranges/)
- Docs in repo: [docs/](docs/)
- Changelog: [CHANGELOG.md](CHANGELOG.md)

The large monolithic README has been replaced with a dedicated docs site so the library can document semantics, examples, and reference material on separate pages instead of one long file.

## At a glance

| Category | UTF-8 | UTF-16 | UTF-32 |
| --- | --- | --- | --- |
| Character | `utf8_char` | `utf16_char` | `utf32_char` |
| Borrowed text | `utf8_string_view` | `utf16_string_view` | `utf32_string_view` |
| Owning text | `utf8_string` | `utf16_string` | `utf32_string` |
| Forward scalar iteration | `views::utf8_view` | `views::utf16_view` | `views::utf32_view` |
| Reverse scalar iteration | `views::reversed_utf8_view` | `views::reversed_utf16_view` | `views::reversed_utf32_view` |
| Grapheme iteration | `views::grapheme_cluster_view<char8_t>` | `views::grapheme_cluster_view<char16_t>` | `views::grapheme_cluster_view<char32_t>` |
| Lossy iteration | `views::lossy_utf8_view<CharT>` | `views::lossy_utf16_view<CharT>` | `views::lossy_utf32_view<CharT>` |

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

    std::println("{}", text);                   // é🇷🇴!
    std::println("{}", text.size());            // 12 UTF-8 code units
    std::println("{}", text.char_count());      // 5 Unicode scalars
    std::println("{}", text.grapheme_count());  // 3 graphemes
    std::println("{}", text.find("!"_u8c));     // 11
    std::println("{}", text.find("🇷"_u8c));    // 3
    std::println("{}", text.chars());           // [e, ́, 🇷, 🇴, !]
    std::println("{::s}", text.graphemes());    // [é, 🇷🇴, !]
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

    std::println("{}", *text);                  // Grüße din România 👋
    std::println("{}", text->char_count());     // 18
    std::println("{}", text->front().value());  // G
    std::println("{}", text->back().value());   // 👋
}
```

## Highlights

- Validated UTF-8, UTF-16, and UTF-32 character wrappers, borrowed views, and owning strings
- Scalar iteration, grapheme iteration, and lossy views for untrusted input
- Checked, unchecked, and lossy construction paths at the UTF boundary
- Search, split, trim, replace, reverse, and boundary-query APIs
- Unicode casing, ASCII fast paths, normalization, and full case folding
- Boundary encoding APIs for external formats, with built-in ASCII and Windows-1252 codecs
- `constexpr`-friendly literals and core operations where practical
- Formatting, streaming, and hashing support for library types
- Docs examples under `docs/examples/` are compiled in CI for sanity

## Build docs locally

```bash
python -m pip install -r docs/requirements.txt
python -m mkdocs serve
```

Then open `http://127.0.0.1:8000/`.
