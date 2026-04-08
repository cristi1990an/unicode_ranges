use std::char;
use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

type Range = (u32, u32);

#[derive(Clone, Copy, PartialEq, Eq)]
struct UnicodeVersion {
    major: u8,
    minor: u8,
    patch: u8,
}

const DEFAULT_UNICODE_VERSION: UnicodeVersion = UnicodeVersion {
    major: 17,
    minor: 0,
    patch: 0,
};

const GRAPHEME_BREAK_VALUES: &[(&str, &str, &str)] = &[
    ("CR", "grapheme_break_cr_ranges", "grapheme_cluster_break_property::cr"),
    ("LF", "grapheme_break_lf_ranges", "grapheme_cluster_break_property::lf"),
    ("Control", "grapheme_break_control_ranges", "grapheme_cluster_break_property::control"),
    ("Extend", "grapheme_break_extend_ranges", "grapheme_cluster_break_property::extend"),
    ("ZWJ", "grapheme_break_zwj_ranges", "grapheme_cluster_break_property::zwj"),
    (
        "Regional_Indicator",
        "grapheme_break_regional_indicator_ranges",
        "grapheme_cluster_break_property::regional_indicator",
    ),
    ("Prepend", "grapheme_break_prepend_ranges", "grapheme_cluster_break_property::prepend"),
    (
        "SpacingMark",
        "grapheme_break_spacing_mark_ranges",
        "grapheme_cluster_break_property::spacing_mark",
    ),
    ("L", "grapheme_break_l_ranges", "grapheme_cluster_break_property::l"),
    ("V", "grapheme_break_v_ranges", "grapheme_cluster_break_property::v"),
    ("T", "grapheme_break_t_ranges", "grapheme_cluster_break_property::t"),
    ("LV", "grapheme_break_lv_ranges", "grapheme_cluster_break_property::lv"),
    ("LVT", "grapheme_break_lvt_ranges", "grapheme_cluster_break_property::lvt"),
];

const INDIC_CONJUNCT_BREAK_VALUES: &[(&str, &str, &str)] = &[
    (
        "Consonant",
        "indic_conjunct_break_consonant_ranges",
        "indic_conjunct_break_property::consonant",
    ),
    (
        "Extend",
        "indic_conjunct_break_extend_ranges",
        "indic_conjunct_break_property::extend",
    ),
    (
        "Linker",
        "indic_conjunct_break_linker_ranges",
        "indic_conjunct_break_property::linker",
    ),
];

struct PropertyRecord {
    first: u32,
    last: u32,
    fields: Vec<String>,
}

struct CaseMappingRecord {
    source: u32,
    mapped: Vec<u32>,
}

struct SimpleCaseMappingRecord {
    source: u32,
    mapped: u32,
}

struct UnicodeDataRecord {
    scalar: u32,
    general_category: String,
    canonical_combining_class: u8,
    decomposition_mapping: Option<DecompositionMappingRecord>,
}

struct DecompositionMappingRecord {
    compatibility: bool,
    mapped: Vec<u32>,
}

struct CanonicalCombiningClassRangeRecord {
    first: u32,
    last: u32,
    canonical_combining_class: u8,
}

struct DecompositionRecord {
    source: u32,
    compatibility: bool,
    mapped: Vec<u32>,
}

struct CompositionMappingRecord {
    first: u32,
    second: u32,
    composed: u32,
}

#[derive(Clone, Copy, PartialEq, Eq)]
struct GraphemePropertiesValue {
    grapheme_break_index: usize,
    indic_conjunct_break_index: usize,
    extended_pictographic: bool,
}

struct GraphemePropertiesRangeRecord {
    first: u32,
    last: u32,
    value: GraphemePropertiesValue,
}

struct PropertyValueRangeRecord {
    first: u32,
    last: u32,
    value: String,
}

fn collect_ranges<F: Fn(char) -> bool>(pred: F) -> Vec<Range> {
    let mut ranges = Vec::new();
    let mut start: Option<u32> = None;
    let mut prev = 0u32;

    for scalar in 0u32..=0x10FFFF {
        if (0xD800..=0xDFFF).contains(&scalar) {
            continue;
        }

        let ch = char::from_u32(scalar).unwrap();
        let matches = pred(ch);
        match (start, matches) {
            (None, true) => {
                start = Some(scalar);
                prev = scalar;
            }
            (Some(_), true) if scalar == prev + 1 => {
                prev = scalar;
            }
            (Some(s), true) => {
                ranges.push((s, prev));
                start = Some(scalar);
                prev = scalar;
            }
            (Some(s), false) => {
                ranges.push((s, prev));
                start = None;
            }
            (None, false) => {}
        }
    }

    if let Some(s) = start {
        ranges.push((s, prev));
    }

    ranges
}

fn collect_case_mappings<F: Fn(char) -> Vec<u32>>(map: F) -> Vec<CaseMappingRecord> {
    let mut mappings = Vec::new();

    for scalar in 0u32..=0x10FFFF {
        if (0xD800..=0xDFFF).contains(&scalar) {
            continue;
        }

        let ch = char::from_u32(scalar).unwrap();
        let mapped = map(ch);
        if mapped.len() == 1 && mapped[0] == scalar {
            continue;
        }

        mappings.push(CaseMappingRecord {
            source: scalar,
            mapped,
        });
    }

    mappings
}

fn split_case_mappings(mappings: &[CaseMappingRecord]) -> (Vec<SimpleCaseMappingRecord>, Vec<CaseMappingRecord>) {
    let mut simple = Vec::new();
    let mut special = Vec::new();

    for mapping in mappings {
        if mapping.mapped.len() == 1 {
            simple.push(SimpleCaseMappingRecord {
                source: mapping.source,
                mapped: mapping.mapped[0],
            });
        } else {
            special.push(CaseMappingRecord {
                source: mapping.source,
                mapped: mapping.mapped.clone(),
            });
        }
    }

    (simple, special)
}

fn parse_unicode_scalar(value: &str) -> io::Result<u32> {
    u32::from_str_radix(value.trim(), 16).map_err(|err| {
        io::Error::new(
            io::ErrorKind::InvalidData,
            format!("invalid scalar `{value}`: {err}"),
        )
    })
}

fn parse_unicode_data(path: &Path) -> io::Result<Vec<UnicodeDataRecord>> {
    let content = fs::read_to_string(path)?;
    let mut records = Vec::new();

    for (line_index, line) in content.lines().enumerate() {
        if line.trim().is_empty() {
            continue;
        }

        let fields: Vec<&str> = line.split(';').collect();
        if fields.len() < 15 {
            return Err(io::Error::new(
                io::ErrorKind::InvalidData,
                format!(
                    "{}:{}: expected 15 semicolon-separated fields",
                    path.display(),
                    line_index + 1
                ),
            ));
        }

        let scalar = parse_unicode_scalar(fields[0])?;
        let general_category = fields[2].trim().to_owned();
        let canonical_combining_class = fields[3].parse::<u8>().map_err(|err| {
            io::Error::new(
                io::ErrorKind::InvalidData,
                format!(
                    "{}:{}: invalid canonical combining class `{}`: {err}",
                    path.display(),
                    line_index + 1,
                    fields[3]
                ),
            )
        })?;
        let decomposition_mapping = if fields[5].trim().is_empty() {
            None
        } else {
            let mut parts = fields[5].split_whitespace();
            let compatibility = parts
                .next()
                .is_some_and(|first| first.starts_with('<'));
            let mapped = if compatibility {
                parts
                    .map(parse_unicode_scalar)
                    .collect::<io::Result<Vec<_>>>()?
            } else {
                fields[5]
                    .split_whitespace()
                    .map(parse_unicode_scalar)
                    .collect::<io::Result<Vec<_>>>()?
            };

            Some(DecompositionMappingRecord {
                compatibility,
                mapped,
            })
        };

        records.push(UnicodeDataRecord {
            scalar,
            general_category,
            canonical_combining_class,
            decomposition_mapping,
        });
    }

    Ok(records)
}

fn collect_canonical_combining_class_ranges(records: &[UnicodeDataRecord]) -> Vec<CanonicalCombiningClassRangeRecord> {
    let mut ranges: Vec<CanonicalCombiningClassRangeRecord> = Vec::new();

    for record in records {
        if record.canonical_combining_class == 0 {
            continue;
        }

        if let Some(previous) = ranges.last_mut() {
            if previous.last + 1 == record.scalar
                && previous.canonical_combining_class == record.canonical_combining_class
            {
                previous.last = record.scalar;
                continue;
            }
        }

        ranges.push(CanonicalCombiningClassRangeRecord {
            first: record.scalar,
            last: record.scalar,
            canonical_combining_class: record.canonical_combining_class,
        });
    }

    ranges
}

