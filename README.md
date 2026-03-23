# unicode_ranges (work in progress)

`unicode_ranges` is a header-only C++ library for representing, validating, iterating, and formatting UTF-8 and UTF-16 text in C++.

It provides validated UTF-8 and UTF-16 character, view, string, and range abstractions for modern C++.

The current public building blocks are:

- `utf8_char`: one Unicode scalar value stored as its encoded UTF-8 byte sequence, guaranteed to represent a valid UTF-8 character
- `utf16_char`: one Unicode scalar value stored as its encoded UTF-16 code units, guaranteed to represent a valid UTF-16 character
- `utf8_string_view`: borrowed UTF-8 byte view, similar to `std::string_view` but guaranteed to represent a valid UTF-8 string slice
- `utf16_string_view`: borrowed UTF-16 code-unit view, similar to `std::u16string_view` but guaranteed to represent a valid UTF-16 string slice
- `utf8_string`: an owning UTF-8 string, similar to `std::string` but guaranteed to store valid UTF-8
- `utf16_string`: an owning UTF-16 string, similar to `std::u16string` but guaranteed to store valid UTF-16
- `unicode_ranges::views::utf8_view`: lazy iteration over valid UTF-8
- `unicode_ranges::views::utf16_view`: lazy iteration over valid UTF-16
- `unicode_ranges::views::reversed_utf8_view`: lazy reverse-order iteration over valid UTF-8
- `unicode_ranges::views::reversed_utf16_view`: lazy reverse-order iteration over valid UTF-16
- `unicode_ranges::views::grapheme_cluster_view<CharT>`: lazy default Unicode grapheme segmentation over valid UTF-8 or UTF-16 text
- `unicode_ranges::views::lossy_utf8_view`: lossy iteration over arbitrary byte sequences
- `unicode_ranges::views::lossy_utf16_view`: lossy iteration over arbitrary UTF-16 code-unit sequences

The public entry point is:

```cpp
#include "unicode_ranges.hpp"
```

The public umbrella header is named `unicode_ranges.hpp`.

All public library types and functions live in namespace `unicode_ranges`.

Literal operators live in the nested namespace `unicode_ranges::literals`.

The nested namespace `unicode_ranges::details` contains implementation details only. It is not part of the supported public API and should not be used directly by library users.

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

Default Unicode grapheme segmentation is supported through `graphemes()`, while locale-specific tailoring and higher-level text layout remain out of scope.

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

## Unicode version

Unicode property predicates are versioned. The library exposes:

```cpp
inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version;
```

This constant aliases the generated Unicode version used by the Unicode property tables.

Those tables are generated from versioned Unicode data files. In particular, the grapheme-segmentation data is sourced from the official Unicode Character Database rather than inferred from ad-hoc predicates.

The update workflow is:

1. refresh the raw Unicode data under `tools/unicode_data/<version>/`
2. rerun `tools/regenerate_unicode_tables.ps1`
3. commit the regenerated `utf8_ranges/unicode_tables.hpp`
4. update the changelog and any affected documentation

Example:

```cpp
static_assert(std::get<0>(unicode_ranges::unicode_version) == 17);
static_assert(std::get<1>(unicode_ranges::unicode_version) == 0);
static_assert(std::get<2>(unicode_ranges::unicode_version) == 0);
```

## Quick start

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

// Cheap interoperability with their raw char8_t equivalent.
static_assert(text.as_view() == u8"café €");

// `size()` counts UTF-8 code units.
static_assert(text.size() == 9);
static_assert(text.as_view() == std::u8string{ 0x63, 0x61, 0x66, 0xC3, 0xA9, 0x20, 0xE2, 0x82, 0xAC });

// `char_count()` counts Unicode scalar values.
static_assert(text.char_count() == 6);
static_assert(std::ranges::equal(text.chars(), std::array{ "c"_u8c, "a"_u8c, "f"_u8c, "é"_u8c, " "_u8c, "€"_u8c }));

