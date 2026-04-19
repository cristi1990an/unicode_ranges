# Third-Party Notices

This repository is dual-licensed under `MIT OR Apache-2.0`. Third-party
components keep their own licenses.

## Current state

- Comparative benchmark dependencies are fetched separately.
- A copied/adapted third-party validation slice is now present in the tracked
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

### simdutf validation slice

- Project: `simdutf`
- Upstream: <https://github.com/simdutf/simdutf>
- Upstream version: `v7.7.0`
- Original file: `src/scalar/utf8.h`
- Original license: `MIT OR Apache-2.0`
- Repository file:
  - `unicode_ranges/internal/simdutf_utf8_scalar_validate.hpp`
- Adaptation summary:
  - reduced to runtime UTF-8 validation
  - mapped errors into `unicode_ranges::utf8_error`
  - removed unrelated APIs and supporting types not needed by the import

## Comparative benchmark dependencies

These projects are used by the comparative benchmark suite through dynamic fetch
steps and local benchmark adapters.

### simdutf

- Project: `simdutf`
- Upstream: <https://github.com/simdutf/simdutf>
- Version used by the comparative benchmark suite: `v7.7.0`
- License: `MIT OR Apache-2.0`
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