fn to_snake_case_identifier(value: &str) -> String {
    let mut result = String::new();
    let mut previous_was_separator = false;
    let mut previous_was_alphanumeric = false;
    let mut previous_was_lower_or_digit = false;
    let chars: Vec<char> = value.chars().collect();

    for (index, ch) in chars.iter().copied().enumerate() {
        if ch.is_ascii_alphanumeric() {
            let next_is_lowercase = chars
                .get(index + 1)
                .is_some_and(|next| next.is_ascii_lowercase());

            if ch.is_ascii_uppercase()
                && !result.is_empty()
                && !previous_was_separator
                && (previous_was_lower_or_digit || (previous_was_alphanumeric && next_is_lowercase))
            {
                result.push('_');
            }
            result.push(ch.to_ascii_lowercase());
            previous_was_separator = false;
            previous_was_alphanumeric = true;
            previous_was_lower_or_digit = ch.is_ascii_lowercase() || ch.is_ascii_digit();
        } else {
            if !result.is_empty() && !previous_was_separator {
                result.push('_');
            }
            previous_was_separator = true;
            previous_was_alphanumeric = false;
            previous_was_lower_or_digit = false;
        }
    }

    while result.ends_with('_') {
        result.pop();
    }

    result
}

fn general_category_enum_variant(value: &str) -> &'static str {
    match value {
        "Lu" => "uppercase_letter",
        "Ll" => "lowercase_letter",
        "Lt" => "titlecase_letter",
        "Lm" => "modifier_letter",
        "Lo" => "other_letter",
        "Mn" => "nonspacing_mark",
        "Mc" => "spacing_mark",
        "Me" => "enclosing_mark",
        "Nd" => "decimal_number",
        "Nl" => "letter_number",
        "No" => "other_number",
        "Pc" => "connector_punctuation",
        "Pd" => "dash_punctuation",
        "Ps" => "open_punctuation",
        "Pe" => "close_punctuation",
        "Pi" => "initial_punctuation",
        "Pf" => "final_punctuation",
        "Po" => "other_punctuation",
        "Sm" => "math_symbol",
        "Sc" => "currency_symbol",
        "Sk" => "modifier_symbol",
        "So" => "other_symbol",
        "Zs" => "space_separator",
        "Zl" => "line_separator",
        "Zp" => "paragraph_separator",
        "Cc" => "control",
        "Cf" => "format",
        "Cs" => "surrogate",
        "Co" => "private_use",
        "Cn" => "unassigned",
        _ => panic!("unsupported general category `{}`", value),
    }
}

fn east_asian_width_enum_variant(value: &str) -> &'static str {
    match value {
        "F" => "fullwidth",
        "H" => "halfwidth",
        "W" => "wide",
        "Na" => "narrow",
        "A" => "ambiguous",
        "N" => "neutral",
        _ => panic!("unsupported East Asian Width `{}`", value),
    }
}

fn bidi_class_enum_variant(value: &str) -> &'static str {
    match value {
        "L" => "left_to_right",
        "R" => "right_to_left",
        "AL" => "arabic_letter",
        "EN" => "european_number",
        "ES" => "european_separator",
        "ET" => "european_terminator",
        "AN" => "arabic_number",
        "CS" => "common_separator",
        "NSM" => "nonspacing_mark",
        "BN" => "boundary_neutral",
        "B" => "paragraph_separator",
        "S" => "segment_separator",
        "WS" => "whitespace",
        "ON" => "other_neutral",
        "LRE" => "left_to_right_embedding",
        "LRO" => "left_to_right_override",
        "RLE" => "right_to_left_embedding",
        "RLO" => "right_to_left_override",
        "PDF" => "pop_directional_format",
        "LRI" => "left_to_right_isolate",
        "RLI" => "right_to_left_isolate",
        "FSI" => "first_strong_isolate",
        "PDI" => "pop_directional_isolate",
        _ => panic!("unsupported bidi class `{}`", value),
    }
}

fn line_break_enum_variant(value: &str) -> &'static str {
    match value {
        "AI" => "ambiguous",
        "AK" => "aksara",
        "AL" => "alphabetic",
        "AP" => "aksara_prebase",
        "AS" => "aksara_start",
        "B2" => "break_both",
        "BA" => "break_after",
        "BB" => "break_before",
        "BK" => "mandatory_break",
        "CB" => "contingent_break",
        "CJ" => "conditional_japanese_starter",
        "CL" => "close_punctuation",
        "CM" => "combining_mark",
        "CP" => "close_parenthesis",
        "CR" => "carriage_return",
        "EB" => "emoji_base",
        "EM" => "emoji_modifier",
        "EX" => "exclamation",
        "GL" => "glue",
        "H2" => "hangul_lv_syllable",
        "H3" => "hangul_lvt_syllable",
        "HH" => "hebrew_hyphen",
        "HL" => "hebrew_letter",
        "HY" => "hyphen",
        "ID" => "ideographic",
        "IN" => "inseparable",
        "IS" => "infix_numeric_separator",
        "JL" => "hangul_l_jamo",
        "JT" => "hangul_t_jamo",
        "JV" => "hangul_v_jamo",
        "LF" => "line_feed",
        "NL" => "next_line",
        "NS" => "nonstarter",
        "NU" => "numeric",
        "OP" => "open_punctuation",
        "PO" => "postfix_numeric",
        "PR" => "prefix_numeric",
        "QU" => "quotation",
        "RI" => "regional_indicator",
        "SA" => "complex_context",
        "SG" => "surrogate",
        "SP" => "space",
        "SY" => "symbols_allowing_break_after",
        "VF" => "virama_final",
        "VI" => "virama",
        "WJ" => "word_joiner",
        "XX" => "unknown",
        "ZW" => "zero_width_space",
        "ZWJ" => "zero_width_joiner",
        _ => panic!("unsupported line break class `{}`", value),
    }
}

fn grapheme_break_enum_variant(value: &str) -> &'static str {
    match value {
        "Other" => "other",
        "CR" => "cr",
        "LF" => "lf",
        "Control" => "control",
        "Extend" => "extend",
        "ZWJ" => "zwj",
        "Regional_Indicator" => "regional_indicator",
        "Prepend" => "prepend",
        "SpacingMark" => "spacing_mark",
        "L" => "l",
        "V" => "v",
        "T" => "t",
        "LV" => "lv",
        "LVT" => "lvt",
        _ => panic!("unsupported grapheme break property `{}`", value),
    }
}

fn collect_decomposition_records(records: &[UnicodeDataRecord]) -> Vec<DecompositionRecord> {
    records
        .iter()
        .filter_map(|record| {
            record.decomposition_mapping.as_ref().map(|mapping| DecompositionRecord {
                source: record.scalar,
                compatibility: mapping.compatibility,
                mapped: mapping.mapped.clone(),
            })
        })
        .collect()
}

fn collect_composition_exclusions(path: &Path) -> io::Result<BTreeSet<u32>> {
    let mut excluded = BTreeSet::new();
    let content = fs::read_to_string(path)?;

    for (line_index, line) in content.lines().enumerate() {
        let line = line.split('#').next().unwrap().trim();
        if line.is_empty() {
            continue;
        }

        let (first, last) = parse_range_spec(line).map_err(|message| {
            io::Error::new(
                io::ErrorKind::InvalidData,
                format!("{}:{}: {message}", path.display(), line_index + 1),
            )
        })?;

        for scalar in first..=last {
            excluded.insert(scalar);
        }
    }

    Ok(excluded)
}

fn collect_composition_records(
    records: &[UnicodeDataRecord],
    composition_exclusions: &BTreeSet<u32>,
) -> Vec<CompositionMappingRecord> {
    let mut compositions = Vec::new();

    for record in records {
        if composition_exclusions.contains(&record.scalar) {
            continue;
        }

        let Some(mapping) = &record.decomposition_mapping else {
            continue;
        };

        if mapping.compatibility || mapping.mapped.len() != 2 {
            continue;
        }

        compositions.push(CompositionMappingRecord {
            first: mapping.mapped[0],
            second: mapping.mapped[1],
            composed: record.scalar,
        });
    }

    compositions.sort_unstable_by_key(|mapping| (mapping.first, mapping.second));
    compositions
}

