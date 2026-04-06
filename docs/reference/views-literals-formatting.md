# Views, Literals, and Formatting

This page covers the helper view types in `unicode_ranges::views`, the compile-time validated literals in `unicode_ranges::literals`, and the shared formatting model used by the library-defined UTF-8, UTF-16, and UTF-32 types.

## `views::utf8_view`, `views::utf16_view`, And `views::utf32_view`

### Synopsis

```cpp
class utf8_view : public std::ranges::view_interface<utf8_view> {
public:
    constexpr std::u8string_view base() const noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

class utf16_view : public std::ranges::view_interface<utf16_view> {
public:
    constexpr std::u16string_view base() const noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

class utf32_view : public std::ranges::view_interface<utf32_view> {
public:
    constexpr std::u32string_view base() const noexcept;
    constexpr iterator begin() const noexcept;
    constexpr iterator end() const noexcept;
    constexpr std::size_t size() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};
```

### Behavior

- These views adapt already-validated code-unit sequences into ranges of `utf8_char`, `utf16_char`, or `utf32_char`.
- The views inherit [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface) and model lazy borrowed forward views.
- They are normally obtained from validated text via `chars()`, for example `"😄🇷🇴✨"_utf8_sv.chars()`, `u"😄🇷🇴✨"_utf16_sv.chars()`, or `U"😄🇷🇴✨"_utf32_sv.chars()`.
- The views are cheap to copy.
- `reserve_hint()` reports the number of source code units, which is a safe upper bound for the number of yielded characters.
- `views::utf32_view` is a borrowed sized common random-access view because UTF-32 is fixed-width.

### Return value

Construction returns the helper view directly.

### Complexity

