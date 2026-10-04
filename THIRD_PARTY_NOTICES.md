# Third-Party Notices

The library source code is dual-licensed under `MIT OR Apache-2.0`. Generated
Unicode data is distributed under the Unicode License V3; see
`LICENSE-UNICODE`. Third-party components keep their own licenses.

## Current state

- The compiled library runtime links to the package-managed `simdutf` dependency.
- The source tree retains the upstream `simdutf` v7.7.0 singleheader distribution for the standalone Visual Studio project and comparative benchmarks; package builds do not compile that copy.
- vcpkg and Conan install simdutf as a separate dependency, whose own package provides the runtime dependency's copyright and license notices.
- Comparative benchmark dependencies besides the retained `simdutf` source are fetched separately.
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
// - Upstream version: <tag / commit / archive>
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

These are upstream files from the pinned `simdutf` `v7.7.0` singleheader distribution and license texts, not locally adapted source files.

## Runtime and comparative benchmark dependencies

These projects are used by the shipped library runtime or the comparative
benchmark suite.

### simdutf

- Project: `simdutf`
- Upstream: <https://github.com/simdutf/simdutf>
- Library runtime package version: `8.2.0` or compatible newer release
- Standalone Visual Studio and comparative benchmark snapshot: `v7.7.0`
- License: `MIT OR Apache-2.0`
- Consumption model:
  - CMake package dependency for CMake, vcpkg, and Conan library package builds
  - the separate `third_party/simdutf` snapshot is used only by non-package development tooling
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
