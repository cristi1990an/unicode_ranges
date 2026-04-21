# Error Model

## UTF-8 validation

Checked UTF-8 construction reports:

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

These are returned by checked APIs such as `utf8_string_view::from_bytes(...)` and `utf8_string::from_bytes(...)`.

At runtime, the hot UTF-8 validation and checked UTF-8 transcoding paths currently use `simdutf` underneath. `unicode_ranges` still presents its own `utf8_error` and `utf8_error_code` surface; the runtime backend result is mapped into this library-specific error model before it reaches the caller.

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
assert(text.error().first_invalid_byte_index == 0);
```

## UTF-16 validation

Checked UTF-16 construction reports:

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

These are returned by checked APIs such as `utf16_string_view::from_code_units(...)` and `utf16_string::from_code_units(...)`.

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
