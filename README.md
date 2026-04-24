# unicode_ranges

`unicode_ranges` is a C++23 library for representing, validating, iterating, transforming, and formatting UTF-8, UTF-16, and UTF-32 text.

It provides validated character types, borrowed string views, owning strings, grapheme-aware iteration, Unicode casing, normalization, and conversion between UTF-8, UTF-16, and UTF-32.

`unicode_ranges` is now a compiled library. The public API stays header-first, but runtime UTF validation and runtime UTF-8 <-> UTF-16/UTF-32 transcoding are provided by the `unicode_ranges` library target built from `unicode_ranges.cpp`, backed by pinned vendored `simdutf` (`v7.7.0`) under [`third_party/simdutf`](third_party/simdutf). Consumers should link the library target, or produce an equivalent static/shared library in their own build. No separate `simdutf` setup step is required for normal consumption.

The repository now also ships a first-party CMake build and install/export package for that compiled library target.

Umbrella headers:

- `unicode_ranges_borrowed.hpp`: lighter borrowed/core surface
- `unicode_ranges_all.hpp`: full umbrella, including owning strings and `unicode_ranges::characters`
- `unicode_ranges.hpp` and `unicode_ranges_full.hpp`: legacy compatibility wrappers retained for older code

## Why this library exists

Many existing C and C++ Unicode/text APIs start from raw byte buffers or raw code-unit strings and rely on documentation, preconditions, and caller discipline to keep invalid text out. Validation, boundary rules, and error handling are often separate from the type that is later passed around.

`unicode_ranges` takes the opposite approach:

- validate once, then operate with invariants
- use lightweight dedicated UTF-8, UTF-16, and UTF-32 types instead of "maybe valid" raw strings
- make invalid states unrepresentable once construction succeeds
- support both compile-time construction through validated literals and runtime construction through checked factories
- keep explicit `_unchecked` escape hatches for callers that already proved validity elsewhere

The goal is predictable Unicode handling with clear invariants and explicit failure modes. You do not pay for what you do not use: checked and unchecked paths are separate, borrowed and owning types are separate, ASCII-only and Unicode-aware operations are separate, and scalar-level and grapheme-level APIs are separate.

## Runtime Backend

`unicode_ranges` now uses `simdutf` as its production runtime backend for the hot UTF boundary operations:

- UTF-8 validation
- UTF-8 -> UTF-16 transcoding
- UTF-8 -> UTF-32 transcoding

That choice is deliberate. In the comparative benchmark suite, `simdutf` has been the strongest raw UTF codec baseline by a clear margin, and using it through its public API lets `unicode_ranges` keep its own validated types and error model while benefiting from best-in-class runtime UTF performance.

This does not replace the rest of the library:

- the public API remains `unicode_ranges`
- the higher-level string/view/value types remain `unicode_ranges`
- compile-time and `constexpr`-oriented functionality remains implemented in `unicode_ranges`
- the runtime backend is specifically about the hot UTF validation/transcoding paths

## Documentation

- Docs site: [https://cristi1990an.github.io/unicode_ranges/](https://cristi1990an.github.io/unicode_ranges/)
- Docs in repo: [docs/](docs/)
- Stability policy: [STABILITY.md](STABILITY.md)
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
#include "unicode_ranges_all.hpp"

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

For runtime validation of raw input without copying:

```cpp
#include "unicode_ranges_borrowed.hpp"

#include <print>
#include <string>

using namespace unicode_ranges;

int main()
{
    std::string raw = "Grüße din România 👋";

    auto text = utf8_string_view::from_bytes(raw);
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

Views also compose cleanly with standard range pipelines:

```cpp
#include "unicode_ranges_all.hpp"

#include <print>
#include <ranges>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
    auto phrase = "Be the change you want to see in the world"_utf8_s;

    phrase = phrase.split_ascii_whitespace()
        | std::views::transform(&utf8_string_view::chars)
        | std::views::join_with("_"_u8c)
        | std::ranges::to<utf8_string>();

    std::println("{}", phrase); // Be_the_change_you_want_to_see_in_the_world
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

## Licensing

This repository is dual-licensed under `MIT OR Apache-2.0`.

The full license texts are in:

- `LICENSE`
- `LICENSE-MIT`
- `LICENSE-APACHE`

The pinned vendored runtime dependency `simdutf` is also dual-licensed under `MIT OR Apache-2.0`, which keeps the licensing model straightforward for the compiled runtime backend.

Third-party dependency notices, pinned versions, and the provenance-header policy for any future copied source files are documented in `THIRD_PARTY_NOTICES.md`.