// STL style APIs
static_assert(text.front() == 'c');
static_assert(text.back() == "€"_u8c);

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
    constexpr std::u8string_view as_view() const noexcept;
    constexpr utf8_string_view as_utf8_view() const noexcept;
    constexpr operator std::u8string_view() const noexcept;

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

#### `from_utf8_bytes_unchecked`

```cpp
template<class CharT>
static constexpr utf8_char from_utf8_bytes_unchecked(
    const CharT* bytes, std::size_t size) noexcept;
```

Constructs by copying an already valid 1-byte, 2-byte, 3-byte, or 4-byte UTF-8 sequence.

Precondition:

- `bytes[0, size)` represents exactly one valid UTF-8 encoded Unicode scalar value

### Conversion and observation

#### `as_scalar`

```cpp
constexpr std::uint32_t as_scalar() const noexcept;
```

Returns the Unicode scalar value represented by this character.

#### `as_view`

```cpp
constexpr std::u8string_view as_view() const noexcept;
```

Returns a `std::u8string_view` over the encoded UTF-8 bytes.

#### `as_utf8_view`

```cpp
constexpr utf8_string_view as_utf8_view() const noexcept;
```

Returns a validated string view of this single character.

#### `operator std::u8string_view`

```cpp
constexpr operator std::u8string_view() const noexcept;
```

Implicit conversion to `std::u8string_view`.

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
- `utf16_char` exposes `as_view()` as a `std::u16string_view`, while `utf8_char` exposes both `as_view()` and `as_utf8_view()`

It supports:

- checked and unchecked scalar construction
- checked and unchecked UTF-16 code-unit construction
- `std::u16string_view` interop via `as_view()` and implicit conversion
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
    constexpr std::u16string_view as_view() const noexcept;
    constexpr operator std::u16string_view() const noexcept;

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

#### `as_view`

```cpp
constexpr std::u16string_view as_view() const noexcept;
```

Returns a `std::u16string_view` over the stored UTF-16 code units.

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

    constexpr auto base() const noexcept;
    constexpr std::u8string_view as_view() const noexcept;
    constexpr operator std::u8string_view() const noexcept;
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

#### `from_bytes_unchecked`

```cpp
static constexpr utf8_string_view
    from_bytes_unchecked(std::u8string_view bytes) noexcept;
```

Constructs without validation.

Use only if the input is already known to be valid UTF-8.

#### `constexpr std::u8string_view as_view() const noexcept`

Returns the underlying `std::u8string_view`.

Complexity:

- Constant

### Read-only API

#### `chars`

```cpp
constexpr auto chars() const noexcept;
```

Returns a `unicode_ranges::views::utf8_view` over the contained Unicode scalar values.

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

#### `contains`

```cpp
constexpr bool contains(utf8_char ch) const noexcept;
```

Returns `true` if the UTF-8 byte sequence of `ch` occurs in the underlying byte sequence.

Preconditions:

- None

Complexity:

- Linear in `size()`

#### `find`

```cpp
constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf8_string_view sv, size_type pos = 0) const noexcept;
```

Returns the byte offset of the first match, or `npos` if no match exists.

The returned index is a byte offset into the underlying UTF-8 sequence.

For the `char8_t` case, `pos` is treated as a raw byte offset.

For the `utf8_char` and `utf8_string_view` overloads, `pos` is clamped to `size()` and rounded up to the next UTF-8 character boundary before searching.

Preconditions:

- None

Complexity:

- Linear in `size()`

#### `rfind`

```cpp
constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf8_string_view sv, size_type pos = npos) const noexcept;
```

Returns the byte offset of the last match, or `npos` if no match exists.

For the `char8_t` case, `pos` is treated as a raw byte offset.

For the `utf8_char` and `utf8_string_view` overloads, `pos` is clamped to `size()` and rounded down to a UTF-8 character boundary before searching.

Preconditions:

- None

Complexity:

- Linear in `size()`

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
constexpr std::pair<utf8_string_view, utf8_string_view>
    split(size_type delim) const;
