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
- Original files:
  - `src/scalar/utf8.h`
  - `include/simdutf/portability.h`
  - `src/simdutf/haswell/begin.h`
  - `src/simdutf/haswell/simd.h`
  - `src/generic/buf_block_reader.h`
  - `src/generic/utf8_validation/utf8_lookup4_algorithm.h`
  - `src/generic/utf8_validation/utf8_validator.h`
  - `src/haswell/avx2_convert_utf8_to_utf16.cpp`
  - `src/haswell/avx2_convert_utf8_to_utf32.cpp`
  - `src/tables/utf8_to_utf16_tables.h`
  - `src/generic/utf8_to_utf16/valid_utf8_to_utf16.h`
  - `src/generic/utf8_to_utf32/valid_utf8_to_utf32.h`
- Original license: `MIT OR Apache-2.0`
- Repository file:
  - `unicode_ranges/internal/simdutf_utf8_scalar_validate.hpp`
  - `unicode_ranges/internal/simdutf_haswell_utf8_validate.hpp`
  - `unicode_ranges/internal/simdutf_utf8_to_utf16_tables.hpp`
  - `unicode_ranges/internal/simdutf_haswell_utf8_to_utf16.hpp`
  - `unicode_ranges/internal/simdutf_haswell_utf8_to_utf32.hpp`
- Adaptation summary:
  - reduced to runtime UTF-8 validation
  - scalar path maps errors into `unicode_ranges::utf8_error`
  - AVX2 path is success/fail only and falls back to the scalar validator for
    exact error reporting
  - AVX2 valid UTF-8 to UTF-16 conversion is imported for the runtime
    trusted-valid path and reused by the checked path after validation succeeds
  - AVX2 valid UTF-8 to UTF-32 conversion is imported for the runtime
    trusted-valid path and reused by the checked path after validation succeeds
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
