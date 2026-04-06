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

If the library is built with `UTF8_RANGES_ENABLE_ICU=1`, additional ICU-backed locale overloads are available for lowercasing, uppercasing, titlecasing, and case folding:

```cpp
--8<-- "examples/casing/locale-case.cpp"
```

You can also check whether the current ICU data set explicitly exposes a locale identifier:

```cpp
assert(is_available_locale("tr"_locale));
```

`is_available_locale(...)` is a non-throwing probe. It returns `false` for invalid or unusable locale identifiers as well as for locale identifiers that are simply not exposed by the current ICU data set.

Behavior note:

- `locale_id` is a raw null-terminated locale-name token.
- `_locale` rejects embedded NULs in string literals at compile time.
- Raw `locale_id{ ... }` values do not own storage; the pointed-to locale name must stay alive for the duration of the call.
- The locale-aware overloads reject obviously unusable tokens such as `locale_id{ nullptr }`.
- Otherwise, locale-aware `to_lowercase(...)`, `to_uppercase(...)`, `to_titlecase(...)`, and `case_fold(...)` forward the locale name to ICU.
- If the locale is not explicitly available in the current ICU data set, ICU may canonicalize the locale or fall back to a more general locale instead of failing the call.
- If ICU rejects the locale name or another ICU operation fails, the locale-aware overload throws `std::runtime_error`.
- If you need an exact availability check before calling a locale-aware casing overload, use `is_available_locale(...)`.

`to_titlecase(locale)` is whole-string only. That is intentional: titlecasing depends on ICU break-iterator context, so partial `pos, count` overloads would have less predictable semantics than lowercasing or uppercasing a checked slice.

Those overloads do not exist in the dependency-free default build.

## Partial casing on owning strings

Owning strings support both whole-string and subrange casing. The checked subrange overloads validate bounds and character boundaries. Whole-string overloads remain the cheaper path.

```cpp
--8<-- "examples/casing/partial-case.cpp"
```

## Case folding

Case folding is the Unicode form intended for caseless matching rather than display transformation.

If ICU is enabled, `case_fold(locale)` is also available. In practice, the locale-sensitive difference is the Turkic special-I fold; most locales produce the same result as the default `case_fold()`.

```cpp
--8<-- "examples/casing/case-fold.cpp"
```

That is different from simply lowercasing text. Case folding handles mappings such as German `ß` in a way intended for case-insensitive comparison behavior.

The string-view types also expose allocation-free helpers built on top of default Unicode case folding:

- `eq_ignore_case(...)`
- `starts_with_ignore_case(...)`
- `ends_with_ignore_case(...)`
- `compare_ignore_case(...)`

Those helpers do not normalize. That is intentional:

- case folding and normalization stay separate operations in this library
- caseless comparison should not silently add normalization work or broaden equivalence
- callers who need canonical-equivalence-aware caseless matching should say so explicitly

So the default rule is: case-fold only. If you want canonical-equivalence-aware caseless matching, normalize explicitly first and then compare.

If ICU is enabled, locale-aware overloads of those helpers are also available:

- `eq_ignore_case(..., locale)`
- `starts_with_ignore_case(..., locale)`
- `ends_with_ignore_case(..., locale)`
- `compare_ignore_case(..., locale)`

They still compare folded scalar sequences without materializing a temporary folded string, but they are not `noexcept` because locale handling follows the same ICU-backed rules as `case_fold(locale)`.

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
