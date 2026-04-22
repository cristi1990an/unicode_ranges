#include "unicode_ranges.hpp"
#include "unicode_ranges/internal/unicode_tables_runtime_support.hpp"

namespace unicode_ranges::details::unicode
{
	namespace
	{
		inline constexpr auto general_category_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(general_category_ranges);

		inline constexpr auto grapheme_break_property_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(grapheme_break_property_ranges);

		inline constexpr auto script_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(script_ranges);

		inline constexpr auto east_asian_width_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(east_asian_width_ranges);

		inline constexpr auto line_break_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(line_break_ranges);

		inline constexpr auto bidi_class_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(bidi_class_ranges);

		inline constexpr auto word_break_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(word_break_ranges);

		inline constexpr auto sentence_break_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(sentence_break_ranges);

		inline constexpr auto lowercase_simple_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(lowercase_simple_mappings);
		inline constexpr auto lowercase_simple_mappings_delta_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(
				lowercase_simple_mappings_delta_ranges.ranges,
				lowercase_simple_mappings_delta_ranges.count);
		inline constexpr auto lowercase_special_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(lowercase_special_mappings);

		inline constexpr auto uppercase_simple_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(uppercase_simple_mappings);
		inline constexpr auto uppercase_simple_mappings_delta_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(
				uppercase_simple_mappings_delta_ranges.ranges,
				uppercase_simple_mappings_delta_ranges.count);
		inline constexpr auto uppercase_special_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(uppercase_special_mappings);

		inline constexpr auto case_fold_simple_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(case_fold_simple_mappings);
		inline constexpr auto case_fold_simple_mappings_delta_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(
				case_fold_simple_mappings_delta_ranges.ranges,
				case_fold_simple_mappings_delta_ranges.count);
		inline constexpr auto case_fold_special_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(case_fold_special_mappings);

		inline constexpr auto decomposition_mappings_page_index =
			unicode_runtime_support::make_source_mapping_page_index(decomposition_mappings);

		inline constexpr auto composition_mappings_page_index =
			unicode_runtime_support::make_first_mapping_page_index(composition_mappings);

		inline constexpr auto canonical_combining_class_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(canonical_combining_class_ranges);

		inline constexpr auto nfc_quick_check_non_yes_ranges_page_slices =
			unicode_runtime_support::make_overlapping_range_page_slices(nfc_quick_check_non_yes_ranges);

		inline constexpr auto lowercase_bmp_case_mapping_table =
			unicode_runtime_support::make_bmp_case_mapping_table(lowercase_simple_mappings, lowercase_special_mappings);

		inline constexpr auto uppercase_bmp_case_mapping_table =
			unicode_runtime_support::make_bmp_case_mapping_table(uppercase_simple_mappings, uppercase_special_mappings);

		inline constexpr auto case_fold_bmp_case_mapping_table =
			unicode_runtime_support::make_bmp_case_mapping_table(case_fold_simple_mappings, case_fold_special_mappings);
	}

