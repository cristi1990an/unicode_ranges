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

Those semantic rules do not require every character type to store a scalar directly. `utf8_char` stores one validated UTF-8 byte sequence, `utf16_char` stores one validated UTF-16 code-unit sequence, and `utf32_char` stores one validated UTF-32 code unit.

## Compiled runtime backend

The library has a compiled runtime backend. The hot runtime UTF boundary operations live in the compiled `unicode_ranges` library target and use `simdutf` as the backend for:

- UTF-8, UTF-16, and UTF-32 validation
- UTF-8, UTF-16, and UTF-32 transcoding on runtime paths
- UTF-8/UTF-16 character counting and selected ASCII-only checks

This is a pragmatic design decision. In the comparative benchmark suite, `simdutf` has been the strongest raw UTF codec baseline, so the library uses it through its public API instead of re-implementing the same runtime dispatch ladder itself.

That backend choice does not change the core model:

- the public API is still `unicode_ranges`
- validated types and higher-level algorithms still belong to `unicode_ranges`
- compile-time and `constexpr`-oriented behavior remains implemented locally
- the `simdutf` dependency is specifically about the runtime hot path for contiguous UTF validation, transcoding, counting, and ASCII scans

## Ownership model

- `utf8_string_view` / `utf16_string_view` / `utf32_string_view` borrow validated storage.
- `utf8_string` / `utf16_string` / `utf32_string` own validated storage.
- range-returning APIs such as `chars()` and `graphemes()` borrow from the source text.

This makes lifetime and mutation rules explicit instead of implicit.

## Rvalue aware API

The library calls the copy-producing and consuming overload pattern the rvalue aware API. Owning-string methods that return another owning string and expose both `const&` and `&&` receiver overloads are rvalue aware methods.

```cpp
auto new_string = my_string.operation();              // keep my_string unchanged
my_string = std::move(my_string).operation();         // consume and assign back
```

The `const&` form returns a new owning string and leaves the source unchanged. The `&&` form treats the source as disposable and tries to optimize the operation by reusing the existing buffer when that is safe and useful.

The pattern is useful because it gives one logical operation one name while still supporting both ownership modes:

- it avoids parallel `operation_copy()` / `operation_inplace()` / `operation_owned()` method families for the same transformation
- it keeps ordinary lvalue calls non-mutating, so accidental source modification requires an explicit `std::move`
- it makes consuming chains efficient because each step receives an owning rvalue and can reuse storage when the operation shape allows it
- it preserves API readability: the operation name describes the text transformation, and the receiver category describes whether the source may be consumed

This is an optimization opportunity, not a no-allocation guarantee. A consuming overload may still allocate when the operation expands the text, allocator or capacity constraints require it, or a fresh result is the safer implementation. After an `&&` call, the moved-from owning string remains valid but has an unspecified value; use the returned object.

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

Borrowed views and owning strings intentionally differ for bound-adjusting APIs. Calling `strip_*`, `trim_*`, `substr`, or `grapheme_substr` on a view returns another view. Calling the same API on an owning string returns an owning string, and rvalue owning receivers reuse their existing buffer when the operation is just a prefix/suffix bound adjustment. APIs that only make sense as borrowed subview producers, such as `split_once`, `rsplit_once`, `split_once_at`, and `grapheme_at`, are deleted on owning rvalues to prevent dangling results.

## Iteration and encoded storage

### Encoded strings do not model ranges directly

`utf8_string_view`, `utf16_string_view`, `utf32_string_view`, and their owning counterparts intentionally do not expose direct range-based iteration.

That is deliberate: `for (auto x : text)` is ambiguous for encoded Unicode text. The obvious candidates are:

- raw UTF code units
- Unicode scalar values
- grapheme clusters

The library requires that choice to be explicit instead of silently picking one interpretation.

### `base()` is the raw-storage escape hatch

When callers explicitly want the encoded storage, the API exposes `base()`.

That member is named `base()` rather than `bytes()` because the same surface is shared across UTF-8, UTF-16, and UTF-32:

- for UTF-8, `base()` exposes the underlying `std::u8string_view` / `std::u8string`
- for UTF-16, `base()` exposes the underlying `std::u16string_view` / `std::u16string`
- for UTF-32, `base()` exposes the underlying `std::u32string_view` / `std::u32string`

The name is intentionally generic because the concept is "underlying validated storage", not specifically "bytes". For UTF-32, that storage is still just the underlying UTF-32 code-unit sequence; it only happens to line up 1:1 with the represented scalar values.

### `chars()` is explicit scalar iteration

`chars()` and `reversed_chars()` return dedicated view types over Unicode scalar values.

These views are created through member functions only:

- their constructors are not part of the public user-facing construction path
- the library does not expose a `range_adaptor_closure`-style pipe API for them

That keeps scalar iteration discoverable and explicit at the string/view API boundary.

Construction wraps existing validated storage and is cheap. Code that depends on exact construction complexity rather than the documented view semantics is relying on an implementation detail.

The iteration strength depends on the encoding:

- UTF-8 and UTF-16 scalar iteration are forward ranges
- UTF-32 scalar iteration is random-access

### `graphemes()` returns borrowed text slices

`graphemes()` returns a forward view whose elements are encoding-matched string views:

- `utf8_string_view` slices for UTF-8 text
- `utf16_string_view` slices for UTF-16 text
- `utf32_string_view` slices for UTF-32 text

Each element represents one grapheme cluster under the default Unicode grapheme-cluster rules.

There is intentionally no `reversed_graphemes()` companion. Default grapheme boundaries are discovered in forward order, so a practical reverse-grapheme implementation has to split forward, store the grapheme slices, and then iterate that storage backwards. That allocation-heavy behavior is too surprising for view creation. Callers that want those semantics can materialize `graphemes()` into a container and reverse that container explicitly.

### Alternatives considered for grapheme iteration

Two obvious alternatives were considered and rejected.

Returning a dedicated grapheme value type, analogous to `utf8_char`, would add another abstraction layer without enough clear payoff. In most places, a borrowed string-view slice already communicates the right semantics, and the `_grapheme_utf8`, `_grapheme_utf16`, and `_grapheme_utf32` literals cover the "single grapheme value" use case.

Returning owning strings instead of borrowed slices would solve some lifetime problems and would often fit inside small-string optimization for short graphemes, but it would also make the common iteration path heavier. When ownership is actually needed, callers can materialize it explicitly with a simple transform step.

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

The non-`constexpr` surface is intentionally the exception, not the rule. The main outliers are operation families that require runtime services outside the library's compile-time tables:

- ICU-backed locale-aware casing and caseless comparison, such as `to_lowercase(locale)`, `to_uppercase(locale)`, `to_titlecase(locale)`, `case_fold(locale)`, and the locale-aware `*_ignore_case(...)` helpers
- runtime locale availability probing through `is_available_locale(...)`
- formatting, printing, and streaming integration, including `std::formatter` specializations, `std::print` / `std::println` usage, and `operator<<`
- hashing integration through `std::hash` specializations

The compiled runtime backend is also runtime-only internally. That does not make the public checked UTF APIs non-`constexpr`; when an API is declared `constexpr`, constant evaluation uses the library's compile-time path instead of the runtime backend.

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
- optional ICU-backed locale-aware casing
- formatting / streaming / hashing for library-defined types

Out of scope:

- locale-aware collation
- built-in locale-specific casing tables without ICU
- bidi or layout/shaping engines
- regex engines
- tailored segmentation rules beyond the default grapheme algorithm
