# Characters

`utf8_char`, `utf16_char`, and `utf32_char` are validated single-scalar value types.

They are useful when you want to store or pass one Unicode scalar value without dropping down to raw UTF-8 bytes or UTF-16 code units.

When the character is known at compile time, the literal operators such as `_u8c`, `_u16c`, and `_u32c` are usually the nicest entry point. `from_scalar(...)` is the runtime path when the scalar value arrives as data.

Unless a section explicitly narrows the discussion, the UTF-8, UTF-16, and UTF-32 character APIs are structurally parallel.

```cpp
--8<-- "examples/reference/characters.cpp"
```

## Constants And Default Construction

### Synopsis

```cpp
utf8_char() = default;
utf16_char() = default;
utf32_char() = default;

static const utf8_char replacement_character;
static const utf8_char null_terminator;

static const utf16_char replacement_character;
static const utf16_char null_terminator;

static const utf32_char replacement_character;
static const utf32_char null_terminator;
```

### Behavior

- Value-initialized `utf8_char`, `utf16_char`, and `utf32_char` hold U+0000.
- `replacement_character` is U+FFFD.
- `null_terminator` is U+0000.

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

Default construction is non-throwing.

### Example

```cpp
--8<-- "examples/reference/character-constants.cpp"
```

## Checked Scalar Construction

### Synopsis

```cpp
static constexpr std::optional<utf8_char> from_scalar(std::uint32_t scalar) noexcept;
static constexpr std::optional<utf16_char> from_scalar(std::uint32_t scalar) noexcept;
static constexpr std::optional<utf32_char> from_scalar(std::uint32_t scalar) noexcept;
```

### Behavior

Constructs a validated character from a Unicode scalar value and reports failure with [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional).

### Return value

- Returns the constructed character when `scalar` is a valid Unicode scalar value.
- Returns [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) for invalid inputs such as surrogate code points or values above U+10FFFF.

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

Always `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-checked-construction.cpp"
```

## Unchecked Construction

### Synopsis

```cpp
static constexpr utf8_char from_scalar_unchecked(std::uint32_t scalar) noexcept;

template<typename CharT>
static constexpr utf8_char from_utf8_bytes_unchecked(const CharT* bytes, std::size_t size) noexcept;

static constexpr utf16_char from_scalar_unchecked(std::uint32_t scalar) noexcept;

template<typename CharT>
static constexpr std::optional<utf16_char>
from_utf16_code_units(const CharT* code_units, std::size_t size) noexcept;

template<typename CharT>
static constexpr utf16_char
from_utf16_code_units_unchecked(const CharT* code_units, std::size_t size) noexcept;

static constexpr utf32_char from_scalar_unchecked(std::uint32_t scalar) noexcept;

template<typename CharT>
static constexpr std::optional<utf32_char>
from_utf32_code_points(const CharT* code_points, std::size_t size) noexcept;

template<typename CharT>
static constexpr utf32_char
from_utf32_code_points_unchecked(const CharT* code_points, std::size_t size) noexcept;
```

### Behavior

- `from_scalar_unchecked` trusts that the supplied scalar is valid.
- `from_utf8_bytes_unchecked` trusts that `bytes[0, size)` encodes exactly one valid UTF-8 scalar.
- `from_utf16_code_units` validates that the range holds exactly one valid UTF-16 scalar.
- `from_utf16_code_units_unchecked` trusts that the supplied code units hold exactly one valid UTF-16 scalar.
- `from_utf32_code_points` validates that the range holds exactly one valid UTF-32 scalar.
- `from_utf32_code_points_unchecked` trusts that the supplied code points hold exactly one valid UTF-32 scalar.

### Return value

- The checked UTF-16 constructor returns `std::nullopt` when the range is not exactly one valid UTF-16 character.
- The checked UTF-32 constructor returns `std::nullopt` when the range is not exactly one valid UTF-32 character.
- The unchecked constructors always return a value.

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

All listed overloads are `noexcept`.

### Example

Prefer the checked constructors and literals in normal user code. The unchecked overloads are for already-trusted input.

```cpp
--8<-- "examples/reference/character-unchecked-construction.cpp"
```

## Scalar Value, Encoding, And Cross-Encoding Conversion

### Synopsis

