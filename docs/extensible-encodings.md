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

    template <typename Writer>
    void encode_one(char32_t scalar, Writer out);
};
```

Optional additions:

- `using encode_error = ...;`
- `static constexpr bool allow_implicit_construction = true;`
- `flush(...)`
- bulk fast paths such as `encode_from_utf8(...)`

The exact primitive signatures are:

```cpp
template <typename Writer>
void encode_one(char32_t scalar, Writer out);
```

or, for fallible encoders:

```cpp
template <typename Writer>
std::expected<void, encode_error> encode_one(char32_t scalar, Writer out);
```

The exact `flush(...)` signatures follow the same rule:

```cpp
template <typename Writer>
void flush(Writer out);
```

or, for fallible encoders:

```cpp
template <typename Writer>
std::expected<void, encode_error> flush(Writer out);
```

If `encode_error` exists, then `encode_one(...)` must return:

```cpp
std::expected<void, encode_error>
```

If `encode_error` does not exist, then `encode_one(...)` must return:

```cpp
void
```

This is the switch that declares whether an encoder is fallible.

- if the user defines `encode_error`, they are declaring a fallible encoder
- once `encode_error` exists, the encoder hooks they provide must use the fallible signatures
- in practice that means `encode_one(...)`, `flush(...)`, and any `encode_from_utf*` bulk hooks must return `std::expected<..., encode_error>` rather than `void`

Conversely:

- if the user does not define `encode_error`, the encoder is treated as infallible at the codec layer
- in that case those hooks must use the non-`expected` signatures

## Decoder Model

A decoder consumes encoded code units and writes Unicode scalars to a writer.

A successful primitive decode step returns only the number of consumed input code units.

One successful step may emit:

- no scalars
- one scalar
- multiple scalars

Minimum shape:

```cpp
struct my_decoder {
    using code_unit_type = unsigned char;

    template <typename Writer>
    std::size_t decode_one(std::basic_string_view<code_unit_type> input, Writer out);
};
```

Here `Writer` is a Unicode-scalar output sink.

The `input` passed to `decode_one(...)` is always the remaining suffix of the original buffer after all previously reported consumption has been removed.

So if earlier calls consumed the first `n` code units of the original buffer, the next `decode_one(...)` call receives `input.substr(n)` conceptually.

Under the library's normal control flow, `decode_one(...)` is called only while unconsumed input remains. That means the library should never call `decode_one(...)` with an empty input view.

If the original input is empty, or once all input has been consumed, the library skips further `decode_one(...)` calls and proceeds directly to `flush(...)`.

Optional additions:

- `using decode_error = ...;`
- `static constexpr bool allow_implicit_construction = true;`
- `flush(...)`
- bulk fast paths such as `decode_to_utf8(...)`

The exact primitive signatures are:

```cpp
template <typename Writer>
std::size_t decode_one(std::basic_string_view<code_unit_type> input, Writer out);
```

or, for fallible decoders:

```cpp
template <typename Writer>
std::expected<std::size_t, decode_error>
decode_one(std::basic_string_view<code_unit_type> input, Writer out);
```

The exact `flush(...)` signatures follow the same rule:

```cpp
template <typename Writer>
void flush(Writer out);
```

or, for fallible decoders:

```cpp
template <typename Writer>
std::expected<void, decode_error> flush(Writer out);
```

If `decode_error` exists, then `decode_one(...)` must return:

```cpp
std::expected<std::size_t, decode_error>
```

If `decode_error` does not exist, then `decode_one(...)` must return:

```cpp
std::size_t
```

The returned consumed count is always measured relative to the `input` view passed to that particular call.

That means:

- on success, the decoder must report exactly how many leading code units from the provided `input` were consumed
- on success, that count must be non-zero
- the library advances by removing that leading prefix and passes the remaining suffix to the next `decode_one(...)` call
- for a successful whole-buffer decode, the cumulative consumed count across all successful calls must match the entire original input buffer

Reporting success with `0` consumed input is a codec contract violation.

So for fallible decoders, `incomplete_input` or similar trailing-input errors should describe a non-empty suffix that is insufficient to complete a sequence, not an empty input view passed to `decode_one(...)`.

This is the switch that declares whether a decoder is fallible.

- if the user defines `decode_error`, they are declaring a fallible decoder
- once `decode_error` exists, the decoder hooks they provide must use the fallible signatures
- in practice that means `decode_one(...)`, `flush(...)`, and any `decode_to_utf*` bulk hooks must return `std::expected<..., decode_error>` rather than plain success values

Conversely:

- if the user does not define `decode_error`, the decoder is treated as infallible at the codec layer
- in that case those hooks must use the non-`expected` signatures

## Traits Layer

The library talks to codecs through `encoder_traits<Encoder>` and `decoder_traits<Decoder>`.

Those traits:

- expose normalized nested types
- call required member functions
- detect optional operations
- provide default implementations where possible

Important default:

- `encoder_traits<Encoder>::flush(encoder, writer)` is always defined
- `decoder_traits<Decoder>::flush(decoder, writer)` is always defined
- if the underlying codec does not define `flush`, the traits provide a no-op default

This means library code can always write:

```cpp
encoder_traits<Encoder>::flush(encoder, writer);
decoder_traits<Decoder>::flush(decoder, writer);
```

without repeatedly checking whether the codec defined flush support.

More generally, library code should dispatch through the traits layer rather than calling codec members directly. Conceptually:

```cpp
auto one = decoder_traits<Decoder>::decode_one(decoder, input, writer);
auto encoded = encoder_traits<Encoder>::encode_one(encoder, scalar, writer);

