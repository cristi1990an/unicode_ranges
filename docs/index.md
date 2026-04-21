# unicode_ranges

`unicode_ranges` is a C++23 library for validated UTF-8, UTF-16, and UTF-32 text.

It is built around a simple idea: Unicode scalar values are the canonical model, while UTF-8, UTF-16, and UTF-32 remain first-class encodings with dedicated APIs. The library gives you validated characters, borrowed views, owning strings, scalar iteration, grapheme iteration, Unicode casing, normalization, formatting support, and conversion between encodings.

## Why this library exists

Existing C and C++ text handling often starts from raw byte buffers, raw code-unit strings, or APIs with preconditions that are easy to violate and hard to see at the call site. Validation rules, boundary rules, and error handling are frequently left to documentation and convention instead of being carried by the type system.

`unicode_ranges` exists to push that cost to the edge:

- validate once, then operate with invariants
- represent UTF-8, UTF-16, and UTF-32 text with lightweight dedicated types instead of "maybe valid" raw strings
- make invalid states unrepresentable once construction succeeds
- keep construction available both at compile time through validated literals and at runtime through checked factories
- still expose explicit unchecked fast paths when the caller has already proved validity elsewhere

The design goal is not "maximum abstraction". It is predictable Unicode handling with clear invariants, explicit failure modes, and no repeated worry about whether the current value is valid text.

The public surface is still header-first, but the runtime UTF hot paths now live in the compiled `unicode_ranges` library target, built from `unicode_ranges.cpp` and backed by pinned `simdutf` (`v7.7.0`). Consumers should link that library target, or an equivalent library in their own build, and keep the `simdutf` singleheader release on the include path.

## New users: start here

- [Install And Integrate](install-and-integrate.md): how to consume the library today, including the current packaging reality.
- [Getting Started](getting-started.md): include, validate, and use the core types quickly.
- [Common Tasks](common-tasks.md): validate input, iterate scalars versus graphemes, normalize, case-map, and convert encodings.
- [Design](design.md): ownership, indexing, boundaries, and what the library treats as a character.
- [Boundary Encodings](extensible-encodings.md): built-in codecs, custom encoder/decoder requirements, generated APIs, and boundary-specific error handling.
- [Benchmarking](benchmarking.md): the cross-library benchmark charter, comparison rules, toolchain matrix, and planned benchmark families.
- [Text Operations](text-operations.md): search, split, trim, replace, reverse, and boundary queries.
- [Casing and Normalization](casing-and-normalization.md): Unicode casing, case folding, and normalization forms.
- [Reference](reference/index.md): grouped API reference by type family.

## Type map

| Category | UTF-8 | UTF-16 | UTF-32 |
| --- | --- | --- | --- |
| Character | `utf8_char` | `utf16_char` | `utf32_char` |
| Borrowed text | `utf8_string_view` | `utf16_string_view` | `utf32_string_view` |
| Owning text | `utf8_string` | `utf16_string` | `utf32_string` |
| Forward scalar iteration | `views::utf8_view` | `views::utf16_view` | `views::utf32_view` |
| Reverse scalar iteration | `views::reversed_utf8_view` | `views::reversed_utf16_view` | `views::reversed_utf32_view` |
| Grapheme iteration | `views::grapheme_cluster_view<char8_t>` | `views::grapheme_cluster_view<char16_t>` | `views::grapheme_cluster_view<char32_t>` |
| Lossy iteration | `views::lossy_utf8_view<CharT>` | `views::lossy_utf16_view<CharT>` | `views::lossy_utf32_view<CharT>` |

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
- Separate checked, unchecked, Unicode-aware, and ASCII-only paths so you do not pay for what you do not use
- `constexpr`-friendly literals and core operations where practical
- Table-driven Unicode properties and grapheme segmentation
- Fast ASCII paths without degrading Unicode correctness

## What it does not try to cover

- Locale-specific casing or collation
- Bidirectional layout, shaping, or font/layout work
- Regex or full text-search engines
- Tailored segmentation beyond default Unicode grapheme rules

Those are deliberate scope boundaries rather than omissions by accident.