fn parse_case_folding(path: &Path) -> io::Result<Vec<CaseMappingRecord>> {
    let content = fs::read_to_string(path)?;
    let mut mappings = BTreeMap::<u32, CaseMappingRecord>::new();

    for (line_index, line) in content.lines().enumerate() {
        let line = line.split('#').next().unwrap().trim();
        if line.is_empty() {
            continue;
        }

        let fields: Vec<&str> = line.split(';').map(str::trim).collect();
        if fields.len() < 3 {
            return Err(io::Error::new(
                io::ErrorKind::InvalidData,
                format!(
                    "{}:{}: expected at least three semicolon-separated fields",
                    path.display(),
                    line_index + 1
                ),
            ));
        }

        let source = parse_unicode_scalar(fields[0])?;
        let status = fields[1];
        let mapped = fields[2]
            .split_whitespace()
            .map(parse_unicode_scalar)
            .collect::<io::Result<Vec<_>>>()?;

        match status {
            "F" => {
                mappings.insert(source, CaseMappingRecord { source, mapped });
            }
            "C" | "S" => {
                mappings.entry(source).or_insert(CaseMappingRecord { source, mapped });
            }
            "T" => {}
            _ => {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidData,
                    format!(
                        "{}:{}: unsupported case folding status `{status}`",
                        path.display(),
                        line_index + 1
                    ),
                ));
            }
        }
    }

    Ok(mappings.into_values().collect())
}

fn contains_scalar_in_ranges(ranges: &[Range], scalar: u32) -> bool {
    let mut left = 0usize;
    let mut right = ranges.len();

    while left < right {
        let mid = left + (right - left) / 2;
        let (first, last) = ranges[mid];
        if scalar < first {
            right = mid;
        } else if scalar > last {
            left = mid + 1;
        } else {
            return true;
        }
    }

    false
}

fn grapheme_break_index_for_scalar(
    grapheme_break_ranges: &BTreeMap<String, Vec<Range>>,
    scalar: u32,
) -> usize {
    for (index, &(name, _, _)) in GRAPHEME_BREAK_VALUES.iter().enumerate() {
        if grapheme_break_ranges
            .get(name)
            .is_some_and(|ranges| contains_scalar_in_ranges(ranges, scalar))
        {
            return index + 1;
        }
    }

    0
}

fn indic_conjunct_break_index_for_scalar(
    indic_conjunct_break_ranges: &BTreeMap<String, Vec<Range>>,
    scalar: u32,
) -> usize {
    for (index, &(name, _, _)) in INDIC_CONJUNCT_BREAK_VALUES.iter().enumerate() {
        if indic_conjunct_break_ranges
            .get(name)
            .is_some_and(|ranges| contains_scalar_in_ranges(ranges, scalar))
        {
            return index + 1;
        }
    }

    0
}

fn push_range_boundaries(boundaries: &mut Vec<u32>, ranges: &[Range]) {
    for &(first, last) in ranges {
        boundaries.push(first);
        boundaries.push(last.saturating_add(1));
    }
}

fn collect_grapheme_properties_ranges(
    grapheme_break_ranges: &BTreeMap<String, Vec<Range>>,
    extended_pictographic_ranges: &[Range],
    indic_conjunct_break_ranges: &BTreeMap<String, Vec<Range>>,
) -> Vec<GraphemePropertiesRangeRecord> {
    let mut boundaries = vec![0, 0x110000];

    for ranges in grapheme_break_ranges.values() {
        push_range_boundaries(&mut boundaries, ranges);
    }

    push_range_boundaries(&mut boundaries, extended_pictographic_ranges);

    for ranges in indic_conjunct_break_ranges.values() {
        push_range_boundaries(&mut boundaries, ranges);
    }

    boundaries.sort_unstable();
    boundaries.dedup();

    let mut result: Vec<GraphemePropertiesRangeRecord> = Vec::new();
    for boundary_pair in boundaries.windows(2) {
        let first = boundary_pair[0];
        let next = boundary_pair[1];
        if first >= 0x110000 || first == next {
            continue;
        }

        let last = next - 1;
        let value = GraphemePropertiesValue {
            grapheme_break_index: grapheme_break_index_for_scalar(grapheme_break_ranges, first),
            indic_conjunct_break_index: indic_conjunct_break_index_for_scalar(indic_conjunct_break_ranges, first),
            extended_pictographic: contains_scalar_in_ranges(extended_pictographic_ranges, first),
        };

        if value.grapheme_break_index == 0
            && value.indic_conjunct_break_index == 0
            && !value.extended_pictographic
        {
            continue;
        }

        if let Some(previous) = result.last_mut() {
            if previous.last + 1 == first && previous.value == value {
                previous.last = last;
                continue;
            }
        }

        result.push(GraphemePropertiesRangeRecord { first, last, value });
    }

    result
}

fn emit_ranges(name: &str, ranges: &[Range]) {
    println!(
        "inline constexpr std::array<unicode_range, {}> {}{{{{",
        ranges.len(),
        name
    );
    for &(first, last) in ranges {
        println!("    {{ 0x{first:04X}u, 0x{last:04X}u }},");
    }
    println!("}}}};");
    println!();
}

fn case_mapping_max_length(mappings: &[CaseMappingRecord]) -> usize {
    mappings
        .iter()
        .map(|mapping| mapping.mapped.len())
        .max()
        .unwrap_or(1)
}