auto bulk_decoded = decoder_traits<Decoder>::decode_to_utf8(decoder, input, writer);
auto bulk_encoded = encoder_traits<Encoder>::encode_from_utf8(encoder, input, writer);

auto flushed_encoder = encoder_traits<Encoder>::flush(encoder, writer);
auto flushed_decoder = decoder_traits<Decoder>::flush(decoder, writer);
```

This is important because the traits layer is what:

- normalizes optional error types
- synthesizes missing bulk hooks from primitive hooks
- provides no-op defaults such as `flush(...)`
- gives library internals one stable dispatch surface

### Flush lifecycle

`flush(...)` is optional at the codec-definition layer, but mandatory in library control flow through the traits layer.

The library should always call:

- `encoder_traits<Encoder>::flush(encoder, writer)` at the end of encoding operations
- `decoder_traits<Decoder>::flush(decoder, writer)` at the end of decoding operations

This applies to:

- `to_encoded(...)`
- `encode_to(...)`
- `encode_append_to(...)`
- `from_encoded(...)`
- wrappers around custom bulk hooks such as `decode_to_utf8(...)`

The intended meaning is:

- encoder `flush(writer)`: emit any buffered trailing encoded units
- decoder `flush(writer)`: emit any buffered trailing Unicode scalars and report trailing incomplete input for strict codecs

On the raw `encode_to(...)` path, `flush(writer)` participates in the same bounded-writer rules as ordinary writes. If a final encoder flush overruns the destination, that is reported as `encode_to_error_kind::overflow`.

If a codec does not need finalization, the traits-provided no-op default preserves a uniform control flow.

## Convenience Construction

The primary APIs take encoder/decoder objects by reference.

That means:

- no copy requirement
- no move requirement
- no default-construction requirement for the core concepts

Convenience overloads without an explicit object may still exist when implicit construction is considered safe.

The rule is:

- if the codec defines `allow_implicit_construction`, use that
- otherwise, an empty default-constructible type may be treated as implicitly constructible

More explicitly:

- if `allow_implicit_construction` is omitted and the codec is both empty and default-constructible, the convenience overloads may behave as if the property had been set to `true`
- if `allow_implicit_construction` is explicitly `false`, that opts out even for empty default-constructible codecs
- if `allow_implicit_construction` is explicitly `true`, that opts in even for non-empty codecs

In the explicit opt-in case for a non-empty codec, the library simply default-constructs a temporary codec object internally and uses it for the operation. That temporary object is not returned to the caller.

If `allow_implicit_construction` is `true` but the codec is not default-constructible, the convenience overload should fail to compile with a clear static assertion.

So the convenience API is enabled for codecs that are either:

- explicitly marked as implicitly constructible
- or implicitly treated that way by the empty-and-default-constructible fallback

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

At this layer, decoding intentionally materializes an owned validated UTF result.

A symmetric destination-oriented decode API such as `decode_to(...)` or `decode_append_to(...)` is not part of this proposal. The library owns the validated UTF buffer construction step and returns an owned UTF string. Callers that need the resulting storage can move from that owned value afterwards.

Conceptually, `from_encoded(...)` is implemented by driving the decoder through its writer-based primitive interface into an internal UTF-building writer.

That internal UTF-building writer will typically be backed by the owned UTF string's underlying storage and use its native append operations internally, usually in terms of `push_back(...)` for single code units and `append_range(...)` or equivalent bulk append operations when available.

### Encoding validated UTF

There are two levels.

#### Convenience API

```cpp
template <typename Encoder>
auto utf8_string::to_encoded(Encoder& encoder) const
    -> to_encoded_result<Encoder>;
