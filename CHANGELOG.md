# Changelog

All notable changes to this project will be documented in this file.

The format is based loosely on Keep a Changelog.

## [1.0.0] - 2026-04-23

### Added

- first-class UTF-16 and UTF-32 character, view, and owning-string types, including literals, lossy views, reversed views, conversions, casing, normalization, and documentation/examples alongside the existing UTF-8 surface
- Unicode scalar property queries on `utf8_char`, `utf16_char`, and `utf32_char`, including general category, script, bidi class, segmentation properties, and emoji-related predicates
- optional ICU-backed locale-aware casing APIs, locale tokens, locale-aware case-insensitive comparisons, and `is_available_locale(...)`
- allocation-free case-insensitive comparison helpers: `eq_ignore_case(...)`, `starts_with_ignore_case(...)`, `ends_with_ignore_case(...)`, and `compare_ignore_case(...)`
- curated `unicode_ranges::characters::utf8`, `unicode_ranges::characters::utf16`, and `unicode_ranges::characters::utf32` convenience constants
- grapheme-reversal APIs and broader UTF-8 owning-string mutation support (`assign`, `insert`, `erase`, `replace`, `replace_with_range`, `operator+`, and allocator accessors)
- first-party CMake build/install/export support, a release smoke consumer, and compile-time measurement probes in CI
- non-blocking MSVC `/analyze` static-analysis CI coverage

### Changed

- `unicode_ranges` is now a compiled library with a first-party `unicode_ranges::unicode_ranges` target instead of a “compile `unicode_ranges.cpp` in every consumer” model
- runtime UTF-8 validation and runtime UTF-8 <-> UTF-16/UTF-32 transcoding now use pinned vendored `simdutf` `v7.7.0`
- the supported umbrella headers are now:
  - `unicode_ranges_borrowed.hpp` for the lighter borrowed/core surface
  - `unicode_ranges_all.hpp` for the all-in surface, including owning strings and `unicode_ranges::characters`
  - `unicode_ranges.hpp` and `unicode_ranges_full.hpp` remain as compatibility wrappers during the rename
- the repository now includes a stability contract in `STABILITY.md`, explicit release validation in CI, and a Linux ARM64 correctness lane
- the generated Unicode tables are split between constexpr-facing data and compiled runtime accelerators
- more runtime-only implementation moved out of headers into compiled translation units, including simdutf runtime glue, Unicode table accelerators, ICU runtime casing helpers, locale-aware comparison helpers, and common owning-string instantiations
- unchecked construction/access APIs now mirror checked preconditions with debug-only assertions
- the repository license and third-party notices are now documented consistently as `MIT OR Apache-2.0`

### Fixed

- GCC constexpr regressions introduced during the Unicode table split
- missing runtime-table objects in coverage/docs/manual archive builds after adding `unicode_tables_runtime.cpp`
- missing explicit instantiations for compiled owning-string helper templates on GCC
- release-validation smoke-test literal parsing on GCC
- compile-time measurement instability by switching to multi-round, rotated-order median reporting

### Documentation

- updated the README and docs site for the compiled-library model, vendored `simdutf` backend, optional ICU support, and current `1.x` stability story
- expanded install/integration guidance, onboarding, terminology, common tasks, and the reference pages for UTF-16/UTF-32, locale-aware casing, case-insensitive comparisons, `characters::*`, and scalar property queries
- clarified the supported umbrella headers and the lighter-vs-all include split
