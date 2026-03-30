# Common Tasks

This page is the recipe layer above the full reference. The examples here are small, runnable, and compiled in CI.

## Validate runtime UTF-8 once

When text arrives as raw bytes, validate it once and keep the validated type:

```cpp
--8<-- "examples/getting-started/runtime-validation.cpp"
```

## Iterate scalars versus graphemes

Use `chars()` when you need Unicode scalar values. Use `graphemes()` when you need user-perceived characters.

```cpp
--8<-- "examples/getting-started/validated-view.cpp"
```

## Inspect grapheme boundaries

When byte or code-unit offsets matter, use the explicit boundary helpers rather than assuming every scalar boundary is also a grapheme boundary:

```cpp
--8<-- "examples/text-operations/boundaries-and-graphemes.cpp"
```

## Choose ASCII-only versus Unicode-aware casing

ASCII-only transforms are intentionally narrow and fast. Unicode-aware transforms are the ones to use for real text:

```cpp
--8<-- "examples/casing/unicode-case.cpp"
```

## Normalize before equality on equivalent spellings

Visually identical Unicode text can be spelled with different scalar sequences. Normalize both sides before comparing:

```cpp
--8<-- "examples/common-tasks/normalize-before-compare.cpp"
```

## Convert UTF-8 and UTF-16

Validated UTF-8 and UTF-16 text can be converted directly without falling back to raw transcoding code:

```cpp
--8<-- "examples/common-tasks/convert-encodings.cpp"
```

## Next steps

- [Getting Started](getting-started.md)
- [Text Operations](text-operations.md)
- [Casing and Normalization](casing-and-normalization.md)
- [Reference](reference/index.md)
