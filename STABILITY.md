# Stability And Support Notes

This document describes the intended public support surface while `unicode_ranges`
continues to stabilize.

No formal source-compatibility promise is active yet. The API can still change
when correctness, safety, naming, or long-term maintainability require it.

## Public API Surface

The intended public include entry points are:

- `unicode_ranges_borrowed.hpp`
- `unicode_ranges_all.hpp`

The repository currently also ships compatibility wrappers:

- `unicode_ranges.hpp`
- `unicode_ranges_full.hpp`

Those wrappers are transitional and are not intended to be the primary umbrella
names long term.

The intended public namespaces are:

- `unicode_ranges`
- `unicode_ranges::literals`
- `unicode_ranges::pmr`
- `unicode_ranges::views`
- `unicode_ranges::characters`

The following are not public API and may change at any time:

- `unicode_ranges::details`
- files under `tools/`
- files under `.github/`
- vendored third-party source under `third_party/`
- private/generated build outputs

Headers under `unicode_ranges/` are installed and may be included directly, but
the umbrella headers above are the preferred compatibility boundary. File-level
layout below the umbrella headers is not a stability promise on its own.

## Build And Integration Surface

The authoritative build and integration surface is:

- CMake
- target: `unicode_ranges::unicode_ranges`
- installed package consumption through:
  - `find_package(unicode_ranges CONFIG REQUIRED)`

The repository also ships Visual Studio project files for Windows convenience
and maintainer workflow. When there is a conflict between those project files
and the first-party CMake build, the CMake build is authoritative.

The intended integration model is:

- build the library from source
- link the compiled `unicode_ranges` library target
- consume the installed/exported CMake package when useful

The project does not currently promise:

- prebuilt binaries
- package-manager distribution for every ecosystem
- C++ modules

## Dependency Policy

`unicode_ranges` is a compiled library with:

- vendored pinned `simdutf` for runtime UTF hot paths
- optional ICU for locale-aware casing

Current dependency policy:

- `simdutf` is a production dependency of the compiled runtime backend
- `simdutf` is vendored and pinned, not fetched from `main` during normal use
- ICU is optional and only affects the locale-aware feature surface

The specific vendored dependency revision is not itself part of the public C++
API contract. It may change as long as:

- licensing remains compatible with the documented project policy
- intended public behavior is preserved
- the documented integration story remains accurate

## Supported Toolchains

The minimum toolchains currently exercised in CI are:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

Support can expand as newer toolchains become available.

Dropping an already-supported toolchain should be treated as a compatibility
decision unless continued support becomes impossible because of an upstream
dependency, security issue, or permanently broken toolchain behavior outside
this project's control.

## Compatibility Goals

The project aims to preserve source compatibility for:

- documented public types
- documented public functions and overloads
- documented public namespaces
- the primary CMake target and package name

The project may still:

- add new APIs
- add new overloads
- add new documentation pages and examples
- improve performance
- restructure internal implementation
- change private file layout
- update vendored dependency revisions

Breaking changes to documented public API should be deliberate and documented.

The main compatibility target is source compatibility, not ABI compatibility.

## Unicode Data And Behavioral Drift

`unicode_ranges` is Unicode-data-driven.

Unicode data updates may change observable results for:

- properties
- casing
- normalization
- segmentation

when the Unicode standard itself changes.

Those standards-aligned behavioral updates are expected and are not treated as
accidental regressions by default.

## Performance And Compile-Time Expectations

Performance matters. Compile-time cost also matters.

Neither runtime performance numbers nor compile-time measurements are a strict
compatibility contract by themselves.

The project will treat major regressions seriously, but reserves the right to:

- change internal backends
- change internal fast paths
- rebalance implementation tradeoffs

as long as the documented public API, behavior, and support story are preserved.

## Non-Goals

The following are intentionally not promised:

- ABI stability across compilers and standard libraries
- binary compatibility between all build configurations
- support for every possible build system
- support for private/internal headers
- support for `unicode_ranges::details`
- permanent stability of internal implementation choices

## Stable API Gate

The repository is ready for stable public versioning when:

- this compatibility policy is accepted
- docs and README reflect the same support story
- install/package validation passes from a fresh clone:
  - configure
  - build
  - test
  - install
  - consumer `find_package(...)` smoke test
- the compile-time cost of the common include surfaces is measured and accepted
- outstanding breaking API questions are resolved