```

This is the "just give me an owned encoded string" path.

Convenience overloads may additionally exist for implicitly constructible encoders:

```cpp
template <typename Encoder>
auto utf8_string::to_encoded() const
    -> to_encoded_result<Encoder>;
```

#### Low-level destination APIs

```cpp
template <typename Encoder, typename Out>
auto utf8_string::encode_to(Out&& out, Encoder& encoder) const
    -> std::expected<void, encode_to_error<Encoder>>;

template <typename Encoder, typename Container>
auto utf8_string::encode_append_to(Container& container, Encoder& encoder) const
    -> /* void or std::expected<void, encode_error> */;
```

Convenience overloads may additionally exist for implicitly constructible encoders:

```cpp
template <typename Encoder, typename Out>
auto utf8_string::encode_to(Out&& out) const
    -> std::expected<void, encode_to_error<Encoder>>;

template <typename Encoder, typename Container>
auto utf8_string::encode_append_to(Container& container) const
    -> /* void or std::expected<void, encode_error> */;
```

These are the low-level paths for:

- raw buffers
- fixed storage
- caller-owned output ranges
- growable sequence-like containers

`encode_to(...)` is the bounded raw-sink path.

`encode_append_to(...)` is the growable-container path. It exists because plain `output_range` is too weak to expose the operations that matter for efficient appending, such as:

- `reserve(...)`
- bulk append
- range insertion
- string-specific `resize_and_overwrite(...)`

The convenience API is implemented in terms of `encode_append_to(...)`.

The traits layer normalizes the exact return types:

- if a decoder does not define `decode_error`, `from_encoded(...)` is infallible and returns the UTF value directly
- if a decoder defines `decode_error`, `from_encoded(...)` returns `std::expected<UTF, decode_error>`
- if an encoder does not define `encode_error`, `to_encoded(...)` returns the encoded string directly
- if an encoder defines `encode_error`, `to_encoded(...)` returns `std::expected<string, encode_error>`
- if an encoder does not define `encode_error`, `encode_append_to(...)` returns `void`
- if an encoder defines `encode_error`, `encode_append_to(...)` returns `std::expected<void, encode_error>`

## Writer Protocol

Codec hooks do not write to arbitrary containers or ranges directly.

Instead, the library adapts sink destinations to a uniform internal `Writer`-like protocol.

This protocol is used in two ways:

- encoding hooks and bulk decode-to-UTF hooks write encoded or UTF code units
- primitive decode hooks write Unicode scalars

So `Writer` is a logical protocol name here, not one fixed element type. The element type depends on which codec hook is being dispatched.

In the remainder of this section, `unit_type` means that writer element type.

`Writer` should be a cheap non-owning handle that encapsulates references or pointers to the underlying sink state. Copying a writer copies only that handle, not the destination itself, and writes through any copy affect the same underlying sink.

That is why codec hooks may take `Writer` by value without accidentally introducing value semantics for the actual output destination.

The intended lifetime model is still call-scoped: codecs should treat writer values as transient handles and should not retain them beyond the boundary operation unless the library explicitly documents that as supported.

For library-owned UTF results, the internal writer will usually forward to the underlying base string storage through its native append primitives, most commonly `push_back(...)` for single-unit writes and `append_range(...)` or equivalent bulk append operations for multi-unit writes.

The writer is responsible for:

- `reserve(additional_units)`
- `push(unit)`
- `append(units)`
- overflow checks for bounded sinks
- opportunistic bulk-copy optimizations

Encoders only express encoding semantics. They do not manage buffer capacity.

`reserve(additional_units)` is an advisory hook:

- it takes a count of additional code units the codec expects to emit soon
- it forwards to the destination's reserve mechanism when one exists
- otherwise it may simply do nothing
- it may help string-backed or otherwise growable writers reduce reallocations
- it does not affect correctness
- for primitive decode writers, it may initially be a no-op until a more specific reservation strategy is designed

Reservation quality is the caller's responsibility through the chosen output destination. The library does not guarantee that a reservation request will succeed or even have an effect.

`append(units)` should support both:

- an exact bulk path for `std::span<const unit_type>`
- a more generic range-based overload for inputs whose reference type is implicitly convertible to `unit_type`

This allows codecs to hand the writer a wide variety of small views and ranges while still preserving an obvious optimized path for contiguous buffers.

### Raw output-range writer

The raw `output_range` overload is adapted to a bounded writer.

Overflow is the library's responsibility there, not the encoder's.

The raw writer's contract is:

- `push` writes one unit if possible
- `append` writes the longest prefix that fits
- if output capacity is exhausted, overflow is reported
- already-written prefix data is preserved

No guarantee is made that the written prefix ends on an encoding boundary. That is acceptable for the low-level bounded sink API.

Overflow on this path is reported through `encode_to_error<Encoder>`, not by throwing.

### Growable container adaptor

`encode_append_to(...)` uses an internal writer adaptor for growable sequence-like containers.

It is intended for sequence-like containers, not associative containers or containers whose insertion semantics are not append-oriented.

It targets sequence-like containers with:

- a compatible `value_type`
- at least one supported insertion primitive

Compatibility means that `code_unit_type` is implicitly convertible to the container's `value_type`.

The adaptor appends after the container's existing contents and never erases the prefix that was already present before encoding began.

Assuming the underlying container operations provide their usual exception guarantees, that original prefix remains valid even if encoding later fails or insertion throws.

The adaptor should detect and use operations such as:

- `reserve(...)`
- `append_range(...)`
- `insert_range(end(), range)`
- string/vector-style bulk `append(...)`
- `insert(end(), first, last)`
- `push_back(...)`
- `emplace_back(...)`
- `insert(end(), value)`

The fallback order should prefer bulk append and range insertion when available, then fall back to repeated single-element insertion.

More explicitly, the preferred dispatch order should be documented and implemented in the style of `ranges::to`.

The appender should not proactively call `reserve(...)` before bulk insertion operations such as `append_range(...)`, `insert_range(...)`, or `insert(end(), first, last)`. Those operations commonly already perform their own capacity management internally.

The practical place where an explicit pre-`reserve(...)` call matters is the repeated-`push(unit)` fallback when the appender knows the appended size up front.

For `push(unit)`, the adaptor should prefer:

```cpp
if constexpr (has_push_back) {
    container.push_back(unit);
}
else if constexpr (has_emplace_back) {
    container.emplace_back(unit);
}
else if constexpr (has_insert_at_end) {
    container.insert(container.end(), unit);
}
else {
    static_assert(/* no supported single-element append primitive */);
}
```

For `append(std::span<const code_unit_type>)`, the adaptor should prefer:

```cpp
if constexpr (has_resize_and_overwrite) {
    // only when profitable and the callback can stay non-throwing
}
else if constexpr (has_append_range) {
    container.append_range(span);
}
else if constexpr (has_string_like_append) {
    container.append(/* span contents */);
}
else if constexpr (has_insert_range_at_end) {
    container.insert_range(container.end(), span);
}
else if constexpr (has_insert_iterator_pair_at_end) {
    container.insert(container.end(), span.begin(), span.end());
}
else {
    if constexpr (has_reserve) {
        container.reserve(container.size() + span.size());
    }
    for (auto unit : span) {
        push(unit);
    }
}
```

For `append(range)`, the adaptor should prefer:

```cpp
if constexpr (range_is_contiguous_and_sized) {
    append(as_span(range));
}
else if constexpr (range_is_sized && has_resize_and_overwrite) {
    // only when profitable and the callback can stay non-throwing
}
else if constexpr (has_append_range) {
    container.append_range(range);
}
else if constexpr (has_insert_range_at_end) {
    container.insert_range(container.end(), range);
}
else if constexpr (has_insert_iterator_pair_at_end) {
    container.insert(container.end(), std::ranges::begin(range), std::ranges::end(range));
}
else {
    if constexpr (range_is_sized && has_reserve) {
        container.reserve(container.size() + std::ranges::size(range));
    }
    for (auto&& unit : range) {
        push(static_cast<code_unit_type>(unit));
    }
}
```

This ordering is intentionally append-oriented:

- prefer bulk operations over repeated single-element insertion
- prefer explicit append primitives over generic insert-at-end primitives
- use `resize_and_overwrite(...)` only as an optimization for known-size appends
- fall back to repeated `push(unit)` only when no bulk path is available

For this path:

- there is no library-defined overflow condition
- allocation and insertion exceptions from the container simply propagate
- ordinary encoder failures still use `std::expected`

### Partial output on codec failure

Codec hooks are allowed to report failure after having already written part of their output for that particular call.

When that happens:

- any prefix already written through the writer is preserved
- the library does not roll that output back
- hooks that need atomic emission for one logical unit must buffer locally before writing

This applies to primitive hooks, bulk hooks, and `flush(...)`.

### resize_and_overwrite optimization

If a string-like container exposes a well-formed `resize_and_overwrite(...)`, the appender should use it when profitable for `append(...)`.

That optimization is specifically for bulk appends:

- `append(std::span<const code_unit_type>)`
- `append(range)` when the range is sized and the appended unit count is known up front

It should not be used for `push(unit)`.

The callback used with `resize_and_overwrite(...)` should do only bounded copying or conversion into the provided buffer and must not throw.

In particular, this optimization should only be used when the appender can determine the appended code-unit count in advance and preserve the existing prefix while writing the new suffix.

That makes it a good fit for known-size append operations, but not for arbitrary unsized input ranges.

### Validation timing

When codec contract checks are enabled, a primitive decode path that writes Unicode scalars should have those scalars validated as they are appended.

When a custom bulk decode path writes directly toward a UTF-owned result, the writer should not try to validate Unicode incrementally on each `push(...)` or `append(...)`.

Instead:

- the codec writes into raw internal storage
- `decoder_traits<Decoder>::flush(decoder, writer)` runs
- the final produced UTF buffer is validated once
- only then is the validated UTF type materialized

This keeps the writer simple and avoids repeated partial-sequence validation. If a custom bulk decode path emits malformed UTF, that is a codec contract violation rather than an ordinary boundary error.

## Raw Sink Error Type

Overflow is not an `Encoder::encode_error`, because it is not an encoding error.

So the raw `output_range` sink API returns a library-defined wrapper:

```cpp
enum class encode_to_error_kind {
    overflow,
    encoding_error
};
```

This keeps the split clear:

- `overflow` means the destination was exhausted
- `encoding_error` means the encoder itself failed

For fallible encoders, `encode_to_error<Encoder>` should be a small library-defined wrapper with:

- `encode_to_error_kind kind`
- `std::optional<typename encoder_traits<Encoder>::encode_error> error`

where `error` is engaged only when `kind == encode_to_error_kind::encoding_error`.

For infallible encoders, the wrapper collapses to an overflow-only form.

The wrapper stays small and does not require the encoder's `encode_error` to be default-constructible.

## Fast Paths

The primitive contract is writer-based:

- `encode_one`
- `decode_one`

More specifically:

- `encode_one` writes encoded code units to a writer
- `decode_one` writes Unicode scalars to a writer

Optional bulk hooks may still exist for performance:

- `encode_from_utf8(...)`
- `encode_from_utf16(...)`
- `encode_from_utf32(...)`
- `decode_to_utf8(...)`
- `decode_to_utf16(...)`
- `decode_to_utf32(...)`

The required signatures for these hooks are:

```cpp
template <typename Writer>
auto encode_from_utf8(utf8_string_view input, Writer out)
    -> /* void or std::expected<void, encode_error> */;