```cpp
constexpr std::uint32_t as_scalar() const noexcept;

constexpr operator utf16_char() const noexcept; // utf8_char only
constexpr operator utf32_char() const noexcept; // utf8_char and utf16_char
constexpr operator utf8_char() const noexcept;  // utf16_char only
constexpr operator utf16_char() const noexcept; // utf32_char only
constexpr operator utf8_char() const noexcept;  // utf32_char only

template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_utf8_owned(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_utf16_owned(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char32_t>>
constexpr basic_utf32_string<Allocator> to_utf32_owned(const Allocator& alloc = Allocator()) const;

constexpr std::size_t code_unit_count() const noexcept;

template<typename CharT, typename OutIt>
constexpr std::size_t encode_utf8(OutIt out) const noexcept;

template<typename CharT, typename OutIt>
constexpr std::size_t encode_utf16(OutIt out) const noexcept;

template<typename CharT, typename OutIt>
constexpr std::size_t encode_utf32(OutIt out) const noexcept;
```

### Behavior

- `as_scalar()` returns the Unicode scalar value represented by the object.
- The conversion operators transcode a single scalar between the UTF-8, UTF-16, and UTF-32 character representations.
- `to_utf8_owned()`, `to_utf16_owned()`, and `to_utf32_owned()` materialize a one-character owning string in the corresponding encoding.
- `code_unit_count()` returns the number of code units used by the current encoding:
  - UTF-8: `1` to `4`
  - UTF-16: `1` or `2`
  - UTF-32: `1`
- `encode_utf8()`, `encode_utf16()`, and `encode_utf32()` copy the current value into an output iterator and return the number of code units written.

### Return value

- `as_scalar()` returns the scalar directly.
- `encode_*()` returns the number of output code units.
- `to_*_owned()` returns an owning validated string containing exactly one character.

### Complexity

Constant.

### Exceptions

- `as_scalar()`, the conversion operators, `code_unit_count()`, and `encode_*()` do not throw.
- `to_*_owned()` may throw allocator or container exceptions.

### `noexcept`

- `as_scalar()`, conversion operators, `code_unit_count()`, and `encode_*()` are `noexcept`.
- `to_*_owned()` is not `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-scalar-and-encoding.cpp"
```

## Scalar Iteration Helpers

### Synopsis

```cpp
constexpr utf8_char& operator++() noexcept;
constexpr utf8_char operator++(int) noexcept;
constexpr utf8_char& operator--() noexcept;
constexpr utf8_char operator--(int) noexcept;

constexpr utf16_char& operator++() noexcept;
constexpr utf16_char operator++(int) noexcept;
constexpr utf16_char& operator--() noexcept;
constexpr utf16_char operator--(int) noexcept;

constexpr utf32_char& operator++() noexcept;
constexpr utf32_char operator++(int) noexcept;
constexpr utf32_char& operator--() noexcept;
constexpr utf32_char operator--(int) noexcept;
```

### Behavior

Advances or retreats across Unicode scalar values, skipping the surrogate range.

The operations wrap:

- decrementing U+0000 produces U+10FFFF
- incrementing U+10FFFF produces U+0000

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

All four operators are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-iteration.cpp"
```

## Unicode Classification Predicates

### Synopsis

```cpp
constexpr bool is_ascii() const noexcept;
constexpr bool is_alphabetic() const noexcept;
constexpr bool is_alphanumeric() const noexcept;
constexpr bool is_control() const noexcept;
constexpr bool is_digit() const noexcept;
constexpr bool is_lowercase() const noexcept;
constexpr bool is_numeric() const noexcept;
constexpr bool is_uppercase() const noexcept;
constexpr bool is_whitespace() const noexcept;
```

### Behavior

- `is_ascii()` tests whether the scalar is in the ASCII range.
- The remaining predicates use the Unicode property tables shipped with the library.
- `is_alphanumeric()` is defined as `is_alphabetic() || is_numeric()`.

### Return value

Returns `true` when the current scalar has the queried property.

### Complexity

Constant, with table lookups for the Unicode property predicates.

### Exceptions

None.

### `noexcept`

All listed overloads are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-unicode-predicates.cpp"
```

## Unicode Property Queries

### Synopsis

```cpp
constexpr unicode_general_category general_category() const noexcept;
constexpr std::uint8_t canonical_combining_class() const noexcept;
constexpr unicode_grapheme_break_property grapheme_break_property() const noexcept;
constexpr unicode_script script() const noexcept;
constexpr unicode_east_asian_width east_asian_width() const noexcept;
constexpr unicode_line_break_class line_break_class() const noexcept;
constexpr unicode_bidi_class bidi_class() const noexcept;
constexpr unicode_word_break_property word_break_property() const noexcept;
constexpr unicode_sentence_break_property sentence_break_property() const noexcept;
constexpr bool is_emoji() const noexcept;
constexpr bool is_emoji_presentation() const noexcept;
constexpr bool is_extended_pictographic() const noexcept;
```

