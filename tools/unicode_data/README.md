This directory holds versioned Unicode Character Database inputs used by `tools/gen_unicode_tables.rs`.

Expected layout for version `17.0.0`:

- `tools/unicode_data/17.0.0/ucd/UnicodeData.txt`
- `tools/unicode_data/17.0.0/ucd/CompositionExclusions.txt`
- `tools/unicode_data/17.0.0/ucd/CaseFolding.txt`
- `tools/unicode_data/17.0.0/ucd/auxiliary/GraphemeBreakProperty.txt`
- `tools/unicode_data/17.0.0/ucd/auxiliary/GraphemeBreakTest.txt`
- `tools/unicode_data/17.0.0/ucd/emoji/emoji-data.txt`
- `tools/unicode_data/17.0.0/ucd/DerivedCoreProperties.txt`

You can populate that tree with:

```powershell
pwsh ./tools/update_unicode_data.ps1 -Version 17.0.0
```

Regenerate the checked-in C++ tables with:

```powershell
pwsh ./tools/regenerate_unicode_tables.ps1
```

The regeneration script writes `unicode_ranges/unicode_tables.hpp` as UTF-8 without BOM so Clang-cl can consume it reliably.

The Rust generator accepts an optional custom data-root argument if you want to point it at a different checked-out Unicode dataset.
