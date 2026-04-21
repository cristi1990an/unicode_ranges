# Third-Party Notices

This repository is dual-licensed under `MIT OR Apache-2.0`. Third-party
components keep their own licenses.

## Current state

- `simdutf` is a pinned runtime dependency and is also reused by the comparative benchmark suite.
- the shipped compiled runtime backend currently uses `simdutf` through its public API and published singleheader release layout
- Comparative benchmark dependencies are fetched separately.
- No copied or adapted third-party source files are currently tracked in the
  library sources.
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

There are currently no copied or adapted third-party source files tracked in
the repository.

## Runtime and comparative benchmark dependencies

These projects are used either by the shipped library runtime, the comparative
benchmark suite, or both. They are consumed through pinned release/tag fetches
instead of tracked source copies.

### simdutf

- Project: `simdutf`
- Upstream: <https://github.com/simdutf/simdutf>
- Version used by the library runtime and comparative benchmark suite: `v7.7.0`
- License: `MIT OR Apache-2.0`
- Consumption model:
  - compiled runtime dependency
  - consumed through the published `simdutf.h` + `simdutf.cpp` singleheader release layout
  - no tracked copied/adapted `simdutf` source files in the library sources at this time
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
