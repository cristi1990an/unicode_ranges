# Licensing

## Repository license

`unicode_ranges` is dual-licensed under `MIT OR Apache-2.0`.

That means you may use the repository under either:

- the MIT license
- the Apache License, Version 2.0

The repository root contains:

- [`LICENSE`](https://github.com/cristi1990an/unicode_ranges/blob/main/LICENSE)
- [`LICENSE-MIT`](https://github.com/cristi1990an/unicode_ranges/blob/main/LICENSE-MIT)
- [`LICENSE-APACHE`](https://github.com/cristi1990an/unicode_ranges/blob/main/LICENSE-APACHE)

Unless otherwise noted, repository source files are available under that dual-license model.

## Runtime dependency license

The compiled runtime backend currently depends on pinned `simdutf` `v7.7.0` for:

- UTF-8 validation
- UTF-8 -> UTF-16 transcoding
- UTF-8 -> UTF-32 transcoding

`simdutf` is also dual-licensed under `MIT OR Apache-2.0`, which keeps the licensing model straightforward for the current compiled-library design.

`unicode_ranges` vendors the pinned `simdutf` singleheader release under `third_party/simdutf`:

- `simdutf.h`
- `simdutf.cpp`
- `LICENSE-MIT`
- `LICENSE-APACHE`

These are tracked upstream distribution files from `simdutf` `v7.7.0`, not ad hoc local fetches.

## Comparative benchmark dependencies

The comparative benchmark suite also uses pinned external libraries:

- `simdutf` `v7.7.0`
- `utfcpp` `v4.0.9`
- `uni-algo` `v1.0.0`

These keep their own licenses. See [`THIRD_PARTY_NOTICES.md`](https://github.com/cristi1990an/unicode_ranges/blob/main/THIRD_PARTY_NOTICES.md) for the current dependency list and license expressions.

## Notices and provenance policy

The authoritative third-party notice file is:

- [`THIRD_PARTY_NOTICES.md`](https://github.com/cristi1990an/unicode_ranges/blob/main/THIRD_PARTY_NOTICES.md)

That file records:

- pinned dependency versions
- third-party license expressions
- the provenance-header format required for any future copied or adapted source files

At the moment:

- `simdutf` is a vendored runtime dependency
- `utfcpp` and `uni-algo` are comparative-benchmark dependencies
- the repository tracks the vendored `simdutf` singleheader release under `third_party/simdutf`
