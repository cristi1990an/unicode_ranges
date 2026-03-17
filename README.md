# utf8_ranges (work in progress)

`utf8_ranges` is a header-only C++ library for representing, validating, iterating, and formatting UTF-8 text in C++.

The library models UTF-8 text in a few distinct layers:

- `utf8_char`: one Unicode scalar value stored as its encoded UTF-8 byte sequence, guaranteed to represent a valid UTF-8 character
- `utf8_string_view`: borrowed UTF-8 byte view, similar to `std::string_view` but guaranteed to represent a valid UTF-8 string slice
- `utf8_string`: an owning UTF-8 string, similar to `std::string` but guaranteed to store valid UTF-8
- `views::utf8_view`: lazy iteration over valid UTF-8
- `views::reversed_utf8_view`: lazy reverse-order iteration over valid UTF-8
- `views::lossy_utf8_view`: lossy iteration over arbitrary byte sequences

The public entry point is:

```cpp
#include "utf8_ranges.hpp"
```

## Contents

1. [Goals](#goals)
2. [Requirements](#requirements)
3. [Unicode version](#unicode-version)
4. [Quick start](#quick-start)
5. [Error model](#error-model)
6. [Reference: utf8_char](#reference-utf8_char)
7. [Reference: utf8_string_view](#reference-utf8_string_view)
8. [Reference: utf8_string](#reference-utf8_string)
9. [Reference: views](#reference-views)
10. [Reference: literals](#reference-literals)
11. [Reference: formatting, streaming, hashing](#reference-formatting-streaming-hashing)
12. [Semantics notes](#semantics-notes)

## Goals

This library is built around a few explicit design choices:

- UTF-8 is the canonical storage format.
- No sacrifices to performance while offering a safe, modern, and versatile API.
- The library is designed to remain `constexpr`-friendly where practical.
- Unsafe APIs are explicitly marked as `unchecked`.
- String-like APIs lean toward C++ STL-style naming and behavior while borrowing design choices from Rust.
- Classification APIs expose both ASCII-only and Unicode-aware predicates.

In particular, UTF-8 literals and many core operations are intended to remain usable in constant evaluation.

The library is not a grapheme cluster library. A single user-perceived "character" may still consist of multiple Unicode scalar values.

## Requirements

The project currently builds with MSVC in `/std:c++latest` mode.

The implementation uses modern language and library facilities, including:

- `char8_t`
- `std::expected`
- `std::format`
- ranges and views
- `std::ranges::range_adaptor_closure`

In practice, use a compiler and standard library with strong C++23/C++26 support.

## Unicode version

Unicode property predicates are versioned. The library exposes:

```cpp
inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version;
```

This constant aliases the generated Unicode version used by the Unicode property tables.

Example:

```cpp
static_assert(std::get<0>(unicode_version) == 17);
static_assert(std::get<1>(unicode_version) == 0);
static_assert(std::get<2>(unicode_version) == 0);
```

## Quick start

Suppose you have UTF-8 text such as `café €`, and you want to:

- validate that it is well-formed UTF-8
- count characters rather than raw bytes
- access the first and last character correctly

```cpp
#include "utf8_ranges.hpp"

#include <algorithm>
#include <array>
#include <string>

using namespace literals;

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
#include "utf8_ranges.hpp"

using namespace literals;

constexpr utf8_char euro = "€"_u8c;
constexpr auto text = "Aé€"_utf8_sv;

static_assert(euro.as_scalar() == 0x20AC);
static_assert(euro.as_view() == u8"€");
static_assert(text.size() == 6);
static_assert(text.char_count() == 3);
```

Another literal example:

```cpp
using namespace literals;

constexpr utf8_char euro = "€"_u8c;
constexpr utf8_string_view text = "Aé€"_utf8_sv;

static_assert(euro.byte_count() == 3);
static_assert(text.size() == 6);
```

Printing and formatting are also supported for the library UTF-8 string types:

```cpp
#include "utf8_ranges.hpp"

#include <cassert>
#include <format>
#include <sstream>

using namespace literals;

const utf8_string text = "café €"_utf8_s;

assert(std::format("{}", text) == "café €");

std::ostringstream oss;
oss << text;
assert(oss.str() == "café €");
```

This is intentionally different from `std::u8string`. Standard library types cannot be extended with custom `std::formatter` specializations or stream insertion overloads by user code. These UTF-8 types are library-defined, so providing formatting and `operator<<` support for them is straightforward and well-formed.

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

    constexpr std::size_t byte_count() const noexcept;

    template<class CharT, class OutIt>
    constexpr std::size_t encode_utf8(OutIt out) const noexcept;
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
constexpr utf8_char euro = "â‚¬"_u8c;
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

#### `byte_count`

```cpp
constexpr std::size_t byte_count() const noexcept;
```

Returns the length of the encoded UTF-8 sequence in bytes.

Example:

```cpp
static_assert("A"_u8c.byte_count() == 1);
static_assert("Ã©"_u8c.byte_count() == 2);
static_assert("â‚¬"_u8c.byte_count() == 3);
static_assert("ðŸ˜€"_u8c.byte_count() == 4);
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
const auto n = "â‚¬"_u8c.encode_utf8<char>(buffer.begin());
assert(std::string_view{ buffer.data(), n } == "\xE2\x82\xAC");
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
assert("Î©"_u8c.is_alphabetic());
assert("Î©"_u8c.is_uppercase());
assert("Ï‰"_u8c.is_lowercase());
assert("5"_u8c.is_digit());
assert("â…§"_u8c.is_numeric());
assert("â€"_u8c.is_whitespace());
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

Returns a `views::utf8_view` over the contained Unicode scalar values.

Preconditions:

- None

Complexity:

- Constant to construct the view
- Linear to iterate the entire view

#### `reversed_chars`

```cpp
constexpr auto reversed_chars() const noexcept;
```

Returns a `views::reversed_utf8_view` over the contained Unicode scalar values, in reverse order.

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

Returns an enumerated view of `chars()`.

Each element is a pair-like value containing:

- the zero-based character index
- the corresponding `utf8_char`

The index is a character index, not a byte offset.

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
template <typename T>
constexpr size_type find(T ch) const noexcept;
```

Constraints:

- `T` is `char8_t` or `utf8_char`

Returns the index of the first matching character, or `npos` if no match exists.

The returned index is a byte offset into the underlying UTF-8 sequence.

For the `char8_t` case, the comparison is against a single UTF-8 code unit. In practice this overload is mainly useful for ASCII queries; use `utf8_char` for general Unicode character lookup.

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
```

Returns the character at zero-based character index `index`.

If `index` is out of range, returns `std::nullopt`.

This function operates on character indices, not byte offsets.

Preconditions:

- None

Complexity:

- Linear in `index`, and linear in the number of characters in the worst case

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
constexpr auto text = "AÃ©â‚¬"_utf8_sv;

assert(text.size() == 6);
assert(text.char_count() == 3);

assert(text.is_char_boundary(1));
assert(!text.is_char_boundary(2));

assert(text.substr(1).value() == "Ã©â‚¬"_utf8_sv);
assert(!text.substr(2, 1).has_value());
```

### Comparison

`utf8_string_view` compares lexicographically by the underlying UTF-8 bytes.

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

### Synopsis

```cpp
template<class Allocator = std::allocator<char8_t>>
class utf8_string
{
public:
    utf8_string() = default;

    constexpr utf8_string(utf8_string_view view,
                          const Allocator& alloc = Allocator());
    constexpr utf8_string(std::size_t count, utf8_char ch,
                          const Allocator& alloc = Allocator());

    template<class R>
    constexpr utf8_string(std::from_range_t, R&& rg,
                          const Allocator& alloc = Allocator());

    constexpr utf8_string(std::initializer_list<utf8_char> ilist,
                          const Allocator& alloc = Allocator());

    template<class It, class Sent>
    constexpr utf8_string(It it, Sent sent,
                          const Allocator& alloc = Allocator());

    constexpr utf8_string& append_range(...);
    constexpr utf8_string& append(std::size_t count, utf8_char ch);
    constexpr utf8_string& append(utf8_string_view sv);
    constexpr utf8_string& append(It it, Sent sent);
    constexpr utf8_string& append(std::initializer_list<utf8_char> ilist);

    constexpr utf8_string& operator=(utf8_string_view sv);
    constexpr utf8_string& operator=(utf8_char ch);
    constexpr utf8_string& operator=(std::initializer_list<utf8_char> ilist);

    constexpr void shrink_to_fit();
    constexpr std::size_t capacity() const;
    constexpr std::size_t size() const;
    constexpr void pop_back();
    constexpr void reserve(std::size_t new_cap);
    constexpr auto base() const& noexcept;
    constexpr auto base() && noexcept;
    constexpr void clear();
    constexpr const char8_t* data() const noexcept;
    constexpr const char8_t* c_str() const noexcept;
    constexpr operator utf8_string_view() const noexcept;
    constexpr utf8_string_view as_view() const noexcept;
    constexpr void push_back(utf8_char ch);
    constexpr void swap(utf8_string& other) noexcept(...);
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

#### `append(std::size_t count, utf8_char ch)`

Appends `count` copies of `ch`.

#### `append(utf8_string_view sv)`

Appends a validated UTF-8 string view.

#### `push_back(utf8_char ch)`

Appends one UTF-8 character.

#### `pop_back()`

Removes the last UTF-8 character by inspecting the reversed character view.

Precondition:

- `!empty()`

Remarks:

- Calling `pop_back()` on an empty string is undefined behavior

### Observers and conversions

- `base()` exposes the underlying `std::basic_string<char8_t, ...>`
- `as_view()` returns an unchecked `utf8_string_view`
- `operator utf8_string_view()` converts to a view
- `data()` and `c_str()` expose the contiguous byte storage

Example:

```cpp
using namespace literals;

utf8_string s{ "AÃ©â‚¬"_utf8_sv };
s.push_back("!"_u8c);

assert(s.char_count() == 4);
assert(s.ends_with("!"_u8c));
```

## Reference: views

### `views::utf8_view`

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

### `views::reversed_utf8_view`

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

### `views::lossy_utf8_view<CharT>`

Lossy forward view over arbitrary integral code units.

Construction:

```cpp
constexpr lossy_utf8_view(std::basic_string_view<CharT> base) noexcept;
```

Semantics:

- valid UTF-8 subsequences are decoded into `utf8_char`
- malformed input yields `utf8_char::replacement_character`
- the iterator advances by the valid character width, or by one code unit when replacement was produced

### `views::lossy_utf8`

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

## Reference: literals

### `operator ""_u8c`

Constructs a `utf8_char` from a string literal containing exactly one valid UTF-8 character.

Validation is performed at compile time.

Examples:

```cpp
using namespace literals;

constexpr utf8_char a = "A"_u8c;
constexpr utf8_char e_acute = "Ã©"_u8c;
constexpr utf8_char euro = "â‚¬"_u8c;
```

### `operator ""_utf8_sv`

Constructs a `utf8_string_view` from a UTF-8 string literal.

Validation is performed at compile time.

Examples:

```cpp
using namespace literals;

constexpr auto a = "AÃ©â‚¬"_utf8_sv;
constexpr auto b = "AÃ©â‚¬"_utf8_sv;
```

### `operator ""_utf8_s`

Constructs a `utf8_string` from a UTF-8 string literal.

Validation is performed at compile time.

Example:

```cpp
using namespace literals;

const auto owned = "Hello"_utf8_s;
```

## Reference: formatting, streaming, hashing

### `utf8_char`

Supported:

- `std::ostream << utf8_char`
- `std::hash<utf8_char>`
- `std::formatter<utf8_char, char>`

Formatting supports character presentation and scalar numeric presentation.

Examples:

```cpp
assert(std::format("{}", "A"_u8c) == "A");
assert(std::format("{:c}", "â‚¬"_u8c) == "â‚¬");
assert(std::format("{:#06x}", "A"_u8c) == "0x0041");
assert(std::format("{:#010b}", "A"_u8c) == "0b01000001");
```

### `utf8_string_view`

Supported:

- `std::ostream << utf8_string_view`
- `std::hash<utf8_string_view>`
- `std::formatter<utf8_string_view, char>`

Formatting delegates to the existing `std::string_view` formatter over the underlying UTF-8 bytes reinterpreted as `char`.

Example:

```cpp
constexpr auto text = "AÃ©â‚¬"_utf8_sv;
assert(std::format("{}", text) == "AÃ©â‚¬");
```

## Semantics notes

### 1. Byte-oriented versus character-oriented APIs

This is the most important rule when using the string types.

Byte-oriented:

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

### 2. Checked versus unchecked APIs

Checked:

- `utf8_string_view::from_bytes`
- compile-time string and character literals

Unchecked:

- `utf8_string_view::from_bytes_unchecked`
- `views::utf8_view::from_bytes_unchecked`
- `views::reversed_utf8_view::from_bytes_unchecked`
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

### 5. This library does not do grapheme segmentation

`utf8_char` is a Unicode scalar value. A user-visible glyph may still consist of multiple `utf8_char` values.
