# Benchmarking

This page defines the comparative benchmark policy for `unicode_ranges`: what is measured, which semantics must match, which baselines are useful, and how results are interpreted.

## Runtime Backend Context

`unicode_ranges` uses `simdutf` as its production runtime backend for hot UTF validation, UTF-8/UTF-16/UTF-32 transcoding, selected ASCII checks, and UTF-8/UTF-16 character-count paths.

For those families, raw `simdutf` rows measure the overhead and ergonomics of the `unicode_ranges` integration layer: API shape, allocation behavior, error mapping, and fallback decisions. They are not a comparison between independent low-level codec implementations.

The benchmark suite is intended to answer a narrow question:

- for a specific Unicode task, with clearly defined semantics, how does `unicode_ranges` compare to the strongest available implementation on each major C++ toolchain?

It is not intended to produce a single marketing number or an "overall winner".

## Goals

- compare `unicode_ranges` against strong existing libraries where the feature overlap is real
- keep every benchmark as close to a semantic 1:1 comparison as possible
- separate algorithm cost from container/allocation cost
- publish results per toolchain, not as one merged score
- keep the suite reproducible enough that regressions are actionable

## Non-goals

- no aggregate "fastest Unicode library" claim
- no comparison rows where the libraries do meaningfully different work
- no hidden switching between strict failure and replacement behavior
- no mixing of lazy view creation with owned materialization in the same benchmark row
- no toolchain-specific tuning that invalidates cross-compiler comparisons

## Comparison Rules

These rules are mandatory. If a candidate library cannot match the row semantics, that row is skipped for that library.

### Match semantics first

The benchmark target is "same contract", not "same-looking API call".

Examples:

- strict validation and replacement-on-error are different benchmarks
- bounded output and growable append are different benchmarks
- owning-result normalization and lazy normalization view are different benchmarks
- default grapheme segmentation and locale-tailored segmentation are different benchmarks

### Prefer the closest realistic public API

Rows use the closest documented public API that a competent user would actually choose for the task.

That means:

- do not reject a comparison just because another library only has a near-match
  with slightly different edge-case behavior outside the benchmarked corpus
- do not compare against a fundamentally different API shape when that shape
  clearly bakes in a performance advantage unrelated to the benchmark goal
- do not use obscure internal hooks or unnatural setup code that ordinary users
  would not write

When exact equivalence is impossible, the row documents the remaining difference and uses the most defensible public approximation.

### Separate raw and convenience paths

Benchmark families use two tracks when both are meaningful:

- raw or caller-provided output
- convenience or owned-result API

That keeps container growth and allocation policy from being confused with the core algorithm cost.

### Keep error handling explicit

Every benchmark row must state which of these semantics it uses:

- strict failure
- replacement
- skip or ignore

Rows with different error behavior are not combined.

### Report per toolchain

Results are reported separately for:

- GCC + libstdc++
- Clang + libc++
- MSVC + MSVC STL

No averages across toolchains. A trend is only considered strong if it appears on at least two toolchains.

### Prefer official or primary implementations

Comparison baselines come from the primary project, not from wrappers or secondary bindings, unless the wrapper is the de facto C++ interface being compared.

## Candidate Libraries

No single library overlaps the full `unicode_ranges` surface. Comparisons are therefore feature-family-specific.

