# Text Operations

## Search

Both borrowed and owning text types expose STL-style search APIs, including:

- `contains`
- `find`
- `rfind`
- `find_first_of` / `find_first_not_of`
- `find_last_of` / `find_last_not_of`
- `starts_with`
- `ends_with`

These work with text, characters, predicates, and span-based character sets depending on the overload.

```cpp
--8<-- "examples/text-operations/search-and-replace.cpp"
```

## Split and match families

String views expose a broad split/match surface:

- `split`, `rsplit`
- `split_terminator`, `rsplit_terminator`
- `splitn`, `rsplitn`
- `split_inclusive`
- `split_trimmed`
- `matches`, `rmatches`, `rmatch_indices`
- `split_once`, `rsplit_once`
- `split_whitespace`, `split_ascii_whitespace`

Delimiter behavior is intentionally explicit. See the split sections in the [string view reference](reference/string-views.md).

```cpp
--8<-- "examples/text-operations/split-and-trim.cpp"
```

The split APIs also compose cleanly with standard range pipelines:

```cpp
--8<-- "examples/text-operations/views-pipeline.cpp"
```

## Trim and prefix/suffix operations

Available operations include:

- `strip_prefix`, `strip_suffix`, `strip_circumfix`
- `trim_prefix`, `trim_suffix`
- `trim_start_matches`, `trim_end_matches`, `trim_matches`
- `trim_start`, `trim_end`, `trim`
- `trim_ascii_start`, `trim_ascii_end`, `trim_ascii`

The matcher-based trim APIs accept characters, text, predicates, and character sets.

## Boundary and access APIs

Important boundary-aware APIs include:

- `is_char_boundary`
- `ceil_char_boundary`
- `floor_char_boundary`
- `is_grapheme_boundary`
- `ceil_grapheme_boundary`
- `floor_grapheme_boundary`
- `char_at`
- `grapheme_at`
- `grapheme_substr`
- `substr`

These are essential whenever offsets are expressed in code units but user-visible semantics depend on characters or graphemes.

```cpp
--8<-- "examples/text-operations/boundaries-and-graphemes.cpp"
```

## Reverse and replace on owning strings

Owning strings add mutating APIs such as:

- `insert`
- `pop_back`
- `erase`
- `reverse()`
- `reverse(pos, count = npos)`
- `reverse_graphemes()`
- `reverse_graphemes(pos, count = npos)`
- `replace(...)`
- `replace_all(...)`
- `replace_n(...)`

Case-transformation APIs also support partial overloads on owning strings:

- `to_ascii_lowercase(pos, count)`
- `to_ascii_uppercase(pos, count)`
- `to_lowercase(pos, count)`
- `to_uppercase(pos, count)`

```cpp
--8<-- "examples/text-operations/reverse.cpp"
```

## Return-unit semantics

The most important rule to keep in mind:

- UTF-8 view/string search offsets are byte offsets unless the API name says otherwise.
- UTF-16 view/string search offsets are code-unit offsets unless the API name says otherwise.

Character- and grapheme-oriented APIs are named explicitly and should be preferred when the distinction matters.

## Grapheme-aware operations

Default Unicode grapheme segmentation is supported through:

- `graphemes()`
- `grapheme_indices()`
- `grapheme_count()`
- grapheme-aware searching and substring APIs

These use default Unicode grapheme-cluster rules rather than locale-specific tailoring.
