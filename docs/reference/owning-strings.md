# Owning Strings

`basic_utf8_string<Allocator>`, `basic_utf16_string<Allocator>`, and `basic_utf32_string<Allocator>` are validated owning string types.

The boundary encoding families `from_encoded(...)`, `to_encoded(...)`, `encode_to(...)`, and `encode_append_to(...)` are documented separately in [Boundary Encodings](encodings.md).

The aliases exported from [`unicode_ranges/core.hpp`](https://github.com/cristi1990an/unicode_ranges/blob/main/unicode_ranges/core.hpp) are:

- `utf8_string = basic_utf8_string<>`
- `utf16_string = basic_utf16_string<>`
- `utf32_string = basic_utf32_string<>`
- `pmr::utf8_string = basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>`
- `pmr::utf16_string = basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>`
- `pmr::utf32_string = basic_utf32_string<std::pmr::polymorphic_allocator<char32_t>>`

Unless otherwise stated, the UTF-8, UTF-16, and UTF-32 surfaces are structurally parallel.

When a signature block uses `Char`, `View`, `String`, or `Predicate`, it refers to the encoding-specific family being described in that section.

Unless a section explicitly narrows the discussion, the UTF-8, UTF-16, and UTF-32 owning-string APIs are structurally parallel.

To keep the longer synopsis blocks manageable, many sections spell out the UTF-8 forms explicitly and describe UTF-16 and UTF-32 by parallel rule. Unless a section says otherwise, replacing `char8_t` / `utf8_char` / `utf8_string_view` / `basic_utf8_string` with the matching UTF-16 or UTF-32 names gives the corresponding owning-string surface.

```cpp
--8<-- "examples/reference/owning-strings.cpp"
```

## Checked Factory Functions

### Synopsis

```cpp
static constexpr std::expected<basic_utf8_string, utf8_error>
from_bytes(std::string_view bytes, const Allocator& alloc = Allocator());

static constexpr std::expected<basic_utf8_string, utf16_error>
from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()); // when sizeof(wchar_t) == 2

static constexpr std::expected<basic_utf8_string, unicode_scalar_error>
from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()); // when sizeof(wchar_t) == 4

static constexpr std::expected<basic_utf8_string, utf8_error>
from_bytes(base_type&& bytes) noexcept;

static constexpr std::expected<basic_utf16_string, utf8_error>
from_bytes(std::string_view bytes, const Allocator& alloc = Allocator());

static constexpr std::expected<basic_utf16_string, utf16_error>
from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()); // when sizeof(wchar_t) == 2

static constexpr std::expected<basic_utf16_string, unicode_scalar_error>
from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()); // when sizeof(wchar_t) == 4

static constexpr std::expected<basic_utf16_string, utf16_error>
from_bytes(base_type&& bytes) noexcept;
```

### Behavior

- These factories validate or transcode runtime input before constructing the owning string.
- `from_bytes(base_type&&)` validates the moved-in standard-library string in place.

### Overload differences

The table rows below use visible source inputs like `"😄🇷🇴✨"`, `u8"😄🇷🇴✨"`, and `L"😄🇷🇴✨"`, depending on the overload.

| Overload | Meaning | Example |
| --- | --- | --- |
| `from_bytes(std::string_view bytes, alloc)` | validate UTF-8 bytes and construct the owning string in the target encoding | `auto text = utf8_string::from_bytes("😄🇷🇴✨");` |
| `from_bytes(std::wstring_view bytes, alloc)` | validate or transcode a wide-character source; the exact validation path depends on `sizeof(wchar_t)` | `auto text = utf16_string::from_bytes(L"😄🇷🇴✨");` |
| `from_bytes(base_type&& bytes)` | validate a moved-in `std::u8string` or `std::u16string` in place | `auto text = utf8_string::from_bytes(std::u8string{u8"😄🇷🇴✨"});` |

The wide-string overload is convenient when interoperating with platform APIs, but it is the least portable surface because the meaning of `wchar_t` differs across platforms. The UTF-8, UTF-16, and UTF-32 overloads have fixed semantics.

### Inspiration

These factories play the role that constructors and parser-style helpers often play in the C++ standard library, but with explicit validation through [`std::expected`](https://en.cppreference.com/w/cpp/utility/expected). The moved-string overload is closest in spirit to taking ownership of an already-built [`std::basic_string`](https://en.cppreference.com/w/cpp/string/basic_string/basic_string), then validating before exposing the stronger type.

### Return value

Returns [`std::unexpected(...)`](https://en.cppreference.com/w/cpp/utility/expected/unexpected) with the relevant UTF or scalar error type when validation or transcoding fails.

### Complexity

Linear in the source length.

### Exceptions

May throw allocator or container exceptions when construction or transcoding needs storage.

### `noexcept`

Only the moved-string overloads are `noexcept`.

## Unchecked Factory Functions

### Synopsis

```cpp
static constexpr basic_utf8_string from_bytes_unchecked(base_type&& bytes) noexcept;
static constexpr basic_utf8_string from_bytes_unchecked(std::string_view bytes, const Allocator& alloc = Allocator());
static constexpr basic_utf8_string from_bytes_unchecked(std::wstring_view bytes, const Allocator& alloc = Allocator());

static constexpr basic_utf16_string from_code_units_unchecked(base_type code_units) noexcept;
static constexpr basic_utf16_string from_code_units_unchecked(base_type code_units, const Allocator& alloc);
static constexpr basic_utf16_string from_code_units_unchecked(std::u16string_view code_units, const Allocator& alloc = Allocator());
static constexpr basic_utf16_string from_bytes_unchecked(base_type&& bytes) noexcept;
static constexpr basic_utf16_string from_bytes_unchecked(std::string_view bytes, const Allocator& alloc = Allocator());
static constexpr basic_utf16_string from_bytes_unchecked(std::wstring_view bytes, const Allocator& alloc = Allocator());
```

### Behavior

These factories assume the caller already knows the supplied bytes or code units are valid for the target type.

### Overload differences

| Overload | Meaning | Example |
| --- | --- | --- |
| `from_bytes_unchecked(base_type&& bytes)` | adopt an already-built standard string without revalidation | `auto text = utf8_string::from_bytes_unchecked(std::u8string{u8"😄🇷🇴✨"});` |
| `from_bytes_unchecked(std::string_view bytes, alloc)` | copy UTF-8 bytes directly into validated storage without checking them | `auto text = utf8_string::from_bytes_unchecked("😄🇷🇴✨");` |
| `from_code_units_unchecked(std::u16string_view code_units, alloc)` | build UTF-16 text directly from trusted code units | `auto text = utf16_string::from_code_units_unchecked(u"😄🇷🇴✨");` |
| `from_bytes_unchecked(std::wstring_view bytes, alloc)` | trust a platform wide string as already valid UTF-16 or scalar text | `auto text = utf16_string::from_bytes_unchecked(L"😄🇷🇴✨");` |

These overloads exist for callers that already validated data at a lower layer, or for compile-time known literals where the invariant is already established elsewhere. They are not recovery-oriented APIs.

### Return value

Returns the constructed owning string directly.

### Complexity

Linear in the source length.

### Exceptions

May throw allocator or container exceptions.

### `noexcept`

- Pure ownership-taking overloads such as `from_bytes_unchecked(base_type&&)` and `from_code_units_unchecked(base_type)` are `noexcept`.
- Borrowed-input and allocator-taking overloads are not `noexcept`, because they may allocate while constructing the owning string.

## Lossy Factory Functions

### Synopsis

```cpp
static constexpr basic_utf8_string from_bytes_lossy(std::string_view bytes, const Allocator& alloc = Allocator());
static constexpr basic_utf8_string from_bytes_lossy(std::u8string_view bytes, const Allocator& alloc = Allocator());
static constexpr basic_utf8_string from_bytes_lossy(base_type&& bytes);

static constexpr basic_utf16_string from_code_units_lossy(std::u16string_view code_units, const Allocator& alloc = Allocator());
static constexpr basic_utf16_string from_code_units_lossy(base_type&& code_units) noexcept;

static constexpr basic_utf32_string from_code_points_lossy(std::u32string_view code_points, const Allocator& alloc = Allocator());
static constexpr basic_utf32_string from_code_points_lossy(base_type&& code_points) noexcept;
```

### Behavior

- These factories repair malformed input by replacing invalid sequences or invalid scalar values with `U+FFFD`.
- Borrowed-input overloads build a new validated owning string.
- Owned `&&` overloads repair in place whenever the target encoding allows it without reallocating.

### Overload differences

| Overload | Meaning | Example |
| --- | --- | --- |
| `from_bytes_lossy(std::string_view bytes, alloc)` | repair malformed UTF-8 bytes while constructing a validated UTF-8 string | `auto text = utf8_string::from_bytes_lossy("A\xFF");` |
| `from_bytes_lossy(base_type&& bytes)` | take ownership of a UTF-8 string and repair it, potentially reallocating if malformed input expands | `auto text = utf8_string::from_bytes_lossy(std::u8string{u8"A\xFF"});` |
| `from_code_units_lossy(std::u16string_view code_units, alloc)` | repair malformed UTF-16 code units while constructing a validated UTF-16 string | `auto text = utf16_string::from_code_units_lossy(std::u16string_view{ u"A\uD800B" });` |
| `from_code_units_lossy(base_type&& code_units)` | take ownership of a UTF-16 string and repair it in place | `auto text = utf16_string::from_code_units_lossy(std::u16string{ u"A\uD800B" });` |
| `from_code_points_lossy(std::u32string_view code_points, alloc)` | repair invalid UTF-32 scalar values while constructing a validated UTF-32 string | `auto text = utf32_string::from_code_points_lossy(std::u32string_view{ U"A\U0000D800B" });` |
| `from_code_points_lossy(base_type&& code_points)` | take ownership of a UTF-32 string and repair it in place | `auto text = utf32_string::from_code_points_lossy(std::u32string{ U"A\U0000D800B" });` |

### Complexity

Linear in the source length.

### Exceptions

Borrowed-input overloads may throw allocator or container exceptions. The UTF-8 owned overload may also throw if malformed repair expands and requires reallocation.

### `noexcept`

- Borrowed-input and allocator-taking lossy overloads are not `noexcept`.
- `utf16_string::from_code_units_lossy(base_type&&)` and `utf32_string::from_code_points_lossy(base_type&&)` are `noexcept` because those repairs are width-preserving and stay in place.
- `utf8_string::from_bytes_lossy(base_type&&)` is not `noexcept`, because malformed UTF-8 may expand when replaced with `U+FFFD`.

## Constructors

### Synopsis

```cpp
basic_utf8_string() = default;
basic_utf8_string(const basic_utf8_string&) = default;
basic_utf8_string(basic_utf8_string&&) = default;
basic_utf8_string& operator=(const basic_utf8_string&) = default;
basic_utf8_string& operator=(basic_utf8_string&&) = default;

constexpr basic_utf8_string(const Allocator& alloc);
constexpr basic_utf8_string(const basic_utf8_string& other, const Allocator& alloc);
constexpr basic_utf8_string(basic_utf8_string&& other, const Allocator& alloc) noexcept(/* conditional */);
constexpr basic_utf8_string(utf8_string_view view, const Allocator& alloc = Allocator());
constexpr basic_utf8_string(utf16_string_view view, const Allocator& alloc = Allocator());
constexpr basic_utf8_string(utf32_string_view view, const Allocator& alloc = Allocator());
constexpr basic_utf8_string(std::size_t count, utf8_char ch, const Allocator& alloc = Allocator());
constexpr basic_utf8_string(std::from_range_t, views::utf8_view rg, const Allocator& alloc = Allocator());
constexpr basic_utf8_string(
    std::from_range_t,
    views::owning_chars_view<basic_utf8_string>&& rg,
    const Allocator& alloc = Allocator());
constexpr basic_utf8_string(
    std::from_range_t,
    views::owning_reversed_chars_view<basic_utf8_string>&& rg,
    const Allocator& alloc = Allocator());
template <details::container_compatible_range<utf8_char> R>
constexpr basic_utf8_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator());
constexpr basic_utf8_string(std::initializer_list<utf8_char> ilist, const Allocator& alloc = Allocator());
template <std::input_iterator It, std::sentinel_for<It> Sent>
constexpr basic_utf8_string(It it, Sent sent, const Allocator& alloc = Allocator());

// The UTF-16 and UTF-32 types expose the same constructor families with utf16_string_view / utf32_string_view and utf16_char / utf32_char.
```

### Behavior

- View constructors copy validated text into owned storage.
- The count constructor repeats the validated character `count` times.
- Range and iterator constructors append validated characters from the source range.
- The cross-encoding view constructors transcode.
- Runtime cross-encoding construction and range mutation may use compiled bulk transcoding paths; constexpr evaluation keeps scalar fallbacks.
- Dedicated same-encoding `chars()` and rvalue `reversed_chars()` view overloads may use direct storage paths instead of generic per-character materialization.
- Rvalue owning views, such as `std::move(text).chars()` and `std::move(text).reversed_chars()`, may reuse storage when allocator compatibility allows it.

### Overload differences

The table rows below start from declarations like `utf8_string text = "😄🇷🇴✨"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `basic_utf8_string(utf8_string_view view, alloc)` | copy validated text in the same encoding | `utf8_string a{"😄🇷🇴✨"_utf8_sv};` |
| `basic_utf8_string(utf16_string_view view, alloc)` | transcode from UTF-16 into owned UTF-8 storage | `utf8_string a{u"😄🇷🇴✨"_utf16_sv};` |
| `basic_utf8_string(count, utf8_char ch, alloc)` | repeat one validated character `count` times | `utf8_string a{3, "✨"_u8c};` |
| `basic_utf8_string(std::from_range, R&& rg, alloc)` | build from a range of validated characters | `utf8_string a{std::from_range, std::array{"😄"_u8c, "✨"_u8c}};` |
| `basic_utf8_string(std::initializer_list<utf8_char>, alloc)` | build from a braced list of validated characters | `utf8_string a{{"😄"_u8c, "✨"_u8c}};` |
| `basic_utf8_string(It, Sent, alloc)` | build from an iterator/sentinel pair over validated characters | `const auto chars = "😄✨"_utf8_sv.chars(); utf8_string a{chars.begin(), chars.end()};` |

The UTF-16 and UTF-32 constructors behave the same way, but operate on their own validated views and character types.
The braced-list constructor uses [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) in the usual C++ sense; the difference is that each element is already a validated Unicode character object rather than a raw code unit.

### Inspiration

This family deliberately mirrors the shape of [`std::basic_string`](https://en.cppreference.com/w/cpp/string/basic_string/basic_string) constructors, with the extra guarantee that every source path is either validated or explicitly marked unchecked elsewhere.

### Complexity

Linear in the size of the constructed string.

### Exceptions

May throw allocator or container exceptions.

### `noexcept`

Only the move-with-allocator constructor is conditionally `noexcept`.

## Assignment And Append Families

### Synopsis

```cpp
constexpr basic_utf8_string& append_range(views::utf8_view rg);
constexpr basic_utf8_string& append_range(views::owning_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& append_range(views::owning_reversed_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& append_range(views::utf16_view rg);
constexpr basic_utf8_string& append_range(views::utf32_view rg);
template <details::container_compatible_range<utf8_char> R>
constexpr basic_utf8_string& append_range(R&& rg);

constexpr basic_utf8_string& assign_range(views::utf8_view rg);
constexpr basic_utf8_string& assign_range(views::owning_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& assign_range(views::owning_reversed_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& assign_range(views::utf16_view rg);
constexpr basic_utf8_string& assign_range(views::utf32_view rg);
template <details::container_compatible_range<utf8_char> R>
constexpr basic_utf8_string& assign_range(R&& rg);

constexpr basic_utf8_string& append(size_type count, utf8_char ch);
constexpr basic_utf8_string& assign(size_type count, utf8_char ch);
constexpr basic_utf8_string& append(utf8_string_view sv);
constexpr basic_utf8_string& assign(utf8_string_view sv);
constexpr basic_utf8_string& assign(utf8_char ch);
template <std::input_iterator It, std::sentinel_for<It> Sent>
constexpr basic_utf8_string& append(It it, Sent sent);
template <std::input_iterator It, std::sentinel_for<It> Sent>
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

// The UTF-16 and UTF-32 types expose the same families with their corresponding view and character types.
```

### Behavior

- `append_*` preserve the existing contents and add new validated text.
- `assign_*` replace the existing contents.
- `append_range` and `assign_range` accept both same-encoding and cross-encoding view helpers.
- Same-encoding `chars()` and rvalue `reversed_chars()` views have dedicated overloads so direct materialization can avoid generic character-by-character paths.
- `operator+=` delegates to the append surface.

### Overload differences

The examples below use `utf8_string text = "😄"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `append_range(views::utf8_view rg)` | append a same-encoding character range without materializing another owning string | `text.append_range("🇷🇴"_utf8_sv.chars());` |
| `append_range(views::utf16_view rg)` | append a cross-encoding character range with transcoding | `text.append_range(u"✨"_utf16_sv.chars());` |
| `append_range(R&& rg)` | append a generic range whose elements are already `utf8_char` | `text.append_range(std::array{"🎉"_u8c, "🔥"_u8c});` |
| `assign_range(...)` | same source shapes as `append_range`, but replaces the whole string first | `text.assign_range("✨😄"_utf8_sv.chars());` |
| `append(count, Char ch)` | append the same validated character repeatedly | `text.append(2, "✨"_u8c);` |
| `assign(count, Char ch)` | replace the whole string with `count` copies of one validated character | `text.assign(3, "🎉"_u8c);` |
| `append(View sv)` | append one validated substring | `text.append("🇷🇴"_utf8_sv);` |
| `assign(View sv)` | replace with one validated substring | `text.assign("😄✨"_utf8_sv);` |
| `append(It, Sent)` / `assign(It, Sent)` | consume an iterator pair of validated characters | `const auto chars = "✨🎉"_utf8_sv.chars(); text.append(chars.begin(), chars.end());` |
| `append(std::initializer_list<Char>)` / `assign(std::initializer_list<Char>)` | operate on a short braced list of validated characters | `text.append({"✨"_u8c, "🎉"_u8c});` |
| `operator=(View)` / `operator=(Char)` | shorthand for replacing the whole string | `text = "😄🇷🇴"_utf8_sv;` |
| `operator+=(View)` / `operator+=(Char)` | shorthand for appending validated text | `text += "✨"_u8c;` |
| cross-encoding `operator+=` | append one validated `utf16_char` or `utf16_string_view` to a UTF-8 string, or the UTF-8 equivalents to a UTF-16 string | `text += u"🔥"_u16c;` |

The range-based overloads are special because they work in terms of validated characters, not raw code units. `append_range("🇷🇴"_utf8_sv.chars())` appends two regional-indicator characters; it does not splice raw UTF-8 bytes into the destination.

### Inspiration

The overall surface intentionally feels familiar to users of [`std::basic_string::append`](https://en.cppreference.com/w/cpp/string/basic_string/append.html), [`std::basic_string::assign`](https://en.cppreference.com/w/cpp/string/basic_string/assign.html), and Rust's [`String`](https://doc.rust-lang.org/std/string/struct.String.html), with additional range forms that work on validated Unicode characters rather than raw code units.

### Return value

Returns `*this`.

### Complexity

Linear in the amount of appended or assigned data.

### Exceptions

May throw allocator or container exceptions.

### `noexcept`

Not `noexcept`.

## Insertion, Erasure, And Reversal

### Synopsis

```cpp
constexpr basic_utf8_string& insert(size_type index, utf8_string_view sv);
constexpr basic_utf8_string& insert(size_type index, utf8_char ch);
constexpr basic_utf8_string& insert(size_type index, size_type count, utf8_char ch);
constexpr basic_utf8_string& insert_range(size_type index, views::utf8_view rg);
constexpr basic_utf8_string& insert_range(size_type index, views::owning_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& insert_range(size_type index, views::owning_reversed_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& insert_range(size_type index, views::utf16_view rg);
constexpr basic_utf8_string& insert_range(size_type index, views::utf32_view rg);
template <details::container_compatible_range<utf8_char> R>
constexpr basic_utf8_string& insert_range(size_type index, R&& rg);
template <std::input_iterator It, std::sentinel_for<It> Sent>
constexpr basic_utf8_string& insert(size_type index, It first, Sent last);
constexpr basic_utf8_string& insert(size_type index, std::initializer_list<utf8_char> ilist);

constexpr std::optional<value_type> pop_back();
constexpr basic_utf8_string& erase(size_type index, size_type count = npos);
constexpr basic_utf8_string& reverse() noexcept;
constexpr basic_utf8_string& reverse(size_type pos, size_type count = npos);
constexpr basic_utf8_string& reverse_graphemes() noexcept;
constexpr basic_utf8_string& reverse_graphemes(size_type pos, size_type count = npos);

// The UTF-16 and UTF-32 types expose the same families with their corresponding view and character types.
```

### Behavior

- Insertion, erasure, partial character reversal, and partial grapheme reversal require valid boundaries.
- `reverse()` reverses characters, not raw code units.
- `reverse_graphemes()` reverses grapheme clusters, not raw code units.
- `pop_back()` removes and returns the last validated character when present.
- Same-encoding `insert_range` overloads can use direct storage paths for `chars()` and rvalue `reversed_chars()` views.

### Overload differences

The examples below use `utf8_string text = "😄🇷🇴✨"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `insert(index, View sv)` | splice one validated substring at a character boundary | `text.insert(4, "🎉"_utf8_sv);` |
| `insert(index, Char ch)` | insert one validated character at a character boundary | `text.insert(4, "🎉"_u8c);` |
| `insert(index, count, Char ch)` | insert `count` copies of one validated character | `text.insert(4, 2, "✨"_u8c);` |
| `insert_range(index, views::utf8_view rg)` | splice a same-encoding character range | `text.insert_range(4, "🎉✨"_utf8_sv.chars());` |
| `insert_range(index, views::utf16_view rg)` | splice a cross-encoding character range with transcoding | `text.insert_range(4, u"🎉✨"_utf16_sv.chars());` |
| `insert_range(index, R&& rg)` | splice a generic range of validated characters | `text.insert_range(4, std::array{"🎉"_u8c, "✨"_u8c});` |
| `insert(index, It, Sent)` / `insert(index, ilist)` | splice validated characters from iterators or a braced list | `text.insert(4, {"🎉"_u8c, "✨"_u8c});` |
| `erase(index, count)` | erase a boundary-aligned validated substring | `text.erase(4, 8);` |
| `reverse()` | reverse the whole string by character | `text.reverse();` |
| `reverse(pos, count)` | reverse one boundary-aligned substring by character | `text.reverse(4, 8);` |
| `reverse_graphemes()` | reverse the whole string by grapheme cluster | `text.reverse_graphemes();` |
| `reverse_graphemes(pos, count)` | reverse one grapheme-boundary-aligned substring by grapheme cluster | `text.reverse_graphemes(0, 12);` |
| `pop_back()` | remove and return the last validated character | `const auto last = text.pop_back();` |

### Inspiration

This family is structurally close to [`std::basic_string::insert`](https://en.cppreference.com/w/cpp/string/basic_string/insert.html), [`erase`](https://en.cppreference.com/w/cpp/string/basic_string/erase.html), and [`replace`](https://en.cppreference.com/w/cpp/string/basic_string/replace.html), but every offset-sensitive overload is Unicode-boundary-aware rather than code-unit-blind.

### Return value

- Mutating members return `*this`.
- `pop_back()` returns the removed character or `std::nullopt` when the string is empty.

### Complexity

Linear in the amount of moved or reversed data.

### Exceptions

- `insert`, `insert_range`, `erase`, `reverse(pos, count)`, and `reverse_graphemes(pos, count)` throw [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) for invalid indices or invalid substring boundaries.
- Allocation may also fail.

### `noexcept`

- `reverse()` without arguments is `noexcept`.
- `reverse_graphemes()` without arguments is `noexcept`.
- The remaining overloads are not `noexcept`.

## Case Mapping, Normalization, And Case Folding

### Synopsis

```cpp
constexpr basic_utf8_string to_ascii_lowercase() const&;
constexpr basic_utf8_string to_ascii_lowercase(size_type pos, size_type count) const&;
constexpr basic_utf8_string to_ascii_lowercase() && noexcept;
constexpr basic_utf8_string to_ascii_lowercase(size_type pos, size_type count) &&;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_ascii_lowercase(const OtherAllocator& alloc) const;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_ascii_lowercase(size_type pos, size_type count, const OtherAllocator& alloc) const;

constexpr basic_utf8_string to_ascii_uppercase() const&;
constexpr basic_utf8_string to_ascii_uppercase(size_type pos, size_type count) const&;
constexpr basic_utf8_string to_ascii_uppercase() && noexcept;
constexpr basic_utf8_string to_ascii_uppercase(size_type pos, size_type count) &&;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_ascii_uppercase(const OtherAllocator& alloc) const;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_ascii_uppercase(size_type pos, size_type count, const OtherAllocator& alloc) const;

constexpr basic_utf8_string to_lowercase() const&;
constexpr basic_utf8_string to_lowercase(size_type pos, size_type count) const&;
constexpr basic_utf8_string to_lowercase() &&;
constexpr basic_utf8_string to_lowercase(size_type pos, size_type count) &&;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_lowercase(const OtherAllocator& alloc) const;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_lowercase(size_type pos, size_type count, const OtherAllocator& alloc) const;

constexpr basic_utf8_string to_uppercase() const&;
constexpr basic_utf8_string to_uppercase(size_type pos, size_type count) const&;
constexpr basic_utf8_string to_uppercase() &&;
constexpr basic_utf8_string to_uppercase(size_type pos, size_type count) &&;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_uppercase(const OtherAllocator& alloc) const;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> to_uppercase(size_type pos, size_type count, const OtherAllocator& alloc) const;

constexpr basic_utf8_string normalize(normalization_form form) const&;
constexpr basic_utf8_string normalize(normalization_form form) &&;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> normalize(normalization_form form, const OtherAllocator& alloc) const;

constexpr basic_utf8_string to_nfc() const&;
constexpr basic_utf8_string to_nfc() &&;
constexpr basic_utf8_string to_nfd() const&;
constexpr basic_utf8_string to_nfd() &&;
constexpr basic_utf8_string to_nfkc() const&;
constexpr basic_utf8_string to_nfkc() &&;
constexpr basic_utf8_string to_nfkd() const&;
constexpr basic_utf8_string to_nfkd() &&;
template <typename OtherAllocator> constexpr basic_utf8_string<OtherAllocator> to_nfc(const OtherAllocator& alloc) const;
template <typename OtherAllocator> constexpr basic_utf8_string<OtherAllocator> to_nfd(const OtherAllocator& alloc) const;
template <typename OtherAllocator> constexpr basic_utf8_string<OtherAllocator> to_nfkc(const OtherAllocator& alloc) const;
template <typename OtherAllocator> constexpr basic_utf8_string<OtherAllocator> to_nfkd(const OtherAllocator& alloc) const;

constexpr basic_utf8_string case_fold() const&;
constexpr basic_utf8_string case_fold() &&;
template <typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> case_fold(const OtherAllocator& alloc) const;

// The UTF-16 and UTF-32 types expose the same method families with basic_utf16_string / basic_utf32_string return types.
```

### Behavior

- `const&` overloads always build a fresh result.
- `&&` overloads may reuse the current allocation when profitable.
- Partial case-transform overloads require both ends of the selected range to be character boundaries.
- `normalize(...)` is whole-string only.
- `case_fold()` implements Unicode case folding for caseless comparison and lookup workflows.

### Overload differences

The examples below use `utf8_string text = "wow 😄"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `to_ascii_lowercase()` / `to_ascii_uppercase()` | ASCII-only mapping; non-ASCII characters such as `😄` are preserved | `const auto loud = text.to_ascii_uppercase();` |
| `to_ascii_lowercase(pos, count)` / `to_ascii_uppercase(pos, count)` | ASCII-only mapping on one boundary-aligned subrange | `const auto loud = text.to_ascii_uppercase(0, 3);` |
| `to_lowercase()` / `to_uppercase()` | full Unicode case mapping on the whole string | `const auto upper = utf8_string{"straße 😄"_utf8_sv}.to_uppercase();` |
| `to_lowercase(pos, count)` / `to_uppercase(pos, count)` | full Unicode case mapping on one boundary-aligned subrange | `const auto upper = text.to_uppercase(0, 3);` |
| allocator-taking overloads | produce the same transformed value with a caller-supplied allocator type | `const auto copy = text.to_uppercase(std::allocator<char8_t>{});` |
| `const&` overloads | keep the source object unchanged and build a fresh result | `const auto a = text.to_uppercase();` |
| `&&` overloads | may reuse the current allocation because the source is disposable | `auto a = utf8_string{"straße 😄"_utf8_sv}.to_uppercase();` |
| `normalize(form)` | choose NFC/NFD/NFKC/NFKD at runtime | `const auto n = utf8_string{"é 😄"_utf8_sv}.normalize(normalization_form::nfc);` |
| `to_nfc()` / `to_nfd()` / `to_nfkc()` / `to_nfkd()` | named normalization wrappers for the common forms | `const auto n = utf8_string{"ｅ́ 😄"_utf8_sv}.to_nfkc();` |
| `case_fold()` | full Unicode case folding for caseless matching, not for presentation | `const auto folded = utf8_string{"Straße 😄"_utf8_sv}.case_fold();` |

Normalization is intentionally whole-string only. Unlike partial case transforms, normalization has cross-boundary composition and decomposition rules, so a `pos, count` overload would be much easier to misuse.

### Inspiration

The ASCII mutators are conceptually close to C++ and Rust ASCII helpers, such as Rust's [`str::make_ascii_uppercase`](https://doc.rust-lang.org/stable/core/primitive.str.html). The full Unicode casing members are closer to Rust's [`str::to_lowercase`](https://doc.rust-lang.org/stable/core/primitive.str.html) and `to_uppercase`. There is no direct standard-library equivalent for full Unicode normalization or case folding in either the C++ or Rust standard libraries, which is why these members are library-specific.

### Return value

Returns a transformed owning string.

### Complexity

Linear in the processed code units, plus extra work for Unicode case expansion and normalization.

### Exceptions

- Partial transforms throw [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) for invalid offsets or invalid UTF substring boundaries.
- Allocation may also fail.

### `noexcept`

- The whole-string ASCII `&&` overloads are `noexcept`.
- The other overloads are not `noexcept`.

### Optional ICU Locale-Aware Overloads

When the library is built with `UTF8_RANGES_ENABLE_ICU=1`, the owning-string types also expose these overload families:

```cpp
basic_utf8_string to_lowercase(locale_id locale) const&;
basic_utf8_string to_lowercase(locale_id locale) &&;
basic_utf8_string to_lowercase(size_type pos, size_type count, locale_id locale) const&;
basic_utf8_string to_lowercase(size_type pos, size_type count, locale_id locale) &&;
template <typename OtherAllocator>
basic_utf8_string<OtherAllocator> to_lowercase(locale_id locale, const OtherAllocator& alloc) const;
template <typename OtherAllocator>
basic_utf8_string<OtherAllocator> to_lowercase(size_type pos, size_type count, locale_id locale, const OtherAllocator& alloc) const;

basic_utf8_string to_uppercase(locale_id locale) const&;
basic_utf8_string to_uppercase(locale_id locale) &&;
basic_utf8_string to_uppercase(size_type pos, size_type count, locale_id locale) const&;
basic_utf8_string to_uppercase(size_type pos, size_type count, locale_id locale) &&;
template <typename OtherAllocator>
basic_utf8_string<OtherAllocator> to_uppercase(locale_id locale, const OtherAllocator& alloc) const;
template <typename OtherAllocator>
basic_utf8_string<OtherAllocator> to_uppercase(size_type pos, size_type count, locale_id locale, const OtherAllocator& alloc) const;

basic_utf8_string to_titlecase(locale_id locale) const&;
basic_utf8_string to_titlecase(locale_id locale) &&;
template <typename OtherAllocator>
basic_utf8_string<OtherAllocator> to_titlecase(locale_id locale, const OtherAllocator& alloc) const;

basic_utf8_string case_fold(locale_id locale) const&;
basic_utf8_string case_fold(locale_id locale) &&;
template <typename OtherAllocator>
basic_utf8_string<OtherAllocator> case_fold(locale_id locale, const OtherAllocator& alloc) const;

// The UTF-16 and UTF-32 types expose the same locale-aware families with basic_utf16_string / basic_utf32_string return types.
```

- These overloads exist only when ICU support is enabled.
- `const&`, `&&`, and allocator-taking locale overloads follow the same ownership rules as the default casing members.
- `to_titlecase(locale)` delegates to ICU titlecasing and lowercases the rest of each titlecased span according to ICU's rules.
- `to_titlecase(locale)` is whole-string only. The library intentionally does not expose partial `pos, count` titlecasing overloads because titlecasing depends on break-iterator context.
- `case_fold(locale)` uses ICU fold options derived from the locale. In practice, the meaningful difference is the Turkic special-I fold; other locales normally produce the same result as `case_fold()`.
- `locale_id` is a non-owning null-terminated token, so raw `locale_id{ ... }` values must outlive the call.
- The locale-aware overloads reject `locale_id{nullptr}` with `std::invalid_argument`.
- Otherwise they pass the locale name through to ICU, which may canonicalize it or fall back to a more general locale instead of failing.
- ICU normalization or casing failures surface as `std::runtime_error`.

```cpp
--8<-- "examples/casing/locale-case.cpp"
```

## Case-Insensitive Comparison Helpers

### Synopsis

```cpp
constexpr bool eq_ignore_case(utf8_string_view sv) const noexcept;
constexpr bool starts_with_ignore_case(utf8_string_view sv) const noexcept;
constexpr bool ends_with_ignore_case(utf8_string_view sv) const noexcept;
constexpr std::weak_ordering compare_ignore_case(utf8_string_view sv) const noexcept;

constexpr bool eq_ignore_case(utf16_string_view sv) const noexcept;
constexpr bool starts_with_ignore_case(utf16_string_view sv) const noexcept;
constexpr bool ends_with_ignore_case(utf16_string_view sv) const noexcept;
constexpr std::weak_ordering compare_ignore_case(utf16_string_view sv) const noexcept;
```

### Behavior

- These helpers operate on the owning string's current contents without allocating.
- They compare Unicode case-folded scalar sequences.
- They do not normalize. Canonically equivalent representations still compare different unless the caller normalizes first.
- This is deliberate: normalization remains an explicit caller choice rather than hidden work inside the case-insensitive comparison helpers.
- `compare_ignore_case(...)` is lexicographic comparison of the folded scalar stream, not locale collation.

### Return value

- The boolean helpers report whether the folded comparison succeeds.
- `compare_ignore_case(...)` returns [`std::weak_ordering`](https://en.cppreference.com/w/cpp/utility/compare/weak_ordering).

### Complexity

Linear in the amount of text read from both operands.

### Exceptions

Do not throw.

### `noexcept`

`noexcept`

### Example

```cpp
--8<-- "examples/casing/ignore-case.cpp"
```

### Optional ICU Locale-Aware Overloads

When the library is built with `UTF8_RANGES_ENABLE_ICU=1`, the owning-string types also expose:

```cpp
bool eq_ignore_case(utf8_string_view sv, locale_id locale) const;
bool starts_with_ignore_case(utf8_string_view sv, locale_id locale) const;
bool ends_with_ignore_case(utf8_string_view sv, locale_id locale) const;
std::weak_ordering compare_ignore_case(utf8_string_view sv, locale_id locale) const;

bool eq_ignore_case(utf16_string_view sv, locale_id locale) const;
bool starts_with_ignore_case(utf16_string_view sv, locale_id locale) const;
bool ends_with_ignore_case(utf16_string_view sv, locale_id locale) const;
std::weak_ordering compare_ignore_case(utf16_string_view sv, locale_id locale) const;
```

- These overloads still stream the current contents instead of allocating a temporary folded string.
- They keep the same explicit non-normalizing semantics as the default helpers.
- They are not `noexcept` because locale handling follows the same ICU-backed rules as the other locale-aware casing members.

## Copying Replacement Families

### Synopsis

```cpp
constexpr basic_utf8_string replace_all(utf8_char from, utf8_char to) const&;
constexpr basic_utf8_string replace_all(utf8_char from, utf8_string_view to) const&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_char to) const&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_string_view to) const&;
constexpr basic_utf8_string replace_all(std::span<const utf8_char> from, utf8_char to) const&;
constexpr basic_utf8_string replace_all(std::span<const utf8_char> from, utf8_string_view to) const&;

constexpr basic_utf8_string replace_all(utf8_char from, utf8_char to) &&;
constexpr basic_utf8_string replace_all(utf8_char from, utf8_string_view to) &&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_char to) &&;
constexpr basic_utf8_string replace_all(utf8_string_view from, utf8_string_view to) &&;
constexpr basic_utf8_string replace_all(std::span<const utf8_char> from, utf8_char to) &&;
constexpr basic_utf8_string replace_all(std::span<const utf8_char> from, utf8_string_view to) &&;

template <typename OtherAllocator> constexpr basic_utf8_string<OtherAllocator> replace_all(..., const OtherAllocator& alloc) const;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_all(Pred pred, utf8_char to) const&;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_all(Pred pred, utf8_string_view to) const&;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_all(Pred pred, utf8_char to) &&;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_all(Pred pred, utf8_string_view to) &&;
template <details::utf8_char_predicate Pred, typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> replace_all(Pred pred, ..., const OtherAllocator& alloc) const;

constexpr basic_utf8_string replace_n(size_type count, utf8_char from, utf8_char to) const&;
constexpr basic_utf8_string replace_n(size_type count, utf8_char from, utf8_string_view to) const&;
constexpr basic_utf8_string replace_n(size_type count, utf8_string_view from, utf8_char to) const&;
constexpr basic_utf8_string replace_n(size_type count, utf8_string_view from, utf8_string_view to) const&;
constexpr basic_utf8_string replace_n(size_type count, std::span<const utf8_char> from, utf8_char to) const&;
constexpr basic_utf8_string replace_n(size_type count, std::span<const utf8_char> from, utf8_string_view to) const&;

constexpr basic_utf8_string replace_n(size_type count, utf8_char from, utf8_char to) &&;
constexpr basic_utf8_string replace_n(size_type count, utf8_char from, utf8_string_view to) &&;
constexpr basic_utf8_string replace_n(size_type count, utf8_string_view from, utf8_char to) &&;
constexpr basic_utf8_string replace_n(size_type count, utf8_string_view from, utf8_string_view to) &&;
constexpr basic_utf8_string replace_n(size_type count, std::span<const utf8_char> from, utf8_char to) &&;
constexpr basic_utf8_string replace_n(size_type count, std::span<const utf8_char> from, utf8_string_view to) &&;

template <typename OtherAllocator> constexpr basic_utf8_string<OtherAllocator> replace_n(..., const OtherAllocator& alloc) const;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_n(size_type count, Pred pred, utf8_char to) const&;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_n(size_type count, Pred pred, utf8_string_view to) const&;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_n(size_type count, Pred pred, utf8_char to) &&;
template <details::utf8_char_predicate Pred> constexpr basic_utf8_string replace_n(size_type count, Pred pred, utf8_string_view to) &&;
template <details::utf8_char_predicate Pred, typename OtherAllocator>
constexpr basic_utf8_string<OtherAllocator> replace_n(size_type count, Pred pred, ..., const OtherAllocator& alloc) const;

// The UTF-16 and UTF-32 types expose the same families with their corresponding character and view types.
```

### Behavior

- `const&` overloads build a replacement copy.
- `&&` overloads may reuse and rewrite the current storage.
- [`std::span`](https://en.cppreference.com/w/cpp/container/span) overloads treat the span as a set of characters.
- Predicate overloads replace each character for which the predicate returns `true`.
- `replace_n` stops after at most `count` replacements.

### Overload differences

The examples below use `const auto text = "😄🇷🇴✨"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `replace_all(Char from, Char to)` | replace one exact validated character with another | `text.replace_all("✨"_u8c, "🔥"_u8c)` |
| `replace_all(Char from, View to)` | replace one character with a validated substring | `text.replace_all("✨"_u8c, "🎉🎉"_utf8_sv)` |
| `replace_all(View from, Char to)` | replace one validated substring with one character | `text.replace_all("🇷🇴"_utf8_sv, "🎉"_u8c)` |
| `replace_all(View from, View to)` | replace one validated substring with another | `text.replace_all("🇷🇴"_utf8_sv, "🎉"_utf8_sv)` |
| `replace_all(std::span<const Char> from, Char/View to)` | replace every character that belongs to a character set | `text.replace_all(std::array{"😄"_u8c, "✨"_u8c}, "🎉"_u8c)` |
| `replace_all(Pred pred, Char/View to)` | replace every character satisfying a predicate | `text.replace_all([](utf8_char ch) { return !ch.is_ascii(); }, "⭐"_utf8_sv)` |
| `replace_n(count, ...)` | same matching rules as `replace_all`, but stop after at most `count` replacements | `text.replace_n(1, "✨"_u8c, "🔥"_u8c)` |
| `const&` overloads | keep the source string unchanged and return a copy | `const auto a = text.replace_all("✨"_u8c, "🔥"_u8c);` |
| `&&` overloads | may reuse the source allocation because the source is disposable | `auto a = utf8_string{"😄🇷🇴✨"_utf8_sv}.replace_all("✨"_u8c, "🔥"_u8c);` |
| allocator-taking overloads | return the same logical result with a caller-supplied allocator type | `const auto a = text.replace_all("✨"_u8c, "🔥"_u8c, std::allocator<char8_t>{});` |

The span overload is special because it is character-set based rather than substring-based. `std::array{"🇷"_u8c, "🇴"_u8c}` matches either regional-indicator character independently; it does not wait for the adjacent grapheme `🇷🇴`.

### Inspiration

This family extends the spirit of [C++ `std::basic_string::replace`](https://en.cppreference.com/w/cpp/string/basic_string/replace.html) and Rust's [`str`/`String` replacement APIs](https://doc.rust-lang.org/std/string/struct.String.html) with character-set and predicate-driven Unicode-aware overloads.

### Return value

Returns the replaced owning string.

### Complexity

Linear in the source size plus the size of the produced output.

### Exceptions

May throw allocator or container exceptions.

### `noexcept`

Not `noexcept`.

## In-Place Replacement

### Synopsis

```cpp
constexpr basic_utf8_string& replace_inplace(size_type pos, size_type count, utf8_string_view other);
constexpr basic_utf8_string& replace_inplace(size_type pos, size_type count, utf8_char other);
constexpr basic_utf8_string& replace_inplace(size_type pos, utf8_string_view other);
constexpr basic_utf8_string& replace_inplace(size_type pos, utf8_char other);

constexpr basic_utf8_string& replace_with_range_inplace(size_type pos, size_type count, views::utf8_view rg);
constexpr basic_utf8_string& replace_with_range_inplace(
    size_type pos,
    size_type count,
    views::owning_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& replace_with_range_inplace(
    size_type pos,
    size_type count,
    views::owning_reversed_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& replace_with_range_inplace(size_type pos, size_type count, views::utf16_view rg);
template <details::container_compatible_range<utf8_char> R>
constexpr basic_utf8_string& replace_with_range_inplace(size_type pos, size_type count, R&& rg);

constexpr basic_utf8_string& replace_with_range_inplace(size_type pos, views::utf8_view rg);
constexpr basic_utf8_string& replace_with_range_inplace(
    size_type pos,
    views::owning_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& replace_with_range_inplace(
    size_type pos,
    views::owning_reversed_chars_view<basic_utf8_string>&& rg);
constexpr basic_utf8_string& replace_with_range_inplace(size_type pos, views::utf16_view rg);
template <details::container_compatible_range<utf8_char> R>
constexpr basic_utf8_string& replace_with_range_inplace(size_type pos, R&& rg);

// The UTF-16 and UTF-32 types expose the same families with their corresponding view, character, and helper-view types.
```

### Behavior

- Count-based overloads replace a validated substring `[pos, pos + count)` after clamping `count` to the remaining length.
- Single-position overloads replace the one validated character that starts at `pos`.
- `replace_with_range_inplace` accepts validated character ranges, including cross-encoding helper views.
- Same-encoding `chars()` and rvalue `reversed_chars()` view overloads can use direct materialization paths.

### Overload differences

The examples below use `utf8_string text = "wow 😄✨"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `replace_inplace(pos, count, View other)` | replace one boundary-aligned validated substring with another | `text.replace_inplace(0, 3, "hey"_utf8_sv);` |
| `replace_inplace(pos, count, Char other)` | replace one boundary-aligned validated substring with one character | `text.replace_inplace(4, 4, "🔥"_u8c);` |
| `replace_inplace(pos, View other)` | replace the single validated character starting at `pos` with a substring | `text.replace_inplace(4, "🎉"_utf8_sv);` |
| `replace_inplace(pos, Char other)` | replace the single validated character starting at `pos` with one character | `text.replace_inplace(4, "🔥"_u8c);` |
| `replace_with_range_inplace(pos, count, views::utf8_view rg)` | replace a boundary-aligned substring with a same-encoding character range | `text.replace_with_range_inplace(4, 4, "🎉✨"_utf8_sv.chars());` |
| `replace_with_range_inplace(pos, count, views::utf16_view rg)` | replace a boundary-aligned substring with a cross-encoding character range | `text.replace_with_range_inplace(4, 4, u"🎉✨"_utf16_sv.chars());` |
| `replace_with_range_inplace(pos, count, R&& rg)` | replace a boundary-aligned substring with a generic range of validated characters | `text.replace_with_range_inplace(4, 4, std::array{"🎉"_u8c, "✨"_u8c});` |
| `replace_with_range_inplace(pos, rg)` | replace the single validated character at `pos` with a validated range | `text.replace_with_range_inplace(4, "🎉"_utf8_sv.chars());` |

The range overloads are special because the replacement is driven by validated characters, not by raw code units. Cross-encoding helper views let the caller describe the replacement in the other encoding without building a temporary owning string first.

Rvalue owning same-encoding views, such as `std::move(other).chars()` and `std::move(other).reversed_chars()`, are accepted directly and may reuse owned storage where the replacement shape allows it.

### Inspiration

This family is closest in spirit to the in-place [`std::basic_string::replace`](https://en.cppreference.com/w/cpp/string/basic_string/replace.html) overload set, but extended with character-range replacement and cross-encoding range sources.

### Return value

Returns `*this`.

### Complexity

Linear in the replaced span plus the size of the replacement range.

### Exceptions

Throws [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) when `pos` is out of range or when the affected range is not a valid UTF substring. Allocation may also fail.

### `noexcept`

Not `noexcept`.

## Capacity, Raw Access, And Borrowed Views

### Synopsis

```cpp
constexpr void shrink_to_fit();
constexpr size_type capacity() const;
constexpr allocator_type get_allocator() const noexcept;
constexpr size_type size() const;
constexpr void reserve(size_type new_cap);
constexpr auto base() const& noexcept -> const base_type&;
constexpr auto base() && noexcept -> base_type&&;
constexpr void clear();
constexpr const char8_t* data() const noexcept;
constexpr const char8_t* c_str() const noexcept;
constexpr equivalent_utf8_string_view as_view() const noexcept;
constexpr operator utf8_string_view() const noexcept;
constexpr void push_back(utf8_char ch);
constexpr void swap(basic_utf8_string& other) noexcept(/* conditional */);

// The UTF-16 and UTF-32 types expose the same families with their corresponding raw code-unit and validated view types.
```

### Behavior

- `base()` exposes the underlying `std::basic_string` storage.
- `as_view()` and the implicit conversion create a borrowed validated view over the current contents.
- `data()` and `c_str()` expose raw code units.
- `push_back()` appends one validated character.
- `swap()` swaps the underlying storage.

### Overload differences

The examples below use `utf8_string text = "😄🇷🇴✨"_utf8_s;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `base() const&` | borrow the underlying `std::u8string` / `std::u16string` storage object | `const std::u8string& raw = text.base();` |
| `base() &&` | move the underlying standard string out of a disposable owning string | `std::u8string raw = std::move(text).base();` |
| `as_view() const` | explicitly create a borrowed `utf8_string_view` / `utf16_string_view` | `utf8_string_view borrowed = text.as_view();` |
| `operator utf8_string_view() const` | implicitly view the string as a borrowed validated view when a view is expected | `utf8_string_view borrowed = text;` |
| `data()` / `c_str()` | expose raw code units for interop with code-unit-oriented APIs | `const char8_t* ptr = text.data();` |
| `reserve(new_cap)` / `shrink_to_fit()` | manage capacity in terms of code units | `text.reserve(32);` |
| `push_back(Char ch)` | append one validated character | `text.push_back("🔥"_u8c);` |
| `swap(other)` | exchange the underlying storage | `text.swap(other);` |

`base()` and `as_view()` solve different problems. `base()` is for interop with APIs that truly need the owning `std::basic_string`. `as_view()` is for APIs that only need a validated borrowed text view and should not take ownership.

### Inspiration

Capacity management follows the vocabulary of [`std::basic_string`](https://en.cppreference.com/w/cpp/string/basic_string/reserve.html). The explicit `as_view()` member is similar in spirit to constructing a [`std::basic_string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view) from a `std::basic_string`, but it preserves the library's validated Unicode view type rather than erasing back to raw code units.

### Return value

- `capacity()`, `get_allocator()`, `size()`, `data()`, `c_str()`, `base()`, and `as_view()` return the corresponding storage handle or observer.
- The mutating members return `void`.

### Complexity

- Observers are constant.
- `reserve`, `shrink_to_fit`, `clear`, `push_back`, and `swap` match the complexity profile of the underlying `std::basic_string`.

### Exceptions

- `get_allocator()`, `data()`, `c_str()`, `base()`, and `as_view()` do not throw.
- `reserve`, `push_back`, and `shrink_to_fit` may throw allocator or container exceptions.

### `noexcept`

- `get_allocator()`, `base()`, `data()`, `c_str()`, and `as_view()` are `noexcept`.
- `swap()` is conditionally `noexcept`.

## Comparison, Concatenation, Formatting, And Literals

### Synopsis

```cpp
friend constexpr bool operator==(const basic_utf8_string&, const basic_utf8_string&) noexcept;
friend constexpr bool operator==(const basic_utf8_string&, utf8_string_view) noexcept;
friend constexpr bool operator==(utf8_string_view, const basic_utf8_string&) noexcept;
friend constexpr auto operator<=>(const basic_utf8_string&, const basic_utf8_string&) noexcept;
friend constexpr auto operator<=>(const basic_utf8_string&, utf8_string_view) noexcept;
friend constexpr auto operator<=>(utf8_string_view, const basic_utf8_string&) noexcept;

friend constexpr basic_utf8_string operator+(const basic_utf8_string&, const basic_utf8_string&);
friend constexpr basic_utf8_string operator+(basic_utf8_string&&, const basic_utf8_string&);
friend constexpr basic_utf8_string operator+(const basic_utf8_string&, basic_utf8_string&&);
friend constexpr basic_utf8_string operator+(basic_utf8_string&&, basic_utf8_string&&);
friend constexpr basic_utf8_string operator+(const basic_utf8_string&, utf8_string_view);
friend constexpr basic_utf8_string operator+(basic_utf8_string&&, utf8_string_view);
friend constexpr basic_utf8_string operator+(utf8_string_view, const basic_utf8_string&);
friend constexpr basic_utf8_string operator+(utf8_string_view, basic_utf8_string&&);
friend constexpr basic_utf8_string operator+(const basic_utf8_string&, utf8_char);
friend constexpr basic_utf8_string operator+(basic_utf8_string&&, utf8_char);
friend constexpr basic_utf8_string operator+(utf8_char, const basic_utf8_string&);
friend constexpr basic_utf8_string operator+(utf8_char, basic_utf8_string&&);

template<typename Allocator> std::ostream& operator<<(std::ostream&, const basic_utf8_string<Allocator>&);
template<typename Allocator> struct std::formatter<basic_utf8_string<Allocator>, char>;
template<typename Allocator, typename OtherAllocator> struct std::uses_allocator<basic_utf8_string<Allocator>, OtherAllocator>;

template<details::literals::constexpr_utf8_string Str>
constexpr auto operator ""_utf8_s();

// The UTF-16 and UTF-32 types expose the parallel comparison, concatenation, formatting, and literal families.
```

### Behavior

- Comparison operators compare encoded contents.
- `operator+` builds a new owning string.
- Stream insertion and the formatter delegate to the borrowed view representation.
- `_utf8_s`, `_utf16_s`, and `_utf32_s` are compile-time validated owning-string literals.

### Overload differences

| Overload | Meaning | Example |
| --- | --- | --- |
| `operator==` / `<=>` with another owning string | compare two owning strings in the same encoding | `"😄"_utf8_s == "😄"_utf8_s` |
| `operator==` / `<=>` with a view | compare an owning string against a borrowed validated view | `"😄"_utf8_s == "😄"_utf8_sv` |
| `operator+(String, String)` | concatenate two owning strings | `"😄"_utf8_s + "✨"_utf8_s` |
| `operator+(String, View)` / `operator+(View, String)` | concatenate an owning string with a borrowed validated substring | `"😄"_utf8_s + "✨"_utf8_sv` |
| `operator+(String, Char)` / `operator+(Char, String)` | concatenate one validated character | `"😄"_utf8_s + "✨"_u8c` |
| `_utf8_s` / `_utf16_s` / `_utf32_s` | construct a compile-time validated owning string literal | `constexpr auto text = "😄🇷🇴✨"_utf8_s;` |

### Inspiration

Comparison and concatenation follow the general shape of [`std::basic_string`](https://en.cppreference.com/w/cpp/string/basic_string/basic_string) and Rust's [`String`](https://doc.rust-lang.org/std/string/struct.String.html), but the literal forms add compile-time Unicode validation instead of accepting arbitrary raw code units.

### Complexity

- Comparison is linear in the compared prefix.
- Concatenation is linear in the size of the produced string.
- Streaming and formatting are linear in the amount of text written.

### Exceptions

- Comparison does not throw.
- Concatenation, formatting, and owning-string literal materialization may throw allocator or container exceptions.
- Invalid `_utf8_s`, `_utf16_s`, or `_utf32_s` literals are rejected during constant evaluation.

### `noexcept`

Only the comparison operators are `noexcept`.
