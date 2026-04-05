# String Views

`utf8_string_view` and `utf16_string_view` are borrowed validated text views.

They expose most of the library's read-only Unicode surface: validated iteration, boundary-aware access, raw code-unit search, character-aware search, grapheme-aware search, split/trim views, and owning transformations.

Unlike the helper adapters in `unicode_ranges::views`, these types are not themselves direct subclasses of [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface). They are validated string-view classes with a `std::basic_string_view`-like owning model and a string-oriented API surface. The lazy range adapters returned by members such as `chars()`, `graphemes()`, `split(...)`, and `matches(...)` are the actual view types.

When a signature block uses `Char`, `View`, or `Predicate`, it refers to the encoding-specific type family for the current section:

- UTF-8: `utf8_char`, `utf8_string_view`, `details::utf8_char_predicate`
- UTF-16: `utf16_char`, `utf16_string_view`, `details::utf16_char_predicate`

## Construction And Raw View Access

### Synopsis

```cpp
class utf8_string_view {
public:
    utf8_string_view() = default;

    static constexpr std::expected<utf8_string_view, utf8_error>
    from_bytes(std::u8string_view bytes) noexcept;

    static constexpr utf8_string_view
    from_bytes_unchecked(std::u8string_view bytes) noexcept;

    constexpr std::u8string_view base() const noexcept;
    constexpr std::u8string_view as_view() const noexcept;
    constexpr operator std::u8string_view() const noexcept;
};

class utf16_string_view {
public:
    utf16_string_view() = default;

    static constexpr std::expected<utf16_string_view, utf16_error>
    from_code_units(std::u16string_view code_units) noexcept;

    static constexpr utf16_string_view
    from_code_units_unchecked(std::u16string_view code_units) noexcept;

    constexpr std::u16string_view base() const noexcept;
    constexpr std::u16string_view as_view() const noexcept;
    constexpr operator std::u16string_view() const noexcept;
};
```

### Behavior

