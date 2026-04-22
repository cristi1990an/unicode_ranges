#ifndef UTF8_RANGES_UNICODE_TABLES_RUNTIME_SUPPORT_HPP
#define UTF8_RANGES_UNICODE_TABLES_RUNTIME_SUPPORT_HPP

#include "../unicode_tables_constexpr.hpp"

namespace unicode_ranges::details::unicode_runtime_support
{
	inline constexpr std::size_t unicode_mapping_page_shift = 8;
	inline constexpr std::size_t unicode_mapping_page_count = (0x10FFFFu >> unicode_mapping_page_shift) + 1u;

	struct unicode_range_page_slice
	{
		std::uint16_t begin;
		std::uint16_t end;
	};

	template <typename Mapping, std::size_t N>
	constexpr auto make_source_mapping_page_index(const std::array<Mapping, N>& mappings) noexcept
	{
		std::array<std::uint16_t, unicode_mapping_page_count + 1> page_index{};
		std::size_t mapping_index = 0;
		for (std::size_t page = 0; page != unicode_mapping_page_count; ++page)
		{
			page_index[page] = static_cast<std::uint16_t>(mapping_index);
			const auto page_end = static_cast<std::uint32_t>((page + 1u) << unicode_mapping_page_shift);
			while (mapping_index < N && mappings[mapping_index].source < page_end)
			{
				++mapping_index;
			}
		}

		page_index[unicode_mapping_page_count] = static_cast<std::uint16_t>(mapping_index);
		return page_index;
	}

	template <typename Mapping, std::size_t N, std::size_t P>
	constexpr const Mapping* find_source_mapping_paged(
		std::uint32_t scalar,
		const std::array<Mapping, N>& mappings,
		const std::array<std::uint16_t, P>& page_index) noexcept
	{
		const auto page = static_cast<std::size_t>(scalar >> unicode_mapping_page_shift);
		std::size_t left = page_index[page];
		std::size_t right = page_index[page + 1u];
		while (left < right)
		{
			const std::size_t mid = left + (right - left) / 2;
			const Mapping& mapping = mappings[mid];
			if (scalar < mapping.source)
			{
				right = mid;
			}
			else if (scalar > mapping.source)
			{
				left = mid + 1;
			}
			else
			{
				return &mapping;
			}
		}

		return nullptr;
	}

	template <typename Range, std::size_t N>
	constexpr auto make_overlapping_range_page_slices(
		const std::array<Range, N>& ranges,
		std::size_t count = N) noexcept
	{
		std::array<unicode_range_page_slice, unicode_mapping_page_count> page_slices{};
		std::size_t begin = 0;
		std::size_t end = 0;
		for (std::size_t page = 0; page != unicode_mapping_page_count; ++page)
		{
			const auto page_start = static_cast<std::uint32_t>(page << unicode_mapping_page_shift);
			const auto page_end = static_cast<std::uint32_t>((page + 1u) << unicode_mapping_page_shift);
			while (begin < count && ranges[begin].last < page_start)
			{
				++begin;
			}
			if (end < begin)
			{
				end = begin;
			}
			while (end < count && ranges[end].first < page_end)
			{
				++end;
			}

			page_slices[page] = unicode_range_page_slice{
				static_cast<std::uint16_t>(begin),
				static_cast<std::uint16_t>(end)
			};
		}

		return page_slices;
	}

	template <typename Range, std::size_t N, std::size_t P>
	constexpr std::size_t find_overlapping_range_index_paged(
		std::uint32_t scalar,
		const std::array<Range, N>& ranges,
		std::size_t count,
		const std::array<unicode_range_page_slice, P>& page_slices) noexcept
	{
		const auto page = static_cast<std::size_t>(scalar >> unicode_mapping_page_shift);
		std::size_t left = page_slices[page].begin;
		std::size_t right = page_slices[page].end;
		while (left < right)
		{
			const std::size_t mid = left + (right - left) / 2;
			const Range& range = ranges[mid];
			if (scalar < range.first)
			{
				right = mid;
			}
			else if (scalar > range.last)
			{
				left = mid + 1;
			}
			else
			{
				return mid;
			}
		}

		return count;
	}

	template <std::size_t N>
	constexpr auto make_first_mapping_page_index(const std::array<unicode::unicode_composition_mapping, N>& mappings) noexcept
	{
		std::array<std::uint16_t, unicode_mapping_page_count + 1> page_index{};
		std::size_t mapping_index = 0;
		for (std::size_t page = 0; page != unicode_mapping_page_count; ++page)
		{
			page_index[page] = static_cast<std::uint16_t>(mapping_index);
			const auto page_end = static_cast<std::uint32_t>((page + 1u) << unicode_mapping_page_shift);
			while (mapping_index < N && mappings[mapping_index].first < page_end)
			{
				++mapping_index;
			}
		}

		page_index[unicode_mapping_page_count] = static_cast<std::uint16_t>(mapping_index);
		return page_index;
	}

	template <std::size_t N, std::size_t P>
	constexpr const unicode::unicode_composition_mapping* find_composition_mapping_paged(
		std::uint32_t first,
		std::uint32_t second,
		const std::array<unicode::unicode_composition_mapping, N>& mappings,
		const std::array<std::uint16_t, P>& page_index) noexcept
	{
		const auto page = static_cast<std::size_t>(first >> unicode_mapping_page_shift);
		std::size_t left = page_index[page];
		std::size_t right = page_index[page + 1u];
		while (left < right)
		{
			const std::size_t mid = left + (right - left) / 2;
			const auto& mapping = mappings[mid];
			if (first < mapping.first || (first == mapping.first && second < mapping.second))
			{
				right = mid;
			}
			else if (first > mapping.first || (first == mapping.first && second > mapping.second))
			{
				left = mid + 1;
			}
			else
			{
				return &mapping;
			}
		}

		return nullptr;
	}

	template <std::size_t SimpleN, std::size_t SpecialN>
	constexpr auto make_bmp_case_mapping_table(
		const std::array<unicode::unicode_simple_case_mapping, SimpleN>& simple_mappings,
		const std::array<unicode::unicode_special_case_mapping, SpecialN>& special_mappings) noexcept
	{
		std::array<unicode::unicode_bmp_case_mapping, 0x10000> table{};
		for (std::size_t scalar = 0; scalar != table.size(); ++scalar)
		{
			table[scalar] = unicode::unicode_bmp_case_mapping{
				static_cast<char16_t>(scalar),
				true
			};
		}

		for (const auto& mapping : simple_mappings)
		{
			if (mapping.source > 0xFFFFu)
			{
				continue;
			}

			table[mapping.source] = unicode::unicode_bmp_case_mapping{
				static_cast<char16_t>(mapping.mapped <= 0xFFFFu ? mapping.mapped : mapping.source),
				mapping.mapped <= 0xFFFFu
			};
		}

		for (const auto& mapping : special_mappings)
		{
			if (mapping.source > 0xFFFFu)
			{
				continue;
			}

			table[mapping.source] = unicode::unicode_bmp_case_mapping{
				static_cast<char16_t>(mapping.source),
				false
			};
		}

		return table;
	}
}

#endif
