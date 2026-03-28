use std::char;
use std::collections::BTreeMap;
use std::env;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

type Range = (u32, u32);

const UNICODE_VERSION: (u8, u8, u8) = (17, 0, 0);

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

fn emit_source_mapping_lookup(fn_name: &str, mapping_type: &str, mappings_name: &str) {
    println!(
        "inline constexpr auto {mappings_name}_page_index = make_source_mapping_page_index({mappings_name});"
    );
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

fn emit_property_lookup(
    fn_name: &str,
    enum_name: &str,
    default_variant: &str,
    properties: &[(&str, &str, &str)],
) {
    println!("constexpr {enum_name} {fn_name}(std::uint32_t scalar) noexcept");
    println!("{{");
    for &(_, ranges_name, enum_variant) in properties {
        println!("    if (in_ranges(scalar, {ranges_name}))");
        println!("    {{");
        println!("        return {enum_variant};");
        println!("    }}");
    }
    println!("    return {enum_name}::{default_variant};");
    println!("}}");
    println!();
}

fn unicode_version_string() -> String {
    format!(
        "{}.{}.{}",
        UNICODE_VERSION.0, UNICODE_VERSION.1, UNICODE_VERSION.2
    )
}

fn default_data_root() -> PathBuf {
    PathBuf::from("tools")
        .join("unicode_data")
        .join(unicode_version_string())
}

fn configured_data_root() -> PathBuf {
    let mut args = env::args_os();
    let _program = args.next();
    match args.next() {
        Some(path) => PathBuf::from(path),
        None => default_data_root(),
    }
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

fn collect_grapheme_break_ranges(path: &Path) -> io::Result<BTreeMap<String, Vec<Range>>> {
    let records = parse_property_file(path)?;
    let names: Vec<&str> = GRAPHEME_BREAK_VALUES
        .iter()
        .map(|(name, _, _)| *name)
        .collect();
    Ok(collect_named_ranges(&records, &names))
}

fn collect_extended_pictographic_ranges(path: &Path) -> io::Result<Vec<Range>> {
    let records = parse_property_file(path)?;
    let mut ranges = Vec::new();

    for record in records {
        if record.fields.iter().any(|field| field == "Extended_Pictographic") {
            ranges.push((record.first, record.last));
        }
    }

    Ok(ranges)
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
    let rust_unicode_version = char::UNICODE_VERSION;
    if rust_unicode_version != UNICODE_VERSION {
        eprintln!(
            "warning: Rust std::char uses Unicode {}.{}.{}, but generated tables are pinned to {}",
            rust_unicode_version.0,
            rust_unicode_version.1,
            rust_unicode_version.2,
            unicode_version_string(),
        );
    }

    let data_root = configured_data_root();
    let grapheme_break_ranges = collect_grapheme_break_ranges(
        &data_root.join("ucd").join("auxiliary").join("GraphemeBreakProperty.txt"),
    )?;
    let extended_pictographic_ranges = collect_extended_pictographic_ranges(
        &data_root.join("ucd").join("emoji").join("emoji-data.txt"),
    )?;
    let indic_conjunct_break_ranges =
        collect_indic_conjunct_break_ranges(&data_root.join("ucd").join("DerivedCoreProperties.txt"))?;
    let grapheme_properties_ranges = collect_grapheme_properties_ranges(
        &grapheme_break_ranges,
        &extended_pictographic_ranges,
        &indic_conjunct_break_ranges,
    );
    let lowercase_mappings =
        collect_case_mappings(|ch| ch.to_lowercase().map(|mapped| mapped as u32).collect());
    let uppercase_mappings =
        collect_case_mappings(|ch| ch.to_uppercase().map(|mapped| mapped as u32).collect());
    let (lowercase_simple_mappings, lowercase_special_mappings) =
        split_case_mappings(&lowercase_mappings);
    let (uppercase_simple_mappings, uppercase_special_mappings) =
        split_case_mappings(&uppercase_mappings);
    let unicode_case_mapping_max_length = case_mapping_max_length(&lowercase_special_mappings)
        .max(case_mapping_max_length(&uppercase_special_mappings));
    println!("#ifndef UTF8_RANGES_UNICODE_TABLES_HPP");
    println!("#define UTF8_RANGES_UNICODE_TABLES_HPP");
    println!();
    println!("namespace unicode_ranges");
    println!("{{");
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
        UNICODE_VERSION.0, UNICODE_VERSION.1, UNICODE_VERSION.2
    );
    println!();
    emit_case_mapping_support(unicode_case_mapping_max_length);
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

    emit_enum(
        "grapheme_cluster_break_property",
        &[
            "other",
            "cr",
            "lf",
            "control",
            "extend",
            "zwj",
            "regional_indicator",
            "prepend",
            "spacing_mark",
            "l",
            "v",
            "t",
            "lv",
            "lvt",
        ],
    );

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
    emit_simple_case_mappings("lowercase_simple_mappings", &lowercase_simple_mappings);
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
    emit_grapheme_property_ranges("grapheme_properties_ranges", &grapheme_properties_ranges);

    emit_bool_lookup("is_alphabetic", "alphabetic_ranges");
    emit_bool_lookup("is_lowercase", "lowercase_ranges");
    emit_bool_lookup("is_uppercase", "uppercase_ranges");
    emit_bool_lookup("is_whitespace", "whitespace_ranges");
    emit_bool_lookup("is_control", "control_ranges");
    emit_bool_lookup("is_numeric", "numeric_ranges");
    emit_bool_lookup("is_digit", "digit_ranges");
    emit_source_mapping_lookup(
        "lowercase_simple_mapping",
        "unicode_simple_case_mapping",
        "lowercase_simple_mappings",
    );
    emit_source_mapping_lookup(
        "lowercase_special_mapping",
        "unicode_special_case_mapping",
        "lowercase_special_mappings",
    );
    emit_source_mapping_lookup(
        "uppercase_simple_mapping",
        "unicode_simple_case_mapping",
        "uppercase_simple_mappings",
    );
    emit_source_mapping_lookup(
        "uppercase_special_mapping",
        "unicode_special_case_mapping",
        "uppercase_special_mappings",
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