template <typename Writer>
auto encode_from_utf16(utf16_string_view input, Writer out)
    -> /* void or std::expected<void, encode_error> */;

template <typename Writer>
auto encode_from_utf32(utf32_string_view input, Writer out)
    -> /* void or std::expected<void, encode_error> */;

template <typename Writer>
auto decode_to_utf8(std::basic_string_view<code_unit_type> input, Writer out)
    -> /* void or std::expected<void, decode_error> */;

template <typename Writer>
auto decode_to_utf16(std::basic_string_view<code_unit_type> input, Writer out)
    -> /* void or std::expected<void, decode_error> */;

template <typename Writer>
auto decode_to_utf32(std::basic_string_view<code_unit_type> input, Writer out)
    -> /* void or std::expected<void, decode_error> */;
```

The return-type rule matches the primitive hooks:

- if the codec does not define `encode_error`, the `encode_from_utf*` hooks return `void`
- if the codec defines `encode_error`, the `encode_from_utf*` hooks return `std::expected<void, encode_error>`
- if the codec does not define `decode_error`, the `decode_to_utf*` hooks return `void`
- if the codec defines `decode_error`, the `decode_to_utf*` hooks return `std::expected<void, decode_error>`

These bulk hooks are whole-input operations:

- on success, they are expected to consume the entire input view they were given
- they may not silently stop early and report success
- if they cannot finish the whole provided input, they must report failure instead

A successful bulk hook does not replace end-of-stream finalization. The surrounding library algorithm still calls `flush(...)` afterwards through the traits layer.

These are also surfaced through the traits layer.

When a matching bulk hook exists, the traits layer should prefer it over repeated primitive `encode_one` / `decode_one` calls.

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

For the raw `output_range` path, it is represented only by:

- `encode_to_error_kind::overflow`

For the `encode_append_to(...)` path, there is no overflow state. If the container cannot grow and throws instead, that exception is simply propagated.

### Codec contract violations are not ordinary errors

If a buggy codec reports impossible results, such as:

- successful decode with zero input consumed
- invalid Unicode scalar output
- malformed UTF emitted by a custom bulk path

that is a codec bug, not an ordinary recoverable boundary error.

Those cases should be treated as contract violations, not folded into the normal `expected` result type.

### Contract-check policy

By default, codec contract checks should be performance-oriented:

- scalar validation for primitive decode writers and final Unicode validation for custom bulk decode-to-UTF paths are enabled in debug builds
- release builds may skip those checks by default

The library should also provide a build-time override so users can:

- force contract checks on even in release builds
- force them off even in debug builds

When contract checks are enabled and a violation is detected, the library should treat that as a hard design error:

- with exceptions enabled, throw a library-defined exception
- without exceptions, terminate

When contract checks are disabled, violating the codec contract is undefined behavior.

The exact macro spelling and the exact exception type name are intentionally left open in this proposal.

## Examples

### Strict ASCII Encoder

```cpp
struct ascii_encoder {
    using code_unit_type = char8_t;
    static constexpr bool allow_implicit_construction = true;