fn emit_case_mapping_support(max_length: usize) {
    println!(
        "inline constexpr std::size_t unicode_case_mapping_max_length = {max_length};"
    );
    println!();
    println!("struct unicode_simple_case_mapping");
    println!("{{");
    println!("    std::uint32_t source;");
    println!("    std::uint32_t mapped;");
    println!("}};");
    println!();
    println!("struct unicode_simple_case_delta_range");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t last;");
    println!("    std::int32_t delta;");
    println!("    std::uint8_t stride;");
    println!("}};");
    println!();
    println!("struct unicode_special_case_mapping");
    println!("{{");
    println!("    std::uint32_t source;");
    println!("    std::uint8_t count;");
    println!(
        "    std::array<std::uint32_t, unicode_case_mapping_max_length> mapped;"
    );
    println!("}};");
    println!();
    println!("inline constexpr std::size_t unicode_mapping_page_shift = 8;");
    println!("inline constexpr std::size_t unicode_mapping_page_count = (0x10FFFFu >> unicode_mapping_page_shift) + 1u;");
    println!();
    println!("struct unicode_range_page_slice");
    println!("{{");
    println!("    std::uint16_t begin;");
    println!("    std::uint16_t end;");
    println!("}};");
    println!();
    println!("template <std::size_t N>");
    println!("struct unicode_simple_case_delta_range_set");
    println!("{{");
    println!("    std::array<unicode_simple_case_delta_range, N> ranges{{}};");
    println!("    std::uint16_t count = 0;");
    println!("}};");
    println!();
    println!("constexpr std::uint32_t apply_case_mapping_delta(std::uint32_t scalar, std::int32_t delta) noexcept");
    println!("{{");
    println!("    return static_cast<std::uint32_t>(static_cast<std::int64_t>(scalar) + static_cast<std::int64_t>(delta));");
    println!("}}");
    println!();
    println!("template <typename Mapping, std::size_t N>");
    println!(
        "constexpr auto make_source_mapping_page_index(const std::array<Mapping, N>& mappings) noexcept"
    );
    println!("{{");
    println!("    std::array<std::uint16_t, unicode_mapping_page_count + 1> page_index{{}};");
    println!("    std::size_t mapping_index = 0;");
    println!("    for (std::size_t page = 0; page != unicode_mapping_page_count; ++page)");
    println!("    {{");
    println!("        page_index[page] = static_cast<std::uint16_t>(mapping_index);");
    println!("        const auto page_end = static_cast<std::uint32_t>((page + 1u) << unicode_mapping_page_shift);");
    println!("        while (mapping_index < N && mappings[mapping_index].source < page_end)");
    println!("        {{");
    println!("            ++mapping_index;");
    println!("        }}");
    println!("    }}");
    println!("    page_index[unicode_mapping_page_count] = static_cast<std::uint16_t>(mapping_index);");
    println!("    return page_index;");
    println!("}}");
    println!();
    println!("template <typename Mapping, std::size_t N, std::size_t P>");
    println!(
        "constexpr const Mapping* find_source_mapping_paged("
    );
    println!("    std::uint32_t scalar,");
    println!("    const std::array<Mapping, N>& mappings,");
    println!("    const std::array<std::uint16_t, P>& page_index) noexcept");
    println!("{{");
    println!("    const auto page = static_cast<std::size_t>(scalar >> unicode_mapping_page_shift);");
    println!("    std::size_t left = page_index[page];");
    println!("    std::size_t right = page_index[page + 1u];");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const Mapping& mapping = mappings[mid];");
    println!("        if (scalar < mapping.source)");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (scalar > mapping.source)");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return &mapping;");
    println!("        }}");
    println!("    }}");
    println!("    return nullptr;");
    println!("}}");
    println!();
    println!("template <typename Mapping, std::size_t N, std::size_t P>");
    println!("constexpr std::size_t find_source_mapping_paged_index(");
    println!("    std::uint32_t scalar,");
    println!("    const std::array<Mapping, N>& mappings,");
    println!("    const std::array<std::uint16_t, P>& page_index) noexcept");
    println!("{{");
    println!("    const auto page = static_cast<std::size_t>(scalar >> unicode_mapping_page_shift);");
    println!("    std::size_t left = page_index[page];");
    println!("    std::size_t right = page_index[page + 1u];");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const Mapping& mapping = mappings[mid];");
    println!("        if (scalar < mapping.source)");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (scalar > mapping.source)");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return mid;");
    println!("        }}");
    println!("    }}");
    println!("    return N;");
    println!("}}");
    println!();
    println!("template <typename Range, std::size_t N>");
    println!("constexpr auto make_overlapping_range_page_slices(");
    println!("    const std::array<Range, N>& ranges,");
    println!("    std::size_t count = N) noexcept");
    println!("{{");
    println!("    std::array<unicode_range_page_slice, unicode_mapping_page_count> page_slices{{}};");
    println!("    std::size_t begin = 0;");
    println!("    std::size_t end = 0;");
    println!("    for (std::size_t page = 0; page != unicode_mapping_page_count; ++page)");
    println!("    {{");
    println!("        const auto page_start = static_cast<std::uint32_t>(page << unicode_mapping_page_shift);");
    println!("        const auto page_end = static_cast<std::uint32_t>((page + 1u) << unicode_mapping_page_shift);");
    println!("        while (begin < count && ranges[begin].last < page_start)");
    println!("        {{");
    println!("            ++begin;");
    println!("        }}");
    println!("        if (end < begin)");
    println!("        {{");
    println!("            end = begin;");
    println!("        }}");
    println!("        while (end < count && ranges[end].first < page_end)");
    println!("        {{");
    println!("            ++end;");
    println!("        }}");
    println!("        page_slices[page] = unicode_range_page_slice{{");
    println!("            static_cast<std::uint16_t>(begin),");
    println!("            static_cast<std::uint16_t>(end)");
    println!("        }};");
    println!("    }}");
    println!("    return page_slices;");
    println!("}}");
    println!();
    println!("template <typename Range, std::size_t N, std::size_t P>");
    println!("constexpr std::size_t find_overlapping_range_index_paged(");
    println!("    std::uint32_t scalar,");
    println!("    const std::array<Range, N>& ranges,");
    println!("    std::size_t count,");
    println!("    const std::array<unicode_range_page_slice, P>& page_slices) noexcept");
    println!("{{");
    println!("    const auto page = static_cast<std::size_t>(scalar >> unicode_mapping_page_shift);");
    println!("    std::size_t left = page_slices[page].begin;");
    println!("    std::size_t right = page_slices[page].end;");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const Range& range = ranges[mid];");
    println!("        if (scalar < range.first)");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (scalar > range.last)");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return mid;");
    println!("        }}");
    println!("    }}");
    println!("    return count;");
    println!("}}");
    println!();
    println!("template <std::size_t N>");
    println!("constexpr auto make_simple_case_delta_ranges(const std::array<unicode_simple_case_mapping, N>& mappings) noexcept");
    println!("{{");
    println!("    unicode_simple_case_delta_range_set<N> result{{}};");
    println!("    std::size_t read = 0;");
    println!("    while (read < N)");
    println!("    {{");
    println!("        const auto first = mappings[read];");
    println!("        const auto delta = static_cast<std::int64_t>(first.mapped) - static_cast<std::int64_t>(first.source);");
    println!("        std::size_t end = read;");
    println!("        std::uint32_t stride = 0;");
    println!("        while (end + 1 < N)");
    println!("        {{");
    println!("            const auto& current = mappings[end];");
    println!("            const auto& next = mappings[end + 1];");
    println!("            const auto next_delta = static_cast<std::int64_t>(next.mapped) - static_cast<std::int64_t>(next.source);");
    println!("            const auto step = next.source - current.source;");
    println!("            if (next_delta != delta || (step != 1u && step != 2u))");
    println!("            {{");
    println!("                break;");
    println!("            }}");
    println!("            if (stride == 0)");
    println!("            {{");
    println!("                stride = step;");
    println!("            }}");
    println!("            else if (step != stride)");
    println!("            {{");
    println!("                break;");
    println!("            }}");
    println!("            ++end;");
    println!("        }}");
    println!();
    println!("        if (end > read)");
    println!("        {{");
    println!("            result.ranges[result.count++] = unicode_simple_case_delta_range{{");
    println!("                first.source,");
    println!("                mappings[end].source,");
    println!("                static_cast<std::int32_t>(delta),");
    println!("                static_cast<std::uint8_t>(stride)");
    println!("            }};");
    println!("            read = end + 1;");
    println!("            continue;");
    println!("        }}");
    println!();
    println!("        ++read;");
    println!("    }}");
    println!("    return result;");
    println!("}}");
    println!();
}

fn emit_simple_case_mappings(name: &str, mappings: &[SimpleCaseMappingRecord]) {
    println!(
        "inline constexpr std::array<unicode_simple_case_mapping, {}> {}{{{{",
        mappings.len(),
        name
    );
    for mapping in mappings {
        println!(
            "    {{ 0x{:04X}u, 0x{:04X}u }},",
            mapping.source,
            mapping.mapped
        );
    }
    println!("}}}};");
    println!();
}

fn emit_special_case_mappings(name: &str, mappings: &[CaseMappingRecord], max_length: usize) {
    println!(
        "inline constexpr std::array<unicode_special_case_mapping, {}> {}{{{{",
        mappings.len(),
        name
    );
    for mapping in mappings {
        print!(
            "    {{ 0x{:04X}u, {}u, {{ ",
            mapping.source,
            mapping.mapped.len()
        );
        for index in 0..max_length {
            let scalar = mapping.mapped.get(index).copied().unwrap_or(0);
            if index != 0 {
                print!(", ");
            }
            print!("0x{scalar:04X}u");
        }
        println!(" }} }},");
    }
    println!("}}}};");
    println!();
}

fn emit_canonical_combining_class_support() {
    println!("struct unicode_canonical_combining_class_range");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t last;");
    println!("    std::uint8_t canonical_combining_class;");
    println!("}};");
    println!();
}

fn emit_canonical_combining_class_ranges(
    name: &str,
    ranges: &[CanonicalCombiningClassRangeRecord],
) {
    println!(
        "inline constexpr std::array<unicode_canonical_combining_class_range, {}> {}{{{{",
        ranges.len(),
        name
    );
    for range in ranges {
        println!(
            "    {{ 0x{:04X}u, 0x{:04X}u, {}u }},",
            range.first,
            range.last,
            range.canonical_combining_class
        );
    }
    println!("}}}};");
    println!();
}

fn emit_canonical_combining_class_lookup(name: &str, ranges_name: &str) {
    println!("inline constexpr auto {ranges_name}_page_slices =");
    println!("    make_overlapping_range_page_slices({ranges_name});");
    println!();
    println!("constexpr std::uint8_t {name}(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    const auto index = find_overlapping_range_index_paged(");
    println!("        scalar,");
    println!("        {ranges_name},");
    println!("        {ranges_name}.size(),");
    println!("        {ranges_name}_page_slices);");
    println!("    if (index == {ranges_name}.size())");
    println!("    {{");
    println!("        return 0u;");
    println!("    }}");
    println!("    return {ranges_name}[index].canonical_combining_class;");
    println!("}}");
    println!();
}

fn decomposition_mapping_max_length(mappings: &[DecompositionRecord]) -> usize {
    mappings
        .iter()
        .map(|mapping| mapping.mapped.len())
        .max()
        .unwrap_or(1)
}

