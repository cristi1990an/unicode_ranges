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

## Unicode data license

The generated tables are derived from Unicode Character Database data and are
distributed under Unicode License V3 (`Unicode-3.0`). The complete notice is
in [`LICENSE-UNICODE`](https://github.com/cristi1990an/unicode_ranges/blob/main/LICENSE-UNICODE)
and is installed with the package.

## Runtime dependency license

The CMake, vcpkg, and Conan library packages link to simdutf as a separately
packaged dependency (Conan uses `simdutf/8.2.0`; vcpkg requires 8.2.0 or newer).
The dependency package supplies its own copyright and license notices.

The source repository retains a `simdutf` `v7.7.0` singleheader snapshot under
`third_party/simdutf` for the standalone Visual Studio project and comparative
benchmark tooling. CMake, vcpkg, and Conan package builds do not compile that
snapshot into the distributed library.

The snapshot's upstream license texts remain alongside those files in the
source tree.

## Comparative benchmark dependencies

The comparative benchmark suite also uses pinned external libraries:

- `simdutf` `v7.7.0`
- `utfcpp` `v4.0.9`
- `uni-algo` `v1.0.0`

These keep their own licenses. See [`THIRD_PARTY_NOTICES.md`](https://github.com/cristi1990an/unicode_ranges/blob/main/THIRD_PARTY_NOTICES.md) for the dependency list and license expressions.

## Notices and provenance policy

The authoritative third-party notice file is:

- [`THIRD_PARTY_NOTICES.md`](https://github.com/cristi1990an/unicode_ranges/blob/main/THIRD_PARTY_NOTICES.md)

That file records:

- pinned dependency versions
- third-party license expressions
- the provenance-header format required for copied or adapted source files

Current use:

- the package-managed simdutf dependency is used by CMake, vcpkg, and Conan package builds
- the source-tree v7.7.0 simdutf snapshot is used only by the standalone Visual Studio project and comparative benchmark tooling
- `utfcpp` and `uni-algo` are comparative-benchmark dependencies
- Unicode-generated data is separately covered by Unicode License V3
