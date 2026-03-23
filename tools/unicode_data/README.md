This directory holds versioned Unicode Character Database inputs used by `tools/gen_unicode_tables.rs`.

Expected layout for version `17.0.0`:

- `tools/unicode_data/17.0.0/ucd/auxiliary/GraphemeBreakProperty.txt`
- `tools/unicode_data/17.0.0/ucd/auxiliary/GraphemeBreakTest.txt`
- `tools/unicode_data/17.0.0/ucd/emoji/emoji-data.txt`
- `tools/unicode_data/17.0.0/ucd/DerivedCoreProperties.txt`

You can populate that tree with:

```powershell
pwsh ./tools/update_unicode_data.ps1 -Version 17.0.0
```

The generator accepts an optional custom data-root argument if you want to point it at a different checked-out Unicode dataset.