- Constructing the view is constant.
- Iterating the full view is linear in the source length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/lossy-views.cpp"
```

## `views::reversed_utf8_view`, `views::reversed_utf16_view`, And `views::reversed_utf32_view`

### Synopsis

```cpp
class reversed_utf8_view : public std::ranges::view_interface<reversed_utf8_view> {
public:
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

class reversed_utf16_view : public std::ranges::view_interface<reversed_utf16_view> {
public:
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

using reversed_utf32_view = std::ranges::reverse_view<utf32_view>;
```

### Behavior

- These helper views inherit [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface).
- They are lazy borrowed forward views over the same underlying storage.
- They are normally obtained from validated text via `reversed_chars()`.
- They iterate validated characters from the end without first materializing a reversed string.
- `views::reversed_utf32_view` is just `std::ranges::reverse_view<views::utf32_view>`, so it is also sized, common, and random-access.

### Complexity

- Constructing the view is constant.
- Iterating the full view is linear in the source length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/helper-views.cpp"
```

## `views::grapheme_cluster_view<CharT>`

### Synopsis

```cpp
template <typename CharT>
class grapheme_cluster_view : public std::ranges::view_interface<grapheme_cluster_view<CharT>> {
public:
    using cluster_type = std::conditional_t<
        std::same_as<CharT, char8_t>,
        utf8_string_view,
        std::conditional_t<std::same_as<CharT, char16_t>, utf16_string_view, utf32_string_view>>;

    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};
```

### Behavior

- `grapheme_cluster_view<char8_t>` yields `utf8_string_view` grapheme clusters.
- `grapheme_cluster_view<char16_t>` yields `utf16_string_view` grapheme clusters.
- `grapheme_cluster_view<char32_t>` yields `utf32_string_view` grapheme clusters.
- The view inherits [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface).
- It is normally obtained from validated text via `graphemes()`.
- It is a lazy borrowed forward view and computes grapheme boundaries on demand during iteration.

### Complexity

- Constructing the view is constant.
- Iterating the full view is linear in the number of code units plus the segmentation work required by Unicode grapheme rules.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## `views::lossy_utf8_view`, `views::lossy_utf16_view`, And `views::lossy_utf32_view`

### Synopsis

```cpp
template <typename CharT>
class lossy_utf8_view : public std::ranges::view_interface<lossy_utf8_view<CharT>> {
public:
    lossy_utf8_view() = default;
    constexpr lossy_utf8_view(std::basic_string_view<CharT> base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

template <typename CharT>
class lossy_utf16_view : public std::ranges::view_interface<lossy_utf16_view<CharT>> {
public:
    lossy_utf16_view() = default;
    constexpr lossy_utf16_view(std::basic_string_view<CharT> base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

template <typename CharT>
class lossy_utf32_view : public std::ranges::view_interface<lossy_utf32_view<CharT>> {
public:
    lossy_utf32_view() = default;
    constexpr lossy_utf32_view(std::basic_string_view<CharT> base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr iterator end() const noexcept;
    constexpr std::size_t size() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

struct lossy_utf8_fn : std::ranges::range_adaptor_closure<lossy_utf8_fn> {
    template<lossy_utf8_viewable_range R>
    constexpr auto operator()(R&& range) const noexcept;
};

struct lossy_utf16_fn : std::ranges::range_adaptor_closure<lossy_utf16_fn> {
    template<lossy_utf16_viewable_range R>
    constexpr auto operator()(R&& range) const noexcept;
};

inline constexpr lossy_utf8_fn lossy_utf8{};
inline constexpr lossy_utf16_fn lossy_utf16{};
inline constexpr lossy_utf32_fn lossy_utf32{};
```

### Behavior

- Lossy views adapt possibly-invalid UTF input into a character range.
- Invalid units are replaced with `replacement_character`.
- Valid subsequences are yielded unchanged.
- The view types inherit [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface) and behave as lazy borrowed forward views.
- The closure objects are [`std::ranges::range_adaptor_closure`](https://en.cppreference.com/w/cpp/ranges/range_adaptor_closure)-style adapters, which makes the lossy views pipe-friendly.
- `lossy_utf32_view<CharT>` is a borrowed sized common random-access view because each input element yields exactly one output scalar.

### Complexity

Linear in the source length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## Compile-Time Validated Literals

### Synopsis

```cpp
using namespace unicode_ranges::literals;

consteval utf8_char operator ""_u8c();
consteval utf16_char operator ""_u16c();
consteval utf32_char operator ""_u32c();

consteval utf8_string_view operator ""_utf8_sv();
consteval utf16_string_view operator ""_utf16_sv();
consteval utf32_string_view operator ""_utf32_sv();

constexpr utf8_string operator ""_utf8_s();
constexpr utf16_string operator ""_utf16_s();
constexpr utf32_string operator ""_utf32_s();

consteval utf8_string_view operator ""_grapheme_utf8();
consteval utf16_string_view operator ""_grapheme_utf16();
consteval utf32_string_view operator ""_grapheme_utf32();
```

### Behavior

- `_u8c`, `_u16c`, and `_u32c` require exactly one valid character in the corresponding encoding.
- `_utf8_sv`, `_utf16_sv`, and `_utf32_sv` require fully valid UTF literals.
- `_utf8_s`, `_utf16_s`, and `_utf32_s` build owning strings from validated literals.
- `_grapheme_utf8`, `_grapheme_utf16`, and `_grapheme_utf32` require exactly one grapheme cluster.

### Return value

Returns the corresponding validated character, view, or owning string.

### Exceptions

Invalid literal contents are rejected during constant evaluation.

### Example

```cpp
--8<-- "examples/reference/literals-and-formatting.cpp"
```

## Optional ICU Locale Tokens

### Synopsis

```cpp
#if UTF8_RANGES_HAS_ICU
struct locale_id
{
    const char* name = nullptr;
};

using namespace unicode_ranges::literals;

consteval locale_id operator ""_locale(const char* name, std::size_t size);
bool is_available_locale(locale_id locale) noexcept;
#endif
```

### Behavior

- This API family exists only when the library is built with `UTF8_RANGES_ENABLE_ICU=1`.
- `locale_id` is a non-owning null-terminated locale token for ICU-backed casing operations.
- `_locale` validates string literals at compile time and rejects embedded NUL bytes.
- Raw `locale_id{ ... }` values must point to storage that stays alive for the duration of the call.
- `is_available_locale(...)` is a non-throwing probe against the current ICU data set.
- Locale-aware casing overloads may still succeed for locales that are not explicitly available, because ICU may canonicalize or fall back to a more general locale.

### Return value

- `_locale` returns a `locale_id`.
- `is_available_locale(...)` returns `true` when the current ICU data set explicitly exposes the locale and `false` otherwise.

### Complexity

- `_locale` is constant evaluation only.
- `is_available_locale(...)` is linear in the number of locales exposed by the current ICU data set.

### Exceptions

- `_locale` rejects embedded NUL bytes during constant evaluation.
- `is_available_locale(...)` does not throw.

### `noexcept`

- `is_available_locale(...)` is `noexcept`.

### Example

```cpp
--8<-- "examples/casing/locale-case.cpp"
```

## Formatting And Printing

### Synopsis

```cpp
template<> struct std::formatter<utf8_char, char>;
template<> struct std::formatter<utf8_char, wchar_t>;
template<> struct std::formatter<utf16_char, char>;
template<> struct std::formatter<utf16_char, wchar_t>;
template<> struct std::formatter<utf32_char, char>;
template<> struct std::formatter<utf32_char, wchar_t>;
template<> struct std::formatter<utf8_string_view, char>;
template<> struct std::formatter<utf16_string_view, char>;
template<> struct std::formatter<utf32_string_view, char>;
template<typename Allocator> struct std::formatter<basic_utf8_string<Allocator>, char>;
template<typename Allocator> struct std::formatter<basic_utf16_string<Allocator>, char>;
template<typename Allocator> struct std::formatter<basic_utf32_string<Allocator>, char>;
```

### Behavior

- Characters format as text by default.
- Character [`std::formatter`](https://en.cppreference.com/w/cpp/utility/format/formatter) specializations also support numeric presentations `d`, `b`, `B`, `o`, `x`, and `X`, which print `as_scalar()`.
- String and string-view formatters print textual content.
- On standard libraries with C++23 range-format support for custom views, range formatting composes with the library formatters, which is why examples such as `std::println("{}", text.chars())` can work directly.

Two practical printing rules used throughout this documentation:

- `std::println("{}", text.chars())` prints a range of validated characters.
- `std::println("{::s}", text.graphemes())` applies string formatting to each grapheme cluster, which is usually the cleanest textual representation.

Compatibility note:

- the library-defined character, string-view, and owning-string types have direct formatter support
- direct formatting of helper views such as `utf8_view` and `grapheme_cluster_view<char8_t>` depends on the standard library's implementation of C++23 range formatting
- this currently works with the MSVC STL and with libc++
- libstdc++ 14 does not currently format these custom helper views directly, so the GCC docs-example CI job is allowed to fail without blocking the overall workflow

### Complexity

Linear in the amount of formatted text.

### Exceptions

Formatter parsing may throw [`std::format_error`](https://en.cppreference.com/w/cpp/utility/format/format_error) for unsupported presentation types.

### `noexcept`

Not `noexcept`.

## Borrowed-Range Status

### Synopsis

```cpp
template <> inline constexpr bool std::ranges::enable_borrowed_range<views::utf8_view> = true;
template <> inline constexpr bool std::ranges::enable_borrowed_range<views::reversed_utf8_view> = true;
template <typename CharT> inline constexpr bool std::ranges::enable_borrowed_range<views::lossy_utf8_view<CharT>> = true;

template <> inline constexpr bool std::ranges::enable_borrowed_range<views::utf16_view> = true;
template <> inline constexpr bool std::ranges::enable_borrowed_range<views::reversed_utf16_view> = true;
template <typename CharT> inline constexpr bool std::ranges::enable_borrowed_range<views::lossy_utf16_view<CharT>> = true;

template <typename CharT> inline constexpr bool std::ranges::enable_borrowed_range<views::grapheme_cluster_view<CharT>> = true;
```

### Behavior

These specializations tell the ranges library that the helper views may safely borrow from the underlying storage instead of forcing owning semantics.

In other words:

- the helper adapters on this page are real range views, implemented as `std::ranges::view_interface` subclasses
- they are lazy rather than eagerly materialized
- and they remain borrowed ranges, so iterators and subviews may refer back to the original storage