```

Splits the view at byte index `delim` and returns the prefix and suffix as two `utf8_string_view` objects.

`delim` must be a UTF-8 character boundary.

Preconditions:

- `delim` is a UTF-8 character boundary

Throws:

- `std::out_of_range` if `delim` is not a UTF-8 character boundary

Complexity:

- Constant

#### `char_at`

```cpp
constexpr std::optional<utf8_char> char_at(size_type index) const noexcept;
constexpr utf8_char char_at_unchecked(size_type index) const noexcept;
```

Returns the character beginning at byte index `index`.

Returns `std::nullopt` if `index` is out of range or does not name a UTF-8 character boundary.

Preconditions:

- None

Complexity:

- Constant

#### `ceil_char_boundary`

```cpp
constexpr size_type ceil_char_boundary(size_type pos) const noexcept;
```

Clamps `pos` to `size()` and rounds it up to the next UTF-8 character boundary.

#### `floor_char_boundary`

```cpp
constexpr size_type floor_char_boundary(size_type pos) const noexcept;
```

Clamps `pos` to `size()` and rounds it down to the previous UTF-8 character boundary.

#### `char_at_unchecked`

```cpp
constexpr utf8_char char_at_unchecked(size_type index) const noexcept;
```

Returns the character beginning at byte index `index` without checking that `index` is in range or lies on a UTF-8 character boundary.

Preconditions:

- `index < size()`
- `index` is a UTF-8 character boundary

Complexity:

- Constant

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
constexpr utf8_char front() const noexcept;
```

Returns the first character in the view.

Preconditions:

- `!empty()`

Remarks:

- Calling `front()` on an empty view is undefined behavior

Complexity:

- Constant

#### `back`

```cpp
constexpr utf8_char back() const noexcept;
```

Returns the last character in the view.

Preconditions:

- `!empty()`

Remarks:

- Calling `back()` on an empty view is undefined behavior

Complexity:

- Constant

#### `starts_with`

```cpp
constexpr bool starts_with(char ch) const noexcept;
constexpr bool starts_with(char8_t ch) const noexcept;
constexpr bool starts_with(utf8_char ch) const noexcept;
constexpr bool starts_with(utf8_string_view sv) const noexcept;
```

Returns `true` if the view begins with the supplied prefix.

For the `char8_t` overload, the comparison is against a single UTF-8 code unit. In practice this is mainly useful for ASCII prefixes; use `utf8_char` or `utf8_string_view` for general Unicode prefixes.

Preconditions:

- None

Complexity:

