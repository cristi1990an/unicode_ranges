# Views, Literals, and Formatting

## Range adaptors and helper views

The library exposes dedicated view types under `unicode_ranges::views`, including:

- `views::utf8_view`
- `views::utf16_view`
- `views::reversed_utf8_view`
- `views::reversed_utf16_view`
- `views::grapheme_cluster_view<CharT>`
- `views::lossy_utf8_view<CharT>`
- `views::lossy_utf16_view<CharT>`
- closure objects such as `views::lossy_utf8` and `views::lossy_utf16`

These are useful when adapting raw code-unit ranges into validated scalar iteration without first materializing an owning string.

## Literal operators

Literal operators live in `unicode_ranges::literals`.

Available operators include:

- `operator ""_u8c`
- `operator ""_u16c`
- `operator ""_utf8_sv`
- `operator ""_utf16_sv`
- `operator ""_utf8_s`
- `operator ""_utf16_s`
- `operator ""_grapheme_utf8`
- `operator ""_grapheme_utf16`

The validated string and grapheme literal operators are a major part of the library's `constexpr`-friendly surface.

## Formatting and streaming

Library-defined UTF-8 and UTF-16 types support:

- `std::formatter`
- stream insertion where appropriate
- hashing

This includes the character and string types documented elsewhere in the reference.

That support is one advantage of using library-defined UTF-8 and UTF-16 types rather than relying on standard library UTF string types directly.