    enum class encode_error {
        unrepresentable_scalar
    };

    template <typename Writer>
    constexpr auto encode_one(char32_t scalar, Writer out)
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
    void encode_one(char32_t scalar, Writer out)
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
    static constexpr bool allow_implicit_construction = true;

    enum class decode_error {
        invalid_input
    };

    template <typename Writer>
    constexpr auto decode_one(std::basic_string_view<char8_t> input, Writer out)
        -> std::expected<std::size_t, decode_error>
    {
        const auto byte = static_cast<unsigned char>(input.front());
        if (byte > 0x7F) {
            return std::unexpected(decode_error::invalid_input);
        }

        out.push(static_cast<char32_t>(byte));
        return 1;
    }
};
```

## Built-in Codec Roadmap

The library should start with codecs that are either trivial or low-maintenance, and only later add codecs that require larger mapping data or more involved state machines.

### Good first built-ins

- `ascii_strict`
- `ascii_lossy`
- `iso_8859_1`
- `windows_1252`

These are attractive first codecs because they are easy to implement and easy to maintain:

- ASCII is almost entirely algorithmic and needs no external mapping tables.
- `iso_8859_1` is also essentially direct mapping in the `U+0000..U+00FF` range.
- `windows_1252` needs only a small mapping table for the non-identity byte range around `0x80..0x9F`.

### Good second-wave built-ins

- `shift_jis`
- `big5`
- `euc_kr`

These are still practically relevant, but they require real mapping data and more careful testing.

### Higher-cost built-ins

- `gb18030`
- `iso_2022_jp`

These are useful, but they should come later:

- `gb18030` has a larger and more complex mapping story than the earlier codecs.
- `iso_2022_jp` is a better stress test for true stateful encoder/decoder behavior than a first implementation target.

### Maintenance expectations

Not all built-in codecs carry the same maintenance burden.

There are roughly three tiers:

- trivial or nearly trivial codecs:
  - ASCII
  - `iso_8859_1`
- small-table codecs:
  - `windows_1252`
- table-heavy or stateful codecs:
  - `shift_jis`
  - `big5`
  - `euc_kr`
  - `gb18030`
  - `iso_2022_jp`

For the first two tiers, native support is cheap enough that it makes sense to ship them directly.

For the table-heavy codecs, support is still feasible, but it should be based on vendored, frozen mapping data rather than hand-maintained ad hoc tables.

In practice, these legacy mappings are fairly stable. The maintenance cost is less about chasing frequent standards churn and more about:

- generating the tables correctly
- documenting which mapping source is being followed
- testing the codec thoroughly

So the recommended built-in order is:

1. `ascii_strict`
2. `ascii_lossy`
3. `iso_8859_1`
4. `windows_1252`
5. one multibyte table-driven codec
6. additional multibyte and stateful codecs only after the model is proven

## Recommendation

If this direction is pursued, the first implementation should stay narrow:

1. Define `Encoder` / `Decoder` concepts.
2. Define `encoder_traits` / `decoder_traits`.
3. Add the checked `from_encoded(...)` family.
4. Add `encode_to(...)`, `encode_append_to(...)`, and convenience `to_encoded(...)`.
5. Add one strict and one lossy ASCII codec as proof of concept.
6. Add bulk fast-path support only after the primitive model is stable.

That is enough to prove the extensibility story without weakening the core validated UTF design.

## Still Open

- exact build-macro spelling and value scheme for the codec contract-check override
- exact exception type and naming for codec contract violations
- exact mapping-data provenance to standardize on once table-driven built-in codecs are added
