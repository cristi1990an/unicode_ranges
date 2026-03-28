# Characters

## `utf8_char`

`utf8_char` stores a validated UTF-8 encoded scalar value.

### Main responsibilities

- construct from a scalar
- construct from already-known-valid UTF-8 bytes
- inspect the scalar value and code-unit count
- encode as UTF-8 or UTF-16
- Unicode-aware and ASCII-only classification
- ASCII transforms

### Important APIs

Construction:

- `from_scalar(...)`
- `from_scalar_unchecked(...)`
- `from_utf8_bytes_unchecked(...)`

Observation and encoding:

- `as_scalar()`
- `code_unit_count()`
- `encode_utf8(...)`
- `encode_utf16(...)`

Classification and transforms:

- Unicode-aware predicates such as `is_alphabetic()`, `is_numeric()`, `is_whitespace()`
- ASCII-only predicates such as `is_ascii_alphabetic()`, `is_ascii_digit()`
- `ascii_lowercase()`, `ascii_uppercase()`
- `eq_ignore_ascii_case(...)`

`utf8_char` also supports increment and decrement across Unicode scalar values.

## `utf16_char`

`utf16_char` stores a validated UTF-16 encoded scalar value, either as one BMP code unit or a surrogate pair.

### Main responsibilities

- construct from a scalar
- construct from already-known-valid UTF-16 code units
- inspect the scalar value and code-unit count
- encode as UTF-16 or UTF-8
- Unicode-aware and ASCII-only classification

### Important APIs

Construction:

- `from_scalar(...)`
- `from_utf16_code_units(...)`
- `from_utf16_code_units_unchecked(...)`

Observation and encoding:

- `as_scalar()`
- `code_unit_count()`
- `encode_utf16(...)`
- `encode_utf8(...)`

Like `utf8_char`, `utf16_char` also supports increment and decrement across Unicode scalar values.

## When to use character types directly

Use `utf8_char` or `utf16_char` when you need a validated scalar object rather than a whole string. Common examples:

- iterating scalars and storing one result
- writing predicates that operate on a single character
- passing validated characters into search or replace APIs
- encoding a single scalar into a small buffer