- Constant for `char`, `char8_t`, and `utf8_char`
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
- `grapheme_indices()` returns `(code_unit_offset, utf16_string_view)` pairs
- `find(char16_t, pos)` is a raw code-unit search
- `find(utf16_char, pos)` and `find(utf16_string_view, pos)` round `pos` up to the next UTF-16 character boundary
- `rfind(utf16_char, pos)` and `rfind(utf16_string_view, pos)` round `pos` down to a UTF-16 character boundary
- `char_at(index)` returns `std::nullopt` if `index` is out of range or lies in the middle of a surrogate pair

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
    constexpr size_type size() const noexcept;
    constexpr bool empty() const noexcept;
    constexpr bool is_ascii() const noexcept;
    constexpr auto char_indices() const noexcept;

    constexpr bool contains(utf16_char ch) const noexcept;
    constexpr bool contains(utf16_string_view sv) const noexcept;

    constexpr size_type find(char16_t ch, size_type pos = 0) const noexcept;
    constexpr size_type find(utf16_char ch, size_type pos = 0) const noexcept;
    constexpr size_type find(utf16_string_view sv, size_type pos = 0) const noexcept;

    constexpr size_type rfind(char16_t ch, size_type pos = npos) const noexcept;
    constexpr size_type rfind(utf16_char ch, size_type pos = npos) const noexcept;
    constexpr size_type rfind(utf16_string_view sv, size_type pos = npos) const noexcept;

    constexpr bool is_char_boundary(size_type index) const noexcept;
    constexpr size_type char_count() const noexcept;
    constexpr std::pair<utf16_string_view, utf16_string_view> split(size_type delim) const;
    constexpr std::optional<utf16_char> char_at(size_type index) const noexcept;
    constexpr utf16_char char_at_unchecked(size_type index) const noexcept;
    constexpr std::optional<utf16_string_view> substr(size_type pos, size_type count = npos) const noexcept;
    constexpr utf16_char front() const noexcept;
    constexpr utf16_char back() const noexcept;
    constexpr bool starts_with(char16_t ch) const noexcept;
    constexpr bool starts_with(utf16_char ch) const noexcept;
    constexpr bool starts_with(utf16_string_view sv) const noexcept;
    constexpr bool ends_with(char16_t ch) const noexcept;
    constexpr bool ends_with(utf16_char ch) const noexcept;
    constexpr bool ends_with(utf16_string_view sv) const noexcept;
    constexpr size_type ceil_char_boundary(size_type pos) const noexcept;
    constexpr size_type floor_char_boundary(size_type pos) const noexcept;

    constexpr auto base() const noexcept;
    constexpr std::u16string_view as_view() const noexcept;
    constexpr operator std::u16string_view() const noexcept;
};
```

### Construction

Use `from_code_units(...)` when the source may be arbitrary UTF-16 and `from_code_units_unchecked(...)` only when validity is already guaranteed.

For compile-time construction from UTF-16 source text, prefer the `_utf16_sv` literal:

```cpp
constexpr auto text = u"Aé😀"_utf16_sv;
static_assert(text.size() == 4);
static_assert(text.char_count() == 3);
```

### Iteration and search

`chars()` yields `utf16_char` values in forward order.

`reversed_chars()` yields the same `utf16_char` values in reverse order.

`find` and `rfind` behave like the UTF-8 variants, but with UTF-16 code-unit indexing and surrogate-pair boundaries.

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

### Mutation APIs

The owning UTF-16 string supports the same mutation family as `utf8_string`:

- `append`, `append_range`
- `assign`, `assign_range`
- `insert`, `insert_range`
- `erase`
- `replace`
- `replace_with_range`
- `push_back`
- `operator+`

All positions and lengths are UTF-16 code-unit indices, and operations that take boundaries throw `std::out_of_range` when asked to split a surrogate pair.

Example:

```cpp
auto s = u"Aé😀"_utf16_s;
s.replace(1, 1, u"Ω"_utf16_sv);
assert(s == u"AΩ😀"_utf16_sv);

s.replace_with_range(1, std::array{ u"β"_u16c, u"!"_u16c });
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

template<class Allocator>
class basic_utf8_string
{
public:
    basic_utf8_string() = default;

    static constexpr basic_utf8_string from_bytes_unchecked(
        std::basic_string<char8_t, std::char_traits<char8_t>, Allocator> bytes) noexcept;
    static constexpr basic_utf8_string from_bytes_unchecked(
        std::u8string_view bytes, Allocator alloc) noexcept;

