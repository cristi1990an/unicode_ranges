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

    emit_ranges("alphabetic_ranges", &collect_ranges(|ch| ch.is_alphabetic()));
    emit_ranges("lowercase_ranges", &collect_ranges(|ch| ch.is_lowercase()));
    emit_ranges("uppercase_ranges", &collect_ranges(|ch| ch.is_uppercase()));
    emit_ranges("whitespace_ranges", &collect_ranges(|ch| ch.is_whitespace()));
    emit_ranges("control_ranges", &collect_ranges(|ch| ch.is_control()));
    emit_ranges("numeric_ranges", &collect_ranges(|ch| ch.is_numeric()));
    emit_ranges("digit_ranges", &collect_ranges(|ch| ch.is_digit(10)));

    for &(name, ranges_name, _) in GRAPHEME_BREAK_VALUES {
        let ranges = grapheme_break_ranges
            .get(name)
            .map_or(&[][..], Vec::as_slice);
        emit_ranges(ranges_name, ranges);
    }

    emit_ranges(
        "extended_pictographic_ranges",
        &extended_pictographic_ranges,
    );

    for &(name, ranges_name, _) in INDIC_CONJUNCT_BREAK_VALUES {
        let ranges = indic_conjunct_break_ranges
            .get(name)
            .map_or(&[][..], Vec::as_slice);
        emit_ranges(ranges_name, ranges);
    }

    emit_bool_lookup("is_alphabetic", "alphabetic_ranges");
    emit_bool_lookup("is_lowercase", "lowercase_ranges");
    emit_bool_lookup("is_uppercase", "uppercase_ranges");
    emit_bool_lookup("is_whitespace", "whitespace_ranges");
    emit_bool_lookup("is_control", "control_ranges");
    emit_bool_lookup("is_numeric", "numeric_ranges");
    emit_bool_lookup("is_digit", "digit_ranges");
    emit_property_lookup(
        "grapheme_cluster_break",
        "grapheme_cluster_break_property",
        "other",
        GRAPHEME_BREAK_VALUES,
    );
    emit_bool_lookup("is_extended_pictographic", "extended_pictographic_ranges");
    emit_property_lookup(
        "indic_conjunct_break",
        "indic_conjunct_break_property",
        "none",
        INDIC_CONJUNCT_BREAK_VALUES,
    );

    println!("}}");
    println!("}}");
    println!();
    println!("#endif // UTF8_RANGES_UNICODE_TABLES_HPP");

    Ok(())
}
