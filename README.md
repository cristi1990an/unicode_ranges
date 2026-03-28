# unicode_ranges (work in progress)

`unicode_ranges` is a header-only C++ library for representing, validating, iterating, and formatting UTF-8 and UTF-16 text.

It provides validated UTF-8 and UTF-16 characters, borrowed views, owning strings, and range abstractions for modern C++.

## At a glance

| Category | UTF-8 | UTF-16 |
| --- | --- | --- |
| Character | `utf8_char` | `utf16_char` |
| Borrowed text | `utf8_string_view` | `utf16_string_view` |
| Owning text | `utf8_string` | `utf16_string` |
| Forward scalar iteration | `views::utf8_view` | `views::utf16_view` |
| Reverse scalar iteration | `views::reversed_utf8_view` | `views::reversed_utf16_view` |
| Grapheme iteration | `views::grapheme_cluster_view<char8_t>` | `views::grapheme_cluster_view<char16_t>` |
| Lossy iteration | `views::lossy_utf8_view<CharT>` | `views::lossy_utf16_view<CharT>` |

> [!TIP]
> Public entry point:
> ```cpp
> #include "unicode_ranges.hpp"
> ```
> All public library types and functions live in namespace `unicode_ranges`.
> Literal operators live in `unicode_ranges::literals`.
> PMR owning-string aliases live in `unicode_ranges::pmr`.

> [!WARNING]
> `unicode_ranges::details` contains implementation details only. It is not part of the supported public API.

> [!IMPORTANT]
> Borrowed APIs return views or ranges into existing storage.
> `chars()`, `reversed_chars()`, `graphemes()`, `char_indices()`, and `grapheme_indices()` do not own their data.
> Do not let them outlive the underlying string storage, and do not keep them across mutations of an owning string.

## Contents

