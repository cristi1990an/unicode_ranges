# Boundary Encodings

The boundary-encoding API extends the validated UTF string types at external encode and decode boundaries without changing the library's core model.

This page documents the implemented public surface. For the design rationale and open-ended discussion, see [Extensible Encodings (Proposal)](../extensible-encodings.md).

```cpp
--8<-- "examples/reference/encodings.cpp"
```

## Header And Namespaces

- Include [`unicode_ranges.hpp`](https://github.com/cristi1990an/unicode_ranges/blob/main/unicode_ranges.hpp) for the full surface.
- The boundary API lives in namespace `unicode_ranges`.
- Built-in codecs currently live in namespace `unicode_ranges::encodings`.

## Core Types

### Synopsis

```cpp
template <typename Encoder>
struct encoder_traits;

template <typename Decoder>
struct decoder_traits;

template <typename T>
concept encoder = /* exposition only */;

template <typename T>
concept decoder = /* exposition only */;

template <typename Decoder, typename UtfString>
using from_encoded_result = /* UtfString or std::expected<UtfString, decode_error> */;

template <typename Encoder, typename OutputAllocator = std::allocator<typename encoder_traits<Encoder>::code_unit_type>>
using to_encoded_result = /* string or std::expected<string, encode_error> */;

enum class encode_to_error_kind {
    overflow,
    encoding_error
};

template <typename Encoder>
struct encode_to_error;

struct codec_contract_violation : std::logic_error {};
```

### Behavior

- `encoder_traits` and `decoder_traits` normalize codec objects into the surface the library actually calls.
- The traits layer always provides `flush(...)` even when the codec object does not define it.
- Decoder `code_unit_type` must be usable with `std::basic_string_view<code_unit_type>`.
- `to_encoded(...)` additionally requires an encoded `code_unit_type` that can back `std::basic_string<code_unit_type, std::char_traits<code_unit_type>, ...>`.
- `from_encoded_result` and `to_encoded_result` follow the optional-error-alias rule:
  - no `decode_error` / `encode_error` alias means a direct UTF value or direct encoded string
  - defining the alias switches the corresponding family to `std::expected`
- `encode_to_error_kind::overflow` is library-owned bounded-sink exhaustion
- `encode_to_error_kind::encoding_error` wraps the codec's `encode_error`
- `codec_contract_violation` is reserved for codec bugs when contract checks are enabled

### Contract checks

`UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS` defaults to:

- `1` in debug builds
- `0` in release builds

When enabled, contract violations throw `codec_contract_violation` in exception-enabled builds and terminate in no-exception builds. When disabled, violating the codec contract is undefined behavior.

## Codec Objects

### Minimum shape

```cpp
struct my_encoder {
    using code_unit_type = char8_t;

    template <typename Writer>
    void encode_one(char32_t scalar, Writer out);
};

struct my_decoder {
    using code_unit_type = char8_t;

    template <typename Writer>
    std::size_t decode_one(std::basic_string_view<char8_t> input, Writer out);
};
```

### Optional additions

```cpp
using encode_error = /* ... */;
using decode_error = /* ... */;
static constexpr bool allow_implicit_construction = true;

template <typename Writer>
void flush(Writer out);

template <typename Writer>
void encode_from_utf8(utf8_string_view input, Writer out);
template <typename Writer>
void encode_from_utf16(utf16_string_view input, Writer out);
template <typename Writer>
void encode_from_utf32(utf32_string_view input, Writer out);

template <typename Writer>
void decode_to_utf8(std::basic_string_view<char8_t> input, Writer out);
template <typename Writer>
void decode_to_utf16(std::basic_string_view<char8_t> input, Writer out);
template <typename Writer>
void decode_to_utf32(std::basic_string_view<char8_t> input, Writer out);
```

### Behavior

- Codec objects are real mutable objects. Any runtime state lives on the object itself.
- Writer parameters are taken by value. The writer is a cheap non-owning handle over external sink state.
- Decoder `code_unit_type` therefore has to be a valid `std::basic_string_view` element type.
- Encoders intended for `to_encoded(...)` must use a `code_unit_type` that is also valid for `std::basic_string` with `std::char_traits<code_unit_type>`.
- If `encode_error` or `decode_error` is defined, the corresponding primitive hook, bulk hooks, and `flush(...)` must return `std::expected<..., error_type>` instead of the infallible form.
- `allow_implicit_construction` is optional.
  - if omitted, empty default-constructible codecs are treated as implicitly constructible
  - explicit `false` opts out
  - explicit `true` opts in even for non-empty codecs
- If `allow_implicit_construction` is `true` but the codec is not default-constructible, the no-object convenience overloads fail with a static assertion

### Whole-input contract

- `encode_from_utf8(...)`, `encode_from_utf16(...)`, `encode_from_utf32(...)`, `decode_to_utf8(...)`, `decode_to_utf16(...)`, and `decode_to_utf32(...)` are whole-input operations
- on success they must consume the full input view they are given
- they cannot silently stop early
- the surrounding library algorithm still calls `flush(...)` afterwards

### Primitive decode contract

- `decode_one(...)` receives the remaining suffix of the original input after previous successful consumption
- the returned consumed count is relative to that suffix
- on success, consumed count must be non-zero and must not exceed `input.size()`
- once the input is exhausted, the library skips further `decode_one(...)` calls and proceeds to `flush(...)`
- `flush(...)` must also be valid when no prior `decode_one(...)` call occurred, which naturally happens for empty input

## Writer Surface

Codecs do not write directly to arbitrary containers or iterators. They receive a library writer handle with this logical contract:

```cpp
struct Writer {
    using unit_type = /* code unit or char32_t scalar, depending on context */;

    void reserve(std::size_t additional_units) const;
    void push(unit_type unit) const;
    void append(std::span<const unit_type> units) const;

    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_reference_t<R>, unit_type>
    void append(R&& units) const;
};
```

### Behavior

- Writer copies share the same underlying destination state.
- Writers are call-scoped handles and should not be retained by codecs.
- Raw bounded writers report overflow through `encode_to(...)`, not by throwing.
- Growable container writers propagate ordinary container exceptions.
- For container appenders, the implementation prefers:
  - `resize_and_overwrite(...)` for suitable span or sized-range appends
  - `append_range(...)`
  - `append(ptr, count)` for string-like containers
  - `insert_range(end(), ...)`
  - `insert(end(), first, last)`
  - repeated `push_back` / `emplace_back` / `insert(end(), value)` with `reserve(...)` only on that repeated-push fallback

## Owning String Boundary Functions

The UTF-8, UTF-16, and UTF-32 owning string types expose structurally parallel boundary APIs. The synopsis below uses the UTF-8 family explicitly.

### Decode into validated UTF

```cpp
template <typename Decoder>
static constexpr auto from_encoded(
    std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
    Decoder& decoder,
    const Allocator& alloc = Allocator())
    -> from_encoded_result<Decoder, basic_utf8_string>;

template <typename Decoder>
    requires decoder_traits<Decoder>::allow_implicit_construction_requested
static constexpr auto from_encoded(
    std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
    const Allocator& alloc = Allocator())
    -> from_encoded_result<Decoder, basic_utf8_string>;
```

### Encode into an owned encoded string

```cpp
template <typename Encoder, typename OutputAllocator = std::allocator<typename encoder_traits<Encoder>::code_unit_type>>
constexpr auto to_encoded(
    Encoder& encoder,
    const OutputAllocator& alloc = OutputAllocator()) const
    -> to_encoded_result<Encoder, OutputAllocator>;

template <typename Encoder, typename OutputAllocator = std::allocator<typename encoder_traits<Encoder>::code_unit_type>>
    requires encoder_traits<Encoder>::allow_implicit_construction_requested
constexpr auto to_encoded(
    const OutputAllocator& alloc = OutputAllocator()) const
    -> to_encoded_result<Encoder, OutputAllocator>;
```

### Encode into a bounded raw sink

```cpp
template <typename Encoder, typename Out>
    requires std::ranges::range<Out>
          && std::ranges::output_range<Out, typename encoder_traits<Encoder>::code_unit_type>
constexpr auto encode_to(Out&& out, Encoder& encoder) const
    -> std::expected<void, encode_to_error<Encoder>>;

template <typename Encoder, typename Out>
    requires encoder_traits<Encoder>::allow_implicit_construction_requested
          && std::ranges::range<Out>
          && std::ranges::output_range<Out, typename encoder_traits<Encoder>::code_unit_type>
constexpr auto encode_to(Out&& out) const
    -> std::expected<void, encode_to_error<Encoder>>;
```

### Append to a growable sequence-like container

```cpp
template <typename Encoder, typename Container>
constexpr auto encode_append_to(Container& container, Encoder& encoder) const
    -> /* void or std::expected<void, encode_error> */;

template <typename Encoder, typename Container>
    requires encoder_traits<Encoder>::allow_implicit_construction_requested
constexpr auto encode_append_to(Container& container) const
    -> /* void or std::expected<void, encode_error> */;
```

### Behavior

- `from_encoded(...)` always materializes an owned validated UTF string
- `to_encoded(...)` builds a growable encoded string result
- `encode_to(...)` targets bounded raw sinks such as iterator/sentinel-backed outputs and reports overflow through `encode_to_error<Encoder>`
- `encode_append_to(...)` appends after the destination container's existing contents and never reports overflow
- `encode_append_to(...)` only participates for sequence-like append containers whose `value_type` can be constructed from the encoder's `code_unit_type`
- partial output written before overflow or codec failure is preserved
- if a growable destination container throws while appending, that exception propagates normally

## Built-in Codecs

### `encodings::ascii_strict`

- `code_unit_type = char8_t`
- defines both `encode_error` and `decode_error`
- encodes and decodes only ASCII
- reports non-ASCII scalars or bytes as ordinary codec errors
- enables implicit construction

### `encodings::ascii_lossy`

- `code_unit_type = char8_t`
- does not define `encode_error` or `decode_error`
- replaces unrepresentable scalars and invalid bytes with replacement output
- tracks replacement counts on the codec object
- does not opt into implicit construction, because callers typically care about the mutated codec object afterwards

### `encodings::windows_1252`

- `code_unit_type = char8_t`
- defines `encode_error`, but decoding is infallible
- follows the WHATWG Windows-1252 index, not the older undefined-hole vendor mapping
- encodes ASCII and the Windows-1252 repertoire, and reports other scalars as ordinary encode errors
- decodes bytes `0x81`, `0x8D`, `0x8F`, `0x90`, and `0x9D` to the corresponding C1 control code points, matching WHATWG
- enables implicit construction

Source table:
- WHATWG index: <https://encoding.spec.whatwg.org/index-windows-1252.txt>
