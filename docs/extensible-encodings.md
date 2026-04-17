# Boundary Encodings

`unicode_ranges` stays UTF-centric internally, but it can now encode to and decode from external non-UTF formats at the boundary.

This is the layer to use when text must cross an interface that speaks something other than validated UTF-8 / UTF-16 / UTF-32:

- legacy single-byte encodings
- protocol-specific byte formats
- application-defined encodings
- bounded output buffers
- growable byte containers

The core rule stays the same:

- validated UTF types are still the semantic center of the library
- arbitrary encodings live at ingress and egress only

For the exact public API surface, see [Boundary Encodings](reference/encodings.md).

## What This Layer Gives You

- built-in codecs for common boundary formats
- a way to define your own `Encoder` and `Decoder` types without specializing library traits
- generated `from_encoded(...)`, `to_encoded(...)`, `encode_to(...)`, and `encode_append_to(...)` entry points on the owning UTF string types
- a uniform error model for codec failures versus bounded-output overflow
- preserved UTF invariants on the library side

## Built-in Codecs

The library currently ships these boundary codecs:

| Codec | Direction | Behavior |
| --- | --- | --- |
| `encodings::ascii_strict` | encode + decode | strict ASCII only; non-ASCII input is an ordinary codec error |
| `encodings::ascii_lossy` | encode + decode | replaces unrepresentable scalars and invalid bytes; tracks replacement count on the codec object |
| `encodings::windows_1252` | encode + decode | strict WHATWG Windows-1252 mapping; encode is fallible, decode is total |

Use built-ins when they already match your boundary contract. Define custom codecs when you need different replacement behavior, diagnostics, protocol rules, or a different encoding altogether.

## Defining Your Own Codecs

You define real codec objects. The library detects their hooks through `encoder_traits` and `decoder_traits`, but users do not specialize those traits directly.

That means:

- codec state lives on the object itself
- stateless codecs can stay empty and cheap
- stateful codecs can keep counters, buffers, runtime configuration, or diagnostics on the object

### Encoder Requirements

A custom encoder must define:

- `using code_unit_type = ...;`
- `encode_one(char32_t scalar, Writer out)`

Optional encoder additions are:

- `using encode_error = ...;`
- `static constexpr bool allow_implicit_construction = true;`
- `flush(Writer out)`
- `encode_from_utf8(...)`
- `encode_from_utf16(...)`
- `encode_from_utf32(...)`

Semantic requirements:

- `code_unit_type` must be usable with `std::basic_string_view<code_unit_type>`
- if the encoder participates in `to_encoded(...)`, `code_unit_type` must also be usable with `std::basic_string<code_unit_type, std::char_traits<code_unit_type>, ...>`
- `encode_one(...)` receives one Unicode scalar and writes encoded code units through the writer handle
- whole-input hooks such as `encode_from_utf8(...)` must consume the entire input view on success
- the library still calls `flush(...)` after a successful whole-input hook

### Decoder Requirements

A custom decoder must define:

- `using code_unit_type = ...;`
- `decode_one(std::basic_string_view<code_unit_type> input, Writer out)`

Optional decoder additions are:

- `using decode_error = ...;`
- `static constexpr bool allow_implicit_construction = true;`
- `flush(Writer out)`
- `decode_to_utf8(...)`
- `decode_to_utf16(...)`
- `decode_to_utf32(...)`

Semantic requirements:

- `code_unit_type` must be usable with `std::basic_string_view<code_unit_type>`
- `decode_one(...)` receives the remaining suffix of the original encoded input after all previously reported consumption has been removed
- the success return value is the number of code units consumed from that suffix
- successful `decode_one(...)` calls must consume at least one code unit and no more than `input.size()`
- once the input is exhausted, the library stops calling `decode_one(...)` and proceeds to `flush(...)`
- `flush(...)` must be callable even if no prior `decode_one(...)` call happened, which naturally occurs for empty input
- whole-input hooks such as `decode_to_utf8(...)` must consume the entire input view on success

### Fallible Versus Infallible Codecs

Whether a codec is fallible is declared by the presence of the error alias:

- define `encode_error` to make the encoder fallible
- define `decode_error` to make the decoder fallible

Once the alias exists, the matching hooks must switch to `std::expected` return types:

- `encode_one(...)`
- `decode_one(...)`
- `flush(...)`
- any whole-input bulk hook you provide

Additional rules:

- `encode_error` and `decode_error` must be non-`void` types
- if `encode_error` exists, `encode_one(...)`, `flush(...)`, and any `encode_from_utf*` hook must return `std::expected<..., encode_error>`
- if `decode_error` exists, `decode_one(...)`, `flush(...)`, and any `decode_to_utf*` hook must return `std::expected<..., decode_error>`
- `using encode_error = void;` and `using decode_error = void;` are not valid; omit the alias entirely for infallible codecs

If the alias is absent, those hooks use the direct success form instead.

Example:

```cpp
// Infallible encoder: no encode_error alias, plain success returns.
struct ascii_encoder {
    using code_unit_type = char8_t;

    template <typename Writer>
    void encode_one(char32_t scalar, Writer out);

    template <typename Writer>
    void flush(Writer out);
};

// Fallible encoder: encode_error exists, matching hooks return expected.
struct strict_legacy_encoder {
    using code_unit_type = char8_t;

    enum class encode_error {
        unrepresentable_scalar
    };

    template <typename Writer>
    std::expected<void, encode_error> encode_one(char32_t scalar, Writer out);

    template <typename Writer>
    std::expected<void, encode_error> flush(Writer out);
};
```