- Checked factories validate the source encoding.
- Unchecked factories assume the input is already valid.
- `base()`, `as_view()`, and the implicit conversion expose the corresponding standard-library [`std::basic_string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view).

### Return value

- Checked factories return [`std::unexpected(...)`](https://en.cppreference.com/w/cpp/utility/expected/unexpected) on invalid UTF data.
- Unchecked factories and raw-view accessors return the view directly.

### Complexity

- Checked factories are linear in the number of code units.
- Unchecked factories and raw-view accessors are constant.

### Exceptions

None.

### `noexcept`

All listed overloads are `noexcept`.

### Example

```cpp
--8<-- "examples/getting-started/validated-view.cpp"
```

## Comparison, Streaming, Hashing, And Formatting

### Synopsis

```cpp
friend constexpr bool operator==(const utf8_string_view&, const utf8_string_view&) noexcept;
friend constexpr auto operator<=>(const utf8_string_view&, const utf8_string_view&) noexcept;
std::ostream& operator<<(std::ostream&, utf8_string_view);
template<> struct std::hash<utf8_string_view>;
template<> struct std::formatter<utf8_string_view, char>;

friend constexpr bool operator==(const utf16_string_view&, const utf16_string_view&) noexcept;
friend constexpr auto operator<=>(const utf16_string_view&, const utf16_string_view&) noexcept;
std::ostream& operator<<(std::ostream&, utf16_string_view);
template<> struct std::hash<utf16_string_view>;
template<> struct std::formatter<utf16_string_view, char>;
```

### Behavior

- Equality and ordering compare encoded contents lexicographically.
- UTF-8 streams directly to `std::ostream`.
- UTF-16 converts each scalar to UTF-8 when written to `std::ostream`.
- The [`std::formatter`](https://en.cppreference.com/w/cpp/utility/format/formatter) specializations format textual output.
- The [`std::hash`](https://en.cppreference.com/w/cpp/utility/hash) specializations hash the underlying standard-library string view.

### Complexity

- Comparison is linear in the compared prefix.
- Streaming, hashing, and formatting are linear in the amount of text processed.

### Exceptions

- Comparison and hashing do not throw.
- Streaming may report stream errors through the stream state.
- UTF-16 formatting may allocate internally while transcoding.

### `noexcept`

- Comparison and hashing are non-throwing.
- Streaming and formatting are not `noexcept`.

## Iteration Families

### Synopsis

```cpp
constexpr auto chars() const noexcept;
constexpr auto reversed_chars() const noexcept;
constexpr auto graphemes() const noexcept;
constexpr auto char_indices() const noexcept;
constexpr auto grapheme_indices() const noexcept;
```

### Behavior

- `chars()` iterates validated Unicode scalar values.
- `reversed_chars()` iterates the same scalar values from the end.
- `graphemes()` iterates default Unicode grapheme clusters.
- `char_indices()` yields [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) objects of the form `(offset, Char)`.
- `grapheme_indices()` yields [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) objects of the form `(offset, View)`.
- All returned ranges borrow from the underlying view.
- All five returned range types are lazy views derived from [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface).
- These five core iteration families expose forward iterators, so they are multi-pass and may be traversed more than once as long as the underlying source view remains alive.

### Return value

Returns a lightweight range or view object.

### Complexity

- Constructing the range is constant.
- Iterating the whole range is linear in the number of scalars or grapheme clusters.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

### Example

```cpp
--8<-- "examples/getting-started/formatting.cpp"
```

## Size, Emptiness, ASCII, And Counts

### Synopsis

```cpp
constexpr size_type size() const noexcept;
constexpr bool empty() const noexcept;
constexpr bool is_ascii() const noexcept;
constexpr size_type char_count() const noexcept;
constexpr size_type grapheme_count() const noexcept;
```

### Behavior

- `size()` counts code units.
- `empty()` tests for zero code units.
- `is_ascii()` returns `true` only if all code units are ASCII.
- `char_count()` counts Unicode scalar values.
- `grapheme_count()` counts default Unicode grapheme clusters.

### Complexity

- `size()` and `empty()` are constant.
- `is_ascii()`, `char_count()`, and `grapheme_count()` are linear in the view length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## Normalization Queries

### Synopsis

```cpp
constexpr bool is_normalized(normalization_form form) const;
constexpr bool is_nfc() const;
constexpr bool is_nfd() const;
constexpr bool is_nfkc() const;
constexpr bool is_nfkd() const;
```

### Behavior

These members normalize the current contents into the requested form and compare the result against the original view.

### Return value

Returns `true` when the current view is already in the requested normalization form.

### Complexity

Linear to super-linear in the input length, depending on the amount of Unicode decomposition and composition required.

### Exceptions

May throw allocator or container exceptions while materializing the normalized copy.

### `noexcept`

Not `noexcept`.

## `contains`

### Synopsis

```cpp
constexpr bool contains(utf8_char ch) const noexcept;
constexpr bool contains(utf8_string_view sv) const noexcept;
constexpr bool contains(std::span<const utf8_char> chars) const noexcept;
template <details::utf8_char_predicate Pred>
constexpr bool contains(Pred pred) const noexcept;

constexpr bool contains(utf16_char ch) const noexcept;
constexpr bool contains(utf16_string_view sv) const noexcept;
constexpr bool contains(std::span<const utf16_char> chars) const noexcept;
template <details::utf16_char_predicate Pred>
constexpr bool contains(Pred pred) const noexcept;
```

### Behavior

- Character, view, span, and predicate overloads are character-aware.
- The [`std::span`](https://en.cppreference.com/w/cpp/container/span) overload treats the span as a character set rather than as one contiguous substring.

### Overload differences

The examples below use `constexpr auto text = "😄🇷🇴✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `contains(Char ch)` | exact validated character search | `text.contains("✨"_u8c)` |
| `contains(View sv)` | exact validated substring search | `text.contains("🇷🇴"_utf8_sv)` |
| `contains(std::span<const Char> chars)` | character-set membership: succeeds if **any one character** in the text equals **any one element** of the span | `text.contains(std::array{"🔥"_u8c, "✨"_u8c})` |
| `contains(Pred pred)` | predicate match on validated characters | `text.contains([](utf8_char ch) { return !ch.is_ascii(); })` |

The span overload is special because it is **not** substring matching. `std::array{"🇷"_u8c, "🇴"_u8c}` does not mean "find the grapheme `🇷🇴`" and it does not require adjacent characters. It means "match a single character that is either `🇷` or `🇴`".

### Inspiration

The character and view overloads are close in spirit to [C++ `std::basic_string_view::contains`](https://en.cppreference.com/w/cpp/string/basic_string_view.html) and Rust's [`str`](https://doc.rust-lang.org/stable/core/primitive.str.html) search surface. The span and predicate overloads extend that familiar shape with character-set and predicate-based matching.

### Return value

Equivalent to `find(...) != npos`.

### Complexity

Linear in the view length.

### Exceptions

None.

### `noexcept`

All overloads are `noexcept`.

## Grapheme-Aware Search

### Synopsis

```cpp
constexpr bool contains_grapheme(utf8_char ch) const noexcept;
constexpr bool contains_grapheme(utf8_string_view sv) const noexcept;
constexpr size_type find_grapheme(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_grapheme(utf8_string_view sv, size_type pos = 0) const noexcept;
constexpr size_type rfind_grapheme(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type rfind_grapheme(utf8_string_view sv, size_type pos = npos) const noexcept;

constexpr bool contains_grapheme(utf16_char ch) const noexcept;
constexpr bool contains_grapheme(utf16_string_view sv) const noexcept;
constexpr size_type find_grapheme(utf16_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_grapheme(utf16_string_view sv, size_type pos = 0) const noexcept;
constexpr size_type rfind_grapheme(utf16_char ch, size_type pos = npos) const noexcept;
constexpr size_type rfind_grapheme(utf16_string_view sv, size_type pos = npos) const noexcept;
```

### Behavior

These overloads only report matches that begin on grapheme boundaries.

### Return value

Returns the matching offset in UTF-8 bytes or UTF-16 code units, or `npos`.

### Complexity

Linear in the view length.

### Exceptions

None.

### `noexcept`

All overloads are `noexcept`.

## `find` And `rfind`

### Synopsis

```cpp
constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf8_string_view sv, size_type pos = 0) const noexcept;
constexpr size_type find(std::span<const utf8_char> chars, size_type pos = 0) const noexcept;
template <details::utf8_char_predicate Pred>
constexpr size_type find(Pred pred, size_type pos = 0) const noexcept;

constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf8_string_view sv, size_type pos = npos) const noexcept;
constexpr size_type rfind(std::span<const utf8_char> chars, size_type pos = npos) const noexcept;
template <details::utf8_char_predicate Pred>
constexpr size_type rfind(Pred pred, size_type pos = npos) const noexcept;

constexpr size_type find(char16_t ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf16_char ch, size_type pos = 0) const noexcept;
constexpr size_type find(utf16_string_view sv, size_type pos = 0) const noexcept;
constexpr size_type find(std::span<const utf16_char> chars, size_type pos = 0) const noexcept;
template <details::utf16_char_predicate Pred>
constexpr size_type find(Pred pred, size_type pos = 0) const noexcept;

constexpr size_type rfind(char16_t ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf16_char ch, size_type pos = npos) const noexcept;
constexpr size_type rfind(utf16_string_view sv, size_type pos = npos) const noexcept;
constexpr size_type rfind(std::span<const utf16_char> chars, size_type pos = npos) const noexcept;
template <details::utf16_char_predicate Pred>
constexpr size_type rfind(Pred pred, size_type pos = npos) const noexcept;
```

### Behavior

- The raw `char8_t` and `char16_t` overloads search code units directly.
- The `Char`, `View`, span, and predicate overloads are character-aware.
- Character-aware forward searches align `pos` upward to the next valid character boundary.
- Character-aware reverse searches align `pos` downward to the previous valid character boundary.
- The span overload treats the span as a character set.

### Overload differences

The examples below use `constexpr auto text = "😄-🇷🇴-✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `find(char8_t ch, pos)` | raw code-unit search; usually most useful for ASCII punctuation or diagnostics | `text.find(u8'-') == 4` |
| `find(Char ch, pos)` | exact validated character search | `text.find("✨"_u8c) == 14` |
| `find(View sv, pos)` | exact validated substring search | `text.find("🇷🇴"_utf8_sv) == 5` |
| `find(std::span<const Char> chars, pos)` | first character belonging to a character set | `text.find(std::array{"🔥"_u8c, "✨"_u8c}) == 14` |
| `find(Pred pred, pos)` | first validated character satisfying a predicate | `text.find([](utf8_char ch) { return ch.is_ascii_punctuation(); }) == 4` |

The same distinctions apply to `rfind(...)`, but searching from the end.

### Inspiration

The raw-code-unit and exact-substring forms are intentionally familiar to users of [C++ `std::basic_string_view::find` and `rfind`](https://en.cppreference.com/w/cpp/string/basic_string_view.html). The predicate-oriented forms are closer to Rust's [`str`](https://doc.rust-lang.org/stable/core/primitive.str.html) pattern-heavy search APIs.

### Return value

Returns the matching code-unit offset, or `npos`.

### Complexity

Linear in the number of remaining code units or scalars.

### Exceptions

None.

### `noexcept`

All overloads are `noexcept`.

### Example

```cpp
--8<-- "examples/text-operations/search-and-replace.cpp"
```

## `find_first_of`, `find_first_not_of`, `find_last_of`, `find_last_not_of`

### Synopsis

```cpp
constexpr size_type find_first_of(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_of(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_of(utf8_string_view sv, size_type pos = 0) const noexcept;

constexpr size_type find_first_not_of(char8_t ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(utf8_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(utf8_string_view sv, size_type pos = 0) const noexcept;

constexpr size_type find_last_of(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_of(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_of(utf8_string_view sv, size_type pos = npos) const noexcept;

constexpr size_type find_last_not_of(char8_t ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(utf8_char ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(utf8_string_view sv, size_type pos = npos) const noexcept;

constexpr size_type find_first_of(char16_t ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_of(utf16_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_of(utf16_string_view sv, size_type pos = 0) const noexcept;

constexpr size_type find_first_not_of(char16_t ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(utf16_char ch, size_type pos = 0) const noexcept;
constexpr size_type find_first_not_of(utf16_string_view sv, size_type pos = 0) const noexcept;

constexpr size_type find_last_of(char16_t ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_of(utf16_char ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_of(utf16_string_view sv, size_type pos = npos) const noexcept;

constexpr size_type find_last_not_of(char16_t ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(utf16_char ch, size_type pos = npos) const noexcept;
constexpr size_type find_last_not_of(utf16_string_view sv, size_type pos = npos) const noexcept;
```

### Behavior

- Raw code-unit overloads examine code units directly.
- `Char` and `View` overloads are character-aware.
- `View` overloads treat the view as a set of characters that may match or fail the current character.

### Overload differences

The examples below use `constexpr auto text = "😄🇷🇴✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `find_first_of(Char ch)` | first exact character match | `text.find_first_of("✨"_u8c)` |
| `find_first_of(View sv)` | first character that is contained in `sv` | `text.find_first_of("🇷✨"_utf8_sv)` |
| `find_first_not_of(View sv)` | first character that is **not** contained in `sv` | `text.find_first_not_of("😄"_utf8_sv)` |
| `find_last_of(View sv)` | last character contained in `sv` | `text.find_last_of("🇷✨"_utf8_sv)` |
| `find_last_not_of(View sv)` | last character not contained in `sv` | `text.find_last_not_of("✨"_utf8_sv)` |

This family is intentionally character-set oriented. A `View` argument here is not a substring delimiter; it is a bag of candidate characters, much like the classic `find_first_of` family on the C++ standard string types.

### Inspiration

This family is directly modeled after [C++ `std::basic_string_view::find_first_of` and related members](https://en.cppreference.com/w/cpp/string/basic_string_view.html).

### Return value

Returns the matching code-unit offset, or `npos`.

### Complexity

Linear in the view length.

### Exceptions

None.

### `noexcept`

All overloads are `noexcept`.

## Boundary Queries

### Synopsis

```cpp
constexpr bool is_char_boundary(size_type index) const noexcept;
constexpr bool is_grapheme_boundary(size_type index) const noexcept;
constexpr size_type ceil_char_boundary(size_type pos) const noexcept;
constexpr size_type floor_char_boundary(size_type pos) const noexcept;
constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept;
constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept;
```

### Behavior

- Character-boundary members use encoding-level scalar boundaries.
- Grapheme-boundary members use Unicode grapheme segmentation.
- `ceil_*` returns the first boundary at or after `pos`.
- `floor_*` returns the last boundary at or before `pos`.

### Return value

Returns a boolean for predicate queries and a boundary offset for `ceil_*` / `floor_*`.

### Complexity

- `is_char_boundary()` is constant.
- The other members are linear in the distance to the nearest boundary.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

### Example

```cpp
--8<-- "examples/text-operations/boundaries-and-graphemes.cpp"
```

## Direct Access And Substrings

### Synopsis

```cpp
constexpr std::optional<utf8_char> char_at(size_type index) const noexcept;
constexpr utf8_char char_at_unchecked(size_type index) const noexcept;
constexpr std::optional<utf8_string_view> grapheme_at(size_type index) const noexcept;
constexpr std::optional<utf8_string_view> substr(size_type pos, size_type count = npos) const noexcept;
constexpr std::optional<utf8_string_view> grapheme_substr(size_type pos, size_type count = npos) const noexcept;
constexpr std::optional<utf8_char> front() const noexcept;
constexpr utf8_char front_unchecked() const noexcept;
constexpr std::optional<utf8_char> back() const noexcept;
constexpr utf8_char back_unchecked() const noexcept;

constexpr std::optional<utf16_char> char_at(size_type index) const noexcept;
constexpr utf16_char char_at_unchecked(size_type index) const noexcept;
constexpr std::optional<utf16_string_view> grapheme_at(size_type index) const noexcept;
constexpr std::optional<utf16_string_view> substr(size_type pos, size_type count = npos) const noexcept;
constexpr std::optional<utf16_string_view> grapheme_substr(size_type pos, size_type count = npos) const noexcept;
constexpr std::optional<utf16_char> front() const noexcept;
constexpr utf16_char front_unchecked() const noexcept;
constexpr std::optional<utf16_char> back() const noexcept;
constexpr utf16_char back_unchecked() const noexcept;
```

### Behavior

- Checked accessors return `std::nullopt` for empty views, invalid indices, or invalid boundaries.
- `char_at_unchecked()`, `front_unchecked()`, and `back_unchecked()` assume their preconditions hold.
- `substr()` requires both ends of the slice to be character boundaries.
- `grapheme_at()` and `grapheme_substr()` require grapheme boundaries.

### Return value

Returns the requested character or borrowed subview when the request is valid, otherwise [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt).

### Complexity

- Checked element access is constant to linear in the size of the selected character.
- Grapheme checks are linear in nearby segmentation work.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## Prefix And Suffix Tests

### Synopsis

```cpp
constexpr bool starts_with(char ch) const noexcept;   // utf8 only
constexpr bool starts_with(char8_t ch) const noexcept;
constexpr bool starts_with(utf8_char ch) const noexcept;
constexpr bool starts_with(utf8_string_view sv) const noexcept;
constexpr bool starts_with(std::span<const utf8_char> chars) const noexcept;
template <details::utf8_char_predicate Pred>
constexpr bool starts_with(Pred pred) const noexcept(/* conditional */);

constexpr bool ends_with(char ch) const noexcept;     // utf8 only
constexpr bool ends_with(char8_t ch) const noexcept;
constexpr bool ends_with(utf8_char ch) const noexcept;
constexpr bool ends_with(utf8_string_view sv) const noexcept;
constexpr bool ends_with(std::span<const utf8_char> chars) const noexcept;

constexpr bool starts_with(char16_t ch) const noexcept;
constexpr bool starts_with(utf16_char ch) const noexcept;
constexpr bool starts_with(utf16_string_view sv) const noexcept;
constexpr bool starts_with(std::span<const utf16_char> chars) const noexcept;
template <details::utf16_char_predicate Pred>
constexpr bool starts_with(Pred pred) const noexcept(/* conditional */);

constexpr bool ends_with(char16_t ch) const noexcept;
constexpr bool ends_with(utf16_char ch) const noexcept;
constexpr bool ends_with(utf16_string_view sv) const noexcept;
constexpr bool ends_with(std::span<const utf16_char> chars) const noexcept;
```

### Behavior

- Character overloads compare the first or last validated character.
- View overloads compare encoded prefixes or suffixes.
- Span overloads treat the span as a set of characters.
- Predicate overloads test the first character only and are conditionally `noexcept`.

### Overload differences

The examples below use `constexpr auto text = "😄🇷🇴✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `starts_with(Char ch)` | compare the first validated character | `text.starts_with("😄"_u8c)` |
| `starts_with(View sv)` | compare an exact encoded prefix | `text.starts_with("😄🇷🇴"_utf8_sv)` |
| `starts_with(std::span<const Char> chars)` | test whether the first character belongs to a character set | `text.starts_with(std::array{"😄"_u8c, "✨"_u8c})` |
| `starts_with(Pred pred)` | test the first character with a predicate | `text.starts_with([](utf8_char ch) { return !ch.is_ascii(); })` |

The same distinctions apply to `ends_with(...)`, but against the last character or suffix.

### Inspiration

The single-character and exact-prefix overloads are close to [C++ `std::basic_string_view::starts_with`](https://en.cppreference.com/w/cpp/string/basic_string_view/starts_with.html) and Rust's [`str`](https://doc.rust-lang.org/stable/core/primitive.str.html) prefix/suffix APIs.

### Return value

Returns `false` when the view is empty and no character is available to test.

### Complexity

Constant for single-character and predicate overloads, linear in the compared prefix or suffix for view overloads.

### Exceptions

Only predicate overloads can throw, and only if the predicate throws.

### `noexcept`

- Non-predicate overloads are `noexcept`.
- Predicate overloads are conditionally `noexcept`.

## Split Views

### Synopsis

```cpp
constexpr auto split(Char ch) const noexcept;
constexpr auto split(View sv) const noexcept;
template <Predicate Pred> constexpr auto split(Pred pred) const noexcept;

constexpr auto split_trimmed(Char ch) const noexcept;
constexpr auto split_trimmed(View sv) const noexcept;
template <Predicate Pred> constexpr auto split_trimmed(Pred pred) const noexcept;

constexpr auto split_whitespace() const noexcept;
constexpr auto split_ascii_whitespace() const noexcept;

constexpr auto rsplit(Char ch) const noexcept;
constexpr auto rsplit(View sv) const noexcept;
template <Predicate Pred> constexpr auto rsplit(Pred pred) const noexcept;

constexpr auto split_terminator(Char ch) const noexcept;
constexpr auto split_terminator(View sv) const noexcept;
template <Predicate Pred> constexpr auto split_terminator(Pred pred) const noexcept;

constexpr auto rsplit_terminator(Char ch) const noexcept;
constexpr auto rsplit_terminator(View sv) const noexcept;
template <Predicate Pred> constexpr auto rsplit_terminator(Pred pred) const noexcept;

constexpr auto splitn(size_type count, Char ch) const noexcept;
constexpr auto splitn(size_type count, View sv) const noexcept;
template <Predicate Pred> constexpr auto splitn(size_type count, Pred pred) const noexcept;

constexpr auto rsplitn(size_type count, Char ch) const noexcept;
constexpr auto rsplitn(size_type count, View sv) const noexcept;
template <Predicate Pred> constexpr auto rsplitn(size_type count, Pred pred) const noexcept;

constexpr auto split_inclusive(Char ch) const noexcept;
constexpr auto split_inclusive(View sv) const noexcept;
template <Predicate Pred> constexpr auto split_inclusive(Pred pred) const noexcept;
```

### Behavior

- These members return lazy split ranges over borrowed subviews.
- The returned split types are `std::ranges::view_interface`-based views rather than eagerly materialized containers.
- `split_trimmed` trims whitespace around each field.
- `split_whitespace` uses Unicode whitespace; `split_ascii_whitespace` uses ASCII whitespace only.
- `split_terminator` suppresses a trailing empty field caused only by a trailing delimiter.
- `split_inclusive` keeps the delimiter in the yielded field.
- `splitn` and `rsplitn` stop after `count` fields.

### Overload differences

The examples below use `constexpr auto text = "😄 | 🇷🇴 | ✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `split(Char ch)` | split on an exact validated character delimiter | `text.split("|"_u8c)` |
| `split(View sv)` | split on an exact validated substring delimiter | `text.split(" | "_utf8_sv)` |
| `split(Pred pred)` | split whenever a validated character satisfies the predicate | `text.split([](utf8_char ch) { return ch.is_ascii_whitespace(); })` |

The same pattern applies to `split_trimmed`, `split_terminator`, `split_inclusive`, `splitn`, `rsplit`, `rsplitn`, and `rsplit_terminator`.

### Inspiration

This family leans heavily on Rust's [`str`](https://doc.rust-lang.org/stable/core/primitive.str.html) split APIs such as `split`, `split_once`, `split_inclusive`, `split_terminator`, `split_whitespace`, and `splitn`.

### Return value

Returns a borrowed range; iteration performs the actual split.

### Complexity

- Constructing the range is constant.
- Iterating the full split result is linear in the source length.

### Exceptions

None, unless a predicate object throws when invoked.

### `noexcept`

All listed overloads are declared `noexcept`.

### Example

```cpp
--8<-- "examples/text-operations/split-and-trim.cpp"
```

## Match Views And One-Shot Splits

### Synopsis

```cpp
constexpr auto matches(Char ch) const noexcept;
constexpr auto matches(View sv) const noexcept;
template <Predicate Pred> constexpr auto matches(Pred pred) const noexcept;

constexpr auto rmatches(Char ch) const noexcept;
constexpr auto rmatches(View sv) const noexcept;
template <Predicate Pred> constexpr auto rmatches(Pred pred) const noexcept;

constexpr auto rmatch_indices(Char ch) const noexcept;
constexpr auto rmatch_indices(View sv) const noexcept;
template <Predicate Pred> constexpr auto rmatch_indices(Pred pred) const noexcept;

constexpr std::optional<std::pair<View, View>> split_once(Char ch) const noexcept;
constexpr std::optional<std::pair<View, View>> split_once(View sv) const noexcept;
constexpr std::optional<std::pair<View, View>> split_once(std::span<const Char> chars) const noexcept;
template <Predicate Pred>
constexpr std::optional<std::pair<View, View>> split_once(Pred pred) const noexcept;

constexpr std::optional<std::pair<View, View>> rsplit_once(Char ch) const noexcept;
constexpr std::optional<std::pair<View, View>> rsplit_once(View sv) const noexcept;
constexpr std::optional<std::pair<View, View>> rsplit_once(std::span<const Char> chars) const noexcept;
template <Predicate Pred>
constexpr std::optional<std::pair<View, View>> rsplit_once(Pred pred) const noexcept;

constexpr std::optional<std::pair<View, View>> split_once_at(size_type delim) const noexcept;
constexpr std::pair<View, View> split_once_at_unchecked(size_type delim) const noexcept;
```

### Behavior

- `matches` and `rmatches` yield matching borrowed subviews.
- `rmatch_indices` yields reverse-ordered `(offset, subview)` pairs.
- `split_once` and `rsplit_once` split around the first or last match.
- `split_once_at` validates that `delim` is a character boundary.
- `split_once_at_unchecked` assumes the supplied offset is already valid.
- The range-returning members in this section are lazy `std::ranges::view_interface`-based borrowed views.

### Overload differences

The examples below use `constexpr auto text = "😄=🇷🇴=✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `split_once(Char ch)` | split at the first exact character delimiter | `text.split_once("="_u8c)` |
| `split_once(View sv)` | split at the first exact substring delimiter | `text.split_once("=🇷🇴="_utf8_sv)` |
| `split_once(std::span<const Char> chars)` | split at the first character that belongs to a character set | `text.split_once(std::array{"="_u8c, "✨"_u8c})` |
| `split_once(Pred pred)` | split at the first character satisfying the predicate | `text.split_once([](utf8_char ch) { return ch.is_ascii_punctuation(); })` |

The same distinctions apply to `rsplit_once(...)`, but from the end.

### Inspiration

The pair-returning surface is deliberately close to Rust's [`str::split_once` and `str::rsplit_once`](https://doc.rust-lang.org/stable/core/primitive.str.html).

### Return value

- Match APIs return lazy ranges.
- One-shot split APIs return [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) when no match exists or when `split_once_at` receives an invalid boundary.

### Complexity

Linear in the view length.

### Exceptions

None, unless a predicate object throws when invoked.

### `noexcept`

All listed overloads are declared `noexcept`.

## Strip And Trim Families

### Synopsis

```cpp
constexpr std::optional<View> strip_prefix(Char ch) const noexcept;
constexpr std::optional<View> strip_prefix(View sv) const noexcept;
constexpr std::optional<View> strip_suffix(Char ch) const noexcept;
constexpr std::optional<View> strip_suffix(View sv) const noexcept;
constexpr std::optional<View> strip_circumfix(Char prefix, Char suffix) const noexcept;
constexpr std::optional<View> strip_circumfix(View prefix, View suffix) const noexcept;

constexpr View trim_prefix(Char ch) const noexcept;
constexpr View trim_prefix(View sv) const noexcept;
constexpr View trim_suffix(Char ch) const noexcept;
constexpr View trim_suffix(View sv) const noexcept;

constexpr View trim_start_matches(Char ch) const noexcept;
constexpr View trim_start_matches(View sv) const noexcept;
constexpr View trim_start_matches(std::span<const Char> chars) const noexcept;
template <Predicate Pred> constexpr View trim_start_matches(Pred pred) const noexcept;

constexpr View trim_end_matches(Char ch) const noexcept;
constexpr View trim_end_matches(View sv) const noexcept;
constexpr View trim_end_matches(std::span<const Char> chars) const noexcept;
template <Predicate Pred> constexpr View trim_end_matches(Pred pred) const noexcept;

constexpr View trim_matches(Char ch) const noexcept;
constexpr View trim_matches(View sv) const noexcept;
constexpr View trim_matches(std::span<const Char> chars) const noexcept;
template <Predicate Pred> constexpr View trim_matches(Pred pred) const noexcept;

constexpr View trim_start() const noexcept;
constexpr View trim_end() const noexcept;
constexpr View trim() const noexcept;
constexpr View trim_ascii_start() const noexcept;
constexpr View trim_ascii_end() const noexcept;
constexpr View trim_ascii() const noexcept;
```

### Behavior

- `strip_*` preserves failure information with [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional).
- `trim_prefix` and `trim_suffix` keep the original view when no removal happens.
- `trim_*_matches` remove repeated matches from one or both ends.
- `trim_*` uses Unicode whitespace; `trim_ascii*` uses ASCII whitespace only.

### Overload differences

The examples below use `constexpr auto framed = "✨✨😄🇷🇴✨✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `strip_prefix(View sv)` | remove one exact prefix occurrence or return `std::nullopt` | `framed.strip_prefix("✨✨"_utf8_sv)` |
| `trim_prefix(View sv)` | remove one exact prefix occurrence or return the original view unchanged | `framed.trim_prefix("✨✨"_utf8_sv)` |
| `trim_start_matches(Char ch)` | repeatedly remove one exact character from the start | `framed.trim_start_matches("✨"_u8c)` |
| `trim_start_matches(View sv)` | repeatedly remove one exact substring from the start | `framed.trim_start_matches("✨"_utf8_sv)` |
| `trim_start_matches(std::span<const Char> chars)` | repeatedly remove any leading character that belongs to a set | `framed.trim_start_matches(std::array{"✨"_u8c, "😄"_u8c})` |
| `trim_start_matches(Pred pred)` | repeatedly remove leading characters satisfying a predicate | `framed.trim_start_matches([](utf8_char ch) { return !ch.is_ascii(); })` |

The same distinctions apply to `trim_end_matches(...)` and `trim_matches(...)`.

For the span overload, adjacency does not matter. `std::array{"✨"_u8c, "😄"_u8c}` means "keep trimming while the next edge character is either `✨` or `😄`". It does **not** mean "trim the substring `✨😄`".

### Inspiration

This family is strongly inspired by Rust's [`str`](https://doc.rust-lang.org/stable/core/primitive.str.html) APIs such as `strip_prefix`, `strip_suffix`, `trim_matches`, `trim_start_matches`, `trim_end_matches`, `trim_prefix`, and `trim_suffix`.

### Return value

Returns a borrowed subview into the original storage.

### Complexity

Linear in the number of leading or trailing characters examined.

### Exceptions

None, unless a predicate object throws when invoked.

### `noexcept`

All listed overloads are declared `noexcept`.

## Owning Transformations

### Synopsis

```cpp
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_utf8_owned(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_utf16(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_ascii_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_ascii_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_lowercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_uppercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> normalize(normalization_form form, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_nfc(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_nfd(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_nfkc(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_nfkd(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> case_fold(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_utf16_owned(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char8_t>>
constexpr basic_utf8_string<Allocator> to_utf8(const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_ascii_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_ascii_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_lowercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_lowercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_uppercase(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_uppercase(size_type pos, size_type count, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> normalize(normalization_form form, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_nfc(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_nfd(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_nfkc(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> to_nfkd(const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
constexpr basic_utf16_string<Allocator> case_fold(const Allocator& alloc = Allocator()) const;
```

### Behavior

- All transformation members return owning validated strings.
- Partial case-mapping overloads require both ends of the selected range to be character boundaries.
- `normalize(...)` is whole-string only.
- `case_fold()` implements Unicode case folding for caseless matching workflows.

### Return value

Returns a new owning string in the target encoding or transformed form.

### Complexity

Linear in the number of processed code units, plus any additional work required by Unicode case expansion or normalization.

### Exceptions

- Partial case transforms may throw [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) for invalid boundaries.
- All owning transforms may throw allocator or container exceptions.

### `noexcept`

Not `noexcept`.

### Example

```cpp
--8<-- "examples/casing/unicode-case.cpp"
```

### Optional ICU Locale-Aware Overloads

When the library is built with `UTF8_RANGES_ENABLE_ICU=1`, the string-view types also expose these overloads:

```cpp
template <typename Allocator = std::allocator<char8_t>>
basic_utf8_string<Allocator> to_lowercase(locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
basic_utf8_string<Allocator> to_lowercase(size_type pos, size_type count, locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
basic_utf8_string<Allocator> to_uppercase(locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
basic_utf8_string<Allocator> to_uppercase(size_type pos, size_type count, locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char8_t>>
basic_utf8_string<Allocator> case_fold(locale_id locale, const Allocator& alloc = Allocator()) const;

template <typename Allocator = std::allocator<char16_t>>
basic_utf16_string<Allocator> to_lowercase(locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
basic_utf16_string<Allocator> to_lowercase(size_type pos, size_type count, locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
basic_utf16_string<Allocator> to_uppercase(locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
basic_utf16_string<Allocator> to_uppercase(size_type pos, size_type count, locale_id locale, const Allocator& alloc = Allocator()) const;
template <typename Allocator = std::allocator<char16_t>>
basic_utf16_string<Allocator> case_fold(locale_id locale, const Allocator& alloc = Allocator()) const;
```

- These overloads do not exist in the dependency-free default build.
- `to_lowercase(locale)` and `to_uppercase(locale)` delegate to ICU locale-sensitive case mapping.
- `case_fold(locale)` delegates to ICU case folding. In practice, the only fold-specific tailoring ICU exposes here is the Turkic special-I mode, so most locales behave the same as the default `case_fold()`.
- `locale_id` is a raw null-terminated token. `locale_id{nullptr}` is rejected with `std::invalid_argument`.
- Non-null locale names are passed through to ICU, which may canonicalize them or fall back to a more general locale instead of failing.
- A null locale token throws `std::invalid_argument`. ICU normalization or casing failures throw `std::runtime_error`.
- If you need an exact availability check before calling a locale-aware overload, use `is_available_locale(locale)`.

```cpp
--8<-- "examples/casing/locale-case.cpp"
```

## View-Based Replacement Families

### Synopsis

```cpp
constexpr basic_utf8_string<> replace_all(utf8_char from, utf8_char to) const;
constexpr basic_utf8_string<> replace_all(utf8_char from, utf8_string_view to) const;
constexpr basic_utf8_string<> replace_all(utf8_string_view from, utf8_char to) const;
constexpr basic_utf8_string<> replace_all(utf8_string_view from, utf8_string_view to) const;
constexpr basic_utf8_string<> replace_all(std::span<const utf8_char> from, utf8_char to) const;
constexpr basic_utf8_string<> replace_all(std::span<const utf8_char> from, utf8_string_view to) const;
template <details::utf8_char_predicate Pred>
constexpr basic_utf8_string<> replace_all(Pred pred, utf8_char to) const;
template <details::utf8_char_predicate Pred>
constexpr basic_utf8_string<> replace_all(Pred pred, utf8_string_view to) const;

constexpr basic_utf8_string<> replace_n(size_type count, utf8_char from, utf8_char to) const;
constexpr basic_utf8_string<> replace_n(size_type count, utf8_char from, utf8_string_view to) const;
constexpr basic_utf8_string<> replace_n(size_type count, utf8_string_view from, utf8_char to) const;
constexpr basic_utf8_string<> replace_n(size_type count, utf8_string_view from, utf8_string_view to) const;
constexpr basic_utf8_string<> replace_n(size_type count, std::span<const utf8_char> from, utf8_char to) const;
constexpr basic_utf8_string<> replace_n(size_type count, std::span<const utf8_char> from, utf8_string_view to) const;
template <details::utf8_char_predicate Pred>
constexpr basic_utf8_string<> replace_n(size_type count, Pred pred, utf8_char to) const;
template <details::utf8_char_predicate Pred>
constexpr basic_utf8_string<> replace_n(size_type count, Pred pred, utf8_string_view to) const;

// Each family also has allocator-taking overloads.
```

The UTF-16 view surface exposes the same overload families with `utf16_char`, `utf16_string_view`, and `basic_utf16_string`.

### Behavior

- Character and view overloads replace exact validated characters or exact validated substrings.
- Span overloads treat the span as a character set.
- Predicate overloads replace every character for which the predicate returns `true`.
- `replace_n` stops after at most `count` replacements.

### Overload differences

The examples below use `const auto text = "😄🇷🇴✨"_utf8_sv;`.

| Overload | Meaning | Example |
| --- | --- | --- |
| `replace_all(Char from, Char to)` | replace an exact character everywhere | `text.replace_all("✨"_u8c, "🔥"_u8c)` |
| `replace_all(View from, View to)` | replace an exact validated substring everywhere | `text.replace_all("🇷🇴"_utf8_sv, "🎉"_utf8_sv)` |
| `replace_all(std::span<const Char> from, Char to)` | replace every character that belongs to a set | `text.replace_all(std::array{"😄"_u8c, "✨"_u8c}, "🎉"_u8c)` |
| `replace_all(Pred pred, View to)` | replace every character satisfying a predicate | `text.replace_all([](utf8_char ch) { return !ch.is_ascii(); }, "⭐"_utf8_sv)` |

The same overload differences apply to `replace_n(...)`, except that it stops after at most `count` replacements.

Again, the span overload is character-set based. `std::array{"🇷"_u8c, "🇴"_u8c}` will replace either regional-indicator character independently; it will not wait for the adjacent pair `🇷🇴`.

### Inspiration

The overall shape is familiar to users of [C++ `std::basic_string::replace`](https://en.cppreference.com/w/cpp/string/basic_string/replace.html) and Rust's [`str` and `String`](https://doc.rust-lang.org/std/string/struct.String.html) replacement APIs, while extending them with character-set and predicate overloads.

### Return value

Returns a new owning string in the same encoding as the source view.

### Complexity

Linear in the source length plus the size of the produced output.

### Exceptions

May throw allocator or container exceptions.

### `noexcept`

Not `noexcept`.
