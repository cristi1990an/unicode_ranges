# Casing and Normalization

## ASCII versus Unicode casing

The library exposes both ASCII-only and Unicode-aware casing APIs.

ASCII-only:

- `to_ascii_lowercase()`
- `to_ascii_uppercase()`

Unicode-aware:

- `to_lowercase()`
- `to_uppercase()`

The Unicode-aware APIs are locale-independent. They follow generated Unicode tables and do not apply locale-specific tailoring.

```cpp
--8<-- "examples/casing/unicode-case.cpp"
```

If the library is built with `UTF8_RANGES_ENABLE_ICU=1`, additional ICU-backed locale overloads are available for lowercasing and uppercasing:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

assert(u8"I\u0130"_utf8_sv.to_lowercase("tr"_locale) == u8"\u0131i"_utf8_sv);
assert(u8"i\u0131"_utf8_sv.to_uppercase("tr"_locale) == u8"\u0130I"_utf8_sv);
```

You can also check whether the current ICU data set explicitly exposes a locale identifier:

```cpp
assert(is_available_locale("tr"_locale));
```

Those overloads do not exist in the dependency-free default build.

## Partial casing on owning strings

Owning strings support both whole-string and subrange casing. The checked subrange overloads validate bounds and character boundaries. Whole-string overloads remain the cheaper path.

```cpp
--8<-- "examples/casing/partial-case.cpp"
```

## Case folding

Case folding is the Unicode form intended for caseless matching rather than display transformation.

```cpp
--8<-- "examples/casing/case-fold.cpp"
```

That is different from simply lowercasing text. Case folding handles mappings such as German `ß` in a way intended for case-insensitive comparison behavior.

## Normalization

Supported normalization forms:

- NFC
- NFD
- NFKC
- NFKD

```cpp
--8<-- "examples/casing/normalization.cpp"
```

And corresponding checks:

```cpp
--8<-- "examples/casing/normalization-checks.cpp"
```

## Why normalization matters

Some text has multiple valid Unicode representations. For example, an accented character may be stored as:

- a single precomposed scalar
- or a base character plus combining mark

Normalization lets callers put text into a stable canonical or compatibility form before storage, comparison, or further processing.

## API design choice: no partial normalization

The library intentionally does not expose `normalize(pos, count)` style APIs.

Normalization is less local than casing because boundary behavior can depend on combining marks and composition opportunities around the edge of a slice. Whole-string normalization is the safer and clearer initial contract.

## Current scope

Implemented:

- Unicode lowercasing and uppercasing
- full Unicode case folding
- NFC, NFD, NFKC, NFKD

Out of scope:

- built-in locale-specific casing tables without ICU
- locale-specific collation
- Turkic-specific case-fold tailoring switches