fn emit_decomposition_support(max_length: usize) {
    println!("inline constexpr std::size_t unicode_decomposition_max_length = {max_length};");
    println!();
    println!("struct unicode_decomposition_mapping");
    println!("{{");
    println!("    std::uint32_t source;");
    println!("    bool compatibility;");
    println!("    std::uint8_t count;");
    println!("    std::array<std::uint32_t, unicode_decomposition_max_length> mapped;");
    println!("}};");
    println!();
    println!("struct unicode_composition_mapping");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t second;");
    println!("    std::uint32_t composed;");
    println!("}};");
    println!();
    println!("template <std::size_t N>");
    println!("constexpr auto make_first_mapping_page_index(const std::array<unicode_composition_mapping, N>& mappings) noexcept");
    println!("{{");
    println!("    std::array<std::uint16_t, unicode_mapping_page_count + 1> page_index{{}};");
    println!("    std::size_t mapping_index = 0;");
    println!("    for (std::size_t page = 0; page != unicode_mapping_page_count; ++page)");
    println!("    {{");
    println!("        page_index[page] = static_cast<std::uint16_t>(mapping_index);");
    println!("        const auto page_end = static_cast<std::uint32_t>((page + 1u) << unicode_mapping_page_shift);");
    println!("        while (mapping_index < N && mappings[mapping_index].first < page_end)");
    println!("        {{");
    println!("            ++mapping_index;");
    println!("        }}");
    println!("    }}");
    println!("    page_index[unicode_mapping_page_count] = static_cast<std::uint16_t>(mapping_index);");
    println!("    return page_index;");
    println!("}}");
    println!();
    println!("template <std::size_t N, std::size_t P>");
    println!("constexpr const unicode_composition_mapping* find_composition_mapping_paged(");
    println!("    std::uint32_t first,");
    println!("    std::uint32_t second,");
    println!("    const std::array<unicode_composition_mapping, N>& mappings,");
    println!("    const std::array<std::uint16_t, P>& page_index) noexcept");
    println!("{{");
    println!("    const auto page = static_cast<std::size_t>(first >> unicode_mapping_page_shift);");
    println!("    std::size_t left = page_index[page];");
    println!("    std::size_t right = page_index[page + 1u];");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const auto& mapping = mappings[mid];");
    println!("        if (first < mapping.first || (first == mapping.first && second < mapping.second))");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (first > mapping.first || (first == mapping.first && second > mapping.second))");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return &mapping;");
    println!("        }}");
    println!("    }}");
    println!("    return nullptr;");
    println!("}}");
    println!();
    println!("template <std::size_t N, std::size_t P>");
    println!("constexpr std::size_t find_composition_mapping_paged_index(");
    println!("    std::uint32_t first,");
    println!("    std::uint32_t second,");
    println!("    const std::array<unicode_composition_mapping, N>& mappings,");
    println!("    const std::array<std::uint16_t, P>& page_index) noexcept");
    println!("{{");
    println!("    const auto page = static_cast<std::size_t>(first >> unicode_mapping_page_shift);");
    println!("    std::size_t left = page_index[page];");
    println!("    std::size_t right = page_index[page + 1u];");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const auto& mapping = mappings[mid];");
    println!("        if (first < mapping.first || (first == mapping.first && second < mapping.second))");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (first > mapping.first || (first == mapping.first && second > mapping.second))");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return mid;");
    println!("        }}");
    println!("    }}");
    println!("    return N;");
    println!("}}");
    println!();
}

fn emit_decomposition_mappings(name: &str, mappings: &[DecompositionRecord], max_length: usize) {
    println!(
        "inline constexpr std::array<unicode_decomposition_mapping, {}> {}{{{{",
        mappings.len(),
        name
    );
    for mapping in mappings {
        print!(
            "    {{ 0x{:04X}u, {}, {}u, {{ ",
            mapping.source,
            if mapping.compatibility { "true" } else { "false" },
            mapping.mapped.len()
        );
        for index in 0..max_length {
            let scalar = mapping.mapped.get(index).copied().unwrap_or(0);
            if index != 0 {
                print!(", ");
            }
            print!("0x{scalar:04X}u");
        }
        println!(" }} }},");
    }
    println!("}}}};");
    println!();
}

fn emit_composition_mappings(name: &str, mappings: &[CompositionMappingRecord]) {
    println!(
        "inline constexpr std::array<unicode_composition_mapping, {}> {}{{{{",
        mappings.len(),
        name
    );
    for mapping in mappings {
        println!(
            "    {{ 0x{:04X}u, 0x{:04X}u, 0x{:04X}u }},",
            mapping.first,
            mapping.second,
            mapping.composed
        );
    }
    println!("}}}};");
    println!();
}

fn emit_source_mapping_lookup(fn_name: &str, mapping_type: &str, mappings_name: &str) {
    println!(
        "inline constexpr auto {mappings_name}_page_index = make_source_mapping_page_index({mappings_name});"
    );
    emit_source_mapping_lookup_without_page_index(fn_name, mapping_type, mappings_name);
}

fn emit_source_mapping_lookup_without_page_index(
    fn_name: &str,
    mapping_type: &str,
    mappings_name: &str,
) {
    println!();
    println!(
        "constexpr std::size_t {fn_name}_index(std::uint32_t scalar) noexcept"
    );
    println!("{{");
    println!(
        "    return find_source_mapping_paged_index(scalar, {mappings_name}, {mappings_name}_page_index);"
    );
    println!("}}");
    println!();
    println!("constexpr const {mapping_type}& {fn_name}_at(std::size_t index) noexcept");
    println!("{{");
    println!("    return {mappings_name}[index];");
    println!("}}");
    println!();
    println!(
        "constexpr const {mapping_type}* {fn_name}(std::uint32_t scalar) noexcept"
    );
    println!("{{");
    println!(
        "    return find_source_mapping_paged(scalar, {mappings_name}, {mappings_name}_page_index);"
    );
    println!("}}");
    println!();
}

fn emit_simple_case_delta_lookup(fn_name: &str, mappings_name: &str) {
    println!("inline constexpr auto {mappings_name}_page_index = make_source_mapping_page_index({mappings_name});");
    println!("inline constexpr auto {mappings_name}_delta_ranges = make_simple_case_delta_ranges({mappings_name});");
    println!("inline constexpr auto {mappings_name}_delta_ranges_page_slices =");
    println!("    make_overlapping_range_page_slices({mappings_name}_delta_ranges.ranges, {mappings_name}_delta_ranges.count);");
    println!();
    println!("constexpr const unicode_simple_case_delta_range* {fn_name}(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    const auto index = find_overlapping_range_index_paged(");
    println!("        scalar,");
    println!("        {mappings_name}_delta_ranges.ranges,");
    println!("        {mappings_name}_delta_ranges.count,");
    println!("        {mappings_name}_delta_ranges_page_slices);");
    println!("    if (index == {mappings_name}_delta_ranges.count)");
    println!("    {{");
    println!("        return nullptr;");
    println!("    }}");
    println!();
    println!("    const auto& range = {mappings_name}_delta_ranges.ranges[index];");
    println!("    const auto offset = scalar - range.first;");
    println!("    return (range.stride != 0 && offset % range.stride == 0) ? &range : nullptr;");
    println!("}}");
    println!();
}

fn emit_grapheme_property_support() {
    println!("struct unicode_grapheme_properties");
    println!("{{");
    println!("    grapheme_cluster_break_property break_property;");
    println!("    indic_conjunct_break_property indic_property;");
    println!("    bool extended_pictographic;");
    println!("}};");
    println!();
    println!("struct unicode_grapheme_property_range");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t last;");
    println!("    unicode_grapheme_properties properties;");
    println!("}};");
    println!();
    println!("template <std::size_t N>");
    println!(
        "constexpr std::size_t find_grapheme_property_range_index(std::uint32_t scalar, const std::array<unicode_grapheme_property_range, N>& ranges) noexcept"
    );
    println!("{{");
    println!("    std::size_t left = 0;");
    println!("    std::size_t right = N;");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const unicode_grapheme_property_range range = ranges[mid];");
    println!("        if (scalar < range.first)");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (scalar > range.last)");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return mid;");
    println!("        }}");
    println!("    }}");
    println!("    return N;");
    println!("}}");
    println!();
}

fn emit_grapheme_property_ranges(name: &str, ranges: &[GraphemePropertiesRangeRecord]) {
    println!(
        "inline constexpr std::array<unicode_grapheme_property_range, {}> {}{{{{",
        ranges.len(),
        name
    );
    for range in ranges {
        let break_property = if range.value.grapheme_break_index == 0 {
            "grapheme_cluster_break_property::other"
        } else {
            GRAPHEME_BREAK_VALUES[range.value.grapheme_break_index - 1].2
        };
        let indic_property = if range.value.indic_conjunct_break_index == 0 {
            "indic_conjunct_break_property::none"
        } else {
            INDIC_CONJUNCT_BREAK_VALUES[range.value.indic_conjunct_break_index - 1].2
        };
        println!(
            "    {{ 0x{:04X}u, 0x{:04X}u, {{ {}, {}, {} }} }},",
            range.first,
            range.last,
            break_property,
            indic_property,
            if range.value.extended_pictographic { "true" } else { "false" }
        );
    }
    println!("}}}};");
    println!();
}

fn emit_grapheme_property_lookup(fn_name: &str, ranges_name: &str) {
    println!(
        "constexpr unicode_grapheme_properties {fn_name}(std::uint32_t scalar) noexcept"
    );
    println!("{{");
    println!("    const auto index = find_grapheme_property_range_index(scalar, {ranges_name});");
    println!("    if (index != {ranges_name}.size())");
    println!("    {{");
    println!("        return {ranges_name}[index].properties;");
    println!("    }}");
    println!();
    println!("    return unicode_grapheme_properties{{");
    println!("        grapheme_cluster_break_property::other,");
    println!("        indic_conjunct_break_property::none,");
    println!("        false");
    println!("    }};");
    println!("}}");
    println!();
}

