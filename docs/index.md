# unicode_ranges

`unicode_ranges` is a header-only C++23 library for validated UTF-8 and UTF-16 text.

It is built around a simple idea: Unicode scalar values are the canonical model, while UTF-8 and UTF-16 remain first-class encodings with dedicated APIs. The library gives you validated characters, borrowed views, owning strings, scalar iteration, grapheme iteration, Unicode casing, normalization, formatting support, and conversion between encodings.

## Start here

- [Getting Started](getting-started.md): include, validate, and use the core types quickly.
- [Design](design.md): ownership, indexing, boundaries, and what the library treats as a character.
- [Text Operations](text-operations.md): search, split, trim, replace, reverse, and boundary queries.
- [Casing and Normalization](casing-and-normalization.md): Unicode casing, case folding, and normalization forms.
- [Reference](reference/index.md): grouped API reference by type family.

## Type map

| Category | UTF-8 | UTF-16 |
| --- | --- | --- |
| Character | `utf8_char` | `utf16_char` |
| Borrowed text | `utf8_string_view` | `utf16_string_view` |
| Owning text | `utf8_string` | `utf16_string` |
| Forward scalar iteration | `views::utf8_view` | `views::utf16_view` |
| Reverse scalar iteration | `views::reversed_utf8_view` | `views::reversed_utf16_view` |
| Grapheme iteration | `views::grapheme_cluster_view<char8_t>` | `views::grapheme_cluster_view<char16_t>` |
| Lossy iteration | `views::lossy_utf8_view<CharT>` | `views::lossy_utf16_view<CharT>` |

## Public entry point

```cpp
#include "unicode_ranges.hpp"
```

Everything public lives in namespace `unicode_ranges`. Literal operators live in `unicode_ranges::literals`. PMR owning-string aliases live in `unicode_ranges::pmr`.

!!! warning
    `unicode_ranges::details` is implementation detail only. It is not part of the supported public API.

## What the library is optimized for

- Validated text types instead of raw `std::u8string_view` / `std::u16string_view`
- Predictable, STL-style APIs with Rust-inspired Unicode ergonomics
- `constexpr`-friendly literals and core operations where practical
- Table-driven Unicode properties and grapheme segmentation
- Fast ASCII paths without degrading Unicode correctness

## What it does not try to cover

- Locale-specific casing or collation
- Bidirectional layout, shaping, or font/layout work
- Regex or full text-search engines
- Tailored segmentation beyond default Unicode grapheme rules

Those are deliberate scope boundaries rather than omissions by accident.