### Implicit Construction

The convenience overloads that do not take an explicit codec object are controlled by `allow_implicit_construction`.

Rules:

- if `allow_implicit_construction` is omitted, an empty default-constructible codec may still be treated as implicitly constructible
- explicit `false` opts out even for empty default-constructible codecs
- explicit `true` opts in even for non-empty codecs
- if `allow_implicit_construction` is `true` but the codec is not default-constructible, the convenience overloads fail with a static assertion because the library must default-construct a temporary codec object internally

This matters because the generated no-object APIs internally create a temporary codec object and do not return it to you afterwards.

That is appropriate for stateless codecs. It is usually the wrong choice for stateful codecs whose counters or diagnostics matter after the operation.

## The Writer Contract

Codec hooks do not write directly to arbitrary iterators or containers. They receive a small writer handle instead.

Important properties:

- writers are passed by value
- copying a writer copies only the handle, not the destination
- writer copies still talk to the same underlying sink
- writers are call-scoped and should not be retained by codec objects

Writers provide three operations:

- `reserve(additional_units)`
- `push(unit)`
- `append(units)`

The same writer model is used for:

- encoded code units on the encode side
- Unicode scalar output on the primitive decode side
- UTF code-unit output inside the library's bulk decode paths

## Generated Owning-String APIs

After you define a decoder, the owning UTF string types gain these entry points:

- `utf8_string::from_encoded<Decoder>(...)`
- `utf16_string::from_encoded<Decoder>(...)`
- `utf32_string::from_encoded<Decoder>(...)`

After you define an encoder, the owning UTF string types gain these entry points:

- `text.to_encoded<Encoder>(...)`
- `text.encode_to<Encoder>(...)`
- `text.encode_append_to<Encoder>(...)`

In practice that means:

- decode always materializes an owned validated UTF string
- `to_encoded(...)` builds an owned encoded string
- `encode_to(...)` targets a bounded raw output range and reports overflow explicitly
- `encode_append_to(...)` appends to an existing growable sequence-like container and never reports overflow

This example shows a custom strict ASCII encoder/decoder pair and the generated methods it unlocks:

```cpp
--8<-- "examples/encodings/custom-ascii-codec.cpp"
```

## Error Handling And Guarantees

Boundary encoding has two separate failure classes:

- codec-defined failures
- bounded-output overflow on the raw `encode_to(...)` path

### Codec Failures

Codec failures belong to the codec object itself:

- strict rejection of unrepresentable scalars
- invalid input sequences
- protocol-specific decode failures

These surface through `encode_error` or `decode_error`.

Return-type rules:

- `from_encoded(...)` returns the UTF value directly for infallible decoders, or `std::expected<UTF, decode_error>` for fallible decoders
- `to_encoded(...)` returns the encoded string directly for infallible encoders, or `std::expected<string, encode_error>` for fallible encoders
- `encode_append_to(...)` returns `void` for infallible encoders, or `std::expected<void, encode_error>` for fallible encoders

### Bounded Output Overflow

Overflow is not a codec error.

It belongs to the library-owned bounded writer used by `encode_to(...)`.

That path always reports:

- `std::expected<void, encode_to_error<Encoder>>`

with:

- `encode_to_error_kind::overflow` for destination exhaustion
- `encode_to_error_kind::encoding_error` for codec-defined encode failures

### Partial Output Preservation

When output has already been written, it is preserved:

- on bounded overflow
- on codec failure after writing a prefix
- on growable-container writes before a container exception escapes

The library does not roll already-written output back.

### Decoder Validation Guarantee

Custom decoders are allowed to write through bulk UTF paths such as `decode_to_utf8(...)`, but the library still validates the final UTF result before constructing the public owning UTF type.

That keeps the invariant intact:

- successful `from_encoded(...)` still returns validated UTF

### Contract Violations

The codec hooks are a contract.

Examples of contract violations:

- reporting success from `decode_one(...)` with `0` consumed input
- emitting an invalid Unicode scalar
- writing malformed UTF through a bulk UTF hook

When `UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS` is enabled:

- exception-enabled builds throw `codec_contract_violation`
- no-exception builds terminate

When contract checks are disabled, violating the codec contract is undefined behavior.

## Native Codecs In Use

The built-ins are meant to demonstrate the supported codec shapes as well as provide immediate value.

This example uses all currently supported native codecs:

```cpp
--8<-- "examples/encodings/built-in-codecs.cpp"
```

## Stateful Codecs

A stateful codec is just a non-empty codec object whose state changes across calls.

Typical uses:

- replacement counters
- warnings and diagnostics
- buffered partial sequences
- runtime configuration

Because state lives on the object itself, stateful codecs usually should not opt into implicit construction. You normally want to inspect the same object after the operation finishes.

This example shows a small stateful lossy encoder:

```cpp
--8<-- "examples/encodings/stateful-lossy-codec.cpp"
```

## Where To Go Next

- [Boundary Encodings](reference/encodings.md) for the exact overload sets and type aliases
- [Owning Strings](reference/owning-strings.md) for the UTF types these APIs attach to
- [Common Tasks](common-tasks.md) for higher-level Unicode operations after text is already validated