fn emit_composition_mapping_lookup(fn_name: &str, mappings_name: &str) {
    println!("inline constexpr auto {mappings_name}_page_index = make_first_mapping_page_index({mappings_name});");
    println!();
    println!("constexpr std::size_t {fn_name}_index(std::uint32_t first, std::uint32_t second) noexcept");
    println!("{{");
    println!("    return find_composition_mapping_paged_index(first, second, {mappings_name}, {mappings_name}_page_index);");
    println!("}}");
    println!();
    println!("constexpr const unicode_composition_mapping& {fn_name}_at(std::size_t index) noexcept");
    println!("{{");
    println!("    return {mappings_name}[index];");
    println!("}}");
    println!();
    println!("constexpr const unicode_composition_mapping* {fn_name}(std::uint32_t first, std::uint32_t second) noexcept");
    println!("{{");
    println!("    return find_composition_mapping_paged(first, second, {mappings_name}, {mappings_name}_page_index);");
    println!("}}");
    println!();
}

fn emit_bool_lookup(name: &str, ranges_name: &str) {
    println!("constexpr bool {name}(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, {ranges_name});");
    println!("}}");
    println!();
}

fn emit_enum(name: &str, variants: &[&str]) {
    println!("enum class {name}");
    println!("{{");
    for variant in variants {
        println!("    {variant},");
    }
    println!("}};");
    println!();
}

fn emit_public_enum(name: &str, underlying_type: &str, variants: &[String]) {
    println!("enum class {name} : {underlying_type}");
    println!("{{");
    for variant in variants {
        println!("    {variant},");
    }
    println!("}};");
    println!();
}

fn emit_property_value_range_support() {
    println!("template <typename Property>");
    println!("struct unicode_property_value_range");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t last;");
    println!("    Property value;");
    println!("}};");
    println!();
}

fn emit_property_value_ranges(
    name: &str,
    enum_name: &str,
    records: &[PropertyValueRangeRecord],
    variant_for_value: impl Fn(&str) -> String,
) {
    println!(
        "inline constexpr std::array<unicode_property_value_range<{enum_name}>, {}> {name}{{{{",
        records.len()
    );
    for record in records {
        println!(
            "    {{ 0x{:04X}u, 0x{:04X}u, {enum_name}::{} }},",
            record.first,
            record.last,
            variant_for_value(&record.value)
        );
    }
    println!("}}}};");
    println!();
}

fn emit_property_value_lookup(
    fn_name: &str,
    enum_name: &str,
    default_variant: &str,
    ranges_name: &str,
) {
    println!("inline constexpr auto {ranges_name}_page_slices =");
    println!("    make_overlapping_range_page_slices({ranges_name});");
    println!();
    println!("constexpr {enum_name} {fn_name}(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    const auto index = find_overlapping_range_index_paged(");
    println!("        scalar,");
    println!("        {ranges_name},");
    println!("        {ranges_name}.size(),");
    println!("        {ranges_name}_page_slices);");
    println!("    if (index == {ranges_name}.size())");
    println!("    {{");
    println!("        return {enum_name}::{default_variant};");
    println!("    }}");
    println!("    return {ranges_name}[index].value;");
    println!("}}");
    println!();
}

fn unicode_version_string(version: UnicodeVersion) -> String {
    format!("{}.{}.{}", version.major, version.minor, version.patch)
}

fn default_data_root() -> PathBuf {
    PathBuf::from("tools")
        .join("unicode_data")
        .join(unicode_version_string(DEFAULT_UNICODE_VERSION))
}

fn parse_unicode_version_string(value: &str) -> io::Result<UnicodeVersion> {
    let components: Vec<&str> = value.trim().split('.').collect();
    if components.len() != 3 {
        return Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            format!("invalid Unicode version `{value}`"),
        ));
    }

    let parse_component = |component: &str| {
        component.parse::<u8>().map_err(|err| {
            io::Error::new(
                io::ErrorKind::InvalidInput,
                format!("invalid Unicode version `{value}`: {err}"),
            )
        })
    };

    Ok(UnicodeVersion {
        major: parse_component(components[0])?,
        minor: parse_component(components[1])?,
        patch: parse_component(components[2])?,
    })
}

fn infer_unicode_version_from_data_root(data_root: &Path) -> Option<UnicodeVersion> {
    let directory = data_root.file_name()?.to_str()?;
    parse_unicode_version_string(directory).ok()
}

fn configured_generation_config() -> io::Result<(PathBuf, UnicodeVersion)> {
    let mut args = env::args_os();
    let _program = args.next();
    let data_root = match args.next() {
        Some(path) => PathBuf::from(path),
        None => default_data_root(),
    };
    let unicode_version = match args.next() {
        Some(version) => parse_unicode_version_string(&version.to_string_lossy())?,
        None => infer_unicode_version_from_data_root(&data_root).unwrap_or(DEFAULT_UNICODE_VERSION),
    };
    Ok((data_root, unicode_version))
}

fn parse_range_spec(spec: &str) -> Result<Range, String> {
    if let Some((first, last)) = spec.split_once("..") {
        let first = u32::from_str_radix(first.trim(), 16)
            .map_err(|err| format!("invalid range start `{first}`: {err}"))?;
        let last = u32::from_str_radix(last.trim(), 16)
            .map_err(|err| format!("invalid range end `{last}`: {err}"))?;
        Ok((first, last))
    } else {
        let scalar = u32::from_str_radix(spec.trim(), 16)
            .map_err(|err| format!("invalid scalar `{spec}`: {err}"))?;
        Ok((scalar, scalar))
    }
}

fn parse_property_file(path: &Path) -> io::Result<Vec<PropertyRecord>> {
    let content = fs::read_to_string(path)?;
    let mut records = Vec::new();

    for (line_index, line) in content.lines().enumerate() {
        let line = line.split('#').next().unwrap().trim();
        if line.is_empty() {
            continue;
        }

        let fields: Vec<String> = line
            .split(';')
            .map(str::trim)
            .filter(|field| !field.is_empty())
            .map(ToOwned::to_owned)
            .collect();

        if fields.len() < 2 {
            return Err(io::Error::new(
                io::ErrorKind::InvalidData,
                format!(
                    "{}:{}: expected at least one property field",
                    path.display(),
                    line_index + 1
                ),
            ));
        }

        let (first, last) = parse_range_spec(&fields[0]).map_err(|message| {
            io::Error::new(
                io::ErrorKind::InvalidData,
                format!("{}:{}: {message}", path.display(), line_index + 1),
            )
        })?;

        records.push(PropertyRecord {
            first,
            last,
            fields: fields[1..].to_vec(),
        });
    }

    Ok(records)
}

fn collect_named_ranges(records: &[PropertyRecord], names: &[&str]) -> BTreeMap<String, Vec<Range>> {
    let mut ranges = BTreeMap::<String, Vec<Range>>::new();

    for record in records {
        if let Some(name) = record.fields.first() {
            if names.contains(&name.as_str()) {
                ranges
                    .entry(name.clone())
                    .or_default()
                    .push((record.first, record.last));
            }
        }
    }

    ranges
}

fn merge_property_value_ranges(
    records: impl IntoIterator<Item = PropertyValueRangeRecord>,
) -> Vec<PropertyValueRangeRecord> {
    let mut sorted: Vec<PropertyValueRangeRecord> = records.into_iter().collect();
    sorted.sort_unstable_by_key(|record| (record.first, record.last));

    let mut merged = Vec::<PropertyValueRangeRecord>::new();

    for record in sorted {
        if let Some(previous) = merged.last_mut() {
            if previous.last + 1 == record.first && previous.value == record.value {
                previous.last = record.last;
                continue;
            }
        }

        merged.push(record);
    }

    merged
}

fn collect_unicode_data_property_ranges<F>(
    records: &[UnicodeDataRecord],
    selector: F,
) -> Vec<PropertyValueRangeRecord>
where
    F: Fn(&UnicodeDataRecord) -> &str,
{
    merge_property_value_ranges(records.iter().map(|record| PropertyValueRangeRecord {
        first: record.scalar,
        last: record.scalar,
        value: selector(record).to_owned(),
    }))
}

fn collect_property_value_ranges(path: &Path) -> io::Result<Vec<PropertyValueRangeRecord>> {
    let records = parse_property_file(path)?;
    Ok(merge_property_value_ranges(records.into_iter().map(|record| {
        let value = match record.fields.as_slice() {
            [value] => value.clone(),
            [_, value] => value.clone(),
            _ => panic!("unsupported property field layout in {}", path.display()),
        };

        PropertyValueRangeRecord {
            first: record.first,
            last: record.last,
            value,
        }
    })))
}

fn collect_enum_variants(
    records: &[PropertyValueRangeRecord],
    variant_for_value: impl Fn(&str) -> String,
    default_variant: Option<&str>,
) -> Vec<String> {
    let mut variants = Vec::<String>::new();
    if let Some(default_variant) = default_variant {
        variants.push(default_variant.to_owned());
    }

    for record in records {
        let variant = variant_for_value(&record.value);
        if !variants.iter().any(|existing| existing == &variant) {
            variants.push(variant);
        }
    }

    variants
}