### Behavior

- All of these methods are locale-independent scalar-property queries.
- `general_category()` returns the Unicode General_Category bucket for the scalar.
- `canonical_combining_class()` returns the canonical combining class used by normalization and canonical reordering.
- `grapheme_break_property()`, `line_break_class()`, `word_break_property()`, and `sentence_break_property()` expose the default Unicode segmentation and line-breaking properties for the scalar.
- `script()` returns the Unicode Script property. Punctuation and separators often come back as `common`, while combining marks are often `inherited`.
- `east_asian_width()` returns the Unicode East Asian Width class used by terminal and grid-width heuristics.
- `bidi_class()` returns the scalar's Unicode bidirectional class. This is a property lookup, not full bidi reordering.
- `is_emoji()` tests the Unicode `Emoji` property.
- `is_emoji_presentation()` tests the Unicode `Emoji_Presentation` property.
- `is_extended_pictographic()` tests the Unicode `Extended_Pictographic` property used by segmentation algorithms.
- These are scalar properties only. They do not inspect grapheme clusters or emoji ZWJ sequences.

### Return value

- The enum-returning methods return the property value for the current scalar.
- `canonical_combining_class()` returns the canonical combining class number.
- The boolean methods return `true` when the scalar has the queried property.

### Complexity

Constant, with table lookups.

### Exceptions

None.

### `noexcept`

All listed overloads are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-unicode-properties.cpp"
```

## ASCII Classification Predicates

### Synopsis

```cpp
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
```

### Behavior

These methods first require the value to be ASCII and then apply the corresponding ASCII-only classification rule.

### Return value

Returns `false` for all non-ASCII characters.

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

All listed overloads are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-ascii-predicates.cpp"
```

## ASCII Transforms And ASCII Comparison

### Synopsis

```cpp
constexpr utf8_char ascii_lowercase() const noexcept;
constexpr utf8_char ascii_uppercase() const noexcept;
constexpr bool eq_ignore_ascii_case(utf8_char other) const noexcept;
constexpr void swap(utf8_char& other) noexcept;

constexpr utf16_char ascii_lowercase() const noexcept;
constexpr utf16_char ascii_uppercase() const noexcept;
constexpr bool eq_ignore_ascii_case(utf16_char other) const noexcept;
constexpr void swap(utf16_char& other) noexcept;

constexpr utf32_char ascii_lowercase() const noexcept;
constexpr utf32_char ascii_uppercase() const noexcept;
constexpr bool eq_ignore_ascii_case(utf32_char other) const noexcept;
constexpr void swap(utf32_char& other) noexcept;
```

### Behavior

- `ascii_lowercase()` and `ascii_uppercase()` only modify ASCII letters.
- Non-ASCII characters are returned unchanged.
- `eq_ignore_ascii_case()` lowercases both operands with the ASCII-only transform and compares the results.
- `swap()` exchanges the stored code units.

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

All listed overloads are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-ascii-transforms.cpp"
```

## Comparison, Streaming, Hashing, And Formatting

### Synopsis

```cpp
friend constexpr bool operator==(const utf8_char&, const utf8_char&) = default;
friend constexpr auto operator<=>(const utf8_char&, const utf8_char&) = default;
friend constexpr bool operator==(const utf8_char& lhs, char rhs) noexcept;
friend constexpr bool operator==(const utf8_char& lhs, char8_t rhs) noexcept;
friend std::ostream& operator<<(std::ostream& os, const utf8_char& ch);

friend constexpr bool operator==(const utf16_char&, const utf16_char&) = default;
friend constexpr auto operator<=>(const utf16_char&, const utf16_char&) = default;
friend constexpr bool operator==(const utf16_char& lhs, char16_t rhs) noexcept;
friend std::ostream& operator<<(std::ostream& os, const utf16_char& ch);

friend constexpr bool operator==(const utf32_char&, const utf32_char&) = default;
friend constexpr auto operator<=>(const utf32_char&, const utf32_char&) = default;
friend constexpr bool operator==(const utf32_char& lhs, char32_t rhs) noexcept;
friend std::ostream& operator<<(std::ostream& os, const utf32_char& ch);

template<> struct std::hash<utf8_char>;
template<> struct std::hash<utf16_char>;
template<> struct std::hash<utf32_char>;

