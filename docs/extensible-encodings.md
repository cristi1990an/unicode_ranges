# Extensible Encodings (Proposal)

This page sketches a way to make `unicode_ranges` expandable beyond UTF-8 / UTF-16 / UTF-32 without giving up its core design: validated UTF types with enforced invariants.

The proposal borrows one idea from `P1629`: extensibility belongs at the encoding boundary. It deliberately does not adopt `P1629`'s free-function-first substrate API as the primary public surface.

Status:

- this is a design proposal, not implemented API
- signatures below are sketch-level and focus on the model, not exact spelling

## Problem

Today the library is strong inside the validated Unicode world:

- validated UTF-8 / UTF-16 / UTF-32 characters and strings
- scalar and grapheme iteration
- normalization
- case mapping and case folding
- explicit checked and unchecked construction

What it does not address well is ingress and egress for non-UTF encodings such as:

- Shift-JIS
- Windows-1252
- ISO-8859-*
- EUC-JP
- GB18030
- custom application-specific encodings

The goal is to support those boundary encodings without turning the entire library into `basic_text<Encoding>`.

## Design Summary

The design is:

- keep the core library UTF-only
- split boundary codecs into `Encoder` and `Decoder` objects
- make those objects stateful if they need to be
- normalize their interface through `encoder_traits` and `decoder_traits`
- decode non-UTF input into Unicode scalars, then materialize validated UTF types
- encode validated UTF text into caller-provided output ranges or convenience-owned strings
- keep graphemes, normalization, casing, and higher-level text APIs Unicode-only

In other words:

- arbitrary encodings are allowed at the edges
- validated UTF remains the internal semantic model

## Core Decisions

### No `basic_text<Encoding>` primary abstraction

This proposal intentionally does not introduce a generic text type parameterized by arbitrary encodings.

Reasons:

- `unicode_ranges` derives most of its value from strong validated UTF invariants
- grapheme segmentation, normalization, and Unicode casing are Unicode operations, not arbitrary-encoding operations
- non-UTF encodings are primarily a boundary concern
- a generic `basic_text<Encoding>` model would multiply API surface and weaken semantics

### No generic error-handler framework

The design does not use `P1629`-style error handlers.

If users want different error behavior, they define different codecs:

- strict codec
- lossy codec
- logging codec
- replacement codec

That is a better fit for `unicode_ranges` than threading handler callables through every boundary API.

### Stateful codecs are first-class

Statefulness matters:

- shift-state encodings
- buffered partial input
- replacement counters
- custom diagnostics
- runtime configuration

This proposal makes encoders and decoders real mutable objects, so all such state lives on the object itself.

## Public Model

There are two user-authored concepts:

- `Encoder`
- `Decoder`

Users define real objects with member functions. The library does not ask users to specialize `encoder_traits` or `decoder_traits`.

Instead:

- user type defines member hooks
- traits detect those hooks
- traits provide defaults for optional operations
- library code talks only to the traits layer

This is intentionally similar in spirit to `allocator_traits`.

## Encoder Model

An encoder converts Unicode scalars into code units of some target encoding.

Minimum shape:

```cpp
struct my_encoder {
    using code_unit_type = unsigned char;

    void encode_one(char32_t scalar, auto& writer);
};
```

Optional additions:

- `using encode_error = ...;`
- `static constexpr bool is_always_equal = true;`
- `flush(auto& writer)`
- bulk fast paths such as `encode_from_utf8(...)`

If `encode_error` exists, then `encode_one(...)` must return:

```cpp
std::expected<void, encode_error>
```

If `encode_error` does not exist, then `encode_one(...)` must return:

```cpp
void
```

## Decoder Model

A decoder consumes encoded code units and produces Unicode scalars.

Minimum shape:

```cpp
struct my_decoder {
    using code_unit_type = unsigned char;

    struct decode_result {
        std::size_t consumed_input = 0;
        char32_t scalar = 0;
    };

    decode_result decode_one(std::basic_string_view<code_unit_type> input);
};
```

Optional additions:

- `using decode_error = ...;`
- `static constexpr bool is_always_equal = true;`
- `flush()`
- bulk fast paths such as `decode_to_utf8(...)`

If `decode_error` exists, then `decode_one(...)` must return:

```cpp
std::expected<decode_result, decode_error>
```

If `decode_error` does not exist, then `decode_one(...)` must return:

```cpp
decode_result
```

## Traits Layer

The library talks to codecs through `encoder_traits<Encoder>` and `decoder_traits<Decoder>`.

Those traits:

- expose normalized nested types
- call required member functions
- detect optional operations
- provide default implementations where possible

Important default:

- `encoder_traits<Encoder>::flush(encoder, writer)` is always defined
- `decoder_traits<Decoder>::flush(decoder)` is always defined
- if the underlying codec does not define `flush`, the traits provide a no-op default

This means library code can always write:

```cpp
encoder_traits<Encoder>::flush(encoder, writer);
decoder_traits<Decoder>::flush(decoder);
```

without repeatedly checking whether the codec defined flush support.

## Convenience Construction

The primary APIs take encoder/decoder objects by reference.

That means:

- no copy requirement
- no move requirement
- no default-construction requirement for the core concepts

Convenience overloads without an explicit object may still exist when implicit construction is considered safe.

The rule is:

- if the codec defines `is_always_equal`, use that
- otherwise, an empty default-constructible type may be treated as implicitly constructible

So the convenience API is enabled for codecs that are effectively stateless and interchangeable.

## Public API Sketch

### Decoding into validated UTF

```cpp
template <typename Decoder>
static auto utf8_string::from_encoded(
    std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
    Decoder& decoder)
    -> from_encoded_result<Decoder, utf8_string>;

template <typename Decoder>
static auto utf16_string::from_encoded(
    std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
    Decoder& decoder)
    -> from_encoded_result<Decoder, utf16_string>;

template <typename Decoder>
static auto utf32_string::from_encoded(
    std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
    Decoder& decoder)
    -> from_encoded_result<Decoder, utf32_string>;
```

Convenience overloads may additionally exist for implicitly constructible decoders:

```cpp
template <typename Decoder>
static auto utf8_string::from_encoded(
    std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input)
    -> from_encoded_result<Decoder, utf8_string>;
```

### Encoding validated UTF

There are two levels.

#### Convenience API

```cpp
template <typename Encoder>
auto utf8_string::to_encoded(Encoder& encoder) const
    -> to_encoded_result<Encoder>;
```

This is the "just give me an owned encoded string" path.

#### Sink API

```cpp
template <typename Encoder, typename Out>
auto utf8_string::encode_to(Out&& out, Encoder& encoder) const
    -> std::expected<void, encode_to_error<Encoder>>;
```

This is the lower-level path for:

- raw buffers
- fixed storage
- inserter-style sinks
- caller-owned output ranges

The convenience API is implemented in terms of this lower-level sink API.

The traits layer normalizes the exact return types:

- if a decoder does not define `decode_error`, `from_encoded(...)` is infallible and returns the UTF value directly
- if a decoder defines `decode_error`, `from_encoded(...)` returns `std::expected<UTF, decode_error>`
- if an encoder does not define `encode_error`, `to_encoded(...)` returns the encoded string directly
- if an encoder defines `encode_error`, `to_encoded(...)` returns `std::expected<string, encode_error>`

## Writer

The public sink API accepts an `output_range`, but encoders do not write to arbitrary ranges directly.

Instead, the library adapts the output range to a uniform internal `Writer`.

The writer is responsible for:

- `push(unit)`
- `append(units)`
- overflow checks
- opportunistic bulk-copy optimizations

Encoders only express encoding semantics. They do not manage buffer capacity.

### Overflow semantics

Overflow is the library's responsibility, not the encoder's.

The writer's contract is:

- `push` writes one unit if possible
- `append` writes the longest prefix that fits
- if output capacity is exhausted, overflow is reported
- already-written prefix data is preserved

No guarantee is made that the written prefix ends on an encoding boundary. That is acceptable for the low-level sink API.

## Sink Error Type

Overflow is not an `Encoder::encode_error`, because it is not an encoding error.

So the sink API returns a library-defined wrapper:

```cpp
enum class encode_to_error_kind {
    overflow,
    encoding_error
};

template <typename Encoder>
struct encode_to_error;
```

This keeps the split clear:

- `overflow` means the destination was exhausted
- `encoding_error` means the encoder itself failed

For fallible encoders, the wrapper stores the encoder's `encode_error` when `kind == encoding_error`.

For infallible encoders, the wrapper collapses to an overflow-only form.

The wrapper stays small and does not require the encoder's `encode_error` to be default-constructible.

## Fast Paths

The primitive contract is scalar-based:

- `encode_one`
- `decode_one`

Optional bulk hooks may still exist for performance:

- `encode_from_utf8(...)`
- `encode_from_utf16(...)`
- `encode_from_utf32(...)`
- `decode_to_utf8(...)`
- `decode_to_utf16(...)`
- `decode_to_utf32(...)`

These are also surfaced through the traits layer.

If a codec does not define a bulk path:

- `encoder_traits` derives the bulk behavior from repeated `encode_one`
- `decoder_traits` derives the bulk behavior from repeated `decode_one`

This means correctness only requires the primitive hooks. Fast paths are optional.

## Error Model

### Ordinary failures

Ordinary failures use `std::expected`:

- malformed external input
- incomplete external input
- unrepresentable scalars
- codec-defined strict failures

These are represented by codec-defined:

- `decode_error`
- `encode_error`

### No generic handled-error counts

This design does not mirror `P1629`'s `handled_errors` model.

If users want replacement counts or diagnostics, they store them on the encoder/decoder object itself.

### Overflow is separate

Output overflow is not a codec error and is not handleable through codec-defined error recovery.

It is represented only by:

- `encode_to_error_kind::overflow`

### Codec contract violations are not ordinary errors

If a buggy codec reports impossible results, such as:

- successful decode with zero input consumed
- invalid Unicode scalar output
- malformed UTF emitted by a custom bulk path

that is a codec bug, not an ordinary recoverable boundary error.

Those cases should be treated as contract violations, not folded into the normal `expected` result type.

## Examples

### Strict ASCII Encoder

```cpp
struct ascii_encoder {
    using code_unit_type = char8_t;
    static constexpr bool is_always_equal = true;

    enum class encode_error {
        unrepresentable_scalar
    };

    template <typename Writer>
    constexpr auto encode_one(char32_t scalar, Writer& out)
        -> std::expected<void, encode_error>
    {
        if (scalar > 0x7F) {
            return std::unexpected(encode_error::unrepresentable_scalar);
        }

        out.push(static_cast<char8_t>(scalar));
        return {};
    }
};
```

### Lossy ASCII Encoder

```cpp
struct ascii_lossy_encoder {
    using code_unit_type = char8_t;

    std::size_t replacement_count = 0;

    template <typename Writer>
    void encode_one(char32_t scalar, Writer& out)
    {
        if (scalar <= 0x7F) {
            out.push(static_cast<char8_t>(scalar));
            return;
        }

        out.push(u8'?');
        ++replacement_count;
    }
};
```

### Strict ASCII Decoder

```cpp
struct ascii_decoder {
    using code_unit_type = char8_t;
    static constexpr bool is_always_equal = true;

    struct decode_result {
        std::size_t consumed_input = 0;
        char32_t scalar = 0;
    };

    enum class decode_error {
        invalid_input,
        incomplete_input
    };

    constexpr auto decode_one(std::basic_string_view<char8_t> input)
        -> std::expected<decode_result, decode_error>
    {
        if (input.empty()) {
            return std::unexpected(decode_error::incomplete_input);
        }

        const auto byte = static_cast<unsigned char>(input.front());
        if (byte > 0x7F) {
            return std::unexpected(decode_error::invalid_input);
        }

        return decode_result{
            .consumed_input = 1,
            .scalar = static_cast<char32_t>(byte)
        };
    }
};
```

## Recommendation

If this direction is pursued, the first implementation should stay narrow:

1. Define `Encoder` / `Decoder` concepts.
2. Define `encoder_traits` / `decoder_traits`.
3. Add the checked `from_encoded(...)` family.
4. Add `encode_to(...)` plus convenience `to_encoded(...)`.
5. Add one strict and one lossy ASCII codec as proof of concept.
6. Add bulk fast-path support only after the primitive model is stable.

That is enough to prove the extensibility story without weakening the core validated UTF design.
