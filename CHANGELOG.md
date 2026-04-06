# Changelog

All notable changes to this project will be documented in this file.

The format is based loosely on Keep a Changelog, with an `Unreleased` section
tracking local work before it is tagged or versioned.

## [Unreleased]

### Added

- generator support for grapheme-segmentation property tables sourced from official Unicode data files
- `tools/regenerate_unicode_tables.ps1` to regenerate `unicode_ranges/unicode_tables.hpp` as UTF-8 without BOM for Clang-cl compatibility
- `utf8_string::reverse_graphemes()` / `reverse_graphemes(pos, count)` and matching `utf16_string` APIs
- optional ICU-backed locale-aware casing APIs:
  `to_lowercase(locale_id)`, `to_uppercase(locale_id)`, `to_titlecase(locale_id)`, and `case_fold(locale_id)`
- `locale_id`, `operator ""_locale`, and `is_available_locale(...)` when ICU support is enabled
- allocation-free case-insensitive comparison helpers:
  `eq_ignore_case(...)`, `starts_with_ignore_case(...)`, `ends_with_ignore_case(...)`, and `compare_ignore_case(...)`
- locale-aware overloads of the case-insensitive comparison helpers when ICU support is enabled
- `unicode_ranges::characters::utf8::...` and `unicode_ranges::characters::utf16::...` curated convenience constants
- `unicode_ranges::characters::utf32::...` curated convenience constants
- scalar property queries on `utf8_char`, `utf16_char`, and `utf32_char`:
  `general_category()`, `canonical_combining_class()`, `grapheme_break_property()`, `script()`,
  `east_asian_width()`, `line_break_class()`, `bidi_class()`, `word_break_property()`,
  `sentence_break_property()`, `is_emoji()`, `is_emoji_presentation()`, and `is_extended_pictographic()`
- `utf8_string::get_allocator()`
- `utf8_string::erase(index, count = npos)`
- `utf8_string::insert(...)` and `utf8_string::insert_range(...)`
- `utf8_string::assign(...)` and `utf8_string::assign_range(...)`
- `utf8_string::replace(pos, count, utf8_string_view)`
- `utf8_string::replace(pos, count, utf8_char)`
- `utf8_string::replace(pos, utf8_string_view)`
- `utf8_string::replace(pos, utf8_char)`
- `utf8_string::replace_with_range(pos, count, range)`
- `utf8_string::replace_with_range(pos, range)`
- `utf8_string::operator+` with `utf8_string_view` and `utf8_char`
- `utf8_string` / `utf8_string_view` mixed `operator==` and `operator<=>`
- `operator ""_utf8_s` for owning UTF-8 string literals
- `utf8_string_view::char_at_unchecked(index)`
- `utf8_string_view::find(...)` and `rfind(...)` overloads for `char8_t`, `utf8_char`, and `utf8_string_view`
- `utf8_string_view::ceil_char_boundary(...)` and `floor_char_boundary(...)`
- `std::uses_allocator` specialization for `utf8_string`
- `utf8_char::encode_utf16(...)`
- `utf16_char`
- `unicode_ranges::views::lossy_utf16_view`
- `unicode_ranges::views::lossy_utf16`
- `unicode_ranges::views::utf16_view`
- `unicode_ranges::views::reversed_utf16_view`
- `utf16_string_view`
- `operator ""_utf16_sv`
- `utf16_string`
- `operator ""_utf16_s`
- `utf32_char`
- `unicode_ranges::views::lossy_utf32_view`
- `unicode_ranges::views::lossy_utf32`
- `unicode_ranges::views::utf32_view`
- `unicode_ranges::views::reversed_utf32_view`
- `utf32_string_view`
- `operator ""_utf32_sv`
- `utf32_string`
- `operator ""_utf32_s`
- `operator ""_u32c`
- `operator ""_grapheme_utf32`
- `std::formatter<utf8_char, wchar_t>`
- `std::formatter<utf16_char, wchar_t>`
- `std::formatter<utf32_char, wchar_t>`
- a non-blocking MSVC `/analyze` static-analysis CI job

### Changed

- unchecked construction/access APIs now mirror the checked preconditions with debug-only assertions
- GCC docs-example compilation is now treated as informational for the known libstdc++ formatting limitation
- locale-aware titlecasing is available only as a whole-string ICU-backed operation; partial locale-aware titlecasing overloads are intentionally not exposed
- the public umbrella header is now `unicode_ranges.hpp`
- the Visual Studio project files are now named `unicode_ranges.*`
- `utf8_char::byte_count()` has been renamed to `code_unit_count()`
- `utf8_string_view::char_at(index)` now interprets `index` as a byte index and returns `std::nullopt` when the index is out of range or not a UTF-8 character boundary
- `utf8_string` is now an alias for `basic_utf8_string<>`, which lets `std::ranges::to<utf8_string>()` work naturally
- library headers now use ordinary include guards instead of `#pragma once`
- UTF-8 views now live in `utf8_views.hpp`, and UTF-16 views live in `utf16_views.hpp`
- the UTF-8 string layer is now split across `utf8_string_crtp.hpp`, `utf8_string_view.hpp`, and `utf8_string.hpp`
- tests and documentation now prefer direct Unicode literals instead of `u8`-prefixed literals where possible
- the library now exposes first-class UTF-32 types, literals, views, conversions, casing, normalization, locale-aware ICU transforms, and docs/examples alongside the existing UTF-8 and UTF-16 surfaces

### Documentation

- documented the optional ICU-backed locale-aware casing APIs, locale tokens, and ICU fallback behavior
- added reference coverage and compiled examples for the new case-insensitive comparison helpers
- added reference coverage and examples for the curated `characters::utf8` / `characters::utf16` / `characters::utf32` namespaces
- added reference coverage and examples for the new scalar Unicode property queries
- expanded onboarding with install/integration guidance, a terminology cheat sheet, common-task recipes, and clearer library rationale
- expanded the `utf8_string_view` reference for `char_at` and `char_at_unchecked`
- added documentation for `utf16_string_view`, `utf16_string`, `_utf16_sv`, and `_utf16_s`
- added documentation for `utf32_string_view`, `utf32_string`, `_u32c`, `_utf32_sv`, `_utf32_s`, and `_grapheme_utf32`
- documented the `utf8_string` mutation APIs, including `assign`, `insert`, `erase`, `replace`, `replace_with_range`, `operator+`, and `get_allocator`
- added documentation for `utf16_char`, `utf16_view`, `reversed_utf16_view`, `lossy_utf16_view`, and the `_utf8_s` literal
- added documentation for `utf32_char`, `utf32_view`, `reversed_utf32_view`, and `lossy_utf32_view`
