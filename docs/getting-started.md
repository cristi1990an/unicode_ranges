# Getting Started

## Requirements

`unicode_ranges` requires a compiler and standard library with strong C++23 support.

Minimum toolchains currently exercised in CI:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

The checked-in Unicode data currently tracks Unicode `17.0.0`.

## Install and integrate

If you have not wired the library into your build yet, start with [Install And Integrate](install-and-integrate.md).

Short version:

- today, the normal consumption path is vendoring, a git submodule, or source-fetching in CMake
- build and link the `unicode_ranges` library target, or an equivalent library target in your own build
- there is not yet a first-party package-manager distribution
- your build needs C++23 and the repository root on the include path
- the repository already vendors pinned `simdutf` (`v7.7.0`) under `third_party/simdutf`
- runtime UTF validation and UTF-8 <-> UTF-16/UTF-32 transcoding currently go through the `simdutf` backend; compile-time and higher-level APIs remain in `unicode_ranges`

## Include the library

```cpp
#include "unicode_ranges.hpp"
```

## Terminology cheat sheet

| Term | Meaning here |
| --- | --- |
| code unit | one UTF-8 byte, one UTF-16 code unit, or one UTF-32 code point unit |
| scalar | one Unicode scalar value |
| grapheme | one user-perceived character under the default Unicode grapheme rules |
| UTF-8 offset | byte offset |
| UTF-16 offset | code-unit offset |
| boundary API | an API such as `is_char_boundary()` or `ceil_grapheme_boundary()` that works in terms of valid semantic cut points |

## Choose the right entry point

- Use `_utf8_sv` / `_utf16_sv` / `_utf32_sv` for validated compile-time views.
- Use `utf8_string::from_bytes(...)`, `utf16_string::from_code_units(...)`, or `utf32_string::from_code_points(...)` when input arrives as raw runtime data.
- Use `_utf8_s` / `_utf16_s` / `_utf32_s` when you want an owning validated string immediately.

## A first validated view

This is the style the docs will use going forward: visible Unicode text, runnable code, `std::println`, and comments showing what to expect.

```cpp
--8<-- "examples/getting-started/validated-view.cpp"
```

!!! info
    Reading the first example:

    - `é` is `U+0065 LATIN SMALL LETTER E` followed by `U+0301 COMBINING ACUTE ACCENT`.
    - That means `é` is one grapheme, but two scalars.
    - `🇷🇴` is one grapheme built from two regional-indicator scalars.
    - This is why `size()`, `char_count()`, and `grapheme_count()` intentionally differ.

## Runtime validation

When text arrives at runtime as raw bytes, validate it once and keep the validated type:

```cpp
--8<-- "examples/getting-started/runtime-validation.cpp"
```

## Formatting and printing

Library-defined UTF-8, UTF-16, and UTF-32 types support formatting and printing directly. Borrowed views such as `chars()` and `graphemes()` are easy to inspect too. For grapheme views, the examples use `"{::s}"` so the printed range stays visually uniform with the underlying text:

!!! warning
    `std::println("{}", text.chars())` and `std::println("{::s}", text.graphemes())` rely on C++23 range-formatting support in the standard library.

    - this works on the MSVC STL and on libc++
    - libstdc++ 14 does not currently format these custom helper views directly
    - the GCC docs-example CI job therefore treats that specific limitation as informational rather than blocking

```cpp
--8<-- "examples/getting-started/formatting.cpp"
```

## Views versus owning strings

- `utf8_string_view` / `utf16_string_view` / `utf32_string_view` borrow existing storage.
- `utf8_string` / `utf16_string` / `utf32_string` own and mutate storage.
- `chars()`, `graphemes()`, `char_indices()`, and `grapheme_indices()` are borrowing range views.

Do not keep borrowed ranges alive after the source storage dies or after the owning string mutates.

## Counting and indexing

The library intentionally distinguishes:

- code units: `size()`
- Unicode scalar values: `char_count()`
- grapheme clusters: `grapheme_count()`

UTF-8 view/string search APIs generally return byte offsets. UTF-16 and UTF-32 view/string search APIs generally return code-unit offsets. Character-oriented APIs are named explicitly, such as `char_at`, `is_char_boundary`, and `ceil_char_boundary`.

## Example sanity checks

The examples under `docs/examples/` are compiled in CI so the docs do not silently drift away from the library surface. The one current exception is direct `std::print` formatting of helper views on GCC/libstdc++ 14, which is tracked as an informational, non-blocking docs-example failure.

## Where to go next

- [Common Tasks](common-tasks.md)
- [Design](design.md)
- [Text Operations](text-operations.md)
- [Casing and Normalization](casing-and-normalization.md)
- [Reference](reference/index.md)