fn collect_grapheme_break_ranges(path: &Path) -> io::Result<BTreeMap<String, Vec<Range>>> {
    let records = parse_property_file(path)?;
    let names: Vec<&str> = GRAPHEME_BREAK_VALUES
        .iter()
        .map(|(name, _, _)| *name)
        .collect();
    Ok(collect_named_ranges(&records, &names))
}

fn collect_emoji_property_ranges(path: &Path, property_name: &str) -> io::Result<Vec<Range>> {
    let records = parse_property_file(path)?;
    let mut ranges = Vec::new();

    for record in records {
        if record.fields.iter().any(|field| field == property_name) {
            ranges.push((record.first, record.last));
        }
    }

    Ok(ranges)
}

fn collect_extended_pictographic_ranges(path: &Path) -> io::Result<Vec<Range>> {
    collect_emoji_property_ranges(path, "Extended_Pictographic")
}

fn collect_indic_conjunct_break_ranges(path: &Path) -> io::Result<BTreeMap<String, Vec<Range>>> {
    let records = parse_property_file(path)?;
    let names: Vec<&str> = INDIC_CONJUNCT_BREAK_VALUES
        .iter()
        .map(|(name, _, _)| *name)
        .collect();
    let mut ranges = BTreeMap::<String, Vec<Range>>::new();

    for record in records {
        let value = match record.fields.as_slice() {
            [field] => field
                .strip_prefix("Indic_Conjunct_Break=")
                .or_else(|| field.strip_prefix("InCB="))
                .map(str::trim),
            [property, value] if property == "Indic_Conjunct_Break" || property == "InCB" => {
                Some(value.as_str())
            }
            _ => None,
        };

        if let Some(value) = value {
            if names.contains(&value) {
                ranges
                    .entry(value.to_owned())
                    .or_default()
                    .push((record.first, record.last));
            }
        }
    }

    Ok(ranges)
}

