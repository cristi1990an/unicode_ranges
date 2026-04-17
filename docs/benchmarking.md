# Benchmarking

This page defines how `unicode_ranges` will be benchmarked against other libraries.

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

### Separate raw and convenience paths

Whenever possible, a benchmark family should have two tracks:

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

Comparison baselines should come from the primary project, not from wrappers or secondary bindings, unless the wrapper is the de facto C++ interface being compared.

## Candidate Libraries

No single library overlaps the full `unicode_ranges` surface. Comparisons are therefore feature-family-specific.

| Library | Best comparison families | Notes |
| --- | --- | --- |
| [simdutf](https://github.com/simdutf/simdutf) | UTF validation, UTF transcoding | strongest raw UTF codec baseline; not a normalization or segmentation library |
| [ICU](https://unicode-org.github.io/icu/userguide/) | normalization, case mapping, segmentation, legacy encoding conversion | broadest feature overlap; use converter APIs for boundary encodings |
| [Boost.Text](https://tzlaine.github.io/text/doc/html/index.html) | transcoding, normalization, segmentation, case mapping | broad algorithm overlap in modern C++ |
| [uni-algo](https://github.com/uni-algo/uni-algo) | conversion, normalization, case mapping, segmentation | strong safe-Unicode algorithm baseline |
| [utf8proc](https://github.com/JuliaStrings/utf8proc) | UTF-8 normalization, case folding | useful narrow baseline for UTF-8-only Unicode algorithms |
| [utfcpp](https://github.com/nemtrif/utfcpp) | UTF-8 validation, iteration, UTF conversion | useful UTF-only C++ baseline |
| [libiconv](https://www.gnu.org/s/libiconv/) | legacy encoding conversion | important baseline once non-UTF boundary encodings expand |

## Benchmark Families

The suite should be organized by feature family, not by library.

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

Initial built-in rows should include:

- `ascii_strict`
- `ascii_lossy`
- `iso_8859_1`
- `iso_8859_15`
- `windows_1251`
- `windows_1252`

Future rows should include:

- `shift_jis`

## Corpus Policy

Synthetic microbenchmarks are useful, but not enough. Each family should use multiple corpora.

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

## Planned Implementation Phases

### Phase 1

- benchmark charter and reporting policy
- benchmark project layout
- corpus layout
- toolchain matrix in CI

### Phase 2

- UTF validation and UTF transcoding comparisons
- initial baselines: `simdutf`, `utfcpp`, `uni-algo`

### Phase 3

- normalization and case mapping comparisons
- initial baselines: `ICU`, `Boost.Text`, `uni-algo`, `utf8proc`

### Phase 4

- grapheme and word segmentation comparisons
- initial baselines: `ICU`, `Boost.Text`, `uni-algo`

### Phase 5

- boundary encoding comparisons
- initial baselines: `ICU` converters and `libiconv`
- start with currently built-in single-byte codecs
- extend to `shift_jis` after native support lands

## Current Status

This page is the design charter only.

It does not yet imply:

- vendored third-party dependencies
- benchmark integration code
- CI result publishing
- cross-library benchmark numbers

Those will be added in later phases on this branch.
