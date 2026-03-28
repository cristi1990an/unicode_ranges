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

## Partial casing on owning strings

Owning strings support both whole-string and subrange casing:

```cpp
text.to_uppercase();
text.to_uppercase(pos, count);
```

The checked subrange overloads validate bounds and character boundaries. Whole-string overloads remain the cheaper path.

## Case folding

Case folding is the Unicode form intended for caseless matching rather than display transformation.

Available API:

```cpp
auto folded = text.case_fold();
```

That is different from simply lowercasing text. Case folding handles mappings such as German `ß` in a way intended for case-insensitive comparison behavior.

## Normalization

Supported normalization forms:

- NFC
- NFD
- NFKC
- NFKD

Available APIs:

```cpp
text.normalize(normalization_form::nfc);
text.to_nfc();
text.to_nfd();
text.to_nfkc();
text.to_nfkd();
```

And corresponding checks:

```cpp
text.is_normalized(normalization_form::nfc);
text.is_nfc();
text.is_nfd();
text.is_nfkc();
text.is_nfkd();
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

- locale-specific casing
- locale-specific collation
- Turkic-specific case-fold tailoring switches