	[[nodiscard]]
	unicode_general_category general_category_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			general_category_ranges,
			general_category_ranges.size(),
			general_category_ranges_page_slices);
		return index == general_category_ranges.size()
			? unicode_general_category::unassigned
			: general_category_ranges[index].value;
	}

	[[nodiscard]]
	unicode_grapheme_break_property grapheme_break_property_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			grapheme_break_property_ranges,
			grapheme_break_property_ranges.size(),
			grapheme_break_property_ranges_page_slices);
		return index == grapheme_break_property_ranges.size()
			? unicode_grapheme_break_property::other
			: grapheme_break_property_ranges[index].value;
	}

	[[nodiscard]]
	unicode_script script_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			script_ranges,
			script_ranges.size(),
			script_ranges_page_slices);
		return index == script_ranges.size()
			? unicode_script::unknown
			: script_ranges[index].value;
	}

	[[nodiscard]]
	unicode_east_asian_width east_asian_width_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			east_asian_width_ranges,
			east_asian_width_ranges.size(),
			east_asian_width_ranges_page_slices);
		return index == east_asian_width_ranges.size()
			? unicode_east_asian_width::neutral
			: east_asian_width_ranges[index].value;
	}

	[[nodiscard]]
	unicode_line_break_class line_break_class_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			line_break_ranges,
			line_break_ranges.size(),
			line_break_ranges_page_slices);
		return index == line_break_ranges.size()
			? unicode_line_break_class::unknown
			: line_break_ranges[index].value;
	}

	[[nodiscard]]
	unicode_bidi_class bidi_class_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			bidi_class_ranges,
			bidi_class_ranges.size(),
			bidi_class_ranges_page_slices);
		return index == bidi_class_ranges.size()
			? unicode_bidi_class::left_to_right
			: bidi_class_ranges[index].value;
	}

	[[nodiscard]]
	unicode_word_break_property word_break_property_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			word_break_ranges,
			word_break_ranges.size(),
			word_break_ranges_page_slices);
		return index == word_break_ranges.size()
			? unicode_word_break_property::other
			: word_break_ranges[index].value;
	}

	[[nodiscard]]
	unicode_sentence_break_property sentence_break_property_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			sentence_break_ranges,
			sentence_break_ranges.size(),
			sentence_break_ranges_page_slices);
		return index == sentence_break_ranges.size()
			? unicode_sentence_break_property::other
			: sentence_break_ranges[index].value;
	}

	[[nodiscard]]
	const unicode_simple_case_mapping* lowercase_simple_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			lowercase_simple_mappings,
			lowercase_simple_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_special_case_mapping* lowercase_special_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			lowercase_special_mappings,
			lowercase_special_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_simple_case_mapping* uppercase_simple_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			uppercase_simple_mappings,
			uppercase_simple_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_special_case_mapping* uppercase_special_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			uppercase_special_mappings,
			uppercase_special_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_simple_case_mapping* case_fold_simple_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			case_fold_simple_mappings,
			case_fold_simple_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_special_case_mapping* case_fold_special_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			case_fold_special_mappings,
			case_fold_special_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_simple_case_delta_range* lowercase_simple_delta_range_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			lowercase_simple_mappings_delta_ranges.ranges,
			lowercase_simple_mappings_delta_ranges.count,
			lowercase_simple_mappings_delta_ranges_page_slices);
		if (index == lowercase_simple_mappings_delta_ranges.count)
		{
			return nullptr;
		}

		const auto& range = lowercase_simple_mappings_delta_ranges.ranges[index];
		const auto offset = scalar - range.first;
		return (range.stride != 0 && offset % range.stride == 0) ? &range : nullptr;
	}

	[[nodiscard]]
	const unicode_simple_case_delta_range* uppercase_simple_delta_range_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			uppercase_simple_mappings_delta_ranges.ranges,
			uppercase_simple_mappings_delta_ranges.count,
			uppercase_simple_mappings_delta_ranges_page_slices);
		if (index == uppercase_simple_mappings_delta_ranges.count)
		{
			return nullptr;
		}

		const auto& range = uppercase_simple_mappings_delta_ranges.ranges[index];
		const auto offset = scalar - range.first;
		return (range.stride != 0 && offset % range.stride == 0) ? &range : nullptr;
	}

	[[nodiscard]]
	const unicode_simple_case_delta_range* case_fold_simple_delta_range_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			case_fold_simple_mappings_delta_ranges.ranges,
			case_fold_simple_mappings_delta_ranges.count,
			case_fold_simple_mappings_delta_ranges_page_slices);
		if (index == case_fold_simple_mappings_delta_ranges.count)
		{
			return nullptr;
		}

		const auto& range = case_fold_simple_mappings_delta_ranges.ranges[index];
		const auto offset = scalar - range.first;
		return (range.stride != 0 && offset % range.stride == 0) ? &range : nullptr;
	}

	[[nodiscard]]
	unicode_bmp_case_mapping lowercase_bmp_case_mapping_runtime(std::uint32_t scalar) noexcept
	{
		if (scalar > 0xFFFFu)
		{
			return unicode_bmp_case_mapping{ 0, false };
		}

		return lowercase_bmp_case_mapping_table[scalar];
	}

	[[nodiscard]]
	unicode_bmp_case_mapping uppercase_bmp_case_mapping_runtime(std::uint32_t scalar) noexcept
	{
		if (scalar > 0xFFFFu)
		{
			return unicode_bmp_case_mapping{ 0, false };
		}

		return uppercase_bmp_case_mapping_table[scalar];
	}

	[[nodiscard]]
	unicode_bmp_case_mapping case_fold_bmp_case_mapping_runtime(std::uint32_t scalar) noexcept
	{
		if (scalar > 0xFFFFu)
		{
			return unicode_bmp_case_mapping{ 0, false };
		}

		return case_fold_bmp_case_mapping_table[scalar];
	}

	[[nodiscard]]
	const unicode_decomposition_mapping* decomposition_mapping_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_source_mapping_paged(
			scalar,
			decomposition_mappings,
			decomposition_mappings_page_index);
	}

	[[nodiscard]]
	const unicode_composition_mapping* composition_mapping_runtime(std::uint32_t first, std::uint32_t second) noexcept
	{
		return unicode_runtime_support::find_composition_mapping_paged(
			first,
			second,
			composition_mappings,
			composition_mappings_page_index);
	}

	[[nodiscard]]
	std::uint8_t canonical_combining_class_runtime(std::uint32_t scalar) noexcept
	{
		const auto index = unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			canonical_combining_class_ranges,
			canonical_combining_class_ranges.size(),
			canonical_combining_class_ranges_page_slices);
		return index == canonical_combining_class_ranges.size()
			? 0u
			: canonical_combining_class_ranges[index].canonical_combining_class;
	}

	[[nodiscard]]
	bool is_nfc_quick_check_non_yes_runtime(std::uint32_t scalar) noexcept
	{
		return unicode_runtime_support::find_overlapping_range_index_paged(
			scalar,
			nfc_quick_check_non_yes_ranges,
			nfc_quick_check_non_yes_ranges.size(),
			nfc_quick_check_non_yes_ranges_page_slices) != nfc_quick_check_non_yes_ranges.size();
	}
}
