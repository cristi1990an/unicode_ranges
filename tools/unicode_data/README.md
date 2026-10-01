This directory holds versioned Unicode Character Database inputs used by `tools/gen_unicode_tables.rs`.

The checked-in data currently includes versions `17.0.0` and `18.0.0`. Each version uses this layout:

- `tools/unicode_data/<version>/ucd/UnicodeData.txt`
- `tools/unicode_data/<version>/ucd/CompositionExclusions.txt`
- `tools/unicode_data/<version>/ucd/CaseFolding.txt`
- `tools/unicode_data/<version>/ucd/Scripts.txt`
- `tools/unicode_data/<version>/ucd/EastAsianWidth.txt`
- `tools/unicode_data/<version>/ucd/LineBreak.txt`
- `tools/unicode_data/<version>/ucd/extracted/DerivedBidiClass.txt`
- `tools/unicode_data/<version>/ucd/auxiliary/GraphemeBreakProperty.txt`
- `tools/unicode_data/<version>/ucd/auxiliary/WordBreakProperty.txt`
- `tools/unicode_data/<version>/ucd/auxiliary/SentenceBreakProperty.txt`
- `tools/unicode_data/<version>/ucd/auxiliary/GraphemeBreakTest.txt` (official grapheme conformance corpus)
- `tools/unicode_data/<version>/ucd/NormalizationTest.txt` (official normalization corpus; fetched separately by the conformance workflow)
- `tools/unicode_data/<version>/ucd/emoji/emoji-data.txt`
- `tools/unicode_data/<version>/ucd/DerivedCoreProperties.txt`
- `tools/unicode_data/<version>/ucd/DerivedNormalizationProps.txt`

The generator inputs can be downloaded for any Unicode version supported by the generator with:

```powershell
pwsh ./tools/update_unicode_data.ps1 -Version 18.0.0
```

Regenerate the checked-in C++ tables with:

```powershell
pwsh ./tools/regenerate_unicode_tables.ps1 -DataRoot ./tools/unicode_data/18.0.0 -UnicodeVersion 18.0.0
```

The regeneration script writes `unicode_ranges/unicode_tables_constexpr.hpp` as UTF-8 without BOM so Clang-cl can consume it reliably. `unicode_ranges/unicode_tables.hpp` is kept as a thin compatibility wrapper.

The Rust generator accepts an optional custom data-root argument if you want to point it at a different checked-out Unicode dataset.
