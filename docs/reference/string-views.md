# String Views

## Overview

`utf8_string_view` and `utf16_string_view` are borrowed validated text views.

They are the read-only workhorses of the library: cheap to copy, explicit about encoding, and rich in Unicode-aware operations.

## Construction

Common entry points:

- validated literal operators:
  - `_utf8_sv`
  - `_utf16_sv`
- checked factories:
  - `utf8_string_view::from_bytes(...)`
  - `utf16_string_view::from_code_units(...)`
- unchecked factories for already-validated storage

## Iteration

Important borrowed iteration families:

- `chars()`
- `reversed_chars()`
- `graphemes()`
- `char_indices()`
- `grapheme_indices()`

These ranges borrow from the underlying storage.

## Counts and predicates

- `size()`
- `empty()`
- `is_ascii()`
- `char_count()`
- `grapheme_count()`

## Search and matching

The view types expose a broad search surface:

- `contains`
- `find`, `rfind`
- `find_first_of`, `find_first_not_of`
- `find_last_of`, `find_last_not_of`
- `starts_with`, `ends_with`
- grapheme-aware search overloads

UTF-8 returns byte offsets. UTF-16 returns code-unit offsets.

## Split and trim

View types support:

- `split`, `rsplit`, `splitn`, `rsplitn`
- `split_inclusive`, `split_trimmed`
- `split_terminator`, `rsplit_terminator`
- `split_whitespace`, `split_ascii_whitespace`
- `strip_prefix`, `strip_suffix`, `strip_circumfix`
- `trim_*` and `trim_*_matches` families

## Boundary-sensitive APIs

- `is_char_boundary`
- `ceil_char_boundary`
- `floor_char_boundary`
- `is_grapheme_boundary`
- `ceil_grapheme_boundary`
- `floor_grapheme_boundary`
- `char_at`, `char_at_unchecked`
- `grapheme_at`
- `grapheme_substr`
- `substr`

These are the APIs to use when the distinction between raw offsets and semantic boundaries matters.

## Conversion and transforms

String views can produce owning strings through:

- `to_utf16()` from UTF-8
- `to_utf8()` from UTF-16
- `to_ascii_lowercase()`, `to_ascii_uppercase()`
- `to_lowercase()`, `to_uppercase()`
- `normalize(...)`, `to_nfc()`, `to_nfd()`, `to_nfkc()`, `to_nfkd()`
- `case_fold()`

Because views are borrowed, transformation APIs return owning strings.

## Important semantics

Remember these rules:

- `size()` counts code units, not scalars.
- UTF-8 search offsets are byte offsets.
- UTF-16 search offsets are code-unit offsets.
- Character-oriented APIs say so explicitly.
- Grapheme APIs use default Unicode grapheme rules.