| Library | Best comparison families | Notes |
| --- | --- | --- |
| [simdutf](https://github.com/simdutf/simdutf) | UTF validation, UTF transcoding | strongest raw UTF codec baseline; also the `unicode_ranges` runtime backend for those hot paths |
| [ICU](https://unicode-org.github.io/icu/userguide/) | normalization, case mapping, segmentation, legacy encoding conversion | broadest feature overlap; use converter APIs for boundary encodings |
| [Boost.Text](https://tzlaine.github.io/text/doc/html/index.html) | transcoding, normalization, segmentation, case mapping | broad algorithm overlap in modern C++ |
| [uni-algo](https://github.com/uni-algo/uni-algo) | conversion, normalization, case mapping, segmentation | strong safe-Unicode algorithm baseline; strict conversion and validation APIs are public in `conv.h` |
| [utf8proc](https://github.com/JuliaStrings/utf8proc) | UTF-8 normalization, case folding | useful narrow baseline for UTF-8-only Unicode algorithms |
| [utfcpp](https://github.com/nemtrif/utfcpp) | UTF-8 validation, iteration, UTF conversion | useful UTF-only C++ baseline |
| [libiconv](https://www.gnu.org/s/libiconv/) | legacy encoding conversion | important baseline once non-UTF boundary encodings expand |

## Benchmark Families

The suite is organized by feature family, not by library.

### UTF Validation

Semantics:

- strict validation
- valid input rows
- invalid input rows with explicit failure

Primary comparisons:

- `unicode_ranges`
- `simdutf`
- `Boost.Text`
- `uni-algo`
- `utfcpp`

Interpretation note:

- `unicode_ranges` rows in this family are wrapper/integration comparisons against raw `simdutf` usage, not independent codec-algorithm competitions

### UTF Transcoding

Semantics:

- strict, validating conversion
- same source encoding and target encoding for every row
- separate owned-result and caller-buffer rows where possible

Primary comparisons:

- `unicode_ranges`
- `simdutf`
- `Boost.Text`
- `uni-algo`
- `utfcpp`

Interpretation note:

- `unicode_ranges` rows in this family are wrapper/integration comparisons against raw `simdutf` usage for the same reason as UTF validation

### Normalization

Semantics:

- exact normalization form per row: NFC, NFD, NFKC, NFKD
- owned materialization rows separate from any lazy/pipeline rows

Primary comparisons:

- `unicode_ranges`
- `ICU`
- `Boost.Text`
- `uni-algo`
- `utf8proc`

### Case Mapping and Case Folding

Semantics:

- ASCII-only rows and full Unicode rows kept separate
- lowercasing, uppercasing, and case folding kept separate
- locale-independent rows only, unless a row is explicitly about locale-sensitive behavior

Primary comparisons:

- `unicode_ranges`
- `ICU`
- `Boost.Text`
- `uni-algo`
- `utf8proc` for case folding and UTF-8 mapping rows

### Grapheme and Word Segmentation

Semantics:

- default Unicode segmentation only
- counting rows separate from materialization or iteration rows

Primary comparisons:

- `unicode_ranges`
- `ICU`
- `Boost.Text`
- `uni-algo`

### Boundary Encodings

Semantics:

- same source and target encoding pair per row
- strict failure rows separate from replacement rows
- bounded sink rows separate from growable output rows

Primary comparisons:

- `unicode_ranges`
- `ICU` converter APIs
- `libiconv`

Built-in rows include:

- `ascii_strict`
- `ascii_lossy`
- `iso_8859_1`
- `iso_8859_15`
- `windows_1251`
- `windows_1252`

Extended rows include:

- `shift_jis`

## Corpus Policy

Synthetic microbenchmarks are useful, but not enough. Each family uses multiple corpora.

Minimum corpus set:

- ASCII-heavy text
- mixed Western European UTF text
- combining-mark-heavy text
- emoji-heavy text
- Cyrillic or other non-Latin script text
- malformed UTF for strict-validation and replacement rows
- medium-sized payloads
- large payloads

Each corpus must be shared across libraries for that row.

## Measurement Policy

- use the same benchmark harness shape across all rows
- keep warm-up and sample policy explicit
- report `ns/op`, throughput, and iteration count
- report allocation-sensitive rows separately when allocation is part of the benchmarked contract
- never hide failed rows; if a library cannot express the required semantics, mark the row unsupported
- unsupported rows appear in the suite output with a short reason instead of silently disappearing

## Result Interpretation

When discussing results:

- compare within one benchmark family first
- compare within one toolchain first
- call out cases where destination/container choice dominates the result
- avoid broad conclusions from a single compiler or one noisy runner

This matters especially for:

- ranges-heavy code
- iterator-heavy code
- growable container output paths
- standard-library-dependent behavior

## Suite Layout

The comparative benchmark implementation lives under:

- `tools/comparative_benchmarks/main.cpp`
- `tools/comparative_benchmarks/`
- `tools/comparative_benchmarks/dependencies.json`
- `tools/fetch_comparative_dependency.ps1`

The dependency manifest pins external baselines used by the comparative suite. Unsupported rows remain visible in the output with a reason, so missing feature overlap does not silently disappear from the report.

## Dependency Model

`simdutf` is both a vendored runtime dependency and a comparative baseline:

- the runtime backend uses the vendored copy under `third_party/simdutf`
- standalone `simdutf` benchmark rows exercise raw public `simdutf` API usage

Other comparative baselines are fetched for the benchmark suite and are not runtime dependencies of `unicode_ranges`.

## Interpreting Backend-Shared Rows

UTF validation, UTF transcoding, selected ASCII checks, and UTF-8/UTF-16 character counting share a runtime backend with `simdutf`. Read those rows as:

- wrapper overhead comparisons
- API-shape and allocation-model comparisons
- error-mapping and fallback-policy comparisons

Do not read them as unrelated low-level algorithm competitions.