template<> struct std::formatter<utf8_char, char>;
template<> struct std::formatter<utf8_char, wchar_t>;
template<> struct std::formatter<utf16_char, char>;
template<> struct std::formatter<utf16_char, wchar_t>;
template<> struct std::formatter<utf32_char, char>;
template<> struct std::formatter<utf32_char, wchar_t>;
```

### Behavior

- The defaulted comparisons compare the stored encoded value.
- `utf8_char` compares directly with `char` and `char8_t` when the value is a single-byte ASCII code point.
- `utf16_char` compares directly with `char16_t` when the value is a single UTF-16 code unit.
- `utf32_char` compares directly with `char32_t` because every UTF-32 character is one code point unit.
- Stream insertion writes a textual representation:
  - `utf8_char` writes its UTF-8 bytes directly
  - `utf16_char` transcodes to UTF-8 for `std::ostream`
  - `utf32_char` transcodes to UTF-8 for `std::ostream`
- [`std::hash`](https://en.cppreference.com/w/cpp/utility/hash) hashes the encoded text representation.
- The [`std::formatter`](https://en.cppreference.com/w/cpp/utility/format/formatter) specializations support:
  - normal textual formatting
  - `'c'` as a text presentation alias
  - numeric presentations `d`, `b`, `B`, `o`, `x`, `X`, which format `as_scalar()`

### Complexity

Constant.

### Exceptions

- The comparison overloads and hashers do not throw.
- Stream insertion may report stream errors through the stream object.
- Formatter parsing may throw [`std::format_error`](https://en.cppreference.com/w/cpp/utility/format/format_error) for unsupported or malformed format specifiers.

### `noexcept`

- The comparison overloads and hashers are non-throwing.
- Stream insertion and formatters are not `noexcept`.

### Example

```cpp
--8<-- "examples/reference/character-formatting.cpp"
```

## Character Literals

### Synopsis

```cpp
using namespace unicode_ranges::literals;

consteval utf8_char operator ""_u8c();
consteval utf16_char operator ""_u16c();
consteval utf32_char operator ""_u32c();
```

### Behavior

The literal must encode exactly one valid UTF-8, UTF-16, or UTF-32 character in the corresponding encoding.

### Return value

Returns the validated `utf8_char`, `utf16_char`, or `utf32_char`.

### Exceptions

Because these are `consteval` literals, invalid input is rejected during compilation rather than at runtime.

### Example

```cpp
--8<-- "examples/reference/character-literals.cpp"
```

## Curated Character Namespaces

### Synopsis

```cpp
namespace unicode_ranges::characters::utf8
{
    namespace punctuation { inline constexpr utf8_char ...; }
    namespace symbols { inline constexpr utf8_char ...; }
    namespace currency { inline constexpr utf8_char ...; }
    namespace math { inline constexpr utf8_char ...; }
    namespace arrows { inline constexpr utf8_char ...; }
    namespace emojis { inline constexpr utf8_char ...; }
}

namespace unicode_ranges::characters::utf16
{
    namespace punctuation { inline constexpr utf16_char ...; }
    namespace symbols { inline constexpr utf16_char ...; }
    namespace currency { inline constexpr utf16_char ...; }
    namespace math { inline constexpr utf16_char ...; }
    namespace arrows { inline constexpr utf16_char ...; }
    namespace emojis { inline constexpr utf16_char ...; }
}

namespace unicode_ranges::characters::utf32
{
    namespace punctuation { inline constexpr utf32_char ...; }
    namespace symbols { inline constexpr utf32_char ...; }
    namespace currency { inline constexpr utf32_char ...; }
    namespace math { inline constexpr utf32_char ...; }
    namespace arrows { inline constexpr utf32_char ...; }
    namespace emojis { inline constexpr utf32_char ...; }
}
```

### Behavior

- These namespaces provide a curated convenience set of commonly used punctuation, symbols, currency signs, arrows, math symbols, and emoji.
- `characters::utf8::...` constants are `utf8_char`.
- `characters::utf16::...` constants are `utf16_char`.
- `characters::utf32::...` constants are `utf32_char`.
- Both trees expose the same names so you can choose the encoding that matches the surrounding API.
- This is intentionally not a complete Unicode catalog.

### Complexity

Constant.

### Exceptions

None.

### `noexcept`

Accessing the constants is non-throwing.

### Example

```cpp
--8<-- "examples/reference/character-namespaces.cpp"
```
