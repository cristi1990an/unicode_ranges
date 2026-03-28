# Unicode Data

## Unicode version

The library exposes:

```cpp
inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version;
```

That constant reflects the generated Unicode tables checked into the repository.

Current version: Unicode `17.0.0`.

## What is generated

The generated tables cover the Unicode data needed by the library's public behavior, including:

- scalar classification predicates
- grapheme segmentation properties
- Unicode casing tables
- normalization decomposition and composition data
- full case-fold mappings

The generated output lives in `unicode_ranges/unicode_tables.hpp`.

## Source data

The update pipeline consumes official Unicode Character Database inputs under `tools/unicode_data/<version>/`.

Important files now in the pipeline include:

- `UnicodeData.txt`
- `CompositionExclusions.txt`
- `CaseFolding.txt`

## Updating Unicode data

Typical workflow:

1. Refresh the raw Unicode data under `tools/unicode_data/<version>/`.
2. Rerun `tools/regenerate_unicode_tables.ps1`.
3. Commit the regenerated `unicode_ranges/unicode_tables.hpp`.
4. Update the changelog and any affected documentation.

## Why tables are checked in

The library is header-only and aims to be easy to consume without a build-time generator dependency. Checking in the generated tables keeps usage simple for downstream users while still allowing the Unicode pipeline to remain explicit and reproducible.

## Notes on semantics

- Grapheme segmentation follows default Unicode rules.
- Unicode casing is locale-independent.
- Case folding uses full Unicode case-fold mappings, but not locale-specific tailorings.
- Normalization supports NFC, NFD, NFKC, and NFKD.
