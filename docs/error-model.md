# Error Model

## Unified validation errors

Checked UTF construction reports a single error payload shape:

```cpp
enum class unicode_error_code
{
    invalid_lead_byte,
    truncated_sequence,
    invalid_sequence,
    truncated_surrogate_pair,
    invalid_scalar
};

struct unicode_error
{
    unicode_error_code code{};
    std::size_t first_invalid_element_index = 0;
};
```

The encoding-specific names are aliases of the unified types:

```cpp
using utf8_error_code = unicode_error_code;
using utf16_error_code = unicode_error_code;
using utf32_error_code = unicode_error_code;
using unicode_scalar_error_code = unicode_error_code;
using wide_string_error_code = unicode_error_code;

using utf8_error = unicode_error;
using utf16_error = unicode_error;
using utf32_error = unicode_error;
using unicode_scalar_error = unicode_error;
using wide_string_error = unicode_error;
```

This keeps the API names tied to the operation that produced the error while giving generic code one error-code type and one index field. The index is always expressed in the input element type for that API:

- UTF-8 byte index for `from_bytes(...)`
- UTF-16 code-unit index for `from_code_units(...)`
- UTF-32 code-point index for `from_code_points(...)`
- `wchar_t` element index for `std::wstring_view` factories

Example:

```cpp
const std::array<char8_t, 3> invalid{
    static_cast<char8_t>(0xE2),
    static_cast<char8_t>(0x28),
    static_cast<char8_t>(0xA1)
};

auto text = unicode_ranges::utf8_string_view::from_bytes(
    { invalid.data(), invalid.size() });

assert(!text);
assert(text.error().code == unicode_ranges::utf8_error_code::invalid_sequence);
assert(text.error().first_invalid_element_index == 0);
```

At runtime, the hot UTF validation and checked UTF transcoding paths use `simdutf` underneath. Runtime backend results are mapped into this library-specific error model before they reach the caller.

## Error code subsets

The unified enum contains every validation failure kind used by checked UTF construction and transcoding. Individual APIs return only the subset that can arise from their input contract.

- UTF-8 validation can report `invalid_lead_byte`, `truncated_sequence`, or `invalid_sequence`.
- UTF-16 validation can report `truncated_surrogate_pair` or `invalid_sequence`.
- UTF-32 and scalar validation can report `invalid_scalar`.
- Wide-string factories report the UTF-16 subset on platforms where `sizeof(wchar_t) == 2`, and `invalid_scalar` on platforms where `sizeof(wchar_t) == 4`.

## Wide-string factories

Checked `std::wstring_view` factories use the same stable error type even though `wchar_t` has platform-dependent width.

On platforms where `sizeof(wchar_t) == 2`, wide input is validated as UTF-16. On platforms where `sizeof(wchar_t) == 4`, wide input is validated as Unicode scalar values. The return type remains source-compatible across platforms:

```cpp
std::expected<utf8_string, wide_string_error>
std::expected<utf16_string, wide_string_error>
std::expected<utf32_string, wide_string_error>
```

## Checked factories versus unchecked constructors

The library distinguishes between:

- checked factories that validate incoming raw input and return [`std::expected`](https://en.cppreference.com/w/cpp/utility/expected)
- unchecked construction APIs that assume the caller already proved validity

Use the unchecked APIs only when validity is already guaranteed by the caller or by a surrounding protocol.

## Bounds and semantic errors

Beyond construction-time validation, checked text operations may throw standard exceptions such as [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) for invalid bounds or boundary misuse. Typical examples include:

- offsets that are out of range
- offsets that do not land on a valid character boundary
- subranges that violate API preconditions

Unchecked variants exist where skipping those checks is intentional.
