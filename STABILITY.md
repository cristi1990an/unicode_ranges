# Stability And Support Policy

This document defines the intended `1.x` stability contract for `unicode_ranges`.

Before `v1.0.0` is tagged, this document is a release target and review checklist.
After `v1.0.0`, it becomes the policy for future `1.x` releases.

## What `v1.0.0` Means

For `unicode_ranges`, `v1.0.0` means:

- the public API and naming are stable
- the supported integration path is explicit
- the dependency policy is explicit
- the supported toolchain matrix is explicit
- the documented behavior and error model are stable
- future `1.x` releases aim for source compatibility

It does **not** mean:

- ABI stability across all toolchains, CRTs, standard libraries, and build modes
- prebuilt binaries for every platform
- no future API additions
- perfect compile times

## Public API Surface

The stable public include entry points are:

- `unicode_ranges_borrowed.hpp`
- `unicode_ranges_all.hpp`

The repository currently also ships compatibility wrappers:

- `unicode_ranges.hpp`
- `unicode_ranges_full.hpp`

Those wrappers are transitional and are not intended to be the primary `1.x`
umbrella names.

The stable public namespaces are:

- `unicode_ranges`
- `unicode_ranges::literals`
- `unicode_ranges::pmr`

The following are **not** public API and may change at any time, including in
minor and patch releases:

- `unicode_ranges::details`
- files under `tools/`
- files under `.github/`
- vendored third-party source under `third_party/`
- private/generated build outputs

Headers under `unicode_ranges/` are installed and may be included directly, but
the umbrella headers above are the long-term compatibility contract. File-level
layout below the umbrella headers is not a stability promise on its own.

## Supported Build And Integration Contract

The authoritative supported build and integration surface is:

- CMake
- target: `unicode_ranges::unicode_ranges`
- installed package consumption through:
  - `find_package(unicode_ranges CONFIG REQUIRED)`

The repository also ships Visual Studio project files for Windows convenience
and maintainer workflow. We intend to keep them working, but when there is a
conflict between those project files and the first-party CMake build, the CMake
build is authoritative.

For `1.x`, the following are part of the supported integration contract:

- building the library from source
- linking the compiled `unicode_ranges` library target
- consuming the installed/exported CMake package

The following are not part of the `1.x` release contract today:

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
API contract. It may change in minor or patch releases as long as:

- licensing remains compatible with the documented project policy
- supported public behavior is preserved
- the documented integration story remains accurate

## Supported Toolchains

The minimum toolchains currently exercised in CI are:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

We may add support for newer toolchains in minor releases.

Dropping an already-supported toolchain should normally require a major release,
unless continued support becomes impossible because of an upstream dependency,
security issue, or permanently broken toolchain behavior outside this project's
control.

## Compatibility Rules For `1.x`

Within `1.x`, the project aims to preserve source compatibility for:

- documented public types
- documented public functions and overloads
- documented public namespaces
- the primary CMake target and package name

Within `1.x`, the project may still:

- add new APIs
- add new overloads
- add new documentation pages and examples
- improve performance
- restructure internal implementation
- change private file layout
- update vendored dependency revisions

Breaking changes to documented public API should require a major release.

The main compatibility promise for `1.x` is **source compatibility**, not ABI
compatibility.

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

SemVer policy for Unicode data updates:

- usually `minor`
- `major` only if the update forces a documented breaking API or support-policy change

## Versioning Policy

The project follows SemVer in this sense:

- `major`
  - breaking public API changes
  - breaking supported integration/build contract changes
  - major support-policy changes
- `minor`
  - additive API/features
  - Unicode data updates
  - new supported toolchains
  - vendored dependency updates with no intended breaking public API impact
- `patch`
  - bug fixes
  - documentation fixes
  - build fixes
  - performance improvements
  - dependency fixes with no intended feature-level or compatibility impact

## Release Artifacts

For the current release model, the official release artifacts are:

- a tagged source release
- GitHub source archives for that tag
- the first-party CMake install/export package produced from source

The project does not currently promise official prebuilt binaries.

## Performance And Compile-Time Expectations

Performance matters. Compile-time cost also matters.

However, neither runtime performance numbers nor compile-time measurements are a
strict SemVer contract by themselves.

The project will treat major regressions seriously, but reserves the right to:

- change internal backends
- change internal fast paths
- rebalance implementation tradeoffs

as long as the documented public API, behavior, and support contract are preserved.

## Non-Goals Of The `1.x` Contract

The following are intentionally not promised by this document:

- ABI stability across compilers and standard libraries
- binary compatibility between all build configurations
- support for every possible build system
- support for private/internal headers
- support for `unicode_ranges::details`
- permanent stability of internal implementation choices

## Release Gate For `v1.0.0`

The repository is ready for `v1.0.0` when:

- this contract is accepted
- docs and README reflect the same support story
- the release path is validated from a fresh clone:
  - configure
  - build
  - test
  - install
  - consumer `find_package(...)` smoke test
- the compile-time cost of the common include surfaces is measured and accepted
- the project version, changelog, and release notes are prepared for tagging
