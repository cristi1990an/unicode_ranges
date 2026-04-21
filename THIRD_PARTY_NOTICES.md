# Third-Party Notices

This repository is dual-licensed under `MIT OR Apache-2.0`. Third-party
components keep their own licenses.

## Current state

- `simdutf` is a pinned vendored runtime dependency and is also reused by the comparative benchmark suite.
- the shipped compiled runtime backend currently uses `simdutf` through its public API and vendored singleheader release layout under `third_party/simdutf`
- Comparative benchmark dependencies besides `simdutf` are fetched separately.
- The repository tracks the vendored upstream `simdutf` singleheader release files under `third_party/simdutf`.
- Any additional copied or adapted third-party source files must carry an
  explicit provenance header as described below.

## Provenance header format for copied or adapted source files

Use a header in this shape at the top of every copied or adapted file:

```cpp
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright (c) 2026 unicode_ranges contributors
//
// Provenance:
// - Adapted from: <project name>
// - Original file: <upstream path or URL>
// - Upstream version: <tag / commit / release>
// - Original license: <license expression>
// - Changes for unicode_ranges: <short summary>
```

Rules:

- Keep the upstream project name and original file reference concrete.
- Record the exact upstream version, tag, or commit used as the source.
- Preserve any required upstream copyright and license notices.
- Update the `Changes for unicode_ranges` line when materially editing the file.

## Copied or adapted source files currently in the repository

The repository currently tracks the following vendored upstream distribution files:

- `third_party/simdutf/simdutf.h`
- `third_party/simdutf/simdutf.cpp`
- `third_party/simdutf/README.md`
- `third_party/simdutf/LICENSE-MIT`
- `third_party/simdutf/LICENSE-APACHE`

These are upstream release files from the pinned `simdutf` `v7.7.0` singleheader distribution and license texts, not locally adapted source files.

## Runtime and comparative benchmark dependencies

These projects are used either by the shipped library runtime, the comparative
benchmark suite, or both.

### simdutf

- Project: `simdutf`
- Upstream: <https://github.com/simdutf/simdutf>
- Version used by the library runtime and comparative benchmark suite: `v7.7.0`
- License: `MIT OR Apache-2.0`
- Consumption model:
  - compiled runtime dependency
  - vendored under `third_party/simdutf`
  - uses the published `simdutf.h` + `simdutf.cpp` singleheader release layout
- Local metadata:
  - `comparative_benchmarks/dependencies.json`

### utfcpp

- Project: `utfcpp`
- Upstream: <https://github.com/nemtrif/utfcpp>
- Version used by the comparative benchmark suite: `v4.0.9`
- License: `Boost Software License 1.0`
- Local metadata:
  - `comparative_benchmarks/dependencies.json`

### uni-algo

- Project: `uni-algo`
- Upstream: <https://github.com/uni-algo/uni-algo>
- Version used by the comparative benchmark suite: `v1.0.0`
- License: `Public Domain OR MIT`
- Local metadata:
  - `comparative_benchmarks/dependencies.json`
