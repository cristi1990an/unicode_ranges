# Changelog

All notable changes to this project will be documented in this file.

The format is based loosely on Keep a Changelog, with an `Unreleased` section
tracking local work before it is tagged or versioned.

## [Unreleased]

### Added

- generator support for grapheme-segmentation property tables sourced from official Unicode data files
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
- `std::formatter<utf8_char, wchar_t>`
- `std::formatter<utf16_char, wchar_t>`

### Changed

- the public umbrella header is now `unicode_ranges.hpp`, with `utf8_ranges.hpp` retained as a compatibility shim during the rename
- the Visual Studio project files are now named `unicode_ranges.*`
- `utf8_char::byte_count()` has been renamed to `code_unit_count()`
- `utf8_string_view::char_at(index)` now interprets `index` as a byte index and returns `std::nullopt` when the index is out of range or not a UTF-8 character boundary
- `utf8_string` is now an alias for `basic_utf8_string<>`, which lets `std::ranges::to<utf8_string>()` work naturally
- library headers now use ordinary include guards instead of `#pragma once`
- UTF-8 views now live in `utf8_views.hpp`, and UTF-16 views live in `utf16_views.hpp`
- the UTF-8 string layer is now split across `utf8_string_crtp.hpp`, `utf8_string_view.hpp`, and `utf8_string.hpp`
- tests and documentation now prefer direct Unicode literals instead of `u8`-prefixed literals where possible

### Documentation

- expanded the `utf8_string_view` reference for `char_at` and `char_at_unchecked`
- added documentation for `utf16_string_view`, `utf16_string`, `_utf16_sv`, and `_utf16_s`
- documented the `utf8_string` mutation APIs, including `assign`, `insert`, `erase`, `replace`, `replace_with_range`, `operator+`, and `get_allocator`
- added documentation for `utf16_char`, `utf16_view`, `reversed_utf16_view`, `lossy_utf16_view`, and the `_utf8_s` literal
