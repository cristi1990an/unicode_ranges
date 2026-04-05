# Reference

This section is intentionally reference-first.

Each named API family gets its own entry with:

- the full overload set exposed by the public headers
- general behavior and indexing rules
- return-value conventions
- error handling
- complexity
- `noexcept` status
- a compiled example when one materially improves understanding

## Conventions

### Offsets and `size_type`

- `utf8_string_view` and `utf8_string` measure offsets in UTF-8 code units, which means byte offsets.
- `utf16_string_view` and `utf16_string` measure offsets in UTF-16 code units.
- `size()` always reports code units, not Unicode scalar values and not grapheme clusters.
- `npos` is `static_cast<size_type>(-1)` on all string and view types.

### Character boundaries

- A UTF-8 character boundary is the start of a UTF-8 scalar encoding.
- A UTF-16 character boundary is any index that is not inside a surrogate pair.
- Character-aware search families such as `find(Char, pos)`, `find(View, pos)`, `rfind(...)`, `find_first_of(View, pos)`, and `find_last_of(View, pos)` first align the starting offset to a valid character boundary.
- Boundary-sensitive accessors such as `char_at`, `substr`, `grapheme_at`, `grapheme_substr`, and `split_once_at` reject invalid boundaries explicitly with [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt).
- Owning-string mutators that splice raw ranges, such as `insert`, `erase`, `reverse(pos, count)`, and partial case transforms, reject invalid boundaries with [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range).

### Grapheme boundaries

- Grapheme-aware APIs use the default Unicode grapheme cluster rules.
- Grapheme offsets are still reported in raw UTF-8 bytes or UTF-16 code units, depending on the type.

### Error reporting

- View factories report validation failures through [`std::expected`](https://en.cppreference.com/w/cpp/utility/expected).
- View accessors that may fail because of boundary issues return [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional).
- Owning-string mutators that cannot represent failure with a return type typically throw [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) for invalid indices or invalid substring boundaries.
- Unicode case mapping, normalization, and transcoding functions in the public API assume the source object is already validated, so they do not report encoding errors again.

### Optional ICU locale tokens

- When `UTF8_RANGES_ENABLE_ICU=1` is enabled, locale-aware casing overloads accept `locale_id`.
- `_locale` is the compile-time checked literal form of that token.
- `is_available_locale(...)` is a non-throwing probe for exact ICU locale availability.
- Locale-aware casing overloads may still succeed for locales that are not explicitly available, because ICU may canonicalize or fall back to a more general locale.

### Complexity notation

- "Code units" means UTF-8 bytes for UTF-8 types and UTF-16 code units for UTF-16 types.
- "Scalars" means Unicode scalar values.
- "Matches" means the number of delimiter or replacement matches actually processed.

## Type Map

| Family | Types |
| --- | --- |
| Characters | `utf8_char`, `utf16_char` |
| Borrowed validated text | `utf8_string_view`, `utf16_string_view` |
| Owning validated text | `basic_utf8_string<Allocator>`, `basic_utf16_string<Allocator>`, `utf8_string`, `utf16_string`, `pmr::utf8_string`, `pmr::utf16_string` |
| Helper ranges | `views::utf8_view`, `views::utf16_view`, reversed views, lossy views, grapheme cluster views |
| Literals | `_u8c`, `_u16c`, `_utf8_sv`, `_utf16_sv`, `_utf8_s`, `_utf16_s`, `_grapheme_utf8`, `_grapheme_utf16` |
| Optional ICU locale support | `locale_id`, `_locale`, `is_available_locale(...)` |

## Pages

- [Characters](chars.md)
- [String Views](string-views.md)
- [Owning Strings](owning-strings.md)
- [Views, Literals, and Formatting](views-literals-formatting.md)