fn main() -> io::Result<()> {
    let (data_root, unicode_version) = configured_generation_config()?;
    let rust_unicode_version = char::UNICODE_VERSION;
    if rust_unicode_version
        != (
            unicode_version.major,
            unicode_version.minor,
            unicode_version.patch,
        )
    {
        eprintln!(
            "warning: Rust std::char uses Unicode {}.{}.{}, but generated tables are pinned to {}",
            rust_unicode_version.0,
            rust_unicode_version.1,
            rust_unicode_version.2,
            unicode_version_string(unicode_version),
        );
    }
    let grapheme_break_path = data_root
        .join("ucd")
        .join("auxiliary")
        .join("GraphemeBreakProperty.txt");
    let grapheme_break_ranges = collect_grapheme_break_ranges(&grapheme_break_path)?;
    let grapheme_break_property_ranges = collect_property_value_ranges(&grapheme_break_path)?;
    let emoji_data_path = data_root.join("ucd").join("emoji").join("emoji-data.txt");
    let emoji_ranges = collect_emoji_property_ranges(&emoji_data_path, "Emoji")?;
    let emoji_presentation_ranges =
        collect_emoji_property_ranges(&emoji_data_path, "Emoji_Presentation")?;
    let extended_pictographic_ranges = collect_extended_pictographic_ranges(&emoji_data_path)?;
    let indic_conjunct_break_ranges =
        collect_indic_conjunct_break_ranges(&data_root.join("ucd").join("DerivedCoreProperties.txt"))?;
    let grapheme_properties_ranges = collect_grapheme_properties_ranges(
        &grapheme_break_ranges,
        &extended_pictographic_ranges,
        &indic_conjunct_break_ranges,
    );
    let unicode_data = parse_unicode_data(&data_root.join("ucd").join("UnicodeData.txt"))?;
    let general_category_ranges =
        collect_unicode_data_property_ranges(&unicode_data, |record| &record.general_category);
    let canonical_combining_class_ranges =
        collect_canonical_combining_class_ranges(&unicode_data);
    let script_ranges = collect_property_value_ranges(&data_root.join("ucd").join("Scripts.txt"))?;
    let east_asian_width_ranges =
        collect_property_value_ranges(&data_root.join("ucd").join("EastAsianWidth.txt"))?;
    let line_break_ranges = collect_property_value_ranges(&data_root.join("ucd").join("LineBreak.txt"))?;
    let bidi_class_ranges = collect_property_value_ranges(
        &data_root
            .join("ucd")
            .join("extracted")
            .join("DerivedBidiClass.txt"),
    )?;
    let word_break_ranges = collect_property_value_ranges(
        &data_root
            .join("ucd")
            .join("auxiliary")
            .join("WordBreakProperty.txt"),
    )?;
    let sentence_break_ranges = collect_property_value_ranges(
        &data_root
            .join("ucd")
            .join("auxiliary")
            .join("SentenceBreakProperty.txt"),
    )?;
    let decomposition_mappings = collect_decomposition_records(&unicode_data);
    let composition_exclusions =
        collect_composition_exclusions(&data_root.join("ucd").join("CompositionExclusions.txt"))?;
    let composition_mappings =
        collect_composition_records(&unicode_data, &composition_exclusions);
    let case_fold_mappings = parse_case_folding(&data_root.join("ucd").join("CaseFolding.txt"))?;
    let lowercase_mappings =
        collect_case_mappings(|ch| ch.to_lowercase().map(|mapped| mapped as u32).collect());
    let uppercase_mappings =
        collect_case_mappings(|ch| ch.to_uppercase().map(|mapped| mapped as u32).collect());
    let (lowercase_simple_mappings, lowercase_special_mappings) =
        split_case_mappings(&lowercase_mappings);
    let (uppercase_simple_mappings, uppercase_special_mappings) =
        split_case_mappings(&uppercase_mappings);
    let (case_fold_simple_mappings, case_fold_special_mappings) =
        split_case_mappings(&case_fold_mappings);
    let unicode_case_mapping_max_length = case_mapping_max_length(&lowercase_special_mappings)
        .max(case_mapping_max_length(&uppercase_special_mappings))
        .max(case_mapping_max_length(&case_fold_special_mappings));
    let unicode_decomposition_max_length =
        decomposition_mapping_max_length(&decomposition_mappings);
    let general_category_variants = collect_enum_variants(
        &general_category_ranges,
        |value| general_category_enum_variant(value).to_owned(),
        Some("unassigned"),
    );
    let grapheme_break_variants = collect_enum_variants(
        &grapheme_break_property_ranges,
        |value| grapheme_break_enum_variant(value).to_owned(),
        Some("other"),
    );
    let script_variants =
        collect_enum_variants(&script_ranges, to_snake_case_identifier, Some("unknown"));
    let east_asian_width_variants = collect_enum_variants(
        &east_asian_width_ranges,
        |value| east_asian_width_enum_variant(value).to_owned(),
        Some("neutral"),
    );
    let line_break_variants = collect_enum_variants(
        &line_break_ranges,
        |value| line_break_enum_variant(value).to_owned(),
        Some("unknown"),
    );
    let bidi_class_variants = collect_enum_variants(
        &bidi_class_ranges,
        |value| bidi_class_enum_variant(value).to_owned(),
        Some("left_to_right"),
    );
    let word_break_variants =
        collect_enum_variants(&word_break_ranges, to_snake_case_identifier, Some("other"));
    let sentence_break_variants = collect_enum_variants(
        &sentence_break_ranges,
        to_snake_case_identifier,
        Some("other"),
    );
    println!("#ifndef UTF8_RANGES_UNICODE_TABLES_HPP");
    println!("#define UTF8_RANGES_UNICODE_TABLES_HPP");
    println!();
    println!("namespace unicode_ranges");
    println!("{{");
    emit_public_enum(
        "unicode_general_category",
        "std::uint8_t",
        &general_category_variants,
    );
    emit_public_enum(
        "unicode_grapheme_break_property",
        "std::uint8_t",
        &grapheme_break_variants,
    );
    emit_public_enum("unicode_script", "std::uint16_t", &script_variants);
    emit_public_enum(
        "unicode_east_asian_width",
        "std::uint8_t",
        &east_asian_width_variants,
    );
    emit_public_enum(
        "unicode_line_break_class",
        "std::uint8_t",
        &line_break_variants,
    );
    emit_public_enum("unicode_bidi_class", "std::uint8_t", &bidi_class_variants);
    emit_public_enum(
        "unicode_word_break_property",
        "std::uint8_t",
        &word_break_variants,
    );
    emit_public_enum(
        "unicode_sentence_break_property",
        "std::uint8_t",
        &sentence_break_variants,
    );
    println!("namespace details::unicode");
    println!("{{");
    println!("struct unicode_range");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t last;");
    println!("}};");
    println!();
    println!(
        "inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version{{ {}, {}, {} }};",
        unicode_version.major,
        unicode_version.minor,
        unicode_version.patch
    );
    println!();
    emit_case_mapping_support(unicode_case_mapping_max_length);
    emit_decomposition_support(unicode_decomposition_max_length);
    emit_canonical_combining_class_support();
    emit_property_value_range_support();
    println!("template <std::size_t N>");
    println!("constexpr bool in_ranges(std::uint32_t scalar, const std::array<unicode_range, N>& ranges) noexcept");
    println!("{{");
    println!("    std::size_t left = 0;");
    println!("    std::size_t right = N;");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const unicode_range range = ranges[mid];");
    println!("        if (scalar < range.first)");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (scalar > range.last)");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return true;");
    println!("        }}");
    println!("    }}");
    println!("    return false;");
    println!("}}");
    println!();

    println!("using grapheme_cluster_break_property = ::unicode_ranges::unicode_grapheme_break_property;");
    println!();

    emit_enum(
        "indic_conjunct_break_property",
        &["none", "consonant", "extend", "linker"],
    );
    emit_grapheme_property_support();

    emit_ranges("alphabetic_ranges", &collect_ranges(|ch| ch.is_alphabetic()));
    emit_ranges("lowercase_ranges", &collect_ranges(|ch| ch.is_lowercase()));
    emit_ranges("uppercase_ranges", &collect_ranges(|ch| ch.is_uppercase()));
    emit_ranges("whitespace_ranges", &collect_ranges(|ch| ch.is_whitespace()));
    emit_ranges("control_ranges", &collect_ranges(|ch| ch.is_control()));
    emit_ranges("numeric_ranges", &collect_ranges(|ch| ch.is_numeric()));
    emit_ranges("digit_ranges", &collect_ranges(|ch| ch.is_digit(10)));
    emit_ranges("emoji_ranges", &emoji_ranges);
    emit_ranges("emoji_presentation_ranges", &emoji_presentation_ranges);
    emit_property_value_ranges(
        "general_category_ranges",
        "unicode_general_category",
        &general_category_ranges,
        |value| general_category_enum_variant(value).to_owned(),
    );
    emit_property_value_ranges(
        "grapheme_break_property_ranges",
        "unicode_grapheme_break_property",
        &grapheme_break_property_ranges,
        |value| grapheme_break_enum_variant(value).to_owned(),
    );
    emit_property_value_ranges(
        "script_ranges",
        "unicode_script",
        &script_ranges,
        to_snake_case_identifier,
    );
    emit_property_value_ranges(
        "east_asian_width_ranges",
        "unicode_east_asian_width",
        &east_asian_width_ranges,
        |value| east_asian_width_enum_variant(value).to_owned(),
    );
    emit_property_value_ranges(
        "line_break_ranges",
        "unicode_line_break_class",
        &line_break_ranges,
        |value| line_break_enum_variant(value).to_owned(),
    );
    emit_property_value_ranges(
        "bidi_class_ranges",
        "unicode_bidi_class",
        &bidi_class_ranges,
        |value| bidi_class_enum_variant(value).to_owned(),
    );
    emit_property_value_ranges(
        "word_break_ranges",
        "unicode_word_break_property",
        &word_break_ranges,
        to_snake_case_identifier,
    );
    emit_property_value_ranges(
        "sentence_break_ranges",
        "unicode_sentence_break_property",
        &sentence_break_ranges,
        to_snake_case_identifier,
    );
    emit_simple_case_mappings("lowercase_simple_mappings", &lowercase_simple_mappings);
    emit_simple_case_mappings("case_fold_simple_mappings", &case_fold_simple_mappings);
    emit_special_case_mappings(
        "lowercase_special_mappings",
        &lowercase_special_mappings,
        unicode_case_mapping_max_length,
    );
    emit_simple_case_mappings("uppercase_simple_mappings", &uppercase_simple_mappings);
    emit_special_case_mappings(
        "uppercase_special_mappings",
        &uppercase_special_mappings,
        unicode_case_mapping_max_length,
    );
    emit_special_case_mappings(
        "case_fold_special_mappings",
        &case_fold_special_mappings,
        unicode_case_mapping_max_length,
    );
    emit_decomposition_mappings(
        "decomposition_mappings",
        &decomposition_mappings,
        unicode_decomposition_max_length,
    );
    emit_composition_mappings("composition_mappings", &composition_mappings);
    emit_canonical_combining_class_ranges(
        "canonical_combining_class_ranges",
        &canonical_combining_class_ranges,
    );
    emit_grapheme_property_ranges("grapheme_properties_ranges", &grapheme_properties_ranges);

    emit_bool_lookup("is_alphabetic", "alphabetic_ranges");
    emit_bool_lookup("is_lowercase", "lowercase_ranges");
    emit_bool_lookup("is_uppercase", "uppercase_ranges");
    emit_bool_lookup("is_whitespace", "whitespace_ranges");
    emit_bool_lookup("is_control", "control_ranges");
    emit_bool_lookup("is_numeric", "numeric_ranges");
    emit_bool_lookup("is_digit", "digit_ranges");
    emit_bool_lookup("is_emoji", "emoji_ranges");
    emit_bool_lookup("is_emoji_presentation", "emoji_presentation_ranges");
    emit_property_value_lookup(
        "general_category",
        "unicode_general_category",
        "unassigned",
        "general_category_ranges",
    );
    emit_property_value_lookup(
        "grapheme_break_property",
        "unicode_grapheme_break_property",
        "other",
        "grapheme_break_property_ranges",
    );
    emit_property_value_lookup(
        "script",
        "unicode_script",
        "unknown",
        "script_ranges",
    );
    emit_property_value_lookup(
        "east_asian_width",
        "unicode_east_asian_width",
        "neutral",
        "east_asian_width_ranges",
    );
    emit_property_value_lookup(
        "line_break_class",
        "unicode_line_break_class",
        "unknown",
        "line_break_ranges",
    );
    emit_property_value_lookup(
        "bidi_class",
        "unicode_bidi_class",
        "left_to_right",
        "bidi_class_ranges",
    );
    emit_property_value_lookup(
        "word_break_property",
        "unicode_word_break_property",
        "other",
        "word_break_ranges",
    );
    emit_property_value_lookup(
        "sentence_break_property",
        "unicode_sentence_break_property",
        "other",
        "sentence_break_ranges",
    );
    emit_simple_case_delta_lookup(
        "lowercase_simple_delta_range",
        "lowercase_simple_mappings",
    );
    emit_source_mapping_lookup_without_page_index(
        "lowercase_simple_mapping",
        "unicode_simple_case_mapping",
        "lowercase_simple_mappings",
    );
    emit_source_mapping_lookup(
        "lowercase_special_mapping",
        "unicode_special_case_mapping",
        "lowercase_special_mappings",
    );
    emit_simple_case_delta_lookup(
        "uppercase_simple_delta_range",
        "uppercase_simple_mappings",
    );
    emit_source_mapping_lookup_without_page_index(
        "uppercase_simple_mapping",
        "unicode_simple_case_mapping",
        "uppercase_simple_mappings",
    );
    emit_source_mapping_lookup(
        "uppercase_special_mapping",
        "unicode_special_case_mapping",
        "uppercase_special_mappings",
    );
    emit_simple_case_delta_lookup(
        "case_fold_simple_delta_range",
        "case_fold_simple_mappings",
    );
    emit_source_mapping_lookup_without_page_index(
        "case_fold_simple_mapping",
        "unicode_simple_case_mapping",
        "case_fold_simple_mappings",
    );
    emit_source_mapping_lookup(
        "case_fold_special_mapping",
        "unicode_special_case_mapping",
        "case_fold_special_mappings",
    );
    emit_source_mapping_lookup(
        "decomposition_mapping",
        "unicode_decomposition_mapping",
        "decomposition_mappings",
    );
    emit_composition_mapping_lookup(
        "composition_mapping",
        "composition_mappings",
    );
    emit_canonical_combining_class_lookup(
        "canonical_combining_class",
        "canonical_combining_class_ranges",
    );
    emit_grapheme_property_lookup("grapheme_properties", "grapheme_properties_ranges");
    println!("constexpr grapheme_cluster_break_property grapheme_cluster_break(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return grapheme_properties(scalar).break_property;");
    println!("}}");
    println!();
    println!("constexpr bool is_extended_pictographic(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return grapheme_properties(scalar).extended_pictographic;");
    println!("}}");
    println!();
    println!("constexpr indic_conjunct_break_property indic_conjunct_break(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return grapheme_properties(scalar).indic_property;");
    println!("}}");
    println!();

    println!("}}");
    println!("}}");
    println!();
    println!("#endif // UTF8_RANGES_UNICODE_TABLES_HPP");

    Ok(())
}
