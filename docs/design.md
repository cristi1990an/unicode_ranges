# Design

## Core model

The library is built around a few explicit rules:

- Unicode scalar values are the semantic model.
- UTF-8 and UTF-16 are both first-class encodings.
- Checked and unchecked APIs are kept distinct.
- Borrowed and owning types are separate.
- Performance matters, especially on ASCII-heavy paths, but not at the expense of Unicode correctness.

## Ownership model

- `utf8_string_view` / `utf16_string_view` borrow validated storage.
- `utf8_string` / `utf16_string` own validated storage.
- range-returning APIs such as `chars()` and `graphemes()` borrow from the source text.

This makes lifetime and mutation rules explicit instead of implicit.

## Code units, scalars, and graphemes

The library exposes all three levels:

- code units: raw UTF-8 bytes or UTF-16 code units
- scalars: Unicode scalar values
- graphemes: user-perceived characters under default Unicode grapheme-cluster rules

That distinction is why the API surface contains both:

- `size()`
- `char_count()`
- `grapheme_count()`

and both:

- `substr(...)`
- `grapheme_substr(...)`

## Checked versus unchecked APIs

Checked construction validates input and reports structured errors. Unchecked construction exists, but it is intentionally named as such:

- `from_bytes(...)`
- `from_bytes_unchecked(...)`
- `char_at(...)`
- `char_at_unchecked(...)`

The unchecked APIs are there for callers that already proved validity elsewhere and want to skip redundant checks.

## ASCII fast paths and Unicode correctness

The library exposes both Unicode-aware and ASCII-only classification and transform APIs. That split is intentional:

- Unicode-aware operations remain table-driven and correct across the supported Unicode version.
- ASCII-only operations stay cheap, explicit, and unsurprising.

This is why APIs are named separately, such as:

- `to_lowercase()` versus `to_ascii_lowercase()`
- `is_alphabetic()` versus `is_ascii_alphabetic()`

## `constexpr` as a design goal

Many literals and core operations are meant to remain usable in constant evaluation. That influences the implementation style across:

- validated literal operators
- character decoding and encoding
- Unicode property lookup
- grapheme segmentation

Not every operation is `constexpr`, but it is a deliberate design target rather than an accidental bonus.

## Scope boundaries

Supported:

- validated UTF-8 and UTF-16 text handling
- Unicode predicates
- default grapheme segmentation
- Unicode casing and normalization
- formatting / streaming / hashing for library-defined types

Out of scope:

- locale-specific casing and collation
- bidi or layout/shaping engines
- regex engines
- tailored segmentation rules beyond the default grapheme algorithm