1. [Goals](#goals)
2. [Requirements](#requirements)
3. [Unicode version](#unicode-version)
4. [Quick start](#quick-start)
5. [Error model](#error-model)
6. [Reference: utf8_char](#reference-utf8_char)
7. [Reference: utf16_char](#reference-utf16_char)
8. [Reference: utf8_string_view](#reference-utf8_string_view)
9. [Reference: utf16_string_view](#reference-utf16_string_view)
10. [Reference: utf16_string](#reference-utf16_string)
11. [Reference: utf8_string](#reference-utf8_string)
12. [Reference: views](#reference-views)
13. [Reference: literals](#reference-literals)
14. [Reference: formatting, streaming, hashing](#reference-formatting-streaming-hashing)
15. [Semantics notes](#semantics-notes)

## Goals

This library is built around a few explicit design choices:

- Unicode scalar values are the canonical model, with UTF-8 and UTF-16 as first-class encodings.
- No sacrifices to performance while offering a safe, modern, and versatile API.
- The library is designed to remain `constexpr`-friendly where practical.
- Unsafe APIs are explicitly marked as `unchecked`.
- String-like APIs lean toward C++ STL-style naming and behavior while borrowing design choices from Rust.
- Classification APIs expose both ASCII-only and Unicode-aware predicates.

In particular, Unicode literals and many core operations are intended to remain usable in constant evaluation.

> [!NOTE]
> Default Unicode grapheme segmentation is supported through `graphemes()`, while locale-specific tailoring and higher-level text layout remain out of scope.

## Requirements

This library requires a compiler and standard library with strong C++23 support.

The implementation relies on language and library facilities including:

- `char8_t`
- `std::expected`
- `std::format`
- ranges and views
- `std::ranges::range_adaptor_closure`

The following versions are the minimum toolchains currently tested in CI:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: the `ClangCL` toolset shipped with current Visual Studio 2022 builds
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with `libc++`: Clang 22 / `libc++` 22 or newer

These are minimum tested versions, not guaranteed absolute minimum versions. Older toolchains may work, but are not currently part of the test matrix.

> [!TIP]
> CI also includes a dedicated Linux/Clang coverage job that publishes function and branch coverage reports as an artifact.
> For local Linux or WSL runs, use:
> ```bash
> bash tools/generate_coverage.sh
> ```

## Unicode version

Unicode property predicates are versioned. The library exposes:

```cpp
inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version;
```

This constant aliases the generated Unicode version used by the Unicode property tables.

Those tables are generated from versioned Unicode data files. In particular, the grapheme-segmentation data is sourced from the official Unicode Character Database rather than inferred from ad-hoc predicates.

> [!IMPORTANT]
> The checked-in generated tables and the current CI matrix track Unicode `17.0.0`.

The update workflow is:

1. refresh the raw Unicode data under `tools/unicode_data/<version>/`
2. rerun `tools/regenerate_unicode_tables.ps1`
3. commit the regenerated `unicode_ranges/unicode_tables.hpp`
4. update the changelog and any affected documentation

Example:

```cpp
static_assert(std::get<0>(unicode_ranges::unicode_version) == 17);
static_assert(std::get<1>(unicode_ranges::unicode_version) == 0);
static_assert(std::get<2>(unicode_ranges::unicode_version) == 0);
```

## Quick start

> [!TIP]
> Choose the entry point by ownership:
> use `_utf8_sv` / `_utf16_sv` for validated compile-time views,
> `utf8_string::from_bytes(...)` / `utf16_string::from_bytes(...)` for runtime validation of raw input,
> and `_utf8_s` / `_utf16_s` when you want an owning validated string immediately.

Suppose you have UTF-8 text such as `café €`, and you want to:

- validate that it is well-formed UTF-8
- count characters rather than raw bytes
- access the first and last character correctly

```cpp
#include "unicode_ranges.hpp"

#include <algorithm>
#include <array>
#include <string>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

// Compile time validation of UTF-8 string literals.
constexpr auto text = "café €"_utf8_sv;

// Cheap interoperability with the underlying char8_t view.
static_assert(text.as_view() == u8"café €");

// `size()` counts UTF-8 code units.
static_assert(text.size() == 9);
static_assert(text.as_view() == std::u8string{ 0x63, 0x61, 0x66, 0xC3, 0xA9, 0x20, 0xE2, 0x82, 0xAC });

// `char_count()` counts Unicode scalar values.
static_assert(text.char_count() == 6);
static_assert(std::ranges::equal(text.chars(), std::array{ "c"_u8c, "a"_u8c, "f"_u8c, "é"_u8c, " "_u8c, "€"_u8c }));

// STL-style APIs.
static_assert(text.front().value() == 'c');
static_assert(text.back().value() == "€"_u8c);

// `find()` returns a byte offset.
static_assert(text.find("€"_u8c) == 6);

```

The library is also `constexpr`-friendly. UTF-8 literals and many operations may be evaluated at compile time:

```cpp
#include "unicode_ranges.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr utf8_char euro = "€"_u8c;
constexpr auto text = "Aé€"_utf8_sv;

static_assert(euro.as_scalar() == 0x20AC);
static_assert(text.size() == 6);
static_assert(text.char_count() == 3);
```

Another literal example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr utf8_char euro = "€"_u8c;
constexpr utf8_string_view text = "Aé€"_utf8_sv;

static_assert(euro.code_unit_count() == 3);
static_assert(text.size() == 6);
```

When your source text arrives at runtime as raw bytes, use the owning checked factory:

```cpp
#include "unicode_ranges.hpp"

#include <fstream>
#include <iterator>
#include <print>
#include <string>

using namespace unicode_ranges;

int main()
{
    std::ifstream input("notes.txt", std::ios::binary);
    std::string raw(
        std::istreambuf_iterator<char>{ input },
        std::istreambuf_iterator<char>{});

    auto text = utf8_string::from_bytes(raw);
    if (!text)
    {
        std::println(stderr,
                     "Invalid UTF-8 at byte {}",
                     text.error().first_invalid_byte_index);
        return 1;
    }

    std::println("Read {} Unicode scalar values", text->char_count());
    std::println("First character: {}", text->front().value());
    std::println("As UTF-16: {}", text->to_utf16());
}
```

`from_bytes(std::string_view)` validates the incoming bytes once and returns a validated owning `utf8_string`. The same pattern is available for `utf16_string::from_bytes(...)`, and both owning factories also accept an optional allocator.

Printing and formatting are also supported for the library UTF-8 string types:

```cpp
#include "unicode_ranges.hpp"

#include <cassert>
#include <format>
#include <sstream>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

const utf8_string text = "café €"_utf8_s;

assert(std::format("{}", text) == "café €");

std::ostringstream oss;
oss << text;
assert(oss.str() == "café €");
```

This is intentionally different from `std::u8string`. Standard library types cannot be extended with custom `std::formatter` specializations or stream insertion overloads by user code. These UTF-8 types are library-defined, so providing formatting and `operator<<` support for them is straightforward and well-formed.

Grapheme segmentation is available on the UTF-8 and UTF-16 string-view/string APIs:

```cpp
#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
    constexpr auto sv = u8"👩‍💻!"_utf8_sv;

    std::println("{}", sv);                // 👩‍💻!
    std::println("{:m}", sv.chars());      // {0: 👩, 1: ‍, 2: 💻, 3: !}
    std::println("{::s}", sv.graphemes()); // [👩‍💻, !]
}
```

`chars()` iterates Unicode scalar values, while `graphemes()` iterates user-perceived characters using the default Unicode grapheme-cluster rules.

> [!NOTE]
> `chars()` and `graphemes()` are borrowing range views. Keep them only while the underlying text storage is still alive and unchanged.
> For formatted output, `{:m}` is useful for scalar indices and `{::s}` is useful for printing grapheme ranges as plain text slices.

## Error model

UTF-8 validation reports:

```cpp
enum class utf8_error_code
{
    invalid_lead_byte,
    truncated_sequence,
    invalid_sequence
};

struct utf8_error
{
    utf8_error_code code{};
    std::size_t first_invalid_byte_index = 0;
};
```

These are returned by checked construction APIs such as `utf8_string_view::from_bytes`.

Example:

```cpp
using namespace unicode_ranges;

const std::array<char8_t, 3> invalid_bytes{
    static_cast<char8_t>(0xE2),
    static_cast<char8_t>(0x28),
    static_cast<char8_t>(0xA1)
};

// Construction of `utf8_string_view` from arbitrary bytes performs runtime validation.
std::expected<utf8_string_view, utf8_error> result = utf8_string_view::from_bytes(
    { invalid_bytes.data(), invalid_bytes.size() });

assert(!result.has_value());
assert(result.error().code == utf8_error_code::invalid_sequence);
assert(result.error().first_invalid_byte_index == 0);
```

UTF-16 validation reports:

```cpp
enum class utf16_error_code
{
    truncated_surrogate_pair,
    invalid_sequence
};

struct utf16_error
{
    utf16_error_code code{};
    std::size_t first_invalid_code_unit_index = 0;
};
```

These are returned by checked construction APIs such as `utf16_string_view::from_code_units`.

Example:

```cpp
using namespace unicode_ranges;

const std::array<char16_t, 1> invalid_code_units{
    static_cast<char16_t>(0xD800u)
};

std::expected<utf16_string_view, utf16_error> result = utf16_string_view::from_code_units(
    { invalid_code_units.data(), invalid_code_units.size() });

assert(!result.has_value());
assert(result.error().code == utf16_error_code::truncated_surrogate_pair);
assert(result.error().first_invalid_code_unit_index == 0);
```

## Reference: utf8_char

### Overview

`utf8_char` is a small value type that stores one Unicode scalar value as UTF-8 encoded bytes.

Compared to Rust's `char`, `utf8_char` has a different layout and storage model:

- Rust's `char` is a decoded Unicode scalar value, conceptually closer to a 32-bit scalar/code point type.
- `utf8_char` stores the UTF-8 encoded form directly, not the decoded scalar as its primary representation.
- `utf8_char` is designed so that its object representation is basically the same layout as four `char8_t` values, i.e. equivalent in layout terms to a fixed 4-byte UTF-8 buffer.

This means `utf8_char` is not a drop-in analogue of Rust's `char`. The two types represent the same abstract kind of value, but Rust optimizes for decoded scalar semantics while `utf8_char` optimizes for UTF-8-native storage and byte-oriented interoperability.

That layout choice has practical consequences:

- conversion to the equivalent `utf8_string_view` for a single character is cheap
- printing and formatting can operate directly on the stored UTF-8 bytes
- appending or inserting a `utf8_char` into an owning UTF-8 buffer is cheap because the encoded bytes are already present in the object
- interoperability with UTF-8-oriented APIs is simpler because no scalar-to-UTF-8 re-encoding step is needed just to expose the character as bytes

It supports:

- checked and unchecked scalar construction
- unchecked UTF-8 byte construction
- conversion back to scalar value
- byte-count queries and UTF-8 emission
- direct increment and decrement across Unicode scalar values
- ASCII-only and Unicode-aware classification
- ASCII-only transforms
- comparison, formatting, streaming, hashing

### Synopsis

```cpp
struct utf8_char
{
    utf8_char() = default;

    static const utf8_char replacement_character;
    static const utf8_char null_terminator;

    static constexpr std::optional<utf8_char> from_scalar(std::uint32_t scalar) noexcept;
    static constexpr utf8_char from_scalar_unchecked(std::uint32_t scalar) noexcept;

    template<class CharT>
    static constexpr utf8_char from_utf8_bytes_unchecked(
        const CharT* bytes, std::size_t size) noexcept;

    constexpr std::uint32_t as_scalar() const noexcept;
    constexpr utf8_char& operator++() noexcept;
    constexpr utf8_char operator++(int) noexcept;
    constexpr utf8_char& operator--() noexcept;
    constexpr utf8_char operator--(int) noexcept;

    constexpr bool is_ascii() const noexcept;
    constexpr bool is_alphabetic() const noexcept;
    constexpr bool is_alphanumeric() const noexcept;
    constexpr bool is_ascii_alphabetic() const noexcept;
    constexpr bool is_ascii_alphanumeric() const noexcept;
    constexpr bool is_ascii_control() const noexcept;
    constexpr bool is_ascii_digit() const noexcept;
    constexpr bool is_ascii_graphic() const noexcept;
    constexpr bool is_ascii_hexdigit() const noexcept;
    constexpr bool is_ascii_lowercase() const noexcept;
    constexpr bool is_ascii_octdigit() const noexcept;
    constexpr bool is_ascii_punctuation() const noexcept;
    constexpr bool is_ascii_uppercase() const noexcept;
    constexpr bool is_ascii_whitespace() const noexcept;
    constexpr bool is_control() const noexcept;
    constexpr bool is_digit() const noexcept;
    constexpr bool is_lowercase() const noexcept;
    constexpr bool is_numeric() const noexcept;
    constexpr bool is_uppercase() const noexcept;
    constexpr bool is_whitespace() const noexcept;

    constexpr utf8_char ascii_lowercase() const noexcept;
    constexpr utf8_char ascii_uppercase() const noexcept;
    constexpr bool eq_ignore_ascii_case(utf8_char other) const noexcept;
    constexpr void swap(utf8_char& other) noexcept;

    constexpr std::size_t code_unit_count() const noexcept;

    template<class CharT, class OutIt>
    constexpr std::size_t encode_utf8(OutIt out) const noexcept;

    template<class CharT, class OutIt>
    constexpr std::size_t encode_utf16(OutIt out) const noexcept;
};
```

### Constants

#### `replacement_character`

The replacement scalar `U+FFFD`.

UTF-8 encoding:

```text
EF BF BD
```

#### `null_terminator`

The null scalar `U+0000`.

### Construction

#### `from_scalar`

```cpp
static constexpr std::optional<utf8_char> from_scalar(std::uint32_t scalar) noexcept;
```

Constructs from a Unicode scalar value.

If `scalar` is not a valid Unicode scalar value, the result is `std::nullopt`.

Example:

```cpp
static_assert(utf8_char::from_scalar(0x20ACu).has_value());
static_assert(!utf8_char::from_scalar(0x110000u).has_value());
```

For basic character conversions such as ASCII, pass the code point value:

```cpp
constexpr auto a = utf8_char::from_scalar('A');
static_assert(a.has_value());
static_assert(a->as_scalar() == 0x41);
```

For compile-time construction from UTF-8 source text, prefer the `_u8c` literal:

```cpp
constexpr utf8_char a = "A"_u8c;
constexpr utf8_char euro = "€"_u8c;
```

#### `from_scalar_unchecked`

```cpp
static constexpr utf8_char from_scalar_unchecked(std::uint32_t scalar) noexcept;
```

Constructs from a scalar value without validation.

Precondition:

- `scalar` is a valid Unicode scalar value

> [!CAUTION]
> This is an unchecked constructor. Passing a non-scalar value is undefined behavior.

#### `from_utf8_bytes_unchecked`

```cpp
template<class CharT>
static constexpr utf8_char from_utf8_bytes_unchecked(
    const CharT* bytes, std::size_t size) noexcept;
```

Constructs by copying an already valid 1-byte, 2-byte, 3-byte, or 4-byte UTF-8 sequence.

Precondition:

- `bytes[0, size)` represents exactly one valid UTF-8 encoded Unicode scalar value

> [!CAUTION]
> This is an unchecked constructor. Passing bytes that do not encode exactly one valid UTF-8 scalar value is undefined behavior.

### Conversion and observation

#### `as_scalar`

```cpp
constexpr std::uint32_t as_scalar() const noexcept;
```

Returns the Unicode scalar value represented by this character.

#### `code_unit_count`

```cpp
constexpr std::size_t code_unit_count() const noexcept;
```

Returns the length of the encoded UTF-8 sequence in code units, i.e. bytes.

Example:

```cpp
static_assert("A"_u8c.code_unit_count() == 1);
static_assert("é"_u8c.code_unit_count() == 2);
static_assert("€"_u8c.code_unit_count() == 3);
static_assert("😀"_u8c.code_unit_count() == 4);
```

### Encoding

#### `encode_utf8`

```cpp
template<class CharT, class OutIt>
constexpr std::size_t encode_utf8(OutIt out) const noexcept;
```

Copies the stored UTF-8 bytes into `out` and returns the number of written code units.

Constraints:

- `CharT` is an integral type
- `CharT` is not `bool`
- `char8_t` is convertible to `CharT`
- `OutIt` models `std::output_iterator<OutIt, CharT>`

Example:

```cpp
std::array<char, 4> buffer{};
const auto n = "€"_u8c.encode_utf8<char>(buffer.begin());
assert(std::string_view{ buffer.data(), n } == "\xE2\x82\xAC");
```

#### `encode_utf16`

```cpp
template<class CharT, class OutIt>
constexpr std::size_t encode_utf16(OutIt out) const noexcept;
```

Copies the UTF-16 encoding of the stored scalar value into `out` and returns the number of written code units.

Constraints:

- `CharT` is an integral type
- `CharT` is not `bool`
- `CharT` accepts `char16_t` without narrowing
- `OutIt` models `std::output_iterator<OutIt, CharT>`

This rejects destinations such as `char8_t` where a UTF-16 code unit would be narrowed.

Example:

```cpp
std::array<char16_t, 2> buffer{};
const auto n = "😀"_u8c.encode_utf16<char16_t>(buffer.begin());

assert(n == 2);
assert(buffer[0] == static_cast<char16_t>(0xD83D));
assert(buffer[1] == static_cast<char16_t>(0xDE00));
```

### Increment and decrement

`utf8_char` supports direct scalar stepping:

- `++` moves to the next Unicode scalar value
- `--` moves to the previous Unicode scalar value
- surrogate code points are skipped
- stepping wraps between `U+0000` and `U+10FFFF`

Examples:

```cpp
utf8_char ch = utf8_char::from_scalar_unchecked(0x7F);
++ch;
assert(ch.as_scalar() == 0x80);

ch = utf8_char::from_scalar_unchecked(0x10FFFF);
++ch;
assert(ch.as_scalar() == 0x0000);
```

### Unicode-aware classification

The following predicates use generated Unicode property tables:

- `is_alphabetic()`
- `is_alphanumeric()`
- `is_control()`
- `is_digit()`
- `is_lowercase()`
- `is_numeric()`
- `is_uppercase()`
- `is_whitespace()`

They are versioned by [`unicode_version`](#unicode-version).

Examples:

```cpp
assert("Ω"_u8c.is_alphabetic());
assert("Ω"_u8c.is_uppercase());
assert("ω"_u8c.is_lowercase());
assert("5"_u8c.is_digit());
assert("Ⅷ"_u8c.is_numeric());
assert(" "_u8c.is_whitespace());
assert(utf8_char::from_scalar_unchecked(0x0085).is_control());
```

### ASCII-only classification

The `is_ascii_*` family performs exact ASCII classification and returns `false` for non-ASCII values:

- `is_ascii()`
- `is_ascii_alphabetic()`
- `is_ascii_alphanumeric()`
- `is_ascii_control()`
- `is_ascii_digit()`
- `is_ascii_graphic()`
- `is_ascii_hexdigit()`
- `is_ascii_lowercase()`
- `is_ascii_octdigit()`
- `is_ascii_punctuation()`
- `is_ascii_uppercase()`
- `is_ascii_whitespace()`

Examples:

```cpp
assert("F"_u8c.is_ascii_hexdigit());
assert("7"_u8c.is_ascii_octdigit());
assert("!"_u8c.is_ascii_punctuation());
assert("\n"_u8c.is_ascii_control());
```

### ASCII transforms

#### `ascii_lowercase`, `ascii_uppercase`

```cpp
constexpr utf8_char ascii_lowercase() const noexcept;
constexpr utf8_char ascii_uppercase() const noexcept;
```

Apply ASCII case mapping and return a new character. Non-ASCII characters are returned unchanged.

#### `eq_ignore_ascii_case`

```cpp
constexpr bool eq_ignore_ascii_case(utf8_char other) const noexcept;
```

Compares after ASCII lowercase normalization.

#### `swap`

```cpp
constexpr void swap(utf8_char& other) noexcept;
```

Swaps two values.

## Reference: utf16_char

### Overview

`utf16_char` is the UTF-16-native peer of `utf8_char`.

It stores one Unicode scalar value as one or two UTF-16 code units and is guaranteed to represent exactly one valid UTF-16 encoded character.

Use `utf16_char` when:

- your source data is naturally UTF-16
- you want a scalar value type that preserves UTF-16-native storage
- you need to emit UTF-16 without re-deriving surrogate pairs by hand
- you want a scalar type that integrates naturally with `utf16_string_view`, `utf16_string`, and the UTF-16 view layer

Compared to `utf8_char`, the main differences are:

- the stored representation is UTF-16 code units rather than UTF-8 bytes
- both `utf8_char` and `utf16_char` expose `code_unit_count()`
- the difference is the encoded code-unit type: UTF-8 bytes versus UTF-16 code units
- construction is from UTF-16 code units rather than UTF-8 bytes
- both character types expose scalar conversion, code-unit counting, encoding, classification, and case helpers

It supports:

- checked and unchecked scalar construction
- checked and unchecked UTF-16 code-unit construction
- conversion back to scalar value
- UTF-8 and UTF-16 emission
- direct increment and decrement across Unicode scalar values
- ASCII-only and Unicode-aware classification
- ASCII-only transforms
- comparison, formatting, streaming, hashing

### Synopsis

```cpp
struct utf16_char
{
    utf16_char() = default;

    static const utf16_char replacement_character;
    static const utf16_char null_terminator;

    static constexpr std::optional<utf16_char> from_scalar(std::uint32_t scalar) noexcept;
    static constexpr utf16_char from_scalar_unchecked(std::uint32_t scalar) noexcept;

    template<class CharT>
    static constexpr std::optional<utf16_char> from_utf16_code_units(
        const CharT* code_units, std::size_t size) noexcept;

    template<class CharT>
    static constexpr utf16_char from_utf16_code_units_unchecked(
        const CharT* code_units, std::size_t size) noexcept;

    constexpr std::uint32_t as_scalar() const noexcept;
    constexpr utf16_char& operator++() noexcept;
    constexpr utf16_char operator++(int) noexcept;
    constexpr utf16_char& operator--() noexcept;
    constexpr utf16_char operator--(int) noexcept;

    constexpr bool is_ascii() const noexcept;
    constexpr bool is_alphabetic() const noexcept;
    constexpr bool is_alphanumeric() const noexcept;
    constexpr bool is_ascii_alphabetic() const noexcept;
    constexpr bool is_ascii_alphanumeric() const noexcept;
    constexpr bool is_ascii_control() const noexcept;
    constexpr bool is_ascii_digit() const noexcept;
    constexpr bool is_ascii_graphic() const noexcept;
    constexpr bool is_ascii_hexdigit() const noexcept;
    constexpr bool is_ascii_lowercase() const noexcept;
    constexpr bool is_ascii_octdigit() const noexcept;
    constexpr bool is_ascii_punctuation() const noexcept;
    constexpr bool is_ascii_uppercase() const noexcept;
    constexpr bool is_ascii_whitespace() const noexcept;
    constexpr bool is_control() const noexcept;
    constexpr bool is_digit() const noexcept;
    constexpr bool is_lowercase() const noexcept;
    constexpr bool is_numeric() const noexcept;
    constexpr bool is_uppercase() const noexcept;
    constexpr bool is_whitespace() const noexcept;

    constexpr utf16_char ascii_lowercase() const noexcept;
    constexpr utf16_char ascii_uppercase() const noexcept;
    constexpr bool eq_ignore_ascii_case(utf16_char other) const noexcept;
    constexpr void swap(utf16_char& other) noexcept;

    constexpr std::size_t code_unit_count() const noexcept;

    template<class CharT, class OutIt>
    constexpr std::size_t encode_utf16(OutIt out) const noexcept;

    template<class CharT, class OutIt>
    constexpr std::size_t encode_utf8(OutIt out) const noexcept;
};
```

### Construction

#### `from_scalar`

```cpp
static constexpr std::optional<utf16_char> from_scalar(std::uint32_t scalar) noexcept;
```

Constructs from a Unicode scalar value.

If `scalar` is not a valid Unicode scalar value, the result is `std::nullopt`.

Examples:

```cpp
static_assert(utf16_char::from_scalar(0x20ACu).has_value());
static_assert(!utf16_char::from_scalar(0x110000u).has_value());
```

#### `from_utf16_code_units`

```cpp
template<class CharT>
static constexpr std::optional<utf16_char> from_utf16_code_units(
    const CharT* code_units, std::size_t size) noexcept;
```

Constructs from one valid UTF-16 scalar encoded as one code unit or one surrogate pair.

Examples:

```cpp
static_assert(utf16_char::from_utf16_code_units(u"€", 1).has_value());
static_assert(!utf16_char::from_utf16_code_units(u"\xD800", 1).has_value());
```

For compile-time construction from UTF-16 source text, prefer the `_u16c` literal:

```cpp
constexpr utf16_char euro = u"€"_u16c;
constexpr utf16_char grinning = u"😀"_u16c;
```

### Conversion and observation

#### `as_scalar`

```cpp
constexpr std::uint32_t as_scalar() const noexcept;
```

Returns the Unicode scalar value represented by this character.

#### `code_unit_count`

```cpp
constexpr std::size_t code_unit_count() const noexcept;
```

Returns `1` for BMP scalars and `2` for supplementary scalars.

Examples:

```cpp
static_assert(u"€"_u16c.code_unit_count() == 1);
static_assert(u"😀"_u16c.code_unit_count() == 2);
```

### Encoding

#### `encode_utf16`

```cpp
template<class CharT, class OutIt>
constexpr std::size_t encode_utf16(OutIt out) const noexcept;
```

Copies the stored UTF-16 code units into `out` and returns the number of written code units.

Constraints:

- `CharT` is an integral type
- `CharT` is not `bool`
- `CharT` accepts `char16_t` without narrowing
- `OutIt` models `std::output_iterator<OutIt, CharT>`

This rejects destinations such as `char8_t` where a UTF-16 code unit would be narrowed.

#### `encode_utf8`

```cpp
template<class CharT, class OutIt>
constexpr std::size_t encode_utf8(OutIt out) const noexcept;
```

Encodes the stored scalar as UTF-8 and writes it to `out`.

Examples:

```cpp
std::array<char16_t, 2> utf16_buffer{};
assert(u"😀"_u16c.encode_utf16<char16_t>(utf16_buffer.begin()) == 2);

std::array<char, 4> utf8_buffer{};
assert(u"€"_u16c.encode_utf8<char>(utf8_buffer.begin()) == 3);
```

### Increment, decrement, and classification

`utf16_char` follows the same scalar-stepping and classification rules as `utf8_char`:

- `++` and `--` move across Unicode scalar values
- surrogate code points are skipped
- ASCII predicates are exact and cheap
- Unicode predicates use the generated Unicode property tables

Examples:

```cpp
utf16_char ch = u"A"_u16c;
++ch;
assert(ch == u"B"_u16c);

assert(u"Ω"_u16c.is_alphabetic());
assert(u"Ω"_u16c.is_uppercase());
assert(u"ω"_u16c.is_lowercase());
assert(u"A"_u16c.eq_ignore_ascii_case(u"a"_u16c));
```

## Reference: utf8_string_view

### Overview

`utf8_string_view` is a validated borrowed UTF-8 byte sequence.

It stores:

```cpp
std::u8string_view
```

### Synopsis

```cpp
class utf8_string_view
{
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    static constexpr size_type npos = static_cast<size_type>(-1);

    utf8_string_view() = default;

    static constexpr std::expected<utf8_string_view, utf8_error>
        from_bytes(std::u8string_view bytes) noexcept;
    static constexpr utf8_string_view
        from_bytes_unchecked(std::u8string_view bytes) noexcept;

    constexpr auto chars() const noexcept;
    constexpr auto reversed_chars() const noexcept;
    constexpr auto graphemes() const noexcept;
    constexpr auto char_indices() const noexcept;
    constexpr auto grapheme_indices() const noexcept;
    constexpr size_type size() const noexcept;
    constexpr bool empty() const noexcept;
    constexpr bool is_ascii() const noexcept;
    constexpr size_type char_count() const noexcept;
    constexpr size_type grapheme_count() const noexcept;
    constexpr bool contains(utf8_char ch) const noexcept;
    constexpr bool contains(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr bool contains(Pred pred) const noexcept;
    constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find(utf8_string_view sv, size_type pos = 0) const noexcept;
    template<class Pred>
    constexpr size_type find(Pred pred, size_type pos = 0) const noexcept;
    constexpr size_type find_first_of(char8_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_of(utf8_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_of(utf8_string_view sv, size_type pos = 0) const noexcept;
    constexpr size_type find_first_not_of(char8_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_not_of(utf8_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_not_of(utf8_string_view sv, size_type pos = 0) const noexcept;
    constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept;
    constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept;
    constexpr size_type rfind(utf8_string_view sv, size_type pos = npos) const noexcept;
    template<class Pred>
    constexpr size_type rfind(Pred pred, size_type pos = npos) const noexcept;
    constexpr size_type find_last_of(char8_t ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_of(utf8_char ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_of(utf8_string_view sv, size_type pos = npos) const noexcept;
    constexpr size_type find_last_not_of(char8_t ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_not_of(utf8_char ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_not_of(utf8_string_view sv, size_type pos = npos) const noexcept;
    constexpr bool is_char_boundary(size_type index) const noexcept;
    constexpr bool is_grapheme_boundary(size_type index) const noexcept;
    constexpr size_type char_count() const noexcept;
    constexpr auto split(utf8_char ch) const noexcept;
    constexpr auto split(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto split(Pred pred) const noexcept;
    constexpr auto rsplit(utf8_char ch) const noexcept;
    constexpr auto rsplit(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto rsplit(Pred pred) const noexcept;
    constexpr auto split_terminator(utf8_char ch) const noexcept;
    constexpr auto split_terminator(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto split_terminator(Pred pred) const noexcept;
    constexpr auto rsplit_terminator(utf8_char ch) const noexcept;
    constexpr auto rsplit_terminator(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto rsplit_terminator(Pred pred) const noexcept;
    constexpr auto splitn(size_type count, utf8_char ch) const noexcept;
    constexpr auto splitn(size_type count, utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto splitn(size_type count, Pred pred) const noexcept;
    constexpr auto rsplitn(size_type count, utf8_char ch) const noexcept;
    constexpr auto rsplitn(size_type count, utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto rsplitn(size_type count, Pred pred) const noexcept;
    constexpr auto split_inclusive(utf8_char ch) const noexcept;
    constexpr auto split_inclusive(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto split_inclusive(Pred pred) const noexcept;
    constexpr auto split_trimmed(utf8_char ch) const noexcept;
    constexpr auto split_trimmed(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto split_trimmed(Pred pred) const noexcept;
    constexpr auto matches(utf8_char ch) const noexcept;
    constexpr auto matches(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto matches(Pred pred) const noexcept;
    constexpr auto rmatches(utf8_char ch) const noexcept;
    constexpr auto rmatches(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto rmatches(Pred pred) const noexcept;
    constexpr auto rmatch_indices(utf8_char ch) const noexcept;
    constexpr auto rmatch_indices(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto rmatch_indices(Pred pred) const noexcept;
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        split_once(utf8_char ch) const noexcept;
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        split_once(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        split_once(Pred pred) const noexcept;
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        rsplit_once(utf8_char ch) const noexcept;
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        rsplit_once(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        rsplit_once(Pred pred) const noexcept;
    constexpr auto split_whitespace() const noexcept;
    constexpr auto split_ascii_whitespace() const noexcept;
    constexpr std::optional<utf8_string_view> strip_prefix(utf8_char ch) const noexcept;
    constexpr std::optional<utf8_string_view> strip_prefix(utf8_string_view sv) const noexcept;
    constexpr std::optional<utf8_string_view> strip_suffix(utf8_char ch) const noexcept;
    constexpr std::optional<utf8_string_view> strip_suffix(utf8_string_view sv) const noexcept;
    constexpr std::optional<utf8_string_view>
        strip_circumfix(utf8_char prefix, utf8_char suffix) const noexcept;
    constexpr std::optional<utf8_string_view>
        strip_circumfix(utf8_string_view prefix, utf8_string_view suffix) const noexcept;
    constexpr utf8_string_view trim_prefix(utf8_char ch) const noexcept;
    constexpr utf8_string_view trim_prefix(utf8_string_view sv) const noexcept;
    constexpr utf8_string_view trim_suffix(utf8_char ch) const noexcept;
    constexpr utf8_string_view trim_suffix(utf8_string_view sv) const noexcept;
    constexpr utf8_string_view trim_start_matches(utf8_char ch) const noexcept;
    constexpr utf8_string_view trim_start_matches(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr utf8_string_view trim_start_matches(Pred pred) const noexcept;
    constexpr utf8_string_view trim_end_matches(utf8_char ch) const noexcept;
    constexpr utf8_string_view trim_end_matches(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr utf8_string_view trim_end_matches(Pred pred) const noexcept;
    constexpr utf8_string_view trim_matches(utf8_char ch) const noexcept;
    constexpr utf8_string_view trim_matches(utf8_string_view sv) const noexcept;
    template<class Pred>
    constexpr utf8_string_view trim_matches(Pred pred) const noexcept;
    constexpr utf8_string_view trim_start() const noexcept;
    constexpr utf8_string_view trim_end() const noexcept;
    constexpr utf8_string_view trim() const noexcept;
    constexpr utf8_string_view trim_ascii_start() const noexcept;
    constexpr utf8_string_view trim_ascii_end() const noexcept;
    constexpr utf8_string_view trim_ascii() const noexcept;
    constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
        split_once_at(size_type delim) const noexcept;
    constexpr std::pair<utf8_string_view, utf8_string_view>
        split_once_at_unchecked(size_type delim) const noexcept;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_ascii_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_ascii_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_lowercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_uppercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        normalize(normalization_form form, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_nfc(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_nfd(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_nfkc(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_nfkd(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        case_fold(const Allocator& alloc = Allocator()) const;
    constexpr utf8_string replace_all(utf8_char from, utf8_char to) const;
    constexpr utf8_string replace_all(utf8_char from, utf8_string_view to) const;
    constexpr utf8_string replace_all(utf8_string_view from, utf8_char to) const;
    constexpr utf8_string replace_all(utf8_string_view from, utf8_string_view to) const;
    constexpr utf8_string replace_n(size_type count, utf8_char from, utf8_char to) const;
    constexpr utf8_string replace_n(size_type count, utf8_char from, utf8_string_view to) const;
    constexpr utf8_string replace_n(size_type count, utf8_string_view from, utf8_char to) const;
    constexpr utf8_string replace_n(size_type count, utf8_string_view from, utf8_string_view to) const;
    constexpr size_type ceil_char_boundary(size_type pos) const noexcept;
    constexpr size_type floor_char_boundary(size_type pos) const noexcept;
    constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept;
    constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept;
    constexpr std::optional<utf8_char> char_at(size_type index) const noexcept;
    constexpr std::optional<utf8_string_view> grapheme_at(size_type index) const noexcept;
    constexpr std::optional<utf8_string_view> substr(size_type pos, size_type count = npos) const noexcept;
    constexpr std::optional<utf8_string_view> grapheme_substr(size_type pos, size_type count = npos) const noexcept;

    constexpr auto base() const noexcept;
    constexpr std::u8string_view as_view() const noexcept;
    constexpr operator std::u8string_view() const noexcept;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_utf16(const Allocator& alloc = Allocator()) const;
};
```

### Construction

#### `utf8_string_view`

```cpp
utf8_string_view() = default;
```

Constructs an empty view.

An empty view is valid UTF-8.

#### `from_bytes`

```cpp
static constexpr std::expected<utf8_string_view, utf8_error>
    from_bytes(std::u8string_view bytes) noexcept;
```

Validates the supplied byte sequence as UTF-8 and returns
`std::expected<utf8_string_view, utf8_error>`.

On success, the `expected` contains a `utf8_string_view`.

On failure, the `expected` contains a `utf8_error`.

Complexity:

- Linear in `bytes.size()`

#### `from_bytes_unchecked`

```cpp
static constexpr utf8_string_view
    from_bytes_unchecked(std::u8string_view bytes) noexcept;
```

Constructs without validation.

Use only if the input is already known to be valid UTF-8.

Complexity:

- Constant

> [!CAUTION]
> This is an unchecked constructor. Passing invalid UTF-8 is undefined behavior.

#### `constexpr std::u8string_view as_view() const noexcept`

Returns the underlying `std::u8string_view`.

> [!WARNING]
> This view borrows from the underlying UTF-8 storage.
> It must not outlive that storage.

Complexity:

- Constant

### Read-only API

#### `chars`

```cpp
constexpr auto chars() const noexcept;
```

Returns a `unicode_ranges::views::utf8_view` over the contained Unicode scalar values.

> [!WARNING]
> The returned range borrows from the underlying UTF-8 storage.
> Do not keep it after the source storage is destroyed or after an owning source string is mutated.

Preconditions:

- None

Complexity:

- Constant to construct the view
- Linear to iterate the entire view

#### `reversed_chars`

```cpp
constexpr auto reversed_chars() const noexcept;
```

Returns a `unicode_ranges::views::reversed_utf8_view` over the contained Unicode scalar values, in reverse order.

Preconditions:

- None

Complexity:

- Constant to construct the view
- Linear to iterate the entire view

#### `graphemes`

```cpp
constexpr auto graphemes() const noexcept;
```

Returns a `unicode_ranges::views::grapheme_cluster_view<char8_t>` over the contained text.

Each element is a `utf8_string_view` representing one default Unicode grapheme cluster.

> [!WARNING]
> Like `chars()`, this is a borrowing range view over the underlying UTF-8 storage.

`find_grapheme`, `rfind_grapheme`, and `contains_grapheme` search at grapheme-cluster boundaries, unlike `find` / `rfind`, which are character-boundary-based for validated Unicode needles. `find_first_of`, `find_first_not_of`, `find_last_of`, and `find_last_not_of` follow the same rule: raw `char8_t` overloads work on bytes, while `utf8_char` and `utf8_string_view` overloads operate on validated UTF-8 characters.

Complexity:

- Constant to construct the view
- Linear to iterate the entire view
- `find_grapheme(...)`, `rfind_grapheme(...)`, and `contains_grapheme(...)`: linear in the number of grapheme clusters examined

#### `size`

```cpp
constexpr size_type size() const noexcept;
```

Returns the length of the view in UTF-8 code units, not in Unicode scalar values.

Preconditions:

- None

Complexity:

- Constant

#### `empty`

```cpp
constexpr bool empty() const noexcept;
```

Returns `true` if `size() == 0`.

Preconditions:

- None

Complexity:

- Constant

#### `is_ascii`

```cpp
constexpr bool is_ascii() const noexcept;
```

Returns `true` if every character in the view is ASCII.

Preconditions:

- None

Complexity:

- Linear in the number of characters

#### `char_indices`

```cpp
constexpr auto char_indices() const noexcept;
```

Returns a view of `(byte_offset, utf8_char)` pairs.

Each element is a pair-like value containing:

- the zero-based byte position of the character in the underlying UTF-8 buffer
- the corresponding `utf8_char`

Preconditions:

- None

Complexity:

- Constant to construct the view
- Linear to iterate the entire view

#### `grapheme_indices`

```cpp
constexpr auto grapheme_indices() const noexcept;
```

Returns a view of `(byte_offset, utf8_string_view)` pairs.

Each element contains:

- the zero-based byte position of the grapheme cluster in the underlying UTF-8 buffer
- the corresponding `utf8_string_view`

Complexity:

- Constant to construct the view
- Linear to iterate the entire view

#### `grapheme_count`

```cpp
constexpr size_type grapheme_count() const noexcept;
```

Returns the number of default Unicode grapheme clusters in the view.

Complexity:

- Linear in the number of grapheme clusters

#### `contains`

```cpp
constexpr bool contains(utf8_char ch) const noexcept;
constexpr bool contains(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr bool contains(Pred pred) const noexcept;
```

Returns `true` if the text contains:

- the exact validated UTF-8 character `ch`
- the exact validated UTF-8 substring `sv`
- any decoded `utf8_char` for which `pred` returns `true`

The predicate overload is character-based. It is invoked once per Unicode scalar value and cannot match multi-character substrings.

```cpp
using namespace unicode_ranges::literals;

const auto text = u8"banané"_utf8_sv;

assert(text.contains(u8"é"_u8c));
assert(text.contains(u8"ana"_utf8_sv));
assert(text.contains([](unicode_ranges::utf8_char ch) { return ch == u8"b"_u8c; }));
assert(!text.contains(u8"xyz"_utf8_sv));
```

Complexity:

- Linear in `size()`

#### `find`

```cpp
constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf8_string_view sv, size_type pos = 0) const noexcept;
template<class Pred>
constexpr size_type find(Pred pred, size_type pos = 0) const noexcept;
```

Returns the byte offset of the first match, or `npos` if no match exists.

The returned index is a byte offset into the underlying UTF-8 sequence.

Behavior:

- `find(char8_t, pos)` treats `pos` as a raw byte offset and searches raw bytes.
- `find(utf8_char, pos)` and `find(utf8_string_view, pos)` clamp `pos` to `size()` and round it up to the next UTF-8 character boundary before searching.
- `find(Pred, pos)` also rounds `pos` up to the next UTF-8 character boundary and returns the byte offset of the first character for which `pred` returns `true`.

```cpp
using namespace unicode_ranges::literals;

const auto text = u8"AéBé"_utf8_sv;

assert(text.find(u8'B') == 3);
assert(text.find(u8"é"_u8c) == 1);
assert(text.find(u8"éB"_utf8_sv) == 1);
assert(text.find([](unicode_ranges::utf8_char ch) { return ch == u8"B"_u8c; }) == 3);
assert(text.find(u8"z"_u8c) == unicode_ranges::utf8_string_view::npos);
```

Complexity:

- Linear in `size()` for `char8_t` and `utf8_char`
- Linear in `size() * sv.char_count()` for `utf8_string_view` in the worst case
- Linear in the number of characters examined for `Pred`

#### `find_first_of`, `find_first_not_of`

```cpp
constexpr size_type find_first_of(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_of(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_of(utf8_string_view sv, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(utf8_string_view sv, size_type pos = 0) const noexcept;
```

Searches for the first byte or validated UTF-8 character that is inside or outside the supplied set.

Complexity:

- Linear in `size()` for `char8_t` and `utf8_char`
- Linear in `size() * sv.char_count()` for `utf8_string_view` in the worst case

#### `to_utf16`

```cpp
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_utf16(const Allocator& alloc = Allocator()) const;
```

Materializes the same text as an owning UTF-16 string by transcoding through Unicode scalar values. By default this returns `utf16_string`, but you can supply a different allocator and get `basic_utf16_string<Allocator>`.

Preconditions:

- None

Complexity:

- Linear in `size()`

#### `find_last_of`, `find_last_not_of`

```cpp
constexpr size_type find_last_of(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_of(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_of(utf8_string_view sv, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(utf8_string_view sv, size_type pos = npos) const noexcept;
```

Searches backward for the last byte or validated UTF-8 character that is inside or outside the supplied set.

Complexity:

- Linear in `size()` for `char8_t` and `utf8_char`
- Linear in `size() * sv.char_count()` for `utf8_string_view` in the worst case

#### `rfind`

```cpp
constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf8_string_view sv, size_type pos = npos) const noexcept;
template<class Pred>
constexpr size_type rfind(Pred pred, size_type pos = npos) const noexcept;
```

Returns the byte offset of the last match, or `npos` if no match exists.

Behavior:

- `rfind(char8_t, pos)` treats `pos` as a raw byte offset.
- `rfind(utf8_char, pos)` and `rfind(utf8_string_view, pos)` clamp `pos` to `size()` and round it down to a UTF-8 character boundary before searching.
- `rfind(Pred, pos)` returns the start byte offset of the last character at or before `pos` for which `pred` returns `true`.

```cpp
using namespace unicode_ranges::literals;

const auto text = u8"AéBé"_utf8_sv;

assert(text.rfind(u8"é"_u8c) == 4);
assert(text.rfind(u8"é"_u8c, 3) == 1);
assert(text.rfind(u8"éB"_utf8_sv) == 1);
assert(text.rfind([](unicode_ranges::utf8_char ch) { return ch == u8"é"_u8c; }) == 4);
```

Complexity:

- Linear in `size()` for `char8_t` and `utf8_char`
- Linear in `size() * sv.char_count()` for `utf8_string_view` in the worst case
- Linear in the number of characters examined for `Pred`

#### `is_char_boundary`

```cpp
constexpr bool is_char_boundary(size_type index) const noexcept;
```

Returns `true` if `index` is a valid UTF-8 character boundary in the underlying byte sequence.

`0` and `size()` are always boundaries. Any value greater than `size()` is not a boundary.

This function operates on byte indices.

Preconditions:

- None

Complexity:

- Constant

#### `char_count`

```cpp
constexpr size_type char_count() const noexcept;
```

Returns the number of Unicode scalar values in the view.

Preconditions:

- None

Complexity:

- Linear in the number of characters

#### `split`

```cpp
constexpr auto split(utf8_char ch) const noexcept;
constexpr auto split(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto split(Pred pred) const noexcept;
```

Returns a lazy view of the segments separated by `ch`, `sv`, or matching characters selected by `pred`.

Behavior:

- exact-delimiter overloads split on non-overlapping matches of `ch` or `sv`
- the predicate overload treats every matching `utf8_char` as a delimiter
- delimiters are not included in the yielded segments
- if the exact delimiter is not found, the whole string is yielded once

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"red,green,blue"_utf8_sv.split(u8","_u8c),
    std::array{ u8"red"_utf8_sv, u8"green"_utf8_sv, u8"blue"_utf8_sv }));

assert(std::ranges::equal(
    u8"a b\tc"_utf8_sv.split([](unicode_ranges::utf8_char ch) { return ch.is_ascii_whitespace(); }),
    std::array{ u8"a"_utf8_sv, u8"b"_utf8_sv, u8"c"_utf8_sv }));
```

#### `rsplit`

```cpp
constexpr auto rsplit(utf8_char ch) const noexcept;
constexpr auto rsplit(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto rsplit(Pred pred) const noexcept;
```

Returns the same lazy split segments as `split(...)`, but in reverse order.

The delimiter is not included in the yielded segments.

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"red,green,blue"_utf8_sv.rsplit(u8","_u8c),
    std::array{ u8"blue"_utf8_sv, u8"green"_utf8_sv, u8"red"_utf8_sv }));
```

#### `split_terminator`, `rsplit_terminator`

```cpp
constexpr auto split_terminator(utf8_char ch) const noexcept;
constexpr auto split_terminator(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto split_terminator(Pred pred) const noexcept;
constexpr auto rsplit_terminator(utf8_char ch) const noexcept;
constexpr auto rsplit_terminator(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto rsplit_terminator(Pred pred) const noexcept;
```

Returns lazy split views like `split(...)` and `rsplit(...)`, but suppresses the final empty segment that would otherwise be produced by a trailing delimiter.

Leading empty segments and interior empty segments are preserved.

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"red,green,blue,"_utf8_sv.split_terminator(u8","_u8c),
    std::array{ u8"red"_utf8_sv, u8"green"_utf8_sv, u8"blue"_utf8_sv }));

assert(std::ranges::equal(
    u8"red,green,blue,"_utf8_sv.rsplit_terminator(u8","_u8c),
    std::array{ u8"blue"_utf8_sv, u8"green"_utf8_sv, u8"red"_utf8_sv }));
```

#### `splitn`, `rsplitn`

```cpp
constexpr auto splitn(size_type count, utf8_char ch) const noexcept;
constexpr auto splitn(size_type count, utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto splitn(size_type count, Pred pred) const noexcept;
constexpr auto rsplitn(size_type count, utf8_char ch) const noexcept;
constexpr auto rsplitn(size_type count, utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto rsplitn(size_type count, Pred pred) const noexcept;
```

Returns a lazy split view that yields at most `count` segments.

`splitn(...)` counts from the front. `rsplitn(...)` counts from the back.

If `count == 0`, the view is empty.

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"red,green,blue"_utf8_sv.splitn(2, u8","_u8c),
    std::array{ u8"red"_utf8_sv, u8"green,blue"_utf8_sv }));

assert(std::ranges::equal(
    u8"red,green,blue"_utf8_sv.rsplitn(2, u8","_u8c),
    std::array{ u8"blue"_utf8_sv, u8"red,green"_utf8_sv }));
```

#### `split_inclusive`

```cpp
constexpr auto split_inclusive(utf8_char ch) const noexcept;
constexpr auto split_inclusive(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto split_inclusive(Pred pred) const noexcept;
```

Returns a lazy split view whose yielded segments keep the matched delimiter at the end of each returned piece.

If the delimiter is not found, the whole string is yielded once.

If the delimiter is empty, the whole string is yielded once.

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"line1\nline2\nline3"_utf8_sv.split_inclusive(u8"\n"_u8c),
    std::array{ u8"line1\n"_utf8_sv, u8"line2\n"_utf8_sv, u8"line3"_utf8_sv }));
```

#### `split_trimmed`

```cpp
constexpr auto split_trimmed(utf8_char ch) const noexcept;
constexpr auto split_trimmed(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto split_trimmed(Pred pred) const noexcept;
```

Returns a lazy view of the non-empty segments separated by `ch`, `sv`, or a per-character predicate.

Leading, trailing, and consecutive delimiters are skipped rather than producing empty substrings.

If the delimiter is empty, the whole string is yielded once when the input is non-empty, and no segments are yielded for an empty input.

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"--alpha----beta--"_utf8_sv.split_trimmed(u8"--"_utf8_sv),
    std::array{ u8"alpha"_utf8_sv, u8"beta"_utf8_sv }));

assert(std::ranges::equal(
    u8" \talpha \t beta\t"_utf8_sv.split_trimmed([](unicode_ranges::utf8_char ch) { return ch.is_ascii_whitespace(); }),
    std::array{ u8"alpha"_utf8_sv, u8"beta"_utf8_sv }));
```

#### `matches`, `rmatches`, `rmatch_indices`

```cpp
constexpr auto matches(utf8_char ch) const noexcept;
constexpr auto matches(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto matches(Pred pred) const noexcept;
constexpr auto rmatches(utf8_char ch) const noexcept;
constexpr auto rmatches(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto rmatches(Pred pred) const noexcept;
constexpr auto rmatch_indices(utf8_char ch) const noexcept;
constexpr auto rmatch_indices(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr auto rmatch_indices(Pred pred) const noexcept;
```

`matches(...)` returns a lazy view of the non-overlapping matches.

`rmatches(...)` returns the same matches in reverse order.

`rmatch_indices(...)` returns a lazy view of non-overlapping matches in reverse order, yielding `(byte_index, match)` pairs.

Behavior:

- exact-delimiter overloads return non-overlapping substring matches
- predicate overloads match individual characters and yield each matching character as a one-character `utf8_string_view`
- empty exact delimiters yield no matches

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8"aba"_utf8_sv.matches(u8"a"_u8c),
    std::array{ u8"a"_utf8_sv, u8"a"_utf8_sv }));

assert(std::ranges::equal(
    u8"aba"_utf8_sv.rmatches(u8"a"_u8c),
    std::array{ u8"a"_utf8_sv, u8"a"_utf8_sv }));

assert(std::ranges::equal(
    u8"aba"_utf8_sv.rmatch_indices(u8"a"_u8c),
    std::array{
        std::pair{ std::size_t{ 2 }, u8"a"_utf8_sv },
        std::pair{ std::size_t{ 0 }, u8"a"_utf8_sv }
    }));
```

#### `split_once`, `rsplit_once`

```cpp
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    split_once(utf8_char ch) const noexcept;
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    split_once(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    split_once(Pred pred) const noexcept;
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    rsplit_once(utf8_char ch) const noexcept;
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    rsplit_once(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    rsplit_once(Pred pred) const noexcept;
```

Returns the prefix and suffix around the first or last delimiter occurrence.

The delimiter is not included in the returned views.

The predicate overloads split around the first or last matching character.

Returns `std::nullopt` if the delimiter is not found.

```cpp
using namespace unicode_ranges::literals;

const auto first = u8"left::middle::right"_utf8_sv.split_once(u8"::"_utf8_sv);
assert(first.has_value());
assert(first->first == u8"left"_utf8_sv);
assert(first->second == u8"middle::right"_utf8_sv);

const auto last = u8"left::middle::right"_utf8_sv.rsplit_once(u8"::"_utf8_sv);
assert(last.has_value());
assert(last->first == u8"left::middle"_utf8_sv);
assert(last->second == u8"right"_utf8_sv);
```

#### Split delimiter rules

For the exact-delimiter split family above:

- empty delimiters are treated as `"not found"`
- `split(...)`, `rsplit(...)`, `split_terminator(...)`, `rsplit_terminator(...)`, `splitn(...)`, `rsplitn(...)`, and `split_inclusive(...)` therefore yield the whole string once when `count != 0` or when no count is involved
- `split_trimmed(...)` yields the whole string once for non-empty input and yields no segments for empty input
- `split_once(...)` and `rsplit_once(...)` return `std::nullopt` for empty delimiters
- `matches(...)`, `rmatches(...)`, and `rmatch_indices(...)` yield no matches for empty delimiters

Predicate overloads do not have an "empty delimiter" case. They operate on one decoded `utf8_char` at a time.

Complexity:

- `split_once(...)`, `rsplit_once(...)`: linear in `size()`

#### `split_whitespace`, `split_ascii_whitespace`

```cpp
constexpr auto split_whitespace() const noexcept;
constexpr auto split_ascii_whitespace() const noexcept;
```

Returns lazy views of the non-empty segments separated by Unicode whitespace or ASCII whitespace.

Whitespace runs are collapsed and not yielded.

Complexity:

- Constant to construct the view

```cpp
using namespace unicode_ranges::literals;

assert(std::ranges::equal(
    u8" \t hello \n world "_utf8_sv.split_whitespace(),
    std::array{ u8"hello"_utf8_sv, u8"world"_utf8_sv }));

assert(std::ranges::equal(
    u8" \t hello \n world "_utf8_sv.split_ascii_whitespace(),
    std::array{ u8"hello"_utf8_sv, u8"world"_utf8_sv }));
```

#### `strip_prefix`, `strip_suffix`, `strip_circumfix`

```cpp
constexpr std::optional<utf8_string_view> strip_prefix(utf8_char ch) const noexcept;
constexpr std::optional<utf8_string_view> strip_prefix(utf8_string_view sv) const noexcept;
constexpr std::optional<utf8_string_view> strip_suffix(utf8_char ch) const noexcept;
constexpr std::optional<utf8_string_view> strip_suffix(utf8_string_view sv) const noexcept;
constexpr std::optional<utf8_string_view>
    strip_circumfix(utf8_char prefix, utf8_char suffix) const noexcept;
constexpr std::optional<utf8_string_view>
    strip_circumfix(utf8_string_view prefix, utf8_string_view suffix) const noexcept;
```

Returns the remaining view after removing a matching prefix, suffix, or prefix-and-suffix pair.

`strip_circumfix(...)` succeeds only when both the prefix and the suffix match.

Returns `std::nullopt` if the requested affix is not present.

Complexity:

- Constant for `utf8_char`
- Linear in the affix length for `utf8_string_view`

```cpp
using namespace unicode_ranges::literals;

assert(u8"foobar"_utf8_sv.strip_prefix(u8"foo"_utf8_sv).value() == u8"bar"_utf8_sv);
assert(u8"foobar"_utf8_sv.strip_suffix(u8"bar"_utf8_sv).value() == u8"foo"_utf8_sv);
assert(u8"[name]"_utf8_sv.strip_circumfix(u8"["_u8c, u8"]"_u8c).value() == u8"name"_utf8_sv);
assert(!u8"[name"_utf8_sv.strip_circumfix(u8"["_u8c, u8"]"_u8c).has_value());
```

#### `trim_prefix`, `trim_suffix`

```cpp
constexpr utf8_string_view trim_prefix(utf8_char ch) const noexcept;
constexpr utf8_string_view trim_prefix(utf8_string_view sv) const noexcept;
constexpr utf8_string_view trim_suffix(utf8_char ch) const noexcept;
constexpr utf8_string_view trim_suffix(utf8_string_view sv) const noexcept;
```

Like `strip_prefix(...)` and `strip_suffix(...)`, but returns the original view unchanged when the affix is not present.

Complexity:

- Constant for `utf8_char`
- Linear in the affix length for `utf8_string_view`

```cpp
using namespace unicode_ranges::literals;

assert(u8"foobar"_utf8_sv.trim_prefix(u8"foo"_utf8_sv) == u8"bar"_utf8_sv);
assert(u8"foobar"_utf8_sv.trim_suffix(u8"baz"_utf8_sv) == u8"foobar"_utf8_sv);
```

#### `trim_start_matches`, `trim_end_matches`, `trim_matches`

```cpp
constexpr utf8_string_view trim_start_matches(utf8_char ch) const noexcept;
constexpr utf8_string_view trim_start_matches(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr utf8_string_view trim_start_matches(Pred pred) const noexcept;
constexpr utf8_string_view trim_end_matches(utf8_char ch) const noexcept;
constexpr utf8_string_view trim_end_matches(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr utf8_string_view trim_end_matches(Pred pred) const noexcept;
constexpr utf8_string_view trim_matches(utf8_char ch) const noexcept;
constexpr utf8_string_view trim_matches(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr utf8_string_view trim_matches(Pred pred) const noexcept;
```

Removes repeated matching characters, repeated matching string views, or repeated edge characters selected by a predicate from the start, end, or both ends.

Behavior:

- `utf8_char` overloads remove matching characters one by one
- `utf8_string_view` overloads remove repeated whole-pattern matches
- predicate overloads remove matching characters one by one
- empty string-view patterns are a no-op

Complexity:

- Linear in `size()`

```cpp
using namespace unicode_ranges::literals;

assert(u8"---name---"_utf8_sv.trim_matches(u8"-"_u8c) == u8"name"_utf8_sv);
assert(u8"foofoo_namefoo"_utf8_sv.trim_start_matches(u8"foo"_utf8_sv) == u8"_namefoo"_utf8_sv);
assert(u8"00120"_utf8_sv.trim_start_matches([](unicode_ranges::utf8_char ch) { return ch == u8"0"_u8c; }) == u8"120"_utf8_sv);
```

#### `trim_start`, `trim_end`, `trim`, `trim_ascii_start`, `trim_ascii_end`, `trim_ascii`

```cpp
constexpr utf8_string_view trim_start() const noexcept;
constexpr utf8_string_view trim_end() const noexcept;
constexpr utf8_string_view trim() const noexcept;
constexpr utf8_string_view trim_ascii_start() const noexcept;
constexpr utf8_string_view trim_ascii_end() const noexcept;
constexpr utf8_string_view trim_ascii() const noexcept;
```

Trims Unicode whitespace or ASCII whitespace from the start, end, or both ends.

Complexity:

- Linear in `size()`

```cpp
using namespace unicode_ranges::literals;

assert(u8" \t hello \n"_utf8_sv.trim_start() == u8"hello \n"_utf8_sv);
assert(u8" \t hello \n"_utf8_sv.trim_end() == u8" \t hello"_utf8_sv);
assert(u8" \t hello \n"_utf8_sv.trim() == u8"hello"_utf8_sv);
assert(u8" \t hello \n"_utf8_sv.trim_ascii() == u8"hello"_utf8_sv);
```

#### `split_once_at`

```cpp
constexpr std::optional<std::pair<utf8_string_view, utf8_string_view>>
    split_once_at(size_type delim) const noexcept;
constexpr std::pair<utf8_string_view, utf8_string_view>
    split_once_at_unchecked(size_type delim) const noexcept;
```

Splits the view at byte index `delim` and returns the prefix and suffix as two `utf8_string_view` objects.

`split_once_at(...)` returns `std::nullopt` if `delim` is out of range or does not name a UTF-8 character boundary.

Preconditions:

- `split_once_at(...)`: none
- `split_once_at_unchecked(...)`: `delim` is a UTF-8 character boundary

Remarks:

- `split_once_at_unchecked(...)` is unchecked. Calling it with an out-of-range or non-boundary index is undefined behavior.

Complexity:

- Constant

```cpp
using namespace unicode_ranges::literals;

const auto text = u8"AéB"_utf8_sv;

const auto checked = text.split_once_at(3);
assert(checked.has_value());
assert(checked->first == u8"Aé"_utf8_sv);
assert(checked->second == u8"B"_utf8_sv);

assert(!text.split_once_at(2).has_value()); // middle of 'é'

const auto unchecked = text.split_once_at_unchecked(1);
assert(unchecked.first == u8"A"_utf8_sv);
assert(unchecked.second == u8"éB"_utf8_sv);
```

#### `char_at`

```cpp
constexpr std::optional<utf8_char> char_at(size_type index) const noexcept;
constexpr utf8_char char_at_unchecked(size_type index) const noexcept;
constexpr std::optional<utf8_char> front() const noexcept;
constexpr utf8_char front_unchecked() const noexcept;
constexpr std::optional<utf8_char> back() const noexcept;
constexpr utf8_char back_unchecked() const noexcept;
```

Returns the character beginning at byte index `index`.

Returns `std::nullopt` if `index` is out of range or does not name a UTF-8 character boundary.

Preconditions:

- None

Complexity:

- Constant

#### `is_grapheme_boundary`

```cpp
constexpr bool is_grapheme_boundary(size_type index) const noexcept;
```

Returns `true` if `index` is a valid grapheme-cluster boundary in the underlying UTF-8 byte sequence.

This function operates on byte indices.

Complexity:

- Linear in the number of grapheme clusters examined

#### `ceil_char_boundary`

```cpp
constexpr size_type ceil_char_boundary(size_type pos) const noexcept;
```

Clamps `pos` to `size()` and rounds it up to the next UTF-8 character boundary.

Complexity:

- Constant

#### `ceil_grapheme_boundary`

```cpp
constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept;
```

Clamps `pos` to `size()` and rounds it up to the next UTF-8 grapheme boundary.

Complexity:

- Linear in the number of grapheme clusters examined

#### `floor_char_boundary`

```cpp
constexpr size_type floor_char_boundary(size_type pos) const noexcept;
```

Clamps `pos` to `size()` and rounds it down to the previous UTF-8 character boundary.

Complexity:

- Constant

#### `floor_grapheme_boundary`

```cpp
constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept;
```

Clamps `pos` to `size()` and rounds it down to the previous UTF-8 grapheme boundary.

Complexity:

- Linear in the number of grapheme clusters examined

#### `char_at_unchecked`

```cpp
constexpr utf8_char char_at_unchecked(size_type index) const noexcept;
```

Returns the character beginning at byte index `index` without checking that `index` is in range or lies on a UTF-8 character boundary.

Preconditions:

- `index < size()`
- `index` is a UTF-8 character boundary

> [!CAUTION]
> This is unchecked. Calling it with an out-of-range index or a non-boundary index is undefined behavior.

Complexity:

- Constant

#### `grapheme_at`

```cpp
constexpr std::optional<utf8_string_view> grapheme_at(size_type index) const noexcept;
```

Returns the grapheme cluster beginning at byte index `index`.

Returns `std::nullopt` if `index` is out of range or does not name a UTF-8 grapheme boundary.

Complexity:

- Linear in the number of grapheme clusters examined

#### `grapheme_substr`

```cpp
constexpr std::optional<utf8_string_view>
    grapheme_substr(size_type pos, size_type count = npos) const noexcept;
```

Returns a subview starting at byte index `pos`.

If `count == npos`, the returned view extends to the end. Otherwise the end is clamped to `size()`.

`pos` and the computed end position must both be UTF-8 grapheme boundaries. If either is not a grapheme boundary, returns `std::nullopt`.

This function operates on byte indices and byte counts.

Complexity:

- Linear in the number of grapheme clusters examined

#### `substr`

```cpp
constexpr std::optional<utf8_string_view>
    substr(size_type pos, size_type count = npos) const noexcept;
```

Returns a subview starting at byte index `pos`.

If `count == npos`, the returned view extends to the end. Otherwise the end is clamped to `size()`.

`pos` and the computed end position must both be UTF-8 character boundaries. If either is not a character boundary, returns `std::nullopt`.

This function operates on byte indices and byte counts.

Preconditions:

- None

Complexity:

- Constant

#### `front`

```cpp
constexpr std::optional<utf8_char> front() const noexcept;
```

Returns the first character in the view, or `std::nullopt` if the view is empty.

Complexity:

- Constant

#### `front_unchecked`

```cpp
constexpr utf8_char front_unchecked() const noexcept;
```

Returns the first character in the view without checking for emptiness.

> [!CAUTION]
> This is unchecked. Calling it on an empty view is undefined behavior.

Complexity:

- Constant

#### `back`

```cpp
constexpr std::optional<utf8_char> back() const noexcept;
```

Returns the last character in the view, or `std::nullopt` if the view is empty.

Complexity:

- Constant

#### `back_unchecked`

```cpp
constexpr utf8_char back_unchecked() const noexcept;
```

Returns the last character in the view without checking for emptiness.

> [!CAUTION]
> This is unchecked. Calling it on an empty view is undefined behavior.

Complexity:

- Constant

#### `starts_with`

```cpp
constexpr bool starts_with(char ch) const noexcept;
constexpr bool starts_with(char8_t ch) const noexcept;
constexpr bool starts_with(utf8_char ch) const noexcept;
constexpr bool starts_with(utf8_string_view sv) const noexcept;
template<class Pred>
constexpr bool starts_with(Pred pred) const noexcept(/* predicate-dependent */);
```

Returns `true` if the view begins with the supplied prefix.

For the `char8_t` overload, the comparison is against a single UTF-8 code unit. In practice this is mainly useful for ASCII prefixes; use `utf8_char` or `utf8_string_view` for general Unicode prefixes.

The predicate overload decodes the first UTF-8 character and applies `pred` to that character.

Preconditions:

- None

Complexity:

- Constant for `char`, `char8_t`, `utf8_char`, and predicate overloads
- Linear in `sv.size()` for `utf8_string_view`

Examples:

```cpp
constexpr auto text = u8"Äbc"_utf8_sv;

static_assert(text.starts_with(u8"Ä"_u8c));
static_assert(text.starts_with([](unicode_ranges::utf8_char ch) {
    return ch == u8"Ä"_u8c;
}));
static_assert(!text.starts_with([](unicode_ranges::utf8_char ch) {
    return ch == u8"b"_u8c;
}));
```

#### `to_ascii_lowercase`, `to_ascii_uppercase`, `to_lowercase`, `to_uppercase`

```cpp
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_ascii_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_ascii_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_lowercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_uppercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    normalize(normalization_form form, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_nfc(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_nfd(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_nfkc(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    to_nfkd(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator>
    case_fold(const Allocator& alloc = Allocator()) const;
```

Return an owning UTF-8 string with Unicode case conversion applied.

- `to_ascii_lowercase` and `to_ascii_uppercase` affect only ASCII letters.
- `to_lowercase` and `to_uppercase` use Unicode case mappings and may expand the output.
- Non-ASCII characters are left untouched by the ASCII-only methods.
- The full Unicode methods are locale-independent.
- The `pos, count` overloads transform only the selected substring.
- Pass `npos` explicitly as `count` to transform from `pos` to the end.
- `pos` and `pos + count` must be valid UTF-8 character boundaries; otherwise the partial overloads throw `std::out_of_range`.

Examples:

```cpp
assert(u8"AbC-éß"_utf8_sv.to_ascii_lowercase() == u8"abc-éß"_utf8_s);
assert(u8"aBc-éß"_utf8_sv.to_ascii_uppercase() == u8"ABC-éß"_utf8_s);
assert(u8"ÄΩİ"_utf8_sv.to_lowercase() == u8"äωi̇"_utf8_s);
assert(u8"äßω"_utf8_sv.to_uppercase() == u8"ÄSSΩ"_utf8_s);
assert(u8"abäßcd"_utf8_sv.to_uppercase(2, 4) == u8"abÄSScd"_utf8_s);
```
- `normalize(form)` selects `nfc`, `nfd`, `nfkc`, or `nfkd`.
- `to_nfc`, `to_nfd`, `to_nfkc`, and `to_nfkd` are convenience wrappers.
- `case_fold()` performs Unicode full case folding for caseless matching.
- Linear in `sv.size()` for `utf8_string_view`

#### `ends_with`

```cpp
constexpr bool ends_with(char ch) const noexcept;
constexpr bool ends_with(char8_t ch) const noexcept;
constexpr bool ends_with(utf8_char ch) const noexcept;
constexpr bool ends_with(utf8_string_view sv) const noexcept;
```

Returns `true` if the view ends with the supplied suffix.

For the `char8_t` overload, the comparison is against a single UTF-8 code unit. In practice this is mainly useful for ASCII suffixes; use `utf8_char` or `utf8_string_view` for general Unicode suffixes.

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto text = "Aé€"_utf8_sv;

static_assert(text.ends_with("€"_u8c));
static_assert(text.ends_with("€"_utf8_sv));
static_assert(!text.ends_with("é"_u8c));
```

Preconditions:

- None

Complexity:

- Constant for `char`, `char8_t`, and `utf8_char`
- Linear in `sv.size()` for `utf8_string_view`

### Important semantics

This API intentionally mixes byte-oriented and scalar-oriented operations.

#### Returns byte counts

- `size()`
- `is_char_boundary()`
- `split()` delimiter argument
- `substr()` arguments

#### Returns character counts or uses character indices

- `char_count()`
- `char_indices()`
- `char_at()`

That distinction matters.

Example:

```cpp
constexpr auto text = "Aé€"_utf8_sv;

assert(text.size() == 6);
assert(text.char_count() == 3);

assert(text.is_char_boundary(1));
assert(!text.is_char_boundary(2));

assert(text.substr(1).value() == "é€"_utf8_sv);
assert(!text.substr(2, 1).has_value());
```

### Comparison

`utf8_string_view` compares lexicographically by the underlying UTF-8 bytes.

### Formatting, streaming, hashing

See [Reference: formatting, streaming, hashing](#reference-formatting-streaming-hashing).

## Reference: utf16_string_view

### Overview

`utf16_string_view` is a validated borrowed UTF-16 code-unit sequence.

It mirrors the public API of `utf8_string_view`, but all sizes, indices, and boundaries are expressed in UTF-16 code units rather than UTF-8 bytes.

That means:

- `size()` returns the number of UTF-16 code units
- `char_count()` returns the number of Unicode scalar values
- `char_indices()` returns `(code_unit_offset, utf16_char)` pairs
- `graphemes()` iterates `utf16_string_view` grapheme-cluster slices
- `grapheme_indices()` returns `(code_unit_offset, utf16_string_view)` pairs
- `find(char16_t, pos)` is a raw code-unit search
- `find(utf16_char, pos)` and `find(utf16_string_view, pos)` round `pos` up to the next UTF-16 character boundary
- `rfind(utf16_char, pos)` and `rfind(utf16_string_view, pos)` round `pos` down to a UTF-16 character boundary
- `char_at(index)` returns `std::nullopt` if `index` is out of range or lies in the middle of a surrogate pair
- `to_utf8()` materializes an owning UTF-8 string

### Synopsis

```cpp
class utf16_string_view
{
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    static constexpr size_type npos = static_cast<size_type>(-1);

    utf16_string_view() = default;

    static constexpr std::expected<utf16_string_view, utf16_error>
        from_code_units(std::u16string_view code_units) noexcept;

    static constexpr utf16_string_view
        from_code_units_unchecked(std::u16string_view code_units) noexcept;

    constexpr auto chars() const noexcept;
    constexpr auto reversed_chars() const noexcept;
    constexpr auto graphemes() const noexcept;
    constexpr size_type size() const noexcept;
    constexpr bool empty() const noexcept;
    constexpr bool is_ascii() const noexcept;
    constexpr auto char_indices() const noexcept;
    constexpr auto grapheme_indices() const noexcept;
    constexpr size_type grapheme_count() const noexcept;

    constexpr bool contains(utf16_char ch) const noexcept;
    constexpr bool contains(utf16_string_view sv) const noexcept;

    constexpr size_type find(char16_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find(utf16_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find(utf16_string_view sv, size_type pos = 0) const noexcept;
    constexpr size_type find_first_of(char16_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_of(utf16_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_of(utf16_string_view sv, size_type pos = 0) const noexcept;
    constexpr size_type find_first_not_of(char16_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_not_of(utf16_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find_first_not_of(utf16_string_view sv, size_type pos = 0) const noexcept;

    constexpr size_type rfind(char16_t ch, size_type pos = npos) const noexcept;
    constexpr size_type rfind(utf16_char ch, size_type pos = npos) const noexcept;
    constexpr size_type rfind(utf16_string_view sv, size_type pos = npos) const noexcept;
    constexpr size_type find_last_of(char16_t ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_of(utf16_char ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_of(utf16_string_view sv, size_type pos = npos) const noexcept;
    constexpr size_type find_last_not_of(char16_t ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_not_of(utf16_char ch, size_type pos = npos) const noexcept;
    constexpr size_type find_last_not_of(utf16_string_view sv, size_type pos = npos) const noexcept;

    constexpr bool is_char_boundary(size_type index) const noexcept;
    constexpr bool is_grapheme_boundary(size_type index) const noexcept;
    constexpr size_type char_count() const noexcept;
    constexpr auto split(utf16_char ch) const noexcept;
    constexpr auto split(utf16_string_view sv) const noexcept;
    constexpr auto rsplit(utf16_char ch) const noexcept;
    constexpr auto rsplit(utf16_string_view sv) const noexcept;
    constexpr auto split_terminator(utf16_char ch) const noexcept;
    constexpr auto split_terminator(utf16_string_view sv) const noexcept;
    constexpr auto rsplit_terminator(utf16_char ch) const noexcept;
    constexpr auto rsplit_terminator(utf16_string_view sv) const noexcept;
    constexpr auto splitn(size_type count, utf16_char ch) const noexcept;
    constexpr auto splitn(size_type count, utf16_string_view sv) const noexcept;
    constexpr auto rsplitn(size_type count, utf16_char ch) const noexcept;
    constexpr auto rsplitn(size_type count, utf16_string_view sv) const noexcept;
    constexpr auto split_inclusive(utf16_char ch) const noexcept;
    constexpr auto split_inclusive(utf16_string_view sv) const noexcept;
    constexpr auto split_trimmed(utf16_char ch) const noexcept;
    constexpr auto split_trimmed(utf16_string_view sv) const noexcept;
    template<class Pred>
    constexpr auto split_trimmed(Pred pred) const noexcept;
    constexpr auto matches(utf16_char ch) const noexcept;
    constexpr auto matches(utf16_string_view sv) const noexcept;
    constexpr auto rmatch_indices(utf16_char ch) const noexcept;
    constexpr auto rmatch_indices(utf16_string_view sv) const noexcept;
    constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
        split_once(utf16_char ch) const noexcept;
    constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
        split_once(utf16_string_view sv) const noexcept;
    constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
        rsplit_once(utf16_char ch) const noexcept;
    constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
        rsplit_once(utf16_string_view sv) const noexcept;
    constexpr auto split_whitespace() const noexcept;
    constexpr auto split_ascii_whitespace() const noexcept;
    constexpr std::optional<utf16_string_view> strip_prefix(utf16_char ch) const noexcept;
    constexpr std::optional<utf16_string_view> strip_prefix(utf16_string_view sv) const noexcept;
    constexpr std::optional<utf16_string_view> strip_suffix(utf16_char ch) const noexcept;
    constexpr std::optional<utf16_string_view> strip_suffix(utf16_string_view sv) const noexcept;
    constexpr std::optional<utf16_string_view>
        strip_circumfix(utf16_char prefix, utf16_char suffix) const noexcept;
    constexpr std::optional<utf16_string_view>
        strip_circumfix(utf16_string_view prefix, utf16_string_view suffix) const noexcept;
    constexpr utf16_string_view trim_prefix(utf16_char ch) const noexcept;
    constexpr utf16_string_view trim_prefix(utf16_string_view sv) const noexcept;
    constexpr utf16_string_view trim_suffix(utf16_char ch) const noexcept;
    constexpr utf16_string_view trim_suffix(utf16_string_view sv) const noexcept;
    constexpr utf16_string_view trim_start_matches(utf16_char ch) const noexcept;
    constexpr utf16_string_view trim_start_matches(utf16_string_view sv) const noexcept;
    constexpr utf16_string_view trim_end_matches(utf16_char ch) const noexcept;
    constexpr utf16_string_view trim_end_matches(utf16_string_view sv) const noexcept;
    constexpr utf16_string_view trim_matches(utf16_char ch) const noexcept;
    constexpr utf16_string_view trim_matches(utf16_string_view sv) const noexcept;
    constexpr utf16_string_view trim_start() const noexcept;
    constexpr utf16_string_view trim_end() const noexcept;
    constexpr utf16_string_view trim() const noexcept;
    constexpr utf16_string_view trim_ascii_start() const noexcept;
    constexpr utf16_string_view trim_ascii_end() const noexcept;
    constexpr utf16_string_view trim_ascii() const noexcept;
    constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
        split_once_at(size_type delim) const noexcept;
    constexpr std::pair<utf16_string_view, utf16_string_view>
        split_once_at_unchecked(size_type delim) const noexcept;
    constexpr utf16_string replace_all(utf16_char from, utf16_char to) const;
    constexpr utf16_string replace_all(utf16_char from, utf16_string_view to) const;
    constexpr utf16_string replace_all(utf16_string_view from, utf16_char to) const;
    constexpr utf16_string replace_all(utf16_string_view from, utf16_string_view to) const;
    constexpr utf16_string replace_n(size_type count, utf16_char from, utf16_char to) const;
    constexpr utf16_string replace_n(size_type count, utf16_char from, utf16_string_view to) const;
    constexpr utf16_string replace_n(size_type count, utf16_string_view from, utf16_char to) const;
    constexpr utf16_string replace_n(size_type count, utf16_string_view from, utf16_string_view to) const;
    constexpr std::optional<utf16_char> char_at(size_type index) const noexcept;
    constexpr utf16_char char_at_unchecked(size_type index) const noexcept;
    constexpr std::optional<utf16_string_view> grapheme_at(size_type index) const noexcept;
    constexpr std::optional<utf16_string_view> substr(size_type pos, size_type count = npos) const noexcept;
    constexpr std::optional<utf16_string_view> grapheme_substr(size_type pos, size_type count = npos) const noexcept;
    constexpr std::optional<utf16_char> front() const noexcept;
    constexpr utf16_char front_unchecked() const noexcept;
    constexpr std::optional<utf16_char> back() const noexcept;
    constexpr utf16_char back_unchecked() const noexcept;
    constexpr bool starts_with(char16_t ch) const noexcept;
    constexpr bool starts_with(utf16_char ch) const noexcept;
    constexpr bool starts_with(utf16_string_view sv) const noexcept;
    template<class Pred>
    constexpr bool starts_with(Pred pred) const noexcept(/* predicate-dependent */);
    constexpr bool ends_with(char16_t ch) const noexcept;
    constexpr bool ends_with(utf16_char ch) const noexcept;
    constexpr bool ends_with(utf16_string_view sv) const noexcept;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_ascii_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_ascii_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_lowercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_uppercase(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        normalize(normalization_form form, const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_nfc(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_nfd(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_nfkc(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_nfkd(const Allocator& alloc = Allocator()) const;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        case_fold(const Allocator& alloc = Allocator()) const;
    constexpr size_type ceil_char_boundary(size_type pos) const noexcept;
    constexpr size_type floor_char_boundary(size_type pos) const noexcept;
    constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept;
    constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept;

    constexpr auto base() const noexcept;
    constexpr std::u16string_view as_view() const noexcept;
    constexpr operator std::u16string_view() const noexcept;
    template<class Allocator = std::allocator<char8_t>>
    constexpr basic_utf8_string<Allocator>
        to_utf8(const Allocator& alloc = Allocator()) const;
};
```

### Construction

Use `from_code_units(...)` when the source may be arbitrary UTF-16 and `from_code_units_unchecked(...)` only when validity is already guaranteed.

The unchecked character constructors `from_scalar_unchecked(...)` and `from_utf16_code_units_unchecked(...)` are also available; like the UTF-8 counterparts, they trade validation for performance and are undefined behavior on invalid input.

> [!CAUTION]
> `from_code_units_unchecked(...)`, `from_scalar_unchecked(...)`, and `from_utf16_code_units_unchecked(...)` are unchecked entry points. Invalid UTF-16 or invalid scalar values are undefined behavior.

Complexity:

- `from_code_units(...)`: linear in `code_units.size()`
- `from_code_units_unchecked(...)`: constant

For compile-time construction from UTF-16 source text, prefer the `_utf16_sv` literal:

```cpp
constexpr auto text = u"Aé😀"_utf16_sv;
static_assert(text.size() == 4);
static_assert(text.char_count() == 3);
```

### Iteration and search

`chars()` yields `utf16_char` values in forward order.

`reversed_chars()` yields the same `utf16_char` values in reverse order.

`graphemes()` yields `utf16_string_view` values, each covering one default Unicode grapheme cluster.

> [!WARNING]
> `as_view()`, `chars()`, `reversed_chars()`, `graphemes()`, `char_indices()`, and `grapheme_indices()` all borrow from the underlying UTF-16 storage.
> Do not keep them after that storage is destroyed or, for owning strings, after mutation.

`grapheme_indices()` yields `(code_unit_offset, utf16_string_view)` pairs.

`grapheme_count()` counts default Unicode grapheme clusters rather than scalar values.

`find_grapheme`, `rfind_grapheme`, and `contains_grapheme` search using grapheme-cluster boundaries rather than only character boundaries. `find_first_of`, `find_first_not_of`, `find_last_of`, and `find_last_not_of` use raw code-unit semantics for `char16_t`, and UTF-16 character semantics for `utf16_char` and `utf16_string_view`.

`find` and `rfind` behave like the UTF-8 variants, but with UTF-16 code-unit indexing and surrogate-pair boundaries.

#### `to_ascii_lowercase`, `to_ascii_uppercase`, `to_lowercase`, `to_uppercase`

```cpp
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_ascii_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_ascii_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_lowercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_uppercase(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    normalize(normalization_form form, const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_nfc(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_nfd(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_nfkc(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    to_nfkd(const Allocator& alloc = Allocator()) const;
template<class Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator>
    case_fold(const Allocator& alloc = Allocator()) const;
```

Returns a new UTF-16 owning string with ASCII-only or full Unicode case conversion applied.

- The `ascii` variants change only ASCII letters.
- The non-ASCII variants use Unicode case mapping and may change the output length.
- The `pos, count` overloads transform only the selected substring.
- Pass `npos` explicitly as `count` to transform from `pos` to the end.
- `pos` and `pos + count` must be valid UTF-16 character boundaries; otherwise the partial overloads throw `std::out_of_range`.

Examples:

```cpp
assert(u"AbC-éß"_utf16_sv.to_ascii_lowercase() == u"abc-éß"_utf16_s);
assert(u"aBc-éß"_utf16_sv.to_ascii_uppercase() == u"ABC-éß"_utf16_s);
assert(u"ÄΩİ"_utf16_sv.to_lowercase() == u"äωi̇"_utf16_s);
assert(u"äßω"_utf16_sv.to_uppercase() == u"ÄSSΩ"_utf16_s);
assert(u"abäßcd"_utf16_sv.to_uppercase(2, 2) == u"abÄSScd"_utf16_s);
```

- `normalize(form)` selects `nfc`, `nfd`, `nfkc`, or `nfkd`.
- `to_nfc`, `to_nfd`, `to_nfkc`, and `to_nfkd` are convenience wrappers.
- `case_fold()` performs Unicode full case folding for caseless matching.
- Linear in `sv.size()` for `utf16_string_view`

#### `split`

```cpp
constexpr auto split(utf16_char ch) const noexcept;
constexpr auto split(utf16_string_view sv) const noexcept;
```

Returns a lazy view of the segments separated by `ch` or `sv`.

The delimiter is not included in the yielded segments.

Complexity:

- Constant to construct the view

#### `rsplit`

```cpp
constexpr auto rsplit(utf16_char ch) const noexcept;
constexpr auto rsplit(utf16_string_view sv) const noexcept;
```

Returns the same lazy split segments as `split(...)`, but in reverse order.

The delimiter is not included in the yielded segments.

Complexity:

- Constant to construct the view

#### `split_terminator`, `rsplit_terminator`

```cpp
constexpr auto split_terminator(utf16_char ch) const noexcept;
constexpr auto split_terminator(utf16_string_view sv) const noexcept;
constexpr auto rsplit_terminator(utf16_char ch) const noexcept;
constexpr auto rsplit_terminator(utf16_string_view sv) const noexcept;
```

Returns lazy split views like `split(...)` and `rsplit(...)`, but suppresses the final empty segment that would otherwise be produced by a trailing delimiter.

Leading empty segments and interior empty segments are preserved.

Complexity:

- Constant to construct the view

#### `splitn`, `rsplitn`

```cpp
constexpr auto splitn(size_type count, utf16_char ch) const noexcept;
constexpr auto splitn(size_type count, utf16_string_view sv) const noexcept;
constexpr auto rsplitn(size_type count, utf16_char ch) const noexcept;
constexpr auto rsplitn(size_type count, utf16_string_view sv) const noexcept;
constexpr auto split_inclusive(utf16_char ch) const noexcept;
constexpr auto split_inclusive(utf16_string_view sv) const noexcept;
```

Returns a lazy split view that yields at most `count` segments.

`splitn(...)` counts from the front. `rsplitn(...)` counts from the back.

If `count == 0`, the view is empty.

Complexity:

- Constant to construct the view

#### `split_inclusive`

```cpp
constexpr auto split_inclusive(utf16_char ch) const noexcept;
constexpr auto split_inclusive(utf16_string_view sv) const noexcept;
```

Returns a lazy split view whose yielded segments keep the matched delimiter at the end of each returned piece.

If the delimiter is not found, the whole string is yielded once.

If the delimiter is empty, the whole string is yielded once.

Complexity:

- Constant to construct the view

#### `split_trimmed`

```cpp
constexpr auto split_trimmed(utf16_char ch) const noexcept;
constexpr auto split_trimmed(utf16_string_view sv) const noexcept;
template<class Pred>
constexpr auto split_trimmed(Pred pred) const noexcept;
```

Returns a lazy view of the non-empty segments separated by `ch`, `sv`, or a per-character predicate.

Leading, trailing, and consecutive delimiters are skipped rather than producing empty substrings.

If the delimiter is empty, the whole string is yielded once when the input is non-empty, and no segments are yielded for an empty input.

Complexity:

- Constant to construct the view

#### `matches`, `rmatch_indices`

```cpp
constexpr auto matches(utf16_char ch) const noexcept;
constexpr auto matches(utf16_string_view sv) const noexcept;
constexpr auto rmatch_indices(utf16_char ch) const noexcept;
constexpr auto rmatch_indices(utf16_string_view sv) const noexcept;
```

`matches(...)` returns a lazy view of the non-overlapping matches.

`rmatch_indices(...)` returns a lazy view of non-overlapping matches in reverse order, yielding `(code_unit_index, match)` pairs.

Empty delimiters yield no matches.

Complexity:

- Constant to construct the view

#### `split_once`, `rsplit_once`

```cpp
constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
    split_once(utf16_char ch) const noexcept;
constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
    split_once(utf16_string_view sv) const noexcept;
constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
    rsplit_once(utf16_char ch) const noexcept;
constexpr std::optional<std::pair<utf16_string_view, utf16_string_view>>
    rsplit_once(utf16_string_view sv) const noexcept;
```

Returns the prefix and suffix around the first or last delimiter occurrence.

The delimiter is not included in the returned views.

Returns `std::nullopt` if the delimiter is not found.

#### Split delimiter rules

For the split family above:

- empty delimiters are treated as `"not found"`
- `split(...)`, `rsplit(...)`, `split_terminator(...)`, `rsplit_terminator(...)`, `splitn(...)`, `rsplitn(...)`, and `split_inclusive(...)` therefore yield the whole string once when `count != 0` or when no count is involved
- `split_trimmed(...)` yields the whole string once for non-empty input and yields no segments for empty input
- `split_once(...)` and `rsplit_once(...)` return `std::nullopt` for empty delimiters
- `matches(...)` and `rmatch_indices(...)` yield no matches for empty delimiters

Complexity:

- `split_once(...)`, `rsplit_once(...)`: linear in `size()`

#### `split_whitespace`, `split_ascii_whitespace`

```cpp
constexpr auto split_whitespace() const noexcept;
constexpr auto split_ascii_whitespace() const noexcept;
```

Returns lazy views of the non-empty segments separated by Unicode whitespace or ASCII whitespace.

Whitespace runs are collapsed and not yielded.

Complexity:

- Constant to construct the view

#### `strip_prefix`, `strip_suffix`, `strip_circumfix`

```cpp
constexpr std::optional<utf16_string_view> strip_prefix(utf16_char ch) const noexcept;
constexpr std::optional<utf16_string_view> strip_prefix(utf16_string_view sv) const noexcept;
constexpr std::optional<utf16_string_view> strip_suffix(utf16_char ch) const noexcept;
constexpr std::optional<utf16_string_view> strip_suffix(utf16_string_view sv) const noexcept;
constexpr std::optional<utf16_string_view>
    strip_circumfix(utf16_char prefix, utf16_char suffix) const noexcept;
constexpr std::optional<utf16_string_view>
    strip_circumfix(utf16_string_view prefix, utf16_string_view suffix) const noexcept;
```

Returns the remaining view after removing a matching prefix, suffix, or prefix-and-suffix pair.

Returns `std::nullopt` if the requested affix is not present.

Complexity:

- Constant for `utf16_char`
- Linear in the affix length for `utf16_string_view`

#### `trim_prefix`, `trim_suffix`

```cpp
constexpr utf16_string_view trim_prefix(utf16_char ch) const noexcept;
constexpr utf16_string_view trim_prefix(utf16_string_view sv) const noexcept;
constexpr utf16_string_view trim_suffix(utf16_char ch) const noexcept;
constexpr utf16_string_view trim_suffix(utf16_string_view sv) const noexcept;
```

Like `strip_prefix(...)` and `strip_suffix(...)`, but returns the original view unchanged when the affix is not present.

Complexity:

- Constant for `utf16_char`
- Linear in the affix length for `utf16_string_view`

#### `trim_start_matches`, `trim_end_matches`, `trim_matches`

```cpp
constexpr utf16_string_view trim_start_matches(utf16_char ch) const noexcept;
constexpr utf16_string_view trim_start_matches(utf16_string_view sv) const noexcept;
constexpr utf16_string_view trim_end_matches(utf16_char ch) const noexcept;
constexpr utf16_string_view trim_end_matches(utf16_string_view sv) const noexcept;
constexpr utf16_string_view trim_matches(utf16_char ch) const noexcept;
constexpr utf16_string_view trim_matches(utf16_string_view sv) const noexcept;
```

Removes repeated matching characters or repeated matching string views from the start, end, or both ends.

Empty string-view patterns are a no-op.

Complexity:

- Linear in `size()`

#### `trim_start`, `trim_end`, `trim`, `trim_ascii_start`, `trim_ascii_end`, `trim_ascii`

```cpp
constexpr utf16_string_view trim_start() const noexcept;
constexpr utf16_string_view trim_end() const noexcept;
constexpr utf16_string_view trim() const noexcept;
constexpr utf16_string_view trim_ascii_start() const noexcept;
constexpr utf16_string_view trim_ascii_end() const noexcept;
constexpr utf16_string_view trim_ascii() const noexcept;
```

Trims Unicode whitespace or ASCII whitespace from the start, end, or both ends.

Complexity:

- Linear in `size()`

`front()` and `back()` return `std::optional<utf16_char>`. Use `front_unchecked()` and `back_unchecked()` only when the view is known to be non-empty.

`to_utf8()` materializes the same text as an owning UTF-8 string by transcoding through Unicode scalar values. By default this returns `utf8_string`, but you can supply a different allocator and get `basic_utf8_string<Allocator>`.

Complexity:

- `chars()`, `reversed_chars()`, `char_indices()`, and `grapheme_indices()`: constant to construct, linear to iterate
- `graphemes()`: constant to construct, linear to iterate the full view
- `contains(...)`, `find(...)`, and `rfind(...)`: linear in `size()`
- `find_grapheme(...)`, `rfind_grapheme(...)`, and `contains_grapheme(...)`: linear in the number of grapheme clusters examined
- `find_first_of(...)`, `find_first_not_of(...)`, `find_last_of(...)`, and `find_last_not_of(...)`: linear in `size()` for `char16_t` / `utf16_char`, and linear in `size() * sv.char_count()` for `utf16_string_view` in the worst case
- `grapheme_count()`, `is_grapheme_boundary()`, `ceil_grapheme_boundary()`, `floor_grapheme_boundary()`, `grapheme_at()`, and `grapheme_substr()`: linear in the number of grapheme clusters examined
- `char_count()`: linear in the number of characters
- `is_char_boundary()`, `char_at()`, `char_at_unchecked()`, `split()`, `rsplit()`, `split_terminator()`, `rsplit_terminator()`, `splitn()`, `rsplitn()`, `split_once_at()`, `split_once_at_unchecked()`, `front()`, and `back()`: constant to construct or constant to execute
- `split_once(...)` and `rsplit_once(...)`: linear in `size()`
- `split_whitespace()` and `split_ascii_whitespace()`: constant to construct, linear to iterate
- `strip_prefix()`, `strip_suffix()`, `trim_prefix()`, and `trim_suffix()`: constant for single-character overloads, linear in the affix length for `utf16_string_view`
- `strip_circumfix()`, `trim_matches()`, `trim_start_matches()`, `trim_end_matches()`, `trim()`, `trim_start()`, `trim_end()`, `trim_ascii()`, `trim_ascii_start()`, and `trim_ascii_end()`: linear in `size()`
- `starts_with(...)` and `ends_with(...)`: constant for single-code-unit and single-character overloads, linear in `sv.size()` for `utf16_string_view`
- `to_utf8()`: linear in `size()`

Example:

```cpp
constexpr auto text = u"Aé😀"_utf16_sv;

static_assert(text.find(u"é"_u16c) == 1);
static_assert(text.find(u"😀"_u16c) == 2);
static_assert(text.rfind(u"😀"_u16c) == 2);
static_assert(text.ceil_char_boundary(3) == 4);
static_assert(text.floor_char_boundary(3) == 2);
```

### Substrings and boundaries

Like `utf8_string_view`, substring operations are boundary-checked.

The grapheme-oriented helpers use the same code-unit-position model as the rest of the UTF-16 API:

- `is_grapheme_boundary(index)` checks whether a code-unit offset starts a grapheme cluster
- `ceil_grapheme_boundary(pos)` and `floor_grapheme_boundary(pos)` round a code-unit offset to the nearest grapheme boundary
- `grapheme_at(index)` returns the grapheme cluster that starts at `index`
- `grapheme_substr(pos, count)` behaves like `substr`, but requires grapheme boundaries instead of only UTF-16 character boundaries

> [!CAUTION]
> `char_at_unchecked()`, `front_unchecked()`, and `back_unchecked()` are unchecked. Calling them with an invalid boundary or on an empty view is undefined behavior.

Example:

```cpp
constexpr auto text = u"Aé😀"_utf16_sv;

static_assert(text.char_at(2).value() == u"😀"_u16c);
static_assert(!text.char_at(3).has_value());
static_assert(text.substr(1).value() == u"é😀"_utf16_sv);
static_assert(!text.substr(3, 1).has_value());
```

### Comparison

`utf16_string_view` compares lexicographically by the underlying UTF-16 code units.

### Formatting, streaming, hashing

See [Reference: formatting, streaming, hashing](#reference-formatting-streaming-hashing).

## Reference: utf16_string

### Overview

`utf16_string` owns UTF-16 text.

It is allocator-aware and is backed by:

```cpp
std::basic_string<char16_t, std::char_traits<char16_t>, Allocator>
```

It shares the same read-only API as `utf16_string_view`.

`utf16_string` also compares lexicographically by the underlying UTF-16 code units, and it can be compared directly with `utf16_string_view`.

The owning API mirrors `utf8_string`, but all sizes and mutation ranges are expressed in UTF-16 code units.

### Construction and literals

Use:

- `utf16_string_view` when you want a borrowed validated view
- `utf16_string` when you want ownership

The `_utf16_s` literal constructs an owning UTF-16 string:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

const auto owned = u"Aé😀"_utf16_s;
```

Cross-encoding is available through the direct `utf16_string(utf8_string_view, alloc)` constructor and through `to_utf8()`, both with allocator-aware target construction.

Checked factory construction is also available from raw source text:

```cpp
static constexpr auto from_bytes(std::string_view bytes,
                                 const Allocator& alloc = Allocator()) noexcept
    -> std::expected<basic_utf16_string, utf8_error>;
static constexpr auto from_bytes(std::wstring_view bytes,
                                 const Allocator& alloc = Allocator()) noexcept;
```

```cpp
constexpr basic_utf16_string(utf8_string_view view,
                             const Allocator& alloc = Allocator());
```

Unchecked factory construction from already-valid UTF-16 code units is also allocator-aware:

```cpp
static constexpr basic_utf16_string
    from_code_units_unchecked(std::u16string_view code_units,
                              const Allocator& alloc = Allocator()) noexcept;
static constexpr basic_utf16_string
    from_code_units_unchecked(base_type code_units,
                              const Allocator& alloc) noexcept;
```

> [!CAUTION]
> These factories are unchecked. Passing invalid UTF-16 code units is undefined behavior.

The standard PMR alias is also provided:

```cpp
namespace unicode_ranges::pmr
{
    using utf16_string =
        basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>;
}
```

Complexity:

- default construction: constant
- construction from `utf16_string_view` or `utf8_string_view`: linear in the source length
- `from_bytes(std::string_view)` / `from_bytes(std::wstring_view)`: linear in the source length
- unchecked construction from `std::u16string_view`: linear in `code_units.size()`
- unchecked construction from `base_type`: linear in `code_units.size()` for copy, constant-time move for moved-in base storage

### Mutation APIs

The owning UTF-16 string supports the same mutation family as `utf8_string`:

- `append`, `append_range`
- `assign`, `assign_range`
- `insert`, `insert_range`
- `erase`
- `replace_inplace`
- `replace_with_range_inplace`
- `replace_all`, `replace_n`
- `push_back`
- `operator+=`
- `operator+`

All positions and lengths are UTF-16 code-unit indices. Mutating operations throw `std::out_of_range` when asked to split a surrogate pair, while query-style operations such as `split_once()`, `rsplit_once()`, `split_once_at()`, `char_at()`, `substr()`, and `grapheme_substr()` report failure with `std::optional`.

> [!WARNING]
> Like `utf8_string`, borrowed results from `as_view()`, `chars()`, `graphemes()`, and related range helpers are tied to the string's internal storage and should not be kept across mutation.

> [!IMPORTANT]
> The same self-referential insertion guarantee applies here as on `utf8_string`, and it is covered by the test suite.
> Appending, inserting, and replacing from validated UTF-16 views or character ranges derived from the same string is valid.

Complexity:

- `append`, `append_range`, `assign`, `assign_range`, and `operator+=`: linear in the appended or assigned text size
- `push_back`: amortized constant
- `reverse(...)`: linear in the reversed substring size
- `insert`, `erase`, `replace_inplace`, `replace_with_range_inplace`, `replace_all`, and `replace_n`: linear in `size() + replacement_size`
- `operator+`: linear in `lhs.size() + rhs.size()`
- `to_utf8()`: linear in `size()`

#### Insert

```cpp
constexpr basic_utf16_string& insert(std::size_t index, utf16_string_view sv);
constexpr basic_utf16_string& insert(std::size_t index, utf16_char ch);
constexpr basic_utf16_string& insert(std::size_t index, std::size_t count, utf16_char ch);
constexpr basic_utf16_string& insert_range(std::size_t index, R&& rg);
constexpr basic_utf16_string& insert(std::size_t index, It it, Sent sent);
constexpr basic_utf16_string& insert(std::size_t index, std::initializer_list<utf16_char> ilist);
```

Inserts validated UTF-16 text or UTF-16 characters at code-unit index `index`.

Supported inputs:

- `utf16_string_view`
- one `utf16_char`
- repeated `utf16_char`
- ranges of `utf16_char`
- iterator/sentinel pairs
- initializer lists of `utf16_char`

Requirements:

- `index <= size()`
- `index` must name a UTF-16 character boundary

Throws:

- `std::out_of_range` if `index` is greater than `size()`
- `std::out_of_range` if `index` does not name a UTF-16 character boundary

Complexity: linear in `size() + inserted_size`.

#### Pop Back

```cpp
constexpr std::optional<utf16_char> pop_back();
```

Removes and returns the last UTF-16 character.

Returns `std::nullopt` if the string is empty.

Complexity: constant.

#### Erase

```cpp
constexpr basic_utf16_string& erase(std::size_t index, std::size_t count = npos);
```

Erases the code-unit range beginning at `index` and extending for at most `count` code units, clamped to the end of the string.

Requirements:

- `index <= size()`
- `index` must be a UTF-16 character boundary
- `index + min(count, size() - index)` must also be a UTF-16 character boundary

Throws:

- `std::out_of_range` if `index` is greater than `size()`
- `std::out_of_range` if the erased range does not align to UTF-16 character boundaries

Complexity: linear in `size()`.

#### Reverse

```cpp
constexpr basic_utf16_string& reverse() noexcept;
constexpr basic_utf16_string& reverse(std::size_t pos, std::size_t count = npos);
```

Reverses UTF-16 characters in place.

Behavior:

- `reverse()` reverses the whole string and is `noexcept`
- `reverse(pos, count)` reverses the substring `[pos, pos + count)`
- `count == npos` reverses from `pos` to the end of the string
- reversal is character-aware, so surrogate pairs stay intact

Requirements:

- `pos <= size()`
- `count == npos` or `count <= size() - pos`
- `pos` must be a UTF-16 character boundary
- `pos + resolved_count` must also be a UTF-16 character boundary

Throws:

- `std::out_of_range` if `pos` is greater than `size()`
- `std::out_of_range` if an explicit `count` extends past the end of the string
- `std::out_of_range` if the reversed range does not align to UTF-16 character boundaries

Complexity: linear in the reversed substring size.

```cpp
auto s = u"Aé😀"_utf16_s;
s.reverse();
assert(s == u"😀éA"_utf16_sv);

auto part = u"AéB😀C"_utf16_s;
part.reverse(1, 4);
assert(part == u"A😀BéC"_utf16_sv);
```

#### Replace In Place

```cpp
constexpr basic_utf16_string& replace_inplace(std::size_t pos, std::size_t count, utf16_string_view other);
constexpr basic_utf16_string& replace_inplace(std::size_t pos, std::size_t count, utf16_char other);
constexpr basic_utf16_string& replace_inplace(std::size_t pos, utf16_string_view other);
constexpr basic_utf16_string& replace_inplace(std::size_t pos, utf16_char other);
constexpr basic_utf16_string& replace_with_range_inplace(std::size_t pos, std::size_t count, R&& rg);
constexpr basic_utf16_string& replace_with_range_inplace(std::size_t pos, R&& rg);
```

Replaces a UTF-16 substring with validated UTF-16 text, a single UTF-16 character, or a range of UTF-16 characters.

Supported replacements:

- validated UTF-16 text via `replace_inplace(..., utf16_string_view)`
- one character via `replace_inplace(..., utf16_char)`
- character ranges via `replace_with_range_inplace(...)`

Requirements:

- `pos <= size()`
- `pos` must be a UTF-16 character boundary
- `pos + min(count, size() - pos)` must also be a UTF-16 character boundary when a counted overload is used

The single-position overloads replace exactly one UTF-16 character beginning at `pos`.

Throws:

- `std::out_of_range` if `pos` is out of range
- `std::out_of_range` if the replaced range does not align to UTF-16 character boundaries

Complexity:

- `replace_inplace(..., utf16_string_view)`: linear in `size() + other.size()`
- `replace_inplace(..., utf16_char)`: linear in `size()`
- `replace_with_range_inplace(...)`: linear in `size() + replacement_size`

#### Replace All, Replace N

```cpp
constexpr basic_utf16_string replace_all(utf16_char from, utf16_char to) const&;
constexpr basic_utf16_string replace_all(utf16_char from, utf16_string_view to) const&;
constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_char to) const&;
constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_string_view to) const&;
constexpr basic_utf16_string replace_all(utf16_char from, utf16_char to) &&;
constexpr basic_utf16_string replace_all(utf16_char from, utf16_string_view to) &&;
constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_char to) &&;
constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_string_view to) &&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_char from, utf16_char to) const&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_char from, utf16_string_view to) const&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_string_view from, utf16_char to) const&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_string_view from, utf16_string_view to) const&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_char from, utf16_char to) &&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_char from, utf16_string_view to) &&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_string_view from, utf16_char to) &&;
constexpr basic_utf16_string replace_n(std::size_t count, utf16_string_view from, utf16_string_view to) &&;
constexpr basic_utf16_string to_ascii_lowercase() const&;
constexpr basic_utf16_string to_ascii_lowercase(size_type pos, size_type count) const&;
constexpr basic_utf16_string to_ascii_lowercase() &&;
constexpr basic_utf16_string to_ascii_lowercase(size_type pos, size_type count) &&;
constexpr basic_utf16_string to_ascii_uppercase() const&;
constexpr basic_utf16_string to_ascii_uppercase(size_type pos, size_type count) const&;
constexpr basic_utf16_string to_ascii_uppercase() &&;
constexpr basic_utf16_string to_ascii_uppercase(size_type pos, size_type count) &&;
constexpr basic_utf16_string to_lowercase() const&;
constexpr basic_utf16_string to_lowercase(size_type pos, size_type count) const&;
constexpr basic_utf16_string to_lowercase() &&;
constexpr basic_utf16_string to_lowercase(size_type pos, size_type count) &&;
constexpr basic_utf16_string to_uppercase() const&;
constexpr basic_utf16_string to_uppercase(size_type pos, size_type count) const&;
constexpr basic_utf16_string to_uppercase() &&;
constexpr basic_utf16_string to_uppercase(size_type pos, size_type count) &&;
constexpr basic_utf16_string normalize(normalization_form form) const&;
constexpr basic_utf16_string normalize(normalization_form form) &&;
constexpr basic_utf16_string to_nfc() const&;
constexpr basic_utf16_string to_nfc() &&;
constexpr basic_utf16_string to_nfd() const&;
constexpr basic_utf16_string to_nfd() &&;
constexpr basic_utf16_string to_nfkc() const&;
constexpr basic_utf16_string to_nfkc() &&;
constexpr basic_utf16_string to_nfkd() const&;
constexpr basic_utf16_string to_nfkd() &&;
constexpr basic_utf16_string case_fold() const&;
constexpr basic_utf16_string case_fold() &&;
```

Returns a new owning string after replacing non-overlapping matches of `from`.

- `replace_all(...)` replaces every non-overlapping match
- `replace_n(...)` replaces at most `count` non-overlapping matches
- empty `from` is a no-op
- `const&` overloads leave the source string unchanged
- `&&` overloads return the current owning string and may reuse storage when profitable
- the partial `to_*case(pos, count)` overloads transform only the selected subrange and throw on invalid bounds

Complexity: linear in `size() + replacement_size * replaced_count`.

Example:

```cpp
auto s = u"Aé😀"_utf16_s;
s.replace_inplace(1, 1, u"Ω"_utf16_sv);
assert(s == u"AΩ😀"_utf16_sv);

s.replace_with_range_inplace(1, std::array{ u"β"_u16c, u"!"_u16c });
assert(s == u"Aβ!😀"_utf16_sv);
```

### Formatting, streaming, hashing

See [Reference: formatting, streaming, hashing](#reference-formatting-streaming-hashing).

## Reference: utf8_string

### Overview

`utf8_string` owns UTF-8 text.

It is allocator-aware and is backed by:

```cpp
std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>
```

It shares the same read-only API as `utf8_string_view`.

`utf8_string` also compares lexicographically by the underlying UTF-8 bytes, and it can be compared directly with `utf8_string_view`.

### Synopsis

```cpp
template<class Allocator = std::allocator<char8_t>>
class basic_utf8_string;

using utf8_string = basic_utf8_string<>;

namespace unicode_ranges::pmr
{
    using utf8_string =
        basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>;
}

template<class Allocator>
class basic_utf8_string
{
public:
    basic_utf8_string() = default;

    static constexpr auto from_bytes(std::string_view bytes,
                                     const Allocator& alloc = Allocator()) noexcept
        -> std::expected<basic_utf8_string, utf8_error>;
    static constexpr auto from_bytes(std::wstring_view bytes,
                                     const Allocator& alloc = Allocator()) noexcept;

    constexpr basic_utf8_string(utf8_string_view view,
                                const Allocator& alloc = Allocator());
    constexpr basic_utf8_string(utf16_string_view view,
                                const Allocator& alloc = Allocator());
    constexpr basic_utf8_string(std::size_t count, utf8_char ch,
                                const Allocator& alloc = Allocator());

    template<class R>
    constexpr basic_utf8_string(std::from_range_t, R&& rg,
                                const Allocator& alloc = Allocator());

    constexpr basic_utf8_string(std::initializer_list<utf8_char> ilist,
                                const Allocator& alloc = Allocator());

    template<class It, class Sent>
    constexpr basic_utf8_string(It it, Sent sent,
                                const Allocator& alloc = Allocator());

    constexpr basic_utf8_string& append_range(...);
    constexpr basic_utf8_string& assign_range(...);
    constexpr basic_utf8_string& append(std::size_t count, utf8_char ch);
    constexpr basic_utf8_string& assign(std::size_t count, utf8_char ch);
    constexpr basic_utf8_string& append(utf8_string_view sv);
    constexpr basic_utf8_string& assign(utf8_string_view sv);
    constexpr basic_utf8_string& assign(utf8_char ch);
    constexpr basic_utf8_string& append(It it, Sent sent);
    constexpr basic_utf8_string& assign(It it, Sent sent);
    constexpr basic_utf8_string& append(std::initializer_list<utf8_char> ilist);
    constexpr basic_utf8_string& assign(std::initializer_list<utf8_char> ilist);

    constexpr basic_utf8_string& operator=(utf8_string_view sv);
    constexpr basic_utf8_string& operator=(utf8_char ch);
    constexpr basic_utf8_string& operator=(std::initializer_list<utf8_char> ilist);
    constexpr basic_utf8_string& operator+=(utf8_string_view sv);
    constexpr basic_utf8_string& operator+=(utf16_string_view sv);
    constexpr basic_utf8_string& operator+=(utf8_char ch);
    constexpr basic_utf8_string& operator+=(utf16_char ch);
    constexpr basic_utf8_string& operator+=(std::initializer_list<utf8_char> ilist);

    constexpr void shrink_to_fit();
    constexpr std::size_t capacity() const;
    constexpr Allocator get_allocator() const noexcept;
    constexpr std::size_t size() const;
    constexpr std::optional<utf8_char> pop_back();
    constexpr basic_utf8_string& reverse() noexcept;
    constexpr basic_utf8_string& reverse(std::size_t pos,
                                         std::size_t count = npos);
    constexpr basic_utf8_string& insert(std::size_t index, utf8_string_view sv);
    constexpr basic_utf8_string& insert(std::size_t index, utf8_char ch);
    constexpr basic_utf8_string& insert(std::size_t index, std::size_t count,
                                        utf8_char ch);
    constexpr basic_utf8_string& insert_range(std::size_t index, R&& rg);
    constexpr basic_utf8_string& insert(std::size_t index, It it, Sent sent);
    constexpr basic_utf8_string& insert(std::size_t index,
                                        std::initializer_list<utf8_char> ilist);
    constexpr basic_utf8_string& erase(std::size_t index,
                                       std::size_t count = npos);
    constexpr basic_utf8_string& replace_inplace(std::size_t pos, std::size_t count,
                                         utf8_string_view other);
    constexpr basic_utf8_string& replace_inplace(std::size_t pos, std::size_t count,
                                         utf8_char other);
    constexpr basic_utf8_string& replace_inplace(std::size_t pos,
                                         utf8_string_view other);
    constexpr basic_utf8_string& replace_inplace(std::size_t pos,
                                         utf8_char other);
    constexpr basic_utf8_string& replace_with_range_inplace(std::size_t pos,
                                                    std::size_t count,
                                                    R&& rg);
    constexpr basic_utf8_string& replace_with_range_inplace(std::size_t pos,
                                                    R&& rg);
    constexpr basic_utf8_string replace_all(utf8_char from, utf8_char to) const&;
    constexpr basic_utf8_string replace_all(utf8_char from, utf8_string_view to) const&;
    constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_char to) const&;
    constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_string_view to) const&;
    constexpr basic_utf8_string replace_all(utf8_char from, utf8_char to) &&;
    constexpr basic_utf8_string replace_all(utf8_char from, utf8_string_view to) &&;
    constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_char to) &&;
    constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_string_view to) &&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_char to) const&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_string_view to) const&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_char to) const&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_string_view to) const&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_char to) &&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_string_view to) &&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_char to) &&;
    constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_string_view to) &&;
    constexpr basic_utf8_string to_ascii_lowercase() const&;
    constexpr basic_utf8_string to_ascii_lowercase(size_type pos, size_type count) const&;
    constexpr basic_utf8_string to_ascii_lowercase() &&;
    constexpr basic_utf8_string to_ascii_lowercase(size_type pos, size_type count) &&;
    constexpr basic_utf8_string to_ascii_uppercase() const&;
    constexpr basic_utf8_string to_ascii_uppercase(size_type pos, size_type count) const&;
    constexpr basic_utf8_string to_ascii_uppercase() &&;
    constexpr basic_utf8_string to_ascii_uppercase(size_type pos, size_type count) &&;
    constexpr basic_utf8_string to_lowercase() const&;
    constexpr basic_utf8_string to_lowercase(size_type pos, size_type count) const&;
    constexpr basic_utf8_string to_lowercase() &&;
    constexpr basic_utf8_string to_lowercase(size_type pos, size_type count) &&;
    constexpr basic_utf8_string to_uppercase() const&;
    constexpr basic_utf8_string to_uppercase(size_type pos, size_type count) const&;
    constexpr basic_utf8_string to_uppercase() &&;
    constexpr basic_utf8_string to_uppercase(size_type pos, size_type count) &&;
    constexpr basic_utf8_string normalize(normalization_form form) const&;
    constexpr basic_utf8_string normalize(normalization_form form) &&;
    constexpr basic_utf8_string to_nfc() const&;
    constexpr basic_utf8_string to_nfc() &&;
    constexpr basic_utf8_string to_nfd() const&;
    constexpr basic_utf8_string to_nfd() &&;
    constexpr basic_utf8_string to_nfkc() const&;
    constexpr basic_utf8_string to_nfkc() &&;
    constexpr basic_utf8_string to_nfkd() const&;
    constexpr basic_utf8_string to_nfkd() &&;
    constexpr basic_utf8_string case_fold() const&;
    constexpr basic_utf8_string case_fold() &&;
    constexpr void reserve(std::size_t new_cap);
    constexpr auto base() const& noexcept;
    constexpr auto base() && noexcept;
    constexpr void clear();
    constexpr const char8_t* data() const noexcept;
    constexpr const char8_t* c_str() const noexcept;
    constexpr operator utf8_string_view() const noexcept;
    constexpr utf8_string_view as_view() const noexcept;
    template<class Allocator = std::allocator<char16_t>>
    constexpr basic_utf16_string<Allocator>
        to_utf16(const Allocator& alloc = Allocator()) const;
    constexpr void push_back(utf8_char ch);
    constexpr void swap(basic_utf8_string& other) noexcept(...);

    friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs,
                                                 const basic_utf8_string& rhs);
    friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs,
                                                 const basic_utf8_string& rhs);
    friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs,
                                                 basic_utf8_string&& rhs);
    friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs,
                                                 basic_utf8_string&& rhs);
    friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs,
                                                 utf8_string_view rhs);
    friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs,
                                                 utf8_string_view rhs);
    friend constexpr basic_utf8_string operator+(utf8_string_view lhs,
                                                 const basic_utf8_string& rhs);
    friend constexpr basic_utf8_string operator+(utf8_string_view lhs,
                                                 basic_utf8_string&& rhs);
    friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs,
                                                 utf8_char rhs);
    friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs,
                                                 utf8_char rhs);
    friend constexpr basic_utf8_string operator+(utf8_char lhs,
                                                 const basic_utf8_string& rhs);
    friend constexpr basic_utf8_string operator+(utf8_char lhs,
                                                 basic_utf8_string&& rhs);
};
```

### Construction

Construction is available from:

- nothing
- a validated UTF-8 view
- a validated UTF-16 view, transcoded into UTF-8
- checked raw source text carried in `std::string_view` or `std::wstring_view`
- repeated `utf8_char`
- ranges of `utf8_char`
- iterator/sentinel pairs
- initializer lists of `utf8_char`

`from_bytes(...)` is the ergonomic bridge from `std::string` / `std::wstring` into the validated UTF-8 owning type, and both overloads accept an optional allocator.

Cross-encoding is also available through the direct `utf8_string(utf16_string_view, alloc)` constructor and `to_utf16()`.

Complexity:

- default construction: constant
- construction from `utf8_string_view` or `utf16_string_view`: linear in the source length
- `from_bytes(std::string_view)` / `from_bytes(std::wstring_view)`: linear in the source length
- range, iterator, and initializer-list construction: linear in the resulting string size

### Modifiers

`utf8_string` supports the usual STL-shaped mutation families:

- `append_range`, `append(It, Sent)`, `append(std::initializer_list<utf8_char>)`
- `assign_range`, `assign(It, Sent)`, `assign(std::initializer_list<utf8_char>)`
- `append(std::size_t count, utf8_char)`, `assign(std::size_t count, utf8_char)`
- `append(utf8_string_view)`, `assign(utf8_string_view)`, `assign(utf8_char)`
- `replace_inplace(...)`, `replace_with_range_inplace(...)`
- `replace_all(...)`, `replace_n(...)`
- `push_back(utf8_char)`
- `operator+=` with `utf8_string_view`, `utf16_string_view`, `utf8_char`, `utf16_char`, and `std::initializer_list<utf8_char>`

The native overloads operate on validated UTF-8 data. The cross-encoding `operator+=` overloads transcode through Unicode scalar values before appending.

> [!IMPORTANT]
> Self-referential insertion and replacement are supported and covered by the test suite.
> It is valid to append, insert, or replace in place using validated views, iterator pairs, or character ranges derived from the same owning string, such as `s.append(s.as_view())`, `s.append_range(s.chars())`, `s.insert(1, s.as_view())`, or `s.replace_inplace(0, 1, s.as_view())`.

Complexity:

- `append_range`, iterator-based `append`, and `append(std::initializer_list<utf8_char>)`: linear in the appended character count
- `assign_range`, iterator-based `assign`, and `assign(std::initializer_list<utf8_char>)`: linear in the replacement size
- `append(std::size_t count, utf8_char)` and `assign(std::size_t count, utf8_char)`: linear in `count`
- `append(utf8_string_view)`, `assign(utf8_string_view)`, and `operator+=` on text/string arguments: linear in the appended or assigned text size
- `reverse(...)`: linear in the reversed substring size
- `replace_inplace(...)`, `replace_with_range_inplace(...)`, `replace_all(...)`, and `replace_n(...)`: linear in `size() + replacement_size`
- `assign(utf8_char)` and `operator+=` on single-character arguments: amortized constant
- `push_back(utf8_char)`: amortized constant

#### Insert

```cpp
constexpr basic_utf8_string& insert(std::size_t index, utf8_string_view sv);
constexpr basic_utf8_string& insert(std::size_t index, utf8_char ch);
constexpr basic_utf8_string& insert(std::size_t index, std::size_t count, utf8_char ch);
constexpr basic_utf8_string& insert_range(std::size_t index, R&& rg);
constexpr basic_utf8_string& insert(std::size_t index, It it, Sent sent);
constexpr basic_utf8_string& insert(std::size_t index, std::initializer_list<utf8_char> ilist);
```

Inserts validated UTF-8 text or UTF-8 characters at byte index `index`.

Supported inputs:

- `utf8_string_view`
- one `utf8_char`
- repeated `utf8_char`
- ranges of `utf8_char`
- iterator/sentinel pairs
- initializer lists of `utf8_char`

Requirements:

- `index <= size()`
- `index` must name a UTF-8 character boundary

Throws:

- `std::out_of_range` if `index` is greater than `size()`
- `std::out_of_range` if `index` does not name a UTF-8 character boundary

Complexity: linear in `size() + inserted_size`.

#### Pop Back

```cpp
constexpr std::optional<utf8_char> pop_back();
```

Removes and returns the last UTF-8 character.

Returns `std::nullopt` if the string is empty.

Complexity: constant.

#### Erase

```cpp
constexpr basic_utf8_string& erase(std::size_t index, std::size_t count = npos);
```

Erases the byte range beginning at `index` and extending for at most `count` bytes, clamped to the end of the string.

Requirements:

- `index <= size()`
- `index` must be a UTF-8 character boundary
- `index + min(count, size() - index)` must also be a UTF-8 character boundary

Throws:

- `std::out_of_range` if `index` is greater than `size()`
- `std::out_of_range` if the erased range does not align to UTF-8 character boundaries

Complexity: linear in `size()`.

#### Reverse

```cpp
constexpr basic_utf8_string& reverse() noexcept;
constexpr basic_utf8_string& reverse(std::size_t pos, std::size_t count = npos);
```

Reverses UTF-8 characters in place.

Behavior:

- `reverse()` reverses the whole string and is `noexcept`
- `reverse(pos, count)` reverses the substring `[pos, pos + count)`
- `count == npos` reverses from `pos` to the end of the string
- reversal is character-aware, so multibyte UTF-8 characters stay intact

Requirements:

- `pos <= size()`
- `count == npos` or `count <= size() - pos`
- `pos` must be a UTF-8 character boundary
- `pos + resolved_count` must also be a UTF-8 character boundary

Throws:

- `std::out_of_range` if `pos` is greater than `size()`
- `std::out_of_range` if an explicit `count` extends past the end of the string
- `std::out_of_range` if the reversed range does not align to UTF-8 character boundaries

Complexity: linear in the reversed substring size.

```cpp
auto s = u8"Aé😀"_utf8_s;
s.reverse();
assert(s == u8"😀éA"_utf8_sv);

auto part = u8"AéB😀C"_utf8_s;
part.reverse(1, 7);
assert(part == u8"A😀BéC"_utf8_sv);
```

#### Replace In Place

```cpp
constexpr basic_utf8_string& replace_inplace(std::size_t pos, std::size_t count, utf8_string_view other);
constexpr basic_utf8_string& replace_inplace(std::size_t pos, std::size_t count, utf8_char other);
constexpr basic_utf8_string& replace_inplace(std::size_t pos, utf8_string_view other);
constexpr basic_utf8_string& replace_inplace(std::size_t pos, utf8_char other);
constexpr basic_utf8_string& replace_with_range_inplace(std::size_t pos, std::size_t count, R&& rg);
constexpr basic_utf8_string& replace_with_range_inplace(std::size_t pos, R&& rg);
```

Replaces a UTF-8 substring with validated UTF-8 text, a single UTF-8 character, or a range of UTF-8 characters.

Supported replacements:

- validated UTF-8 text via `replace_inplace(..., utf8_string_view)`
- one character via `replace_inplace(..., utf8_char)`
- character ranges via `replace_with_range_inplace(...)`

Requirements:

- `pos <= size()`
- `pos` must be a UTF-8 character boundary
- `pos + min(count, size() - pos)` must also be a UTF-8 character boundary when a counted overload is used

The single-position overloads replace exactly one UTF-8 character beginning at `pos`.

Throws:

- `std::out_of_range` if `pos` is out of range
- `std::out_of_range` if the replaced range does not align to UTF-8 character boundaries

Complexity:

- `replace_inplace(..., utf8_string_view)`: linear in `size() + other.size()`
- `replace_inplace(..., utf8_char)`: linear in `size()`
- `replace_with_range_inplace(...)`: linear in `size() + replacement_size`

#### Replace All, Replace N

```cpp
constexpr basic_utf8_string replace_all(utf8_char from, utf8_char to) const&;
constexpr basic_utf8_string replace_all(utf8_char from, utf8_string_view to) const&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_char to) const&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_string_view to) const&;
template<class Pred>
constexpr basic_utf8_string replace_all(Pred pred, utf8_char to) const&;
template<class Pred>
constexpr basic_utf8_string replace_all(Pred pred, utf8_string_view to) const&;
template<class Allocator>
constexpr basic_utf8_string<Allocator> replace_all(..., const Allocator& alloc) const&;
constexpr basic_utf8_string replace_all(utf8_char from, utf8_char to) &&;
constexpr basic_utf8_string replace_all(utf8_char from, utf8_string_view to) &&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_char to) &&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_string_view to) &&;
template<class Pred>
constexpr basic_utf8_string replace_all(Pred pred, utf8_char to) &&;
template<class Pred>
constexpr basic_utf8_string replace_all(Pred pred, utf8_string_view to) &&;
template<class Allocator>
constexpr basic_utf8_string<Allocator> replace_all(..., const Allocator& alloc) &&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_char to) const&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_string_view to) const&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_char to) const&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_string_view to) const&;
template<class Pred>
constexpr basic_utf8_string replace_n(std::size_t count, Pred pred, utf8_char to) const&;
template<class Pred>
constexpr basic_utf8_string replace_n(std::size_t count, Pred pred, utf8_string_view to) const&;
template<class Allocator>
constexpr basic_utf8_string<Allocator> replace_n(..., const Allocator& alloc) const&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_char to) &&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_char from, utf8_string_view to) &&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_char to) &&;
constexpr basic_utf8_string replace_n(std::size_t count, utf8_string_view from, utf8_string_view to) &&;
template<class Pred>
constexpr basic_utf8_string replace_n(std::size_t count, Pred pred, utf8_char to) &&;
template<class Pred>
constexpr basic_utf8_string replace_n(std::size_t count, Pred pred, utf8_string_view to) &&;
template<class Allocator>
constexpr basic_utf8_string<Allocator> replace_n(..., const Allocator& alloc) &&;
```

Returns a new owning string after replacing non-overlapping matches.

Behavior:

- `replace_all(...)` replaces every non-overlapping match
- `replace_n(...)` replaces at most `count` non-overlapping matches from left to right
- `utf8_char` and `utf8_string_view` overloads replace exact validated UTF-8 matches
- predicate overloads replace each matching character independently
- empty `utf8_string_view` search patterns are a no-op
- `const&` overloads leave the source string unchanged
- `&&` overloads return the current owning string and may reuse storage when profitable
- allocator-taking overloads construct the result with the supplied allocator

Complexity: linear in `size() + replacement_size * replaced_count`.

```cpp
using namespace unicode_ranges::literals;

const auto view = u8"bananas"_utf8_sv;
assert(view.replace_all(u8"na"_utf8_sv, u8"NO"_utf8_sv) == u8"baNONOs"_utf8_s);
assert(view.replace_n(1, u8"na"_utf8_sv, u8"NO"_utf8_sv) == u8"baNOnas"_utf8_s);

assert(
    u8"123-456"_utf8_sv.replace_all(
        [](unicode_ranges::utf8_char ch) { return ch >= u8"0"_u8c && ch <= u8"9"_u8c; },
        u8"x"_u8c)
    == u8"xxx-xxx"_utf8_s);

auto owned = u8"banana"_utf8_s;
assert(std::move(owned).replace_all(u8"na"_utf8_sv, u8"NO"_utf8_sv) == u8"baNONO"_utf8_s);

std::pmr::monotonic_buffer_resource memory;
auto pmr_result = view.replace_all(
    u8"a"_u8c,
    u8"o"_u8c,
    std::pmr::polymorphic_allocator<char8_t>{ &memory });
assert(pmr_result.get_allocator().resource() == &memory);
```

### Concatenation

`utf8_string` supports `operator+` with:

- `utf8_string_view`
- `utf8_char`

on either side when one operand is an owning `utf8_string`.

Complexity:

- Linear in `lhs.size() + rhs.size()`

### Observers and conversions

- `base()` exposes the underlying `std::basic_string<char8_t, ...>`
- `get_allocator()` returns the stored allocator
- `as_view()` returns an unchecked `utf8_string_view`
- `operator utf8_string_view()` converts to a view
- `to_utf16()` transcodes into an owning UTF-16 string, with optional allocator control
- `data()` and `c_str()` expose the contiguous byte storage

> [!WARNING]
> `as_view()` and the inherited read-only range helpers such as `chars()` and `graphemes()` borrow from the string's internal storage.
> They become invalid if the string is destroyed, and they should not be kept across mutation.

Complexity:

- `base()`, `get_allocator()`, `as_view()`, conversion to `utf8_string_view`, `data()`, and `c_str()`: constant
- `to_utf16()`: linear in `size()`

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

utf8_string s = "Aé€"_utf8_s;
s.push_back("!"_u8c);
s.erase(1, 2); // removes "é"
s.replace_inplace(1, "Ω"_u8c); // replaces '€' with 'Ω'
s.replace_with_range_inplace(1, std::array{ "β"_u8c, "!"_u8c });

assert(s.char_count() == 4);
assert(s == "Aβ!!"_utf8_sv);
assert(s.ends_with("!"_u8c));
assert(s.to_utf16() == u"AÎ²!!"_utf16_sv);
```

## Reference: views

The library provides forward, reverse, grapheme-oriented, and lossy views for both UTF-8 and UTF-16 text.

### `unicode_ranges::views::utf8_view`

Unchecked forward view over valid UTF-8 bytes.

Construction:

```cpp
static constexpr utf8_view from_bytes_unchecked(std::u8string_view base) noexcept;
```

> [!CAUTION]
> This constructor is unchecked. Passing invalid UTF-8 is undefined behavior.

Semantics:

- the input must already be valid UTF-8
- this is a forward range
- this is not a common range
- dereferencing yields `utf8_char` by value
- increment uses the current UTF-8 lead byte to advance

Complexity:

- Constant to construct the view
- Linear to iterate the full view

### `unicode_ranges::views::utf16_view`

Unchecked forward view over valid UTF-16 code units.

Construction:

```cpp
static constexpr utf16_view from_code_units_unchecked(std::u16string_view base) noexcept;
```

> [!CAUTION]
> This constructor is unchecked. Passing invalid UTF-16 is undefined behavior.

Semantics:

- the input must already be valid UTF-16
- this is a forward range
- this is not a common range
- dereferencing yields `utf16_char` by value
- increment advances by one code unit for BMP scalars and by two code units for surrogate pairs

Complexity:

- Constant to construct the view
- Linear to iterate the full view

Example:

```cpp
constexpr std::u16string_view text = u"Aé😀";
const auto view = views::utf16_view::from_code_units_unchecked(text);

assert(std::ranges::equal(view, std::array{ u"A"_u16c, u"é"_u16c, u"😀"_u16c }));
```

### `unicode_ranges::views::reversed_utf16_view`

Unchecked forward view that yields the same UTF-16 characters in reverse order.

Construction:

```cpp
static constexpr reversed_utf16_view from_code_units_unchecked(std::u16string_view base) noexcept;
```

> [!CAUTION]
> This constructor is unchecked. Passing invalid UTF-16 is undefined behavior.

Semantics:

- the input must already be valid UTF-16
- this is a forward range
- this is not a common range
- dereferencing yields `utf16_char` by value
- `begin()` positions at the last UTF-16 character
- increment walks backward to the previous UTF-16 character boundary, including surrogate-pair starts

Complexity:

- Constant to construct the view
- Linear to iterate the full view

Example:

```cpp
constexpr std::u16string_view text = u"Aé😀";
const auto view = views::reversed_utf16_view::from_code_units_unchecked(text);

assert(std::ranges::equal(view, std::array{ u"😀"_u16c, u"é"_u16c, u"A"_u16c }));
```

### `unicode_ranges::views::reversed_utf8_view`

Unchecked forward view that yields the same characters in reverse order.

Construction:

```cpp
static constexpr reversed_utf8_view from_bytes_unchecked(std::u8string_view base) noexcept;
```

> [!CAUTION]
> This constructor is unchecked. Passing invalid UTF-8 is undefined behavior.

Semantics:

- the input must already be valid UTF-8
- this is a forward range
- this is not a common range
- dereferencing yields `utf8_char` by value
- `begin()` positions at the last UTF-8 character
- increment walks backward to the previous UTF-8 lead byte

Complexity:

- Constant to construct the view
- Linear to iterate the full view

### `unicode_ranges::views::grapheme_cluster_view<CharT>`

Unchecked forward view over default Unicode grapheme clusters.

In normal use, you typically obtain this view through `text.graphemes()` on a UTF-8 or UTF-16 string/view type.

> [!CAUTION]
> The standalone constructor is unchecked. Passing invalid source text for the chosen encoding is undefined behavior.

`CharT` is either:

- `char8_t`, yielding `utf8_string_view`
- `char16_t`, yielding `utf16_string_view`

Semantics:

- the input must already be valid for the chosen encoding
- this is a forward range
- this is not a common range
- dereferencing yields one borrowed string-view cluster at a time
- cluster boundaries follow the default extended grapheme-cluster rules from UAX #29

Complexity:

- Constant to construct the view
- Linear to iterate the full view

Example:

```cpp
constexpr auto text = u8"👩‍💻!"_utf8_sv;

static_assert(std::ranges::equal(text.graphemes(), std::array{
    u8"👩‍💻"_grapheme_utf8,
    u8"!"_grapheme_utf8
}));
```

When formatting a grapheme range, `"{::s}"` is usually the most readable presentation because it formats each cluster as text rather than as an escaped debug string.

### `unicode_ranges::views::lossy_utf8_view<CharT>`

Lossy forward view over arbitrary integral code units.

Construction:

```cpp
constexpr lossy_utf8_view(std::basic_string_view<CharT> base) noexcept;
```

Semantics:

- valid UTF-8 subsequences are decoded into `utf8_char`
- malformed input yields `utf8_char::replacement_character`
- the iterator advances by the valid character width, or by one code unit when replacement was produced

Complexity:

- Constant to construct the view
- Linear to iterate the full view

### `unicode_ranges::views::lossy_utf8`

Adaptor closure for `lossy_utf8_view`.

Malformed subsequences become `utf8_char::replacement_character`, while valid ASCII bytes still decode normally.

Example:

```cpp
const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
assert(std::ranges::equal(
    input | views::lossy_utf8,
    std::array{
        "A"_u8c,
        utf8_char::replacement_character,
        utf8_char::replacement_character,
        "("_u8c,
        utf8_char::replacement_character
    }));
```

### `unicode_ranges::views::lossy_utf16_view<CharT>`

Lossy forward view over arbitrary UTF-16 code units.

Construction:

```cpp
constexpr lossy_utf16_view(std::basic_string_view<CharT> base) noexcept;
```

Semantics:

- valid UTF-16 code unit sequences are decoded into `utf16_char`
- malformed input yields `utf16_char::replacement_character`
- the iterator advances by two code units for a valid surrogate pair, otherwise by one code unit

Complexity:

- Constant to construct the view
- Linear to iterate the full view

### `unicode_ranges::views::lossy_utf16`

Adaptor closure for `lossy_utf16_view`.

Malformed surrogate structure becomes `utf16_char::replacement_character`, while valid code units continue to decode normally.

Example:

```cpp
const std::u16string input{
    static_cast<char16_t>(u'A'),
    static_cast<char16_t>(0xD800), // invalid lone high surrogate
    static_cast<char16_t>(u'B'),
    static_cast<char16_t>(0xD83D),
    static_cast<char16_t>(0xDE00)  // 😀
};

assert(std::ranges::equal(
    input | views::lossy_utf16,
    std::array{
        u"A"_u16c,
        utf16_char::replacement_character,
        u"B"_u16c,
        u"😀"_u16c
    }));
```

## Reference: literals

### `operator ""_u8c`

Constructs a `utf8_char` from a string literal containing exactly one valid UTF-8 character.

Validation is performed at compile time.

Examples:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr utf8_char a = "A"_u8c;
constexpr utf8_char e_acute = "é"_u8c;
constexpr utf8_char euro = "€"_u8c;
```

### `operator ""_utf8_sv`

Constructs a `utf8_string_view` from a UTF-8 string literal.

Validation is performed at compile time.

Examples:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto a = "Aé€"_utf8_sv;
constexpr auto b = "Aé€"_utf8_sv;
```

### `operator ""_utf16_sv`

Constructs a `utf16_string_view` from a UTF-16 string literal.

Validation is performed at compile time.

Examples:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto text = u"Aé😀"_utf16_sv;
static_assert(text.size() == 4);
static_assert(text.char_count() == 3);
```

### `operator ""_utf8_s`

Constructs a `utf8_string` from a UTF-8 string literal.

The literal contents are validated before the owning string is produced.

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

const auto owned = "Hello"_utf8_s;
```

### `operator ""_utf16_s`

Constructs a `utf16_string` from a UTF-16 string literal.

The literal contents are validated before the owning string is produced.

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

const auto owned = u"Hello"_utf16_s;
```

### `operator ""_grapheme_utf8`

Constructs a `utf8_string_view` from a UTF-8 string literal containing exactly one default Unicode grapheme cluster.

Both UTF-8 validation and grapheme-cluster validation are performed at compile time.

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto accented = u8"e\u0301"_grapheme_utf8;
constexpr auto technologist = u8"👩‍💻"_grapheme_utf8;
```

### `operator ""_grapheme_utf16`

Constructs a `utf16_string_view` from a UTF-16 string literal containing exactly one default Unicode grapheme cluster.

Both UTF-16 validation and grapheme-cluster validation are performed at compile time.

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

constexpr auto accented = u"e\u0301"_grapheme_utf16;
constexpr auto technologist = u"👩‍💻"_grapheme_utf16;
```

## Reference: formatting, streaming, hashing

### `utf8_char`

Supported:

- `std::ostream << utf8_char`
- `std::hash<utf8_char>`
- `std::formatter<utf8_char, char>`
- `std::formatter<utf8_char, wchar_t>`

Formatting supports character presentation and scalar numeric presentation.

Examples:

```cpp
assert(std::format("{}", "A"_u8c) == "A");
assert(std::format("{:c}", "€"_u8c) == "€");
assert(std::format("{:>4c}", "€"_u8c) == "   €");
assert(std::format("{:*^5c}", "é"_u8c) == "**é**");
assert(std::format("{:#06x}", "A"_u8c) == "0x0041");
assert(std::format("{:#010b}", "A"_u8c) == "0b01000001");
assert(std::format(L"{}", "€"_u8c) == L"€");
```

### `utf16_char`

Supported:

- `std::ostream << utf16_char`
- `std::hash<utf16_char>`
- `std::formatter<utf16_char, char>`
- `std::formatter<utf16_char, wchar_t>`

Formatting follows the same character and scalar numeric presentation rules as `utf8_char`.

Examples:

```cpp
assert(std::format("{}", u"€"_u16c) == "€");
assert(std::format("{:x}", u"€"_u16c) == "20ac");
assert(std::format(L"{}", u"😀"_u16c) == L"😀");
```

### `utf8_string_view`

Supported:

- `std::ostream << utf8_string_view`
- `std::hash<utf8_string_view>`
- `std::formatter<utf8_string_view, char>`

Formatting delegates to the existing `std::string_view` formatter over the underlying UTF-8 bytes reinterpreted as `char`.

Example:

```cpp
constexpr auto text = "Aé€"_utf8_sv;
assert(std::format("{}", text) == "Aé€");
assert(std::format("{:>6}", text) == "   Aé€");
assert(std::format("{:_<6}", text) == "Aé€___");
```

### `utf16_string_view`

Supported:

- `std::ostream << utf16_string_view`
- `std::hash<utf16_string_view>`
- `std::formatter<utf16_string_view, char>`

Formatting and narrow streaming encode the UTF-16 contents as UTF-8 text.

Example:

```cpp
constexpr auto text = u"Aé😀"_utf16_sv;
assert(std::format("{}", text) == "Aé😀");

std::ostringstream oss;
oss << text;
assert(oss.str() == "Aé😀");
```

### `utf16_string`

Supported:

- `std::ostream << utf16_string`
- `std::formatter<utf16_string, char>`

Formatting and narrow streaming encode the UTF-16 contents as UTF-8 text.

Example:

```cpp
const utf16_string text = u"Aé😀"_utf16_s;
assert(std::format("{}", text) == "Aé😀");

std::ostringstream oss;
oss << text;
assert(oss.str() == "Aé😀");
```

## Semantics notes

### 1. Code-unit-oriented versus character-oriented APIs

This is the most important rule when using the string types.

Code-unit-oriented:

- `size()`
- `is_char_boundary()`
- `split()`
- `substr()`

Character-oriented:

- `char_count()`
- `char_indices()`
- `char_at()`
- `front()`
- `back()`

`char_indices()` and `grapheme_indices()` still report underlying buffer positions:

- byte offsets for UTF-8 text
- code-unit offsets for UTF-16 text

### 2. Checked versus unchecked APIs

Checked:

- `utf8_string_view::from_bytes`
- `utf16_string_view::from_code_units`
- compile-time string and character literals

Unchecked:

- `utf8_string_view::from_bytes_unchecked`
- `utf16_string_view::from_code_units_unchecked`
- `views::utf8_view::from_bytes_unchecked`
- `views::utf16_view::from_code_units_unchecked`
- `views::reversed_utf8_view::from_bytes_unchecked`
- `views::reversed_utf16_view::from_code_units_unchecked`
- `utf8_char::from_scalar_unchecked`
- `utf8_char::from_utf8_bytes_unchecked`

Unchecked APIs are intended for performance-sensitive cases where UTF-8 or UTF-16 validity falls on the caller. Passing invalid data to an unchecked API is undefined behavior.

### 3. Unicode predicates are table-driven

The broad predicates:

- `is_alphabetic`
- `is_alphanumeric`
- `is_control`
- `is_digit`
- `is_lowercase`
- `is_numeric`
- `is_uppercase`
- `is_whitespace`

use generated Unicode tables.

In particular, `is_digit()` follows Rust-style semantics and is intentionally narrower than `is_numeric()`.

### 4. ASCII predicates are exact and cheap

The `is_ascii_*` family does not consult Unicode tables and always returns `false` for non-ASCII code points.

### 5. Grapheme segmentation follows default Unicode rules

`utf8_char` and `utf16_char` still represent single Unicode scalar values. When you need user-perceived characters, use `graphemes()` to generate a `views::grapheme_cluster_view<CharT>`, which segments text according to the default extended grapheme-cluster rules from UAX #29.

Unlike `is_char_boundary()`, grapheme-boundary checks are not naturally constant-time. A grapheme boundary depends on surrounding scalar context, so it cannot be answered by a simple encoding-form test alone.
