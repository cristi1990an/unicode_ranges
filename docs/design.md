# Design

## Problem statement

The library is meant to solve a recurring problem in C and C++ Unicode handling: too many APIs operate on raw strings with important preconditions left implicit.

Typical pain points are:

- validation is separate from the type that is later passed around
- invalid UTF-8, UTF-16, or UTF-32 can survive too long in ordinary string types
- boundary-sensitive operations rely on callers remembering byte or code-unit rules
- error handling is inconsistent across libraries and easy to lose when composing APIs

`unicode_ranges` takes a different approach:

- validate once at construction
- encode the result in dedicated lightweight types with clear invariants
- keep those invariants stable across later operations

Once you have a `utf8_char`, `utf16_char`, `utf32_char`, `utf8_string_view`, `utf16_string_view`, `utf32_string_view`, `utf8_string`, `utf16_string`, or `utf32_string`, you are working with validated text, not with raw storage that might or might not be valid.

## Core model

The library is built around a few explicit rules:

- Unicode scalar values are the semantic model.
- UTF-8, UTF-16, and UTF-32 are all first-class encodings.
- Checked and unchecked APIs are kept distinct.
- Borrowed and owning types are separate.
- Performance matters, especially on ASCII-heavy paths, but not at the expense of Unicode correctness.

## Ownership model

- `utf8_string_view` / `utf16_string_view` / `utf32_string_view` borrow validated storage.
- `utf8_string` / `utf16_string` / `utf32_string` own validated storage.
- range-returning APIs such as `chars()` and `graphemes()` borrow from the source text.

This makes lifetime and mutation rules explicit instead of implicit.

## Code units, scalars, and graphemes

The library exposes all three levels:

- code units: raw UTF-8 bytes, UTF-16 code units, or UTF-32 code points
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

The unchecked APIs are there for callers that already proved validity elsewhere and want to skip redundant checks. This is the core "validate once, operate without worry" rule of the library: checked APIs establish the invariant, and unchecked APIs are the explicit escape hatch when that invariant is already known by other means.

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

## You do not pay for what you do not use

The library tries to keep costs explicit instead of hidden:

- checked and unchecked entry points are separate
- borrowed and owning types are separate
- ASCII-only and Unicode-aware operations are separate
- scalar iteration and grapheme iteration are separate

That separation is deliberate. Callers who need full validation and Unicode semantics can opt into them directly. Callers who already have validated text or only need ASCII behavior do not have to keep paying for heavier paths at every call site.

## Scope boundaries

Supported:

- validated UTF-8, UTF-16, and UTF-32 text handling
- Unicode predicates
- default grapheme segmentation
- Unicode casing and normalization
- formatting / streaming / hashing for library-defined types

Out of scope:

- locale-specific casing and collation
- bidi or layout/shaping engines
- regex engines
- tailored segmentation rules beyond the default grapheme algorithm