    constexpr basic_utf8_string(utf8_string_view view,
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

    constexpr void shrink_to_fit();
    constexpr std::size_t capacity() const;
    constexpr Allocator get_allocator() const noexcept;
    constexpr std::size_t size() const;
    constexpr void pop_back();
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
    constexpr basic_utf8_string& replace(std::size_t pos, std::size_t count,
                                         utf8_string_view other);
    constexpr basic_utf8_string& replace(std::size_t pos, std::size_t count,
                                         utf8_char other);
    constexpr basic_utf8_string& replace(std::size_t pos,
                                         utf8_string_view other);
    constexpr basic_utf8_string& replace(std::size_t pos,
                                         utf8_char other);
    constexpr basic_utf8_string& replace_with_range(std::size_t pos,
                                                    std::size_t count,
                                                    R&& rg);
    constexpr basic_utf8_string& replace_with_range(std::size_t pos,
                                                    R&& rg);
    constexpr void reserve(std::size_t new_cap);
    constexpr auto base() const& noexcept;
    constexpr auto base() && noexcept;
    constexpr void clear();
    constexpr const char8_t* data() const noexcept;
    constexpr const char8_t* c_str() const noexcept;
    constexpr operator utf8_string_view() const noexcept;
    constexpr utf8_string_view as_view() const noexcept;
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
- repeated `utf8_char`
- ranges of `utf8_char`
- iterator/sentinel pairs
- initializer lists of `utf8_char`

### Modifiers

#### `append_range`

Appends a range whose references are convertible to `utf8_char`.

#### `assign_range`

Replaces the contents with a range whose references are convertible to `utf8_char`.

#### `append(std::size_t count, utf8_char ch)`

Appends `count` copies of `ch`.

#### `append(utf8_string_view sv)`

Appends a validated UTF-8 string view.

#### `push_back(utf8_char ch)`

Appends one UTF-8 character.

#### `insert`

Insertion is supported for:

- `utf8_string_view`
- `utf8_char`
- repeated `utf8_char`
- ranges of `utf8_char`
- iterator/sentinel pairs
- initializer lists of `utf8_char`

The insertion index is a byte index and must name a UTF-8 character boundary.

Throws:

- `std::out_of_range` if the insertion index is greater than `size()`
- `std::out_of_range` if the insertion index is not a UTF-8 character boundary

#### `pop_back()`

Removes the last UTF-8 character by inspecting the reversed character view.

Precondition:

- `!empty()`

Remarks:

- Calling `pop_back()` on an empty string is undefined behavior

#### `erase(std::size_t index, std::size_t count = npos)`

Erases the byte range beginning at `index` and extending for at most `count` bytes, clamped to the end of the string.

The erased range must define a valid UTF-8 substring. In practice this means:

- `index <= size()`
- `index` must be a UTF-8 character boundary
- `index + min(count, size() - index)` must also be a UTF-8 character boundary

Throws:

- `std::out_of_range` if `index` is greater than `size()`
- `std::out_of_range` if the requested erased range does not align to UTF-8 character boundaries

#### `replace(std::size_t pos, std::size_t count, utf8_string_view other)`

Replaces the byte range beginning at `pos` and extending for at most `count` bytes, clamped to the end of the string, with `other`.

The replaced range must define a valid UTF-8 substring. In practice this means:

- `pos <= size()`
- `pos` must be a UTF-8 character boundary
- `pos + min(count, size() - pos)` must also be a UTF-8 character boundary

Throws:

- `std::out_of_range` if `pos` is greater than `size()`
- `std::out_of_range` if the requested replaced range does not align to UTF-8 character boundaries

#### `replace(std::size_t pos, std::size_t count, utf8_char other)`

Equivalent to replacing the validated UTF-8 substring with the UTF-8 encoding of `other`.

Throws:

- `std::out_of_range` under the same conditions as `replace(pos, count, utf8_string_view)`

#### `replace(std::size_t pos, utf8_string_view other)`

Replaces the single UTF-8 character beginning at byte index `pos` with `other`.

Throws:

- `std::out_of_range` if `pos` is out of range
- `std::out_of_range` if `pos` is not a UTF-8 character boundary

#### `replace(std::size_t pos, utf8_char other)`

Replaces the single UTF-8 character beginning at byte index `pos` with `other`.

Throws:

- `std::out_of_range` if `pos` is out of range
- `std::out_of_range` if `pos` is not a UTF-8 character boundary

#### `replace_with_range(std::size_t pos, std::size_t count, R&& rg)`

Replaces the byte range beginning at `pos` and extending for at most `count` bytes, clamped to the end of the string, with the UTF-8 encoding of the `utf8_char` range `rg`.

The replaced range must define a valid UTF-8 substring. In practice this means:

- `pos <= size()`
- `pos` must be a UTF-8 character boundary
- `pos + min(count, size() - pos)` must also be a UTF-8 character boundary

Throws:

- `std::out_of_range` if `pos` is greater than `size()`
- `std::out_of_range` if the requested replaced range does not align to UTF-8 character boundaries

#### `replace_with_range(std::size_t pos, R&& rg)`

Replaces the single UTF-8 character beginning at byte index `pos` with the UTF-8 encoding of the `utf8_char` range `rg`.

Throws:

- `std::out_of_range` if `pos` is out of range
- `std::out_of_range` if `pos` is not a UTF-8 character boundary

### Concatenation

`utf8_string` supports `operator+` with:

- `utf8_string_view`
- `utf8_char`

on either side when one operand is an owning `utf8_string`.

### Observers and conversions

- `base()` exposes the underlying `std::basic_string<char8_t, ...>`
- `get_allocator()` returns the stored allocator
- `as_view()` returns an unchecked `utf8_string_view`
- `operator utf8_string_view()` converts to a view
- `data()` and `c_str()` expose the contiguous byte storage

Example:

```cpp
using namespace unicode_ranges;
using namespace unicode_ranges::literals;

utf8_string s = "Aé€"_utf8_s;
s.push_back("!"_u8c);
s.erase(1, 2); // removes "é"
s.replace(1, "Ω"_u8c); // replaces '€' with 'Ω'
s.replace_with_range(1, std::array{ "β"_u8c, "!"_u8c });

assert(s.char_count() == 4);
assert(s == "Aβ!!"_utf8_sv);
assert(s.ends_with("!"_u8c));
```

## Reference: views

The view types are split by encoding:

- UTF-8 views are declared in `utf8_ranges/utf8_views.hpp`
- UTF-16 views are declared in `utf8_ranges/utf16_views.hpp`

### `unicode_ranges::views::utf8_view`

Unchecked forward view over valid UTF-8 bytes.

Construction:

```cpp
static constexpr utf8_view from_bytes_unchecked(std::u8string_view base) noexcept;
```

Semantics:

- the input must already be valid UTF-8
- this is a forward range
- this is not a common range
- dereferencing yields `utf8_char` by value
- increment uses the current UTF-8 lead byte to advance

### `unicode_ranges::views::utf16_view`

Unchecked forward view over valid UTF-16 code units.

Construction:

```cpp
static constexpr utf16_view from_code_units_unchecked(std::u16string_view base) noexcept;
```

Semantics:

- the input must already be valid UTF-16
- this is a forward range
- this is not a common range
- dereferencing yields `utf16_char` by value
- increment advances by one code unit for BMP scalars and by two code units for surrogate pairs

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

Semantics:

- the input must already be valid UTF-16
- this is a forward range
- this is not a common range
- dereferencing yields `utf16_char` by value
- `begin()` positions at the last UTF-16 character
- increment walks backward to the previous UTF-16 character boundary, including surrogate-pair starts

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

Semantics:

- the input must already be valid UTF-8
- this is a forward range
- this is not a common range
- dereferencing yields `utf8_char` by value
- `begin()` positions at the last UTF-8 character
- increment walks backward to the previous UTF-8 lead byte

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

### `unicode_ranges::views::lossy_utf8`

Adaptor closure for `lossy_utf8_view`.

Example:

```cpp
const std::string input{ "A\xFF\xE2\x28\xA1", 5 };
std::string decoded;

for (utf8_char ch : input | views::lossy_utf8)
{
    ch.encode_utf8<char>(std::back_inserter(decoded));
}

assert(decoded == "A\xEF\xBF\xBD\xEF\xBF\xBD(\xEF\xBF\xBD");
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

### `unicode_ranges::views::lossy_utf16`

Adaptor closure for `lossy_utf16_view`.

Example:

```cpp
const std::u16string input{
    static_cast<char16_t>(u'A'),
    static_cast<char16_t>(0xD800), // invalid lone high surrogate
    static_cast<char16_t>(u'B'),
    static_cast<char16_t>(0xD83D),
    static_cast<char16_t>(0xDE00)  // 😀
};

std::string decoded;
for (utf16_char ch : input | views::lossy_utf16)
{
    ch.encode_utf8<char>(std::back_inserter(decoded));
}

assert(decoded == "A\xEF\xBF\xBD" "B\xF0\x9F\x98\x80");
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

Unchecked APIs are intended for cases where validity is already guaranteed by construction.

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

`utf8_char` and `utf16_char` still represent single Unicode scalar values. When you need user-perceived characters, use `graphemes()` or `views::grapheme_cluster_view<CharT>`, which segment text according to the default extended grapheme cluster rules from UAX #29.
