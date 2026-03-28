# Owning Strings

## Overview

`utf8_string` and `utf16_string` are validated owning text types.

They provide the same observation and search surface as the corresponding views, plus mutation APIs and storage-management behavior.

## Construction

Common entry points:

- `_utf8_s`
- `_utf16_s`
- checked factories from runtime data
- conversion from the corresponding view types

PMR aliases are available under `unicode_ranges::pmr`.

## Mutation APIs

Important mutating operations include:

- `insert`
- `pop_back`
- `erase`
- `append_range`
- `assign_range`
- `replace`
- `replace_all`
- `replace_n`
- `reverse()`
- `reverse(pos, count = npos)`

## Case transforms

Owning strings expose:

- `to_ascii_lowercase()`
- `to_ascii_uppercase()`
- `to_lowercase()`
- `to_uppercase()`
- partial overloads with `pos, count = npos`
- `case_fold()`

The `&&` overloads may reuse storage when profitable, but that reuse is an optimization detail rather than a guaranteed contract.

## Normalization

Owning strings also expose:

- `normalize(normalization_form)`
- `to_nfc()`
- `to_nfd()`
- `to_nfkc()`
- `to_nfkd()`
- `is_normalized(...)`
- `is_nfc()`, `is_nfd()`, `is_nfkc()`, `is_nfkd()`

Normalization is whole-string only.

## Interoperability

Owning strings interoperate with:

- their corresponding view types
- formatting and stream output
- hashing
- conversion between UTF-8 and UTF-16

For code that mostly reads, prefer the view types. For code that owns and mutates validated text, prefer the owning types.
