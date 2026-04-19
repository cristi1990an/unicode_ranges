#ifndef UTF8_RANGES_CORE_HPP
#define UTF8_RANGES_CORE_HPP

#include <ranges>
#include <algorithm>

#include <array>
#include <cassert>
#include <charconv>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

#include <uchar.h>

#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L
#define UTF8_RANGES_HAS_JTHREAD 1
#else
#define UTF8_RANGES_HAS_JTHREAD 0
#endif

#if defined(__has_include)
#if __has_include(<emmintrin.h>) && (defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2))
#define UTF8_RANGES_HAS_SSE2_INTRINSICS 1
#endif
#endif

#ifndef UTF8_RANGES_HAS_SSE2_INTRINSICS
#define UTF8_RANGES_HAS_SSE2_INTRINSICS 0
#endif

#if UTF8_RANGES_HAS_SSE2_INTRINSICS
#include <emmintrin.h>
#endif

#if defined(UTF8_RANGES_ENABLE_ICU) && UTF8_RANGES_ENABLE_ICU
#if defined(__has_include)
#if !__has_include(<unicode/ucasemap.h>)
#error "UTF8_RANGES_ENABLE_ICU requires ICU headers such as <unicode/ucasemap.h>"
#endif
#endif
#include <unicode/ucasemap.h>
#include <unicode/ustring.h>
#include <unicode/uloc.h>
#define UTF8_RANGES_HAS_ICU 1
#else
#define UTF8_RANGES_HAS_ICU 0
#endif

#if defined(NDEBUG)
#define UTF8_RANGES_DEBUG_ASSERT(expr) ((void)0)
#else
#define UTF8_RANGES_DEBUG_ASSERT(expr) \
	do \
	{ \
		if (!std::is_constant_evaluated()) \
		{ \
			assert(expr); \
		} \
	} while (false)
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define UTF8_RANGES_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(no_unique_address)
#define UTF8_RANGES_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define UTF8_RANGES_NO_UNIQUE_ADDRESS
#endif
#else
#define UTF8_RANGES_NO_UNIQUE_ADDRESS
#endif

#include "unicode_tables.hpp"
#include "internal/simdutf_haswell_utf8_validate.hpp"
#include "internal/simdutf_haswell_utf8_to_utf16.hpp"
#include "internal/simdutf_haswell_utf8_to_utf32.hpp"

namespace unicode_ranges
{

struct utf8_char;
class utf8_string_view;
class utf16_string_view;
struct utf16_char;
class utf32_string_view;
struct utf32_char;

template <typename Allocator = std::allocator<char8_t>>
class basic_utf8_string;

using utf8_string = basic_utf8_string<>;

template <typename Allocator = std::allocator<char16_t>>
class basic_utf16_string;

using utf16_string = basic_utf16_string<>;

template <typename Allocator = std::allocator<char32_t>>
class basic_utf32_string;

using utf32_string = basic_utf32_string<>;

#if UTF8_RANGES_HAS_ICU
struct locale_id
{
	const char* name = nullptr;
};

[[nodiscard]] bool is_available_locale(locale_id locale) noexcept;
#endif

namespace pmr
{

using utf8_string = basic_utf8_string<std::pmr::polymorphic_allocator<char8_t>>;
using utf16_string = basic_utf16_string<std::pmr::polymorphic_allocator<char16_t>>;
using utf32_string = basic_utf32_string<std::pmr::polymorphic_allocator<char32_t>>;

}

template <typename T>
concept unicode_character =
	std::same_as<std::remove_cvref_t<T>, utf8_char>
	|| std::same_as<std::remove_cvref_t<T>, utf16_char>
	|| std::same_as<std::remove_cvref_t<T>, utf32_char>;

enum class utf8_error_code
{
	invalid_lead_byte,
	truncated_sequence,
	invalid_sequence
};

struct utf8_error
{
	utf8_error_code code{};
	std::size_t first_invalid_byte_index = 0;
};

enum class utf16_error_code
{
	truncated_surrogate_pair,
	invalid_sequence
};

struct utf16_error
{
	utf16_error_code code{};
	std::size_t first_invalid_code_unit_index = 0;
};

enum class utf32_error_code
{
	invalid_scalar
};

struct utf32_error
{
	utf32_error_code code{};
	std::size_t first_invalid_code_point_index = 0;
};

#include "internal/simdutf_utf8_scalar_validate.hpp"

enum class unicode_scalar_error_code
{
	invalid_scalar
};

struct unicode_scalar_error
{
	unicode_scalar_error_code code{};
	std::size_t first_invalid_element_index = 0;
};

enum class normalization_form
{
	nfc,
	nfd,
	nfkc,
	nfkd
};

namespace views
{
	class utf8_view;

	class reversed_utf8_view;

	template <typename CharT>
	class grapheme_cluster_view;

	class utf16_view;

	class reversed_utf16_view;

	class utf32_view;

	template <typename CharT>
	class lossy_utf8_view;

	template <typename CharT>
	class lossy_utf16_view;

	template <typename CharT>
	class lossy_utf32_view;
}

namespace details
{
	[[nodiscard]]
	constexpr utf8_string_view utf8_string_view_from_bytes_unchecked(std::u8string_view bytes) noexcept;

	[[nodiscard]]
	constexpr utf16_string_view utf16_string_view_from_code_units_unchecked(std::u16string_view code_units) noexcept;

	[[nodiscard]]
	constexpr utf32_string_view utf32_string_view_from_code_points_unchecked(std::u32string_view code_points) noexcept;

	template <typename Derived, typename View = utf8_string_view>
	class utf8_string_crtp;

	template <typename Derived, typename View = utf16_string_view>
	class utf16_string_crtp;

	template <typename Derived, typename View = utf32_string_view>
	class utf32_string_crtp;

	template<typename From, typename To>
	concept non_narrowing_convertible =
		requires(From value)
	{
		To{ value };
	};

	namespace encoding_constants
	{
		inline constexpr std::uint32_t ascii_scalar_max = 0x7Fu;
		inline constexpr std::uint32_t two_byte_scalar_max = 0x7FFu;
		inline constexpr std::uint32_t bmp_scalar_max = 0xFFFFu;
		inline constexpr std::uint32_t supplementary_plane_base = 0x10000u;
		inline constexpr std::uint32_t max_unicode_scalar = 0x10FFFFu;
		inline constexpr std::uint32_t replacement_character_scalar = 0xFFFDu;
		inline constexpr std::uint32_t high_surrogate_min = 0xD800u;
		inline constexpr std::uint32_t high_surrogate_max = 0xDBFFu;
		inline constexpr std::uint32_t low_surrogate_min = 0xDC00u;
		inline constexpr std::uint32_t low_surrogate_max = 0xDFFFu;
		inline constexpr std::uint32_t scalar_before_surrogate_range = 0xD7FFu;
		inline constexpr std::uint32_t scalar_after_surrogate_range = 0xE000u;
		inline constexpr std::uint32_t surrogate_payload_mask = 0x3FFu;
		inline constexpr std::size_t single_code_unit_count = 1;
		inline constexpr std::size_t two_code_unit_count = 2;
		inline constexpr std::size_t three_code_unit_count = 3;
		inline constexpr std::size_t four_code_unit_count = 4;
		inline constexpr std::size_t utf16_surrogate_code_unit_count = two_code_unit_count;
		inline constexpr std::size_t utf32_surrogate_code_unit_count = single_code_unit_count;
		inline constexpr std::size_t max_utf8_code_units = four_code_unit_count;
		inline constexpr std::uint32_t utf8_continuation_payload_bits = 6u;
		inline constexpr std::uint32_t utf8_two_byte_lead_shift = utf8_continuation_payload_bits;
		inline constexpr std::uint32_t utf8_three_byte_lead_shift = utf8_continuation_payload_bits * 2u;
		inline constexpr std::uint32_t utf8_four_byte_lead_shift = utf8_continuation_payload_bits * 3u;
		inline constexpr std::uint32_t utf16_high_surrogate_shift = 10u;
		inline constexpr std::uint8_t ascii_control_max = 0x1Fu;
		inline constexpr std::uint8_t ascii_delete = 0x7Fu;
		inline constexpr std::uint8_t ascii_graphic_first = 0x21u;
		inline constexpr std::uint8_t ascii_graphic_last = 0x7Eu;
		inline constexpr std::uint8_t utf8_continuation_mask = 0xC0u;
		inline constexpr std::uint8_t utf8_continuation_tag = 0x80u;
		inline constexpr std::uint8_t utf8_continuation_min = 0x80u;
		inline constexpr std::uint8_t utf8_continuation_max = 0xBFu;
		inline constexpr std::uint8_t utf8_two_byte_lead_min = 0xC2u;
		inline constexpr std::uint8_t utf8_two_byte_lead_max = 0xDFu;
		inline constexpr std::uint8_t utf8_three_byte_lead_min = 0xE0u;
		inline constexpr std::uint8_t utf8_three_byte_lead_max = 0xEFu;
		inline constexpr std::uint8_t utf8_four_byte_lead_min = 0xF0u;
		inline constexpr std::uint8_t utf8_four_byte_lead_max = 0xF4u;
		inline constexpr std::uint8_t utf8_three_byte_lead_after_e0_min = 0xE1u;
		inline constexpr std::uint8_t utf8_three_byte_lead_before_surrogate_max = 0xECu;
		inline constexpr std::uint8_t utf8_surrogate_boundary_lead = 0xEDu;
		inline constexpr std::uint8_t utf8_three_byte_lead_after_surrogate_min = 0xEEu;
		inline constexpr std::uint8_t utf8_four_byte_lead_after_f0_min = 0xF1u;
		inline constexpr std::uint8_t utf8_four_byte_lead_before_f4_max = 0xF3u;
		inline constexpr std::uint8_t utf8_two_byte_prefix_mask = 0xE0u;
		inline constexpr std::uint8_t utf8_three_byte_prefix_mask = 0xF0u;
		inline constexpr std::uint8_t utf8_four_byte_prefix_mask = 0xF8u;
		inline constexpr std::uint8_t utf8_two_byte_prefix_value = 0xC0u;
		inline constexpr std::uint8_t utf8_three_byte_prefix_value = 0xE0u;
		inline constexpr std::uint8_t utf8_four_byte_prefix_value = 0xF0u;
		inline constexpr std::uint8_t utf8_two_byte_payload_mask = 0x1Fu;
		inline constexpr std::uint8_t utf8_three_byte_payload_mask = 0x0Fu;
		inline constexpr std::uint8_t utf8_four_byte_payload_mask = 0x07u;
		inline constexpr std::uint8_t utf8_continuation_payload_mask = 0x3Fu;
		inline constexpr std::uint8_t utf8_e0_second_byte_min = 0xA0u;
		inline constexpr std::uint8_t utf8_ed_second_byte_max = 0x9Fu;
		inline constexpr std::uint8_t utf8_f0_second_byte_min = 0x90u;
		inline constexpr std::uint8_t utf8_f4_second_byte_max = 0x8Fu;
	}

	struct utf8_lead_validation_traits
	{
		std::uint8_t size = 0;
		std::uint8_t second_min = 0;
		std::uint8_t second_max = 0;
	};

	inline constexpr auto utf8_lead_validation_table = []
	{
		std::array<utf8_lead_validation_traits, 256> table{};

		for (std::size_t lead = 0; lead <= encoding_constants::ascii_scalar_max; ++lead)
		{
			table[lead].size = static_cast<std::uint8_t>(encoding_constants::single_code_unit_count);
		}

		for (std::size_t lead = encoding_constants::utf8_two_byte_lead_min;
			lead <= encoding_constants::utf8_two_byte_lead_max;
			++lead)
		{
			table[lead] = utf8_lead_validation_traits{
				.size = static_cast<std::uint8_t>(encoding_constants::two_code_unit_count),
				.second_min = encoding_constants::utf8_continuation_min,
				.second_max = encoding_constants::utf8_continuation_max
			};
		}

		table[encoding_constants::utf8_three_byte_lead_min] = utf8_lead_validation_traits{
			.size = static_cast<std::uint8_t>(encoding_constants::three_code_unit_count),
			.second_min = encoding_constants::utf8_e0_second_byte_min,
			.second_max = encoding_constants::utf8_continuation_max
		};

		for (std::size_t lead = encoding_constants::utf8_three_byte_lead_after_e0_min;
			lead <= encoding_constants::utf8_three_byte_lead_before_surrogate_max;
			++lead)
		{
			table[lead] = utf8_lead_validation_traits{
				.size = static_cast<std::uint8_t>(encoding_constants::three_code_unit_count),
				.second_min = encoding_constants::utf8_continuation_min,
				.second_max = encoding_constants::utf8_continuation_max
			};
		}

		table[encoding_constants::utf8_surrogate_boundary_lead] = utf8_lead_validation_traits{
			.size = static_cast<std::uint8_t>(encoding_constants::three_code_unit_count),
			.second_min = encoding_constants::utf8_continuation_min,
			.second_max = encoding_constants::utf8_ed_second_byte_max
		};

		for (std::size_t lead = encoding_constants::utf8_three_byte_lead_after_surrogate_min;
			lead <= encoding_constants::utf8_three_byte_lead_max;
			++lead)
		{
			table[lead] = utf8_lead_validation_traits{
				.size = static_cast<std::uint8_t>(encoding_constants::three_code_unit_count),
				.second_min = encoding_constants::utf8_continuation_min,
				.second_max = encoding_constants::utf8_continuation_max
			};
		}

		table[encoding_constants::utf8_four_byte_lead_min] = utf8_lead_validation_traits{
			.size = static_cast<std::uint8_t>(encoding_constants::max_utf8_code_units),
			.second_min = encoding_constants::utf8_f0_second_byte_min,
			.second_max = encoding_constants::utf8_continuation_max
		};

		for (std::size_t lead = encoding_constants::utf8_four_byte_lead_after_f0_min;
			lead <= encoding_constants::utf8_four_byte_lead_before_f4_max;
			++lead)
		{
			table[lead] = utf8_lead_validation_traits{
				.size = static_cast<std::uint8_t>(encoding_constants::max_utf8_code_units),
				.second_min = encoding_constants::utf8_continuation_min,
				.second_max = encoding_constants::utf8_continuation_max
			};
		}

		table[encoding_constants::utf8_four_byte_lead_max] = utf8_lead_validation_traits{
			.size = static_cast<std::uint8_t>(encoding_constants::max_utf8_code_units),
			.second_min = encoding_constants::utf8_continuation_min,
			.second_max = encoding_constants::utf8_f4_second_byte_max
		};

		return table;
	}();

	template <typename CharT>
	requires (sizeof(CharT) == 1)
	inline constexpr std::size_t ascii_prefix_length_scalar(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size()
			&& static_cast<std::uint8_t>(value[index]) <= encoding_constants::ascii_scalar_max)
		{
			++index;
		}

		return index;
	}

	template <typename CharT>
	requires (sizeof(CharT) == 2)
	inline constexpr std::size_t ascii_prefix_length_scalar(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size()
			&& static_cast<std::uint16_t>(value[index]) <= encoding_constants::ascii_scalar_max)
		{
			++index;
		}

		return index;
	}

	template <typename CharT>
	requires (sizeof(CharT) == 4)
	inline constexpr std::size_t ascii_prefix_length_scalar(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size()
			&& static_cast<std::uint32_t>(value[index]) <= encoding_constants::ascii_scalar_max)
		{
			++index;
		}

		return index;
	}

	template <typename CharT>
	requires (sizeof(CharT) == 1)
	inline std::size_t ascii_prefix_length_runtime(std::basic_string_view<CharT> value) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const std::uint8_t*>(value.data());
		std::size_t index = 0;
		while (index + 16 <= value.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			if (_mm_movemask_epi8(block) != 0)
			{
				for (std::size_t lane = 0; lane != 16; ++lane)
				{
					if (data[index + lane] > encoding_constants::ascii_scalar_max)
					{
						return index + lane;
					}
				}
			}

			index += 16;
		}

		while (index < value.size() && data[index] <= encoding_constants::ascii_scalar_max)
		{
			++index;
		}

		return index;
#else
		return ascii_prefix_length_scalar(value);
#endif
	}

	template <typename CharT>
	requires (sizeof(CharT) == 2)
	inline std::size_t ascii_prefix_length_runtime(std::basic_string_view<CharT> value) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto ascii_mask = _mm_set1_epi16(static_cast<short>(-128));
		const auto zero = _mm_setzero_si128();
		std::size_t index = 0;
		while (index + 8 <= value.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value.data() + index));
			const auto high_bits = _mm_and_si128(block, ascii_mask);
			const auto ascii_lanes = _mm_cmpeq_epi16(high_bits, zero);
			if (_mm_movemask_epi8(ascii_lanes) != 0xFFFF)
			{
				for (std::size_t lane = 0; lane != 8; ++lane)
				{
					if (static_cast<std::uint16_t>(value[index + lane]) > encoding_constants::ascii_scalar_max)
					{
						return index + lane;
					}
				}
			}

			index += 8;
		}

		while (index < value.size()
			&& static_cast<std::uint16_t>(value[index]) <= encoding_constants::ascii_scalar_max)
		{
			++index;
		}

		return index;
#else
		return ascii_prefix_length_scalar(value);
#endif
	}

	template <typename CharT>
	requires (sizeof(CharT) == 4)
	inline std::size_t ascii_prefix_length_runtime(std::basic_string_view<CharT> value) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto ascii_mask = _mm_set1_epi32(static_cast<int>(~0x7Fu));
		const auto zero = _mm_setzero_si128();
		std::size_t index = 0;
		while (index + 4 <= value.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value.data() + index));
			const auto high_bits = _mm_and_si128(block, ascii_mask);
			const auto ascii_lanes = _mm_cmpeq_epi32(high_bits, zero);
			if (_mm_movemask_epi8(ascii_lanes) != 0xFFFF)
			{
				for (std::size_t lane = 0; lane != 4; ++lane)
				{
					if (static_cast<std::uint32_t>(value[index + lane]) > encoding_constants::ascii_scalar_max)
					{
						return index + lane;
					}
				}
			}

			index += 4;
		}

		while (index < value.size()
			&& static_cast<std::uint32_t>(value[index]) <= encoding_constants::ascii_scalar_max)
		{
			++index;
		}

		return index;
#else
		return ascii_prefix_length_scalar(value);
#endif
	}

	template <typename CharT>
	inline constexpr std::size_t ascii_prefix_length(std::basic_string_view<CharT> value) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_prefix_length_scalar(value);
		}

		return ascii_prefix_length_runtime(value);
	}

	template <typename CharT>
	inline constexpr bool is_ascii_only(std::basic_string_view<CharT> value) noexcept
	{
		return ascii_prefix_length(value) == value.size();
	}

	// Runtime-only thresholding for the UTF-32 parallel paths. These are intentionally
	// conservative so we do not pay thread setup costs on mid-sized inputs.
#if UINTPTR_MAX > 0xFFFFFFFFu
	inline constexpr std::size_t runtime_parallel_min_total_bytes = 1u << 20;
	inline constexpr std::size_t runtime_parallel_min_bytes_per_worker = 1u << 18;
	inline constexpr std::size_t runtime_parallel_max_worker_count = (std::numeric_limits<std::size_t>::max)();
#else
	// 32-bit runners have noticeably higher thread/setup pressure relative to the
	// UTF-32 workloads we parallelize here, so they use a stricter activation policy.
	inline constexpr std::size_t runtime_parallel_min_total_bytes = 2u << 20;
	inline constexpr std::size_t runtime_parallel_min_bytes_per_worker = 1u << 20;
	inline constexpr std::size_t runtime_parallel_max_worker_count = 2;
#endif

#ifndef UTF8_RANGES_ENABLE_TEST_HOOKS
#define UTF8_RANGES_ENABLE_TEST_HOOKS 0
#endif

#ifndef UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL
#define UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL 0
#endif

#if UTF8_RANGES_ENABLE_TEST_HOOKS
	inline constexpr bool test_force_utf32_parallel = UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL != 0;
#else
	inline constexpr bool test_force_utf32_parallel = false;
#endif

	struct utf32_parallel_plan
	{
		std::size_t worker_count = 1;
		std::size_t chunk_size = 0;
	};

	inline constexpr utf32_parallel_plan make_utf32_parallel_plan(
		std::size_t code_point_count,
		std::size_t available_workers) noexcept
	{
		if (code_point_count == 0)
		{
			return { 1, code_point_count };
		}

		if (test_force_utf32_parallel && available_workers >= 2)
		{
			const auto forced_workers = std::max<std::size_t>(
				2,
				std::min(available_workers, runtime_parallel_max_worker_count));
			return {
				forced_workers,
				(code_point_count + forced_workers - 1) / forced_workers
			};
		}

		const auto total_bytes = code_point_count * sizeof(char32_t);
		if (total_bytes < runtime_parallel_min_total_bytes)
		{
			return { 1, code_point_count };
		}

		// UTF-32 is fixed-width, so chunking by code-point index is always boundary-safe.
		std::size_t worker_count = available_workers;
		if (worker_count < 2)
		{
			return { 1, code_point_count };
		}

		worker_count = std::min(worker_count, runtime_parallel_max_worker_count);
		worker_count = std::min(worker_count, total_bytes / runtime_parallel_min_bytes_per_worker);
		worker_count = std::min(worker_count, code_point_count);
		if (worker_count < 2)
		{
			return { 1, code_point_count };
		}

		return {
			worker_count,
			(code_point_count + worker_count - 1) / worker_count
		};
	}

	inline utf32_parallel_plan make_utf32_parallel_plan(std::size_t code_point_count) noexcept
	{
		return make_utf32_parallel_plan(code_point_count, std::thread::hardware_concurrency());
	}

	inline constexpr std::u32string_view utf32_parallel_chunk(
		std::u32string_view code_points,
		utf32_parallel_plan plan,
		std::size_t chunk_index) noexcept
	{
		// Each worker gets a disjoint contiguous subspan so it can measure and write
		// independently without any boundary repair.
		const auto start = chunk_index * plan.chunk_size;
		if (start >= code_points.size())
		{
			return {};
		}

		return code_points.substr(start, std::min(plan.chunk_size, code_points.size() - start));
	}

	template <typename Fn>
	inline void run_parallel_jobs(std::size_t worker_count, Fn&& fn)
	{
		if (worker_count <= 1)
		{
			fn(0);
			return;
		}

		// The calling thread participates as the last worker to avoid spinning up an
		// extra thread just to sit idle waiting for joins.
#if UTF8_RANGES_HAS_JTHREAD
		auto workers = std::make_unique<std::jthread[]>(worker_count - 1);
		for (std::size_t worker_index = 0; worker_index + 1 < worker_count; ++worker_index)
		{
			workers[worker_index] = std::jthread([&, worker_index]
				{
					fn(worker_index);
				});
		}
		fn(worker_count - 1);
#else
		auto workers = std::make_unique<std::thread[]>(worker_count - 1);
		std::size_t started_workers = 0;
		try
		{
			for (std::size_t worker_index = 0; worker_index + 1 < worker_count; ++worker_index)
			{
				workers[worker_index] = std::thread([&, worker_index]
					{
						fn(worker_index);
					});
				++started_workers;
			}
		}
		catch (...)
		{
			for (std::size_t worker_index = 0; worker_index != started_workers; ++worker_index)
			{
				if (workers[worker_index].joinable())
				{
					workers[worker_index].join();
				}
			}
			throw;
		}
		fn(worker_count - 1);
		for (std::size_t worker_index = 0; worker_index != started_workers; ++worker_index)
		{
			if (workers[worker_index].joinable())
			{
				workers[worker_index].join();
			}
		}
#endif
	}

	inline constexpr bool ascii_lowercase_copy_scalar(char8_t* out, std::u8string_view bytes) noexcept
	{
		bool changed = false;
		for (std::size_t index = 0; index != bytes.size(); ++index)
		{
			auto value = static_cast<std::uint8_t>(bytes[index]);
			if (value >= static_cast<std::uint8_t>('A') && value <= static_cast<std::uint8_t>('Z'))
			{
				value = static_cast<std::uint8_t>(value + (static_cast<std::uint8_t>('a') - static_cast<std::uint8_t>('A')));
				changed = true;
			}

			out[index] = static_cast<char8_t>(value);
		}

		return changed;
	}

	inline constexpr bool ascii_uppercase_copy_scalar(char8_t* out, std::u8string_view bytes) noexcept
	{
		bool changed = false;
		for (std::size_t index = 0; index != bytes.size(); ++index)
		{
			auto value = static_cast<std::uint8_t>(bytes[index]);
			if (value >= static_cast<std::uint8_t>('a') && value <= static_cast<std::uint8_t>('z'))
			{
				value = static_cast<std::uint8_t>(value - (static_cast<std::uint8_t>('a') - static_cast<std::uint8_t>('A')));
				changed = true;
			}

			out[index] = static_cast<char8_t>(value);
		}

		return changed;
	}

	inline constexpr bool ascii_lowercase_copy_scalar(char16_t* out, std::u16string_view code_units) noexcept
	{
		bool changed = false;
		for (std::size_t index = 0; index != code_units.size(); ++index)
		{
			auto value = code_units[index];
			if (value >= u'A' && value <= u'Z')
			{
				value = static_cast<char16_t>(value + (u'a' - u'A'));
				changed = true;
			}

			out[index] = value;
		}

		return changed;
	}

	inline constexpr bool ascii_uppercase_copy_scalar(char16_t* out, std::u16string_view code_units) noexcept
	{
		bool changed = false;
		for (std::size_t index = 0; index != code_units.size(); ++index)
		{
			auto value = code_units[index];
			if (value >= u'a' && value <= u'z')
			{
				value = static_cast<char16_t>(value - (u'a' - u'A'));
				changed = true;
			}

			out[index] = value;
		}

		return changed;
	}

	inline constexpr bool ascii_lowercase_copy_scalar(char32_t* out, std::u32string_view code_points) noexcept
	{
		bool changed = false;
		for (std::size_t index = 0; index != code_points.size(); ++index)
		{
			auto value = code_points[index];
			if (value >= U'A' && value <= U'Z')
			{
				value = static_cast<char32_t>(value + (U'a' - U'A'));
				changed = true;
			}

			out[index] = value;
		}

		return changed;
	}

	inline constexpr bool ascii_uppercase_copy_scalar(char32_t* out, std::u32string_view code_points) noexcept
	{
		bool changed = false;
		for (std::size_t index = 0; index != code_points.size(); ++index)
		{
			auto value = code_points[index];
			if (value >= U'a' && value <= U'z')
			{
				value = static_cast<char32_t>(value - (U'a' - U'A'));
				changed = true;
			}

			out[index] = value;
		}

		return changed;
	}

	inline bool ascii_lowercase_copy_runtime(char32_t* out, std::u32string_view code_points) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char32_t*>(code_points.data());
		auto* dest = reinterpret_cast<char32_t*>(out);
		const auto upper_min = _mm_set1_epi32(U'A' - 1);
		const auto upper_max = _mm_set1_epi32(U'Z' + 1);
		const auto delta = _mm_set1_epi32(U'a' - U'A');
		bool changed = false;
		std::size_t index = 0;
		while (index + 4 <= code_points.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto is_after_min = _mm_cmpgt_epi32(block, upper_min);
			const auto is_before_max = _mm_cmpgt_epi32(upper_max, block);
			const auto upper = _mm_and_si128(is_after_min, is_before_max);
			changed = changed || (_mm_movemask_epi8(upper) != 0);
			const auto add = _mm_and_si128(upper, delta);
			const auto mapped = _mm_add_epi32(block, add);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), mapped);
			index += 4;
		}

		if (index != code_points.size())
		{
			changed = ascii_lowercase_copy_scalar(out + index, code_points.substr(index)) || changed;
		}

		return changed;
#else
		bool changed = false;
		for (std::size_t index = 0; index != code_points.size(); ++index)
		{
			auto value = static_cast<std::uint32_t>(code_points[index]);
			const auto is_upper = static_cast<std::uint32_t>(value - U'A') <= static_cast<std::uint32_t>(U'Z' - U'A');
			value += is_upper * static_cast<std::uint32_t>(U'a' - U'A');
			changed = changed || (is_upper != 0);
			out[index] = static_cast<char32_t>(value);
		}

		return changed;
#endif
	}

	inline bool ascii_uppercase_copy_runtime(char32_t* out, std::u32string_view code_points) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char32_t*>(code_points.data());
		auto* dest = reinterpret_cast<char32_t*>(out);
		const auto lower_min = _mm_set1_epi32(U'a' - 1);
		const auto lower_max = _mm_set1_epi32(U'z' + 1);
		const auto delta = _mm_set1_epi32(U'a' - U'A');
		bool changed = false;
		std::size_t index = 0;
		while (index + 4 <= code_points.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto is_after_min = _mm_cmpgt_epi32(block, lower_min);
			const auto is_before_max = _mm_cmpgt_epi32(lower_max, block);
			const auto lower = _mm_and_si128(is_after_min, is_before_max);
			changed = changed || (_mm_movemask_epi8(lower) != 0);
			const auto sub = _mm_and_si128(lower, delta);
			const auto mapped = _mm_sub_epi32(block, sub);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), mapped);
			index += 4;
		}

		if (index != code_points.size())
		{
			changed = ascii_uppercase_copy_scalar(out + index, code_points.substr(index)) || changed;
		}

		return changed;
#else
		bool changed = false;
		for (std::size_t index = 0; index != code_points.size(); ++index)
		{
			auto value = static_cast<std::uint32_t>(code_points[index]);
			const auto is_lower = static_cast<std::uint32_t>(value - U'a') <= static_cast<std::uint32_t>(U'z' - U'a');
			value -= is_lower * static_cast<std::uint32_t>(U'a' - U'A');
			changed = changed || (is_lower != 0);
			out[index] = static_cast<char32_t>(value);
		}

		return changed;
#endif
	}

	inline bool ascii_lowercase_copy_runtime(char8_t* out, std::u8string_view bytes) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char*>(bytes.data());
		auto* dest = reinterpret_cast<char*>(out);
		const auto upper_min = _mm_set1_epi8('A' - 1);
		const auto upper_max = _mm_set1_epi8('Z' + 1);
		const auto delta = _mm_set1_epi8(static_cast<char>('a' - 'A'));
		bool changed = false;
		std::size_t index = 0;
		while (index + 16 <= bytes.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto is_after_min = _mm_cmpgt_epi8(block, upper_min);
			const auto is_before_max = _mm_cmplt_epi8(block, upper_max);
			const auto upper = _mm_and_si128(is_after_min, is_before_max);
			changed = changed || (_mm_movemask_epi8(upper) != 0);
			const auto add = _mm_and_si128(upper, delta);
			const auto mapped = _mm_add_epi8(block, add);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), mapped);
			index += 16;
		}

		if (index != bytes.size())
		{
			changed = ascii_lowercase_copy_scalar(out + index, bytes.substr(index)) || changed;
		}

		return changed;
#else
		return ascii_lowercase_copy_scalar(out, bytes);
#endif
	}

	inline bool ascii_uppercase_copy_runtime(char8_t* out, std::u8string_view bytes) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char*>(bytes.data());
		auto* dest = reinterpret_cast<char*>(out);
		const auto lower_min = _mm_set1_epi8('a' - 1);
		const auto lower_max = _mm_set1_epi8('z' + 1);
		const auto delta = _mm_set1_epi8(static_cast<char>('a' - 'A'));
		bool changed = false;
		std::size_t index = 0;
		while (index + 16 <= bytes.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto is_after_min = _mm_cmpgt_epi8(block, lower_min);
			const auto is_before_max = _mm_cmplt_epi8(block, lower_max);
			const auto lower = _mm_and_si128(is_after_min, is_before_max);
			changed = changed || (_mm_movemask_epi8(lower) != 0);
			const auto sub = _mm_and_si128(lower, delta);
			const auto mapped = _mm_sub_epi8(block, sub);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), mapped);
			index += 16;
		}

		if (index != bytes.size())
		{
			changed = ascii_uppercase_copy_scalar(out + index, bytes.substr(index)) || changed;
		}

		return changed;
#else
		return ascii_uppercase_copy_scalar(out, bytes);
#endif
	}

	inline bool ascii_lowercase_copy_runtime(char16_t* out, std::u16string_view code_units) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char16_t*>(code_units.data());
		auto* dest = reinterpret_cast<char16_t*>(out);
		const auto upper_min = _mm_set1_epi16(u'A' - 1);
		const auto upper_max = _mm_set1_epi16(u'Z' + 1);
		const auto delta = _mm_set1_epi16(u'a' - u'A');
		bool changed = false;
		std::size_t index = 0;
		while (index + 8 <= code_units.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto is_after_min = _mm_cmpgt_epi16(block, upper_min);
			const auto is_before_max = _mm_cmplt_epi16(block, upper_max);
			const auto upper = _mm_and_si128(is_after_min, is_before_max);
			changed = changed || (_mm_movemask_epi8(upper) != 0);
			const auto add = _mm_and_si128(upper, delta);
			const auto mapped = _mm_add_epi16(block, add);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), mapped);
			index += 8;
		}

		if (index != code_units.size())
		{
			changed = ascii_lowercase_copy_scalar(out + index, code_units.substr(index)) || changed;
		}

		return changed;
#else
		return ascii_lowercase_copy_scalar(out, code_units);
#endif
	}

	inline bool ascii_uppercase_copy_runtime(char16_t* out, std::u16string_view code_units) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char16_t*>(code_units.data());
		auto* dest = reinterpret_cast<char16_t*>(out);
		const auto lower_min = _mm_set1_epi16(u'a' - 1);
		const auto lower_max = _mm_set1_epi16(u'z' + 1);
		const auto delta = _mm_set1_epi16(u'a' - u'A');
		bool changed = false;
		std::size_t index = 0;
		while (index + 8 <= code_units.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto is_after_min = _mm_cmpgt_epi16(block, lower_min);
			const auto is_before_max = _mm_cmplt_epi16(block, lower_max);
			const auto lower = _mm_and_si128(is_after_min, is_before_max);
			changed = changed || (_mm_movemask_epi8(lower) != 0);
			const auto sub = _mm_and_si128(lower, delta);
			const auto mapped = _mm_sub_epi16(block, sub);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), mapped);
			index += 8;
		}

		if (index != code_units.size())
		{
			changed = ascii_uppercase_copy_scalar(out + index, code_units.substr(index)) || changed;
		}

		return changed;
#else
		return ascii_uppercase_copy_scalar(out, code_units);
#endif
	}

	inline constexpr bool ascii_lowercase_copy(char8_t* out, std::u8string_view bytes) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_lowercase_copy_scalar(out, bytes);
		}

		return ascii_lowercase_copy_runtime(out, bytes);
	}

	inline constexpr bool ascii_uppercase_copy(char8_t* out, std::u8string_view bytes) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_uppercase_copy_scalar(out, bytes);
		}

		return ascii_uppercase_copy_runtime(out, bytes);
	}

	inline constexpr bool ascii_lowercase_copy(char16_t* out, std::u16string_view code_units) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_lowercase_copy_scalar(out, code_units);
		}

		return ascii_lowercase_copy_runtime(out, code_units);
	}

	inline constexpr bool ascii_uppercase_copy(char16_t* out, std::u16string_view code_units) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_uppercase_copy_scalar(out, code_units);
		}

		return ascii_uppercase_copy_runtime(out, code_units);
	}

	inline constexpr bool ascii_lowercase_copy(char32_t* out, std::u32string_view code_points) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_lowercase_copy_scalar(out, code_points);
		}

		return ascii_lowercase_copy_runtime(out, code_points);
	}

	inline constexpr bool ascii_uppercase_copy(char32_t* out, std::u32string_view code_points) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return ascii_uppercase_copy_scalar(out, code_points);
		}

		return ascii_uppercase_copy_runtime(out, code_points);
	}

	inline constexpr bool ascii_lowercase_inplace(char8_t* bytes, std::size_t size) noexcept
	{
		return ascii_lowercase_copy(bytes, std::u8string_view{ bytes, size });
	}

	inline constexpr bool ascii_uppercase_inplace(char8_t* bytes, std::size_t size) noexcept
	{
		return ascii_uppercase_copy(bytes, std::u8string_view{ bytes, size });
	}

	inline constexpr bool ascii_lowercase_inplace(char16_t* code_units, std::size_t size) noexcept
	{
		return ascii_lowercase_copy(code_units, std::u16string_view{ code_units, size });
	}

	inline constexpr bool ascii_uppercase_inplace(char16_t* code_units, std::size_t size) noexcept
	{
		return ascii_uppercase_copy(code_units, std::u16string_view{ code_units, size });
	}

	inline constexpr bool ascii_lowercase_inplace(char32_t* code_points, std::size_t size) noexcept
	{
		return ascii_lowercase_copy(code_points, std::u32string_view{ code_points, size });
	}

	inline constexpr bool ascii_uppercase_inplace(char32_t* code_points, std::size_t size) noexcept
	{
		return ascii_uppercase_copy(code_points, std::u32string_view{ code_points, size });
	}

	inline constexpr void copy_ascii_bytes_to_utf8_scalar(char8_t* out, std::string_view bytes) noexcept
	{
		for (std::size_t index = 0; index != bytes.size(); ++index)
		{
			out[index] = static_cast<char8_t>(static_cast<std::uint8_t>(bytes[index]));
		}
	}

	inline void copy_ascii_bytes_to_utf8_runtime(char8_t* out, std::string_view bytes) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		const auto* data = reinterpret_cast<const char*>(bytes.data());
		auto* dest = reinterpret_cast<char*>(out);
		std::size_t index = 0;
		while (index + 16 <= bytes.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(dest + index), block);
			index += 16;
		}

		copy_ascii_bytes_to_utf8_scalar(out + index, bytes.substr(index));
#else
		copy_ascii_bytes_to_utf8_scalar(out, bytes);
#endif
	}

	inline constexpr void copy_ascii_bytes_to_utf8(char8_t* out, std::string_view bytes) noexcept
	{
		if (std::is_constant_evaluated())
		{
			copy_ascii_bytes_to_utf8_scalar(out, bytes);
			return;
		}

		copy_ascii_bytes_to_utf8_runtime(out, bytes);
	}

	template <typename CharT>
	requires (sizeof(CharT) == 1)
	inline constexpr void copy_ascii_bytes_to_utf16_scalar(char16_t* out, std::basic_string_view<CharT> bytes) noexcept
	{
		for (std::size_t index = 0; index != bytes.size(); ++index)
		{
			out[index] = static_cast<char16_t>(static_cast<std::uint8_t>(bytes[index]));
		}
	}

	template <typename CharT>
	requires (sizeof(CharT) == 1)
	inline void copy_ascii_bytes_to_utf16_runtime(char16_t* out, std::basic_string_view<CharT> bytes) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		if (bytes.size() < 16) [[unlikely]]
		{
			copy_ascii_bytes_to_utf16_scalar(out, bytes);
			return;
		}

		const auto* data = reinterpret_cast<const char*>(bytes.data());
		const auto zero = _mm_setzero_si128();
		std::size_t index = 0;
		while (index + 16 <= bytes.size())
		{
			const auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + index));
			const auto lower = _mm_unpacklo_epi8(block, zero);
			const auto upper = _mm_unpackhi_epi8(block, zero);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(out + index), lower);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(out + index + 8), upper);
			index += 16;
		}

		copy_ascii_bytes_to_utf16_scalar(out + index, bytes.substr(index));
#else
		copy_ascii_bytes_to_utf16_scalar(out, bytes);
#endif
	}

	template <typename CharT>
	requires (sizeof(CharT) == 1)
	inline constexpr void copy_ascii_bytes_to_utf16(char16_t* out, std::basic_string_view<CharT> bytes) noexcept
	{
		if (std::is_constant_evaluated())
		{
			copy_ascii_bytes_to_utf16_scalar(out, bytes);
			return;
		}

		copy_ascii_bytes_to_utf16_runtime(out, bytes);
	}

	inline constexpr void copy_ascii_utf8_to_utf16_scalar(char16_t* out, std::u8string_view bytes) noexcept
	{
		for (std::size_t index = 0; index != bytes.size(); ++index)
		{
			out[index] = static_cast<char16_t>(static_cast<std::uint8_t>(bytes[index]));
		}
	}

	inline void copy_ascii_utf8_to_utf16_runtime(char16_t* out, std::u8string_view bytes) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		if (bytes.size() < 16) [[unlikely]]
		{
			copy_ascii_utf8_to_utf16_scalar(out, bytes);
			return;
		}

		const auto* data = reinterpret_cast<const char*>(bytes.data());
		const auto zero = _mm_setzero_si128();
		std::size_t index = 0;
		while (index + 16 <= bytes.size())
		{
			__m128i block;
			std::memcpy(&block, data + index, 16);
			const auto lower = _mm_unpacklo_epi8(block, zero);
			const auto upper = _mm_unpackhi_epi8(block, zero);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(out + index), lower);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(out + index + 8), upper);
			index += 16;
		}

		copy_ascii_utf8_to_utf16_scalar(out + index, bytes.substr(index));
#else
		copy_ascii_utf8_to_utf16_scalar(out, bytes);
#endif
	}

	inline constexpr void copy_ascii_utf8_to_utf16(char16_t* out, std::u8string_view bytes) noexcept
	{
		if (std::is_constant_evaluated())
		{
			copy_ascii_utf8_to_utf16_scalar(out, bytes);
			return;
		}

		copy_ascii_utf8_to_utf16_runtime(out, bytes);
	}

	inline constexpr void copy_ascii_utf8_to_utf32_scalar(char32_t* out, std::u8string_view bytes) noexcept
	{
		for (std::size_t index = 0; index != bytes.size(); ++index)
		{
			out[index] = static_cast<char32_t>(static_cast<std::uint8_t>(bytes[index]));
		}
	}

	template <typename CharT>
	requires (sizeof(CharT) >= 2)
	inline constexpr void copy_ascii_code_units_to_utf8_scalar(char8_t* out, std::basic_string_view<CharT> code_units) noexcept
	{
		for (std::size_t index = 0; index != code_units.size(); ++index)
		{
			out[index] = static_cast<char8_t>(static_cast<std::uint32_t>(code_units[index]));
		}
	}

	template <typename CharT>
	requires (sizeof(CharT) >= 2)
	inline void copy_ascii_code_units_to_utf8_runtime(char8_t* out, std::basic_string_view<CharT> code_units) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		if constexpr (sizeof(CharT) == 2)
		{
			std::size_t index = 0;
			while (index + 16 <= code_units.size())
			{
				const auto lower = _mm_loadu_si128(reinterpret_cast<const __m128i*>(code_units.data() + index));
				const auto upper = _mm_loadu_si128(reinterpret_cast<const __m128i*>(code_units.data() + index + 8));
				const auto packed = _mm_packus_epi16(lower, upper);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(out + index), packed);
				index += 16;
			}

			copy_ascii_code_units_to_utf8_scalar(out + index, code_units.substr(index));
			return;
		}
		else
		{
			copy_ascii_code_units_to_utf8_scalar(out, code_units);
			return;
		}
#endif
		copy_ascii_code_units_to_utf8_scalar(out, code_units);
	}

	template <typename CharT>
	requires (sizeof(CharT) >= 2)
	inline constexpr void copy_ascii_code_units_to_utf8(char8_t* out, std::basic_string_view<CharT> code_units) noexcept
	{
		if (std::is_constant_evaluated())
		{
			copy_ascii_code_units_to_utf8_scalar(out, code_units);
			return;
		}

		copy_ascii_code_units_to_utf8_runtime(out, code_units);
	}

	inline constexpr void copy_ascii_utf16_to_utf8_scalar(char8_t* out, std::u16string_view code_units) noexcept
	{
		for (std::size_t index = 0; index != code_units.size(); ++index)
		{
			out[index] = static_cast<char8_t>(code_units[index]);
		}
	}

	inline void copy_ascii_utf16_to_utf8_runtime(char8_t* out, std::u16string_view code_units) noexcept
	{
#if UTF8_RANGES_HAS_SSE2_INTRINSICS
		std::size_t index = 0;
		while (index + 16 <= code_units.size())
		{
			const auto lower = _mm_loadu_si128(reinterpret_cast<const __m128i*>(code_units.data() + index));
			const auto upper = _mm_loadu_si128(reinterpret_cast<const __m128i*>(code_units.data() + index + 8));
			const auto packed = _mm_packus_epi16(lower, upper);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(out + index), packed);
			index += 16;
		}

		copy_ascii_utf16_to_utf8_scalar(out + index, code_units.substr(index));
#else
		copy_ascii_utf16_to_utf8_scalar(out, code_units);
#endif
	}

	inline constexpr void copy_ascii_utf16_to_utf8(char8_t* out, std::u16string_view code_units) noexcept
	{
		if (std::is_constant_evaluated())
		{
			copy_ascii_utf16_to_utf8_scalar(out, code_units);
			return;
		}

		copy_ascii_utf16_to_utf8_runtime(out, code_units);
	}

	template <typename Allocator>
	using utf8_base_string = std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>;

	template <typename Allocator>
	using utf16_base_string = std::basic_string<char16_t, std::char_traits<char16_t>, Allocator>;

	template <typename Allocator>
	using utf32_base_string = std::basic_string<char32_t, std::char_traits<char32_t>, Allocator>;

	template<typename CharT>
	inline constexpr bool is_single_valid_utf8_char(std::basic_string_view<CharT> value) noexcept
	{
		if (value.empty()) [[unlikely]]
		{
			return false;
		}

		const auto byte = [&value](std::size_t index) noexcept -> std::uint8_t
		{
			return static_cast<std::uint8_t>(value[index]);
		};

		const auto is_cont = [&byte](std::size_t index) noexcept -> bool
		{
			return (byte(index) & encoding_constants::utf8_continuation_mask) == encoding_constants::utf8_continuation_tag;
		};

		const std::size_t size = value.size();
		const unsigned char b1 = byte(0);

		if (size == encoding_constants::single_code_unit_count) [[likely]]
		{
			return b1 <= encoding_constants::ascii_scalar_max;
		}

		if (size == encoding_constants::two_code_unit_count)
		{
			return b1 >= encoding_constants::utf8_two_byte_lead_min
				&& b1 <= encoding_constants::utf8_two_byte_lead_max
				&& is_cont(1);
		}

		if (size == encoding_constants::three_code_unit_count)
		{
			const unsigned char b2 = byte(1);
			return
				(
					b1 == encoding_constants::utf8_three_byte_lead_min &&
					b2 >= encoding_constants::utf8_e0_second_byte_min
					&& b2 <= encoding_constants::utf8_continuation_max &&
					is_cont(2)
				) ||
				(
					b1 >= encoding_constants::utf8_three_byte_lead_after_e0_min
					&& b1 <= encoding_constants::utf8_three_byte_lead_before_surrogate_max
					&& is_cont(1) && is_cont(2)
				) ||
				(
					b1 == encoding_constants::utf8_surrogate_boundary_lead &&
					b2 >= encoding_constants::utf8_continuation_min
					&& b2 <= encoding_constants::utf8_ed_second_byte_max &&
					is_cont(1) && is_cont(2)
				) ||
				(
					b1 >= encoding_constants::utf8_three_byte_lead_after_surrogate_min
					&& b1 <= encoding_constants::utf8_three_byte_lead_max &&
					is_cont(1) && is_cont(2)
				);
		}

		if (size == encoding_constants::max_utf8_code_units)
		{
			const unsigned char b2 = byte(1);
			return
				(
					b1 == encoding_constants::utf8_four_byte_lead_min &&
					b2 >= encoding_constants::utf8_f0_second_byte_min
					&& b2 <= encoding_constants::utf8_continuation_max &&
					is_cont(2) && is_cont(3)
				) ||
				(
					b1 >= encoding_constants::utf8_four_byte_lead_after_f0_min
					&& b1 <= encoding_constants::utf8_four_byte_lead_before_f4_max &&
					is_cont(1) && is_cont(2) && is_cont(3)
				) ||
				(
					b1 == encoding_constants::utf8_four_byte_lead_max &&
					b2 >= encoding_constants::utf8_continuation_min
					&& b2 <= encoding_constants::utf8_f4_second_byte_max &&
					is_cont(2) && is_cont(3)
				);
		}

		return false;
	}

	template <typename R, typename T>
	concept container_compatible_range =
		std::ranges::input_range<R> &&
		std::convertible_to<std::ranges::range_reference_t<R>, T>;

	template <typename R, typename T>
	concept contiguous_sized_range_of =
		std::ranges::contiguous_range<R>
		&& std::ranges::sized_range<R>
		&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, T>;

	inline constexpr bool is_valid_unicode_scalar(std::uint32_t scalar) noexcept
	{
		return scalar <= encoding_constants::max_unicode_scalar
			&& !(scalar >= encoding_constants::high_surrogate_min && scalar <= encoding_constants::low_surrogate_max);
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf8_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		if (scalar <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			out[0] = static_cast<CharT>(scalar);
			return encoding_constants::single_code_unit_count;
		}

		if (scalar <= encoding_constants::two_byte_scalar_max)
		{
			out[0] = static_cast<CharT>(encoding_constants::utf8_two_byte_prefix_value | ((scalar >> encoding_constants::utf8_two_byte_lead_shift) & encoding_constants::utf8_two_byte_payload_mask));
			out[1] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | (scalar & encoding_constants::utf8_continuation_payload_mask));
			return encoding_constants::two_code_unit_count;
		}

		if (scalar <= encoding_constants::bmp_scalar_max)
		{
			out[0] = static_cast<CharT>(encoding_constants::utf8_three_byte_lead_min | ((scalar >> encoding_constants::utf8_three_byte_lead_shift) & encoding_constants::utf8_three_byte_payload_mask));
			out[1] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | ((scalar >> encoding_constants::utf8_continuation_payload_bits) & encoding_constants::utf8_continuation_payload_mask));
			out[2] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | (scalar & encoding_constants::utf8_continuation_payload_mask));
			return encoding_constants::three_code_unit_count;
		}

		out[0] = static_cast<CharT>(encoding_constants::utf8_four_byte_lead_min | ((scalar >> encoding_constants::utf8_four_byte_lead_shift) & encoding_constants::utf8_four_byte_payload_mask));
		out[1] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | ((scalar >> encoding_constants::utf8_three_byte_lead_shift) & encoding_constants::utf8_continuation_payload_mask));
		out[2] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | ((scalar >> encoding_constants::utf8_continuation_payload_bits) & encoding_constants::utf8_continuation_payload_mask));
		out[3] = static_cast<CharT>(encoding_constants::utf8_continuation_tag | (scalar & encoding_constants::utf8_continuation_payload_mask));
		return encoding_constants::max_utf8_code_units;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& std::is_convertible_v<char8_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf8(std::uint32_t scalar, CharT* out) noexcept
	{
		if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return 0;
		}
		return encode_unicode_scalar_utf8_unchecked(scalar, out);
	}

	template<typename CharT>
	inline constexpr bool is_single_valid_utf16_char(std::basic_string_view<CharT> value) noexcept
	{
		if (value.empty()) [[unlikely]]
		{
			return false;
		}

		const auto code_unit = [&value](std::size_t index) noexcept -> std::uint16_t
		{
			return static_cast<std::uint16_t>(value[index]);
		};

		const auto first = code_unit(0);
		const bool first_is_high_surrogate = first >= encoding_constants::high_surrogate_min && first <= encoding_constants::high_surrogate_max;
		const bool first_is_low_surrogate = first >= encoding_constants::low_surrogate_min && first <= encoding_constants::low_surrogate_max;

		if (value.size() == encoding_constants::single_code_unit_count) [[likely]]
		{
			return !first_is_high_surrogate && !first_is_low_surrogate;
		}

		if (value.size() == encoding_constants::utf16_surrogate_code_unit_count)
		{
			const auto second = code_unit(1);
			const bool second_is_low_surrogate = second >= encoding_constants::low_surrogate_min && second <= encoding_constants::low_surrogate_max;
			return first_is_high_surrogate && second_is_low_surrogate;
		}

		return false;
	}

	template<typename CharT>
	inline constexpr bool is_single_valid_utf32_char(std::basic_string_view<CharT> value) noexcept
	{
		return value.size() == encoding_constants::single_code_unit_count
			&& is_valid_unicode_scalar(static_cast<std::uint32_t>(value.front()));
	}

	inline constexpr bool is_utf16_high_surrogate(std::uint16_t value) noexcept
	{
		return value >= encoding_constants::high_surrogate_min && value <= encoding_constants::high_surrogate_max;
	}

	inline constexpr bool is_utf16_low_surrogate(std::uint16_t value) noexcept
	{
		return value >= encoding_constants::low_surrogate_min && value <= encoding_constants::low_surrogate_max;
	}

	inline constexpr bool is_utf32_high_surrogate(std::uint16_t) noexcept
	{
		return false;
	}

	inline constexpr bool is_utf32_low_surrogate(std::uint16_t) noexcept
	{
		return false;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char16_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf16_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		if (scalar <= encoding_constants::bmp_scalar_max) [[likely]]
		{
			out[0] = static_cast<CharT>(scalar);
			return encoding_constants::single_code_unit_count;
		}

		const auto shifted = scalar - encoding_constants::supplementary_plane_base;
		out[0] = static_cast<CharT>(encoding_constants::high_surrogate_min + (shifted >> encoding_constants::utf16_high_surrogate_shift));
		out[1] = static_cast<CharT>(encoding_constants::low_surrogate_min + (shifted & encoding_constants::surrogate_payload_mask));
		return encoding_constants::utf16_surrogate_code_unit_count;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char16_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf16(std::uint32_t scalar, CharT* out) noexcept
	{
		if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return 0;
		}
		return encode_unicode_scalar_utf16_unchecked(scalar, out);
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char32_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf32_unchecked(std::uint32_t scalar, CharT* out) noexcept
	{
		out[0] = static_cast<CharT>(scalar);
		return encoding_constants::single_code_unit_count;
	}

	template<typename CharT>
	requires (std::is_integral_v<CharT>
		&& !std::is_same_v<CharT, bool>
		&& non_narrowing_convertible<char32_t, CharT>)
	inline constexpr std::size_t encode_unicode_scalar_utf32(std::uint32_t scalar, CharT* out) noexcept
	{
		if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
		{
			return 0;
		}

		return encode_unicode_scalar_utf32_unchecked(scalar, out);
	}

	inline constexpr std::size_t encode_unicode_scalar_wchar_unchecked(std::uint32_t scalar, wchar_t* out) noexcept
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			return encode_unicode_scalar_utf16_unchecked(scalar, out);
		}
		else
		{
			out[0] = static_cast<wchar_t>(scalar);
			return encoding_constants::single_code_unit_count;
		}
	}

	inline constexpr std::size_t utf8_byte_count_from_lead(std::uint8_t lead) noexcept
	{
		if (lead <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			return encoding_constants::single_code_unit_count;
		}
		if ((lead & encoding_constants::utf8_two_byte_prefix_mask) == encoding_constants::utf8_two_byte_prefix_value)
		{
			return encoding_constants::two_code_unit_count;
		}
		if ((lead & encoding_constants::utf8_three_byte_prefix_mask) == encoding_constants::utf8_three_byte_prefix_value)
		{
			return encoding_constants::three_code_unit_count;
		}
		return encoding_constants::max_utf8_code_units;
	}

	inline constexpr bool is_utf8_lead_byte(std::uint8_t byte) noexcept
	{
		return (byte & encoding_constants::utf8_continuation_mask) != encoding_constants::utf8_continuation_tag;
	}

	inline constexpr bool is_utf8_continuation_byte(std::uint8_t byte) noexcept
	{
		return byte >= encoding_constants::utf8_continuation_min
			&& byte <= encoding_constants::utf8_continuation_max;
	}

	struct utf8_sequence_validation_result
	{
		std::size_t size = 0;
		utf8_error_code code = utf8_error_code::invalid_sequence;
	};

	template<typename CharT>
	inline constexpr auto validate_utf8_sequence_at_raw(
		std::basic_string_view<CharT> value,
		std::size_t index) noexcept -> utf8_sequence_validation_result
	{
		const std::uint8_t lead = static_cast<std::uint8_t>(value[index]);
		const auto traits = utf8_lead_validation_table[lead];
		const auto remaining = value.size() - index;
		const auto byte_at = [&](std::size_t offset) noexcept -> std::uint8_t
		{
			return static_cast<std::uint8_t>(value[index + offset]);
		};

		if (traits.size == 0) [[unlikely]]
		{
			return utf8_sequence_validation_result{
				.size = 0,
				.code = utf8_error_code::invalid_lead_byte
			};
		}

		if (remaining < traits.size) [[unlikely]]
		{
			return utf8_sequence_validation_result{
				.size = 0,
				.code = utf8_error_code::truncated_sequence
			};
		}

		if (traits.size == encoding_constants::single_code_unit_count)
		{
			return utf8_sequence_validation_result{
				.size = encoding_constants::single_code_unit_count
			};
		}

		const auto second = byte_at(1);
		if (second < traits.second_min || second > traits.second_max) [[unlikely]]
		{
			return utf8_sequence_validation_result{
				.size = 0,
				.code = utf8_error_code::invalid_sequence
			};
		}

		if (traits.size == encoding_constants::two_code_unit_count)
		{
			return utf8_sequence_validation_result{
				.size = encoding_constants::two_code_unit_count
			};
		}

		if (!is_utf8_continuation_byte(byte_at(2))) [[unlikely]]
		{
			return utf8_sequence_validation_result{
				.size = 0,
				.code = utf8_error_code::invalid_sequence
			};
		}

		if (traits.size == encoding_constants::three_code_unit_count)
		{
			return utf8_sequence_validation_result{
				.size = encoding_constants::three_code_unit_count
			};
		}

		if (!is_utf8_continuation_byte(byte_at(3))) [[unlikely]]
		{
			return utf8_sequence_validation_result{
				.size = 0,
				.code = utf8_error_code::invalid_sequence
			};
		}

		return utf8_sequence_validation_result{
			.size = encoding_constants::max_utf8_code_units
		};
	}

	template<typename CharT>
	inline constexpr auto validate_utf8_sequence_at(
		std::basic_string_view<CharT> value,
		std::size_t index) noexcept -> std::expected<std::size_t, utf8_error>
	{
		const auto result = validate_utf8_sequence_at_raw(value, index);
		if (result.size != 0) [[likely]]
		{
			return result.size;
		}

		return std::unexpected(utf8_error{
			.code = result.code,
			.first_invalid_byte_index = index
		});
	}

	template<typename CharT>
	inline constexpr std::size_t lossy_utf8_sequence_width(std::basic_string_view<CharT> value) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(!value.empty());

		const std::uint8_t lead = static_cast<std::uint8_t>(value.front());
		const auto traits = utf8_lead_validation_table[lead];
		if (traits.size <= encoding_constants::single_code_unit_count)
		{
			return encoding_constants::single_code_unit_count;
		}

		std::size_t width = encoding_constants::single_code_unit_count;
		if (value.size() == width)
		{
			return width;
		}

		const auto second = static_cast<std::uint8_t>(value[1]);
		if (second < traits.second_min || second > traits.second_max)
		{
			return width;
		}

		width = encoding_constants::two_code_unit_count;
		if (traits.size == width || value.size() == width)
		{
			return width;
		}

		if (!is_utf8_continuation_byte(static_cast<std::uint8_t>(value[2])))
		{
			return width;
		}

		width = encoding_constants::three_code_unit_count;
		if (traits.size == width || value.size() == width)
		{
			return width;
		}

		if (!is_utf8_continuation_byte(static_cast<std::uint8_t>(value[3])))
		{
			return width;
		}

		return encoding_constants::max_utf8_code_units;
	}

	template<typename CharT>
	inline constexpr auto validate_utf16_sequence_at(
		std::basic_string_view<CharT> value,
		std::size_t index) noexcept -> std::expected<std::size_t, utf16_error>
	{
		const auto first = static_cast<std::uint16_t>(value[index]);
		if (!is_utf16_high_surrogate(first) && !is_utf16_low_surrogate(first)) [[likely]]
		{
			return encoding_constants::single_code_unit_count;
		}

		if (is_utf16_low_surrogate(first))
		{
			return std::unexpected(utf16_error{
				.code = utf16_error_code::invalid_sequence,
				.first_invalid_code_unit_index = index
			});
		}

		if (index + encoding_constants::single_code_unit_count >= value.size()) [[unlikely]]
		{
			return std::unexpected(utf16_error{
				.code = utf16_error_code::truncated_surrogate_pair,
				.first_invalid_code_unit_index = index
			});
		}

		const auto second = static_cast<std::uint16_t>(value[index + encoding_constants::single_code_unit_count]);
		if (!is_utf16_low_surrogate(second))
		{
			return std::unexpected(utf16_error{
				.code = utf16_error_code::invalid_sequence,
				.first_invalid_code_unit_index = index
			});
		}

		return encoding_constants::utf16_surrogate_code_unit_count;
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(const CharT* ch, std::size_t size) noexcept;

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(std::basic_string_view<CharT> ch) noexcept;

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(const CharT* ch, std::size_t size) noexcept;

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(std::basic_string_view<CharT> ch) noexcept;

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf32_char(const CharT* ch) noexcept;

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf32_char(std::basic_string_view<CharT> ch) noexcept;

	template<typename CharT>
	inline constexpr std::expected<void, utf8_error> validate_utf8(std::basic_string_view<CharT> value) noexcept
	{
		if (!std::is_constant_evaluated())
		{
			if constexpr (sizeof(CharT) == 1)
			{
				if (internal::simdutf_haswell_runtime_available())
				{
					if (internal::simdutf_haswell_validation::validate_utf8(
						reinterpret_cast<const char*>(value.data()),
						value.size()))
					{
						return {};
					}
				}

				return internal::simdutf_scalar_validate_utf8(value);
			}
		}

		std::size_t index = 0;
		while (index < value.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ value.data() + index, value.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			index += ascii_run;
			if (index == value.size())
			{
				break;
			}

			const auto sequence = validate_utf8_sequence_at_raw(value, index);
			if (sequence.size == 0) [[unlikely]]
			{
				return std::unexpected(utf8_error{
					.code = sequence.code,
					.first_invalid_byte_index = index
				});
			}

			index += sequence.size;
		}

		return {};
	}

	template<typename CharT>
	inline constexpr std::expected<void, utf16_error> validate_utf16(std::basic_string_view<CharT> value) noexcept
	{
		std::size_t index = 0;
		while (index < value.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ value.data() + index, value.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			index += ascii_run;
			if (index == value.size())
			{
				break;
			}

			const auto sequence_length = validate_utf16_sequence_at(value, index);
			if (!sequence_length) [[unlikely]]
			{
				return std::unexpected(sequence_length.error());
			}

			index += *sequence_length;
		}

		return {};
	}

	template<typename CharT>
	inline constexpr std::expected<void, utf32_error> validate_utf32(std::basic_string_view<CharT> value) noexcept
	{
		for (std::size_t index = 0; index != value.size(); ++index)
		{
			if (!is_valid_unicode_scalar(static_cast<std::uint32_t>(value[index])))
			{
				return std::unexpected(utf32_error{
					.code = utf32_error_code::invalid_scalar,
					.first_invalid_code_point_index = index
				});
			}
		}

		return {};
	}

	template<typename CharT>
	requires (sizeof(CharT) >= 4)
	inline constexpr std::expected<void, unicode_scalar_error> validate_unicode_scalars(std::basic_string_view<CharT> value) noexcept
	{
		for (std::size_t index = 0; index != value.size(); ++index)
		{
			if (!is_valid_unicode_scalar(static_cast<std::uint32_t>(value[index])))
			{
				return std::unexpected(unicode_scalar_error{
					.code = unicode_scalar_error_code::invalid_scalar,
					.first_invalid_element_index = index
				});
			}
		}

		return {};
	}

	inline constexpr std::expected<void, unicode_scalar_error> validate_unicode_scalars(std::wstring_view value) noexcept
	{
		for (std::size_t index = 0; index != value.size(); ++index)
		{
			if (!is_valid_unicode_scalar(static_cast<std::uint32_t>(value[index])))
			{
				return std::unexpected(unicode_scalar_error{
					.code = unicode_scalar_error_code::invalid_scalar,
					.first_invalid_element_index = index
				});
			}
		}

		return {};
	}

	inline constexpr std::expected<void, unicode_scalar_error> validate_unicode_scalars(std::u32string_view value) noexcept
	{
		return validate_unicode_scalars<char32_t>(value);
	}

	template<typename CharT>
	inline constexpr auto analyze_lossy_utf8(std::basic_string_view<CharT> value) noexcept
		-> std::pair<std::size_t, bool>
	{
		std::size_t output_size = 0;
		bool has_expanding_sequence = false;
		std::size_t index = 0;
		while (index < value.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ value.data() + index, value.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			output_size += ascii_run;
			index += ascii_run;
			if (index == value.size())
			{
				break;
			}

			const auto width = lossy_utf8_sequence_width(remaining);
			const auto traits = utf8_lead_validation_table[static_cast<std::uint8_t>(value[index])];
			if (traits.size != 0 && width == traits.size)
			{
				output_size += width;
			}
			else
			{
				output_size += encoding_constants::three_code_unit_count;
				has_expanding_sequence = has_expanding_sequence || width < encoding_constants::three_code_unit_count;
			}

			index += width;
		}

		return { output_size, has_expanding_sequence };
	}

	template<typename CharT>
	inline constexpr std::size_t write_lossy_utf8_bytes(
		std::basic_string_view<CharT> bytes,
		char8_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		std::size_t read_index = 0;
		while (read_index < bytes.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ bytes.data() + read_index, bytes.size() - read_index };
			const auto ascii_run = ascii_prefix_length(remaining);
			for (std::size_t i = 0; i != ascii_run; ++i)
			{
				buffer[write_index + i] = static_cast<char8_t>(remaining[i]);
			}

			write_index += ascii_run;
			read_index += ascii_run;
			if (read_index == bytes.size())
			{
				break;
			}

			const auto width = lossy_utf8_sequence_width(remaining);
			const auto traits = utf8_lead_validation_table[static_cast<std::uint8_t>(bytes[read_index])];
			if (traits.size != 0 && width == traits.size)
			{
				for (std::size_t i = 0; i != width; ++i)
				{
					buffer[write_index + i] = static_cast<char8_t>(bytes[read_index + i]);
				}

				write_index += width;
			}
			else
			{
				write_index += encode_unicode_scalar_utf8_unchecked(
					encoding_constants::replacement_character_scalar,
					buffer + write_index);
			}

			read_index += width;
		}

		return write_index;
	}

	template<typename CharT>
	inline constexpr std::size_t write_lossy_utf16_code_units(
		std::basic_string_view<CharT> code_units,
		char16_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		std::size_t read_index = 0;
		while (read_index < code_units.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ code_units.data() + read_index, code_units.size() - read_index };
			const auto ascii_run = ascii_prefix_length(remaining);
			for (std::size_t i = 0; i != ascii_run; ++i)
			{
				buffer[write_index + i] = static_cast<char16_t>(remaining[i]);
			}

			write_index += ascii_run;
			read_index += ascii_run;
			if (read_index == code_units.size())
			{
				break;
			}

			const auto first = static_cast<std::uint16_t>(code_units[read_index]);
			if (first < 0xD800u || first > 0xDFFFu)
			{
				buffer[write_index++] = static_cast<char16_t>(first);
				++read_index;
				continue;
			}

			if (first <= 0xDBFFu && read_index + 1 < code_units.size())
			{
				const auto second = static_cast<std::uint16_t>(code_units[read_index + 1]);
				if (is_utf16_low_surrogate(second))
				{
					buffer[write_index++] = static_cast<char16_t>(first);
					buffer[write_index++] = static_cast<char16_t>(second);
					read_index += encoding_constants::utf16_surrogate_code_unit_count;
					continue;
				}
			}

			buffer[write_index++] = static_cast<char16_t>(encoding_constants::replacement_character_scalar);
			++read_index;
		}

		return write_index;
	}

	template<typename CharT>
	inline constexpr std::size_t write_lossy_utf32_code_points(
		std::basic_string_view<CharT> code_points,
		char32_t* buffer) noexcept
	{
		for (std::size_t index = 0; index != code_points.size(); ++index)
		{
			const auto scalar = static_cast<std::uint32_t>(code_points[index]);
			buffer[index] = is_valid_unicode_scalar(scalar)
				? static_cast<char32_t>(scalar)
				: static_cast<char32_t>(encoding_constants::replacement_character_scalar);
		}

		return code_points.size();
	}

	template <typename Allocator, typename CharT>
	inline constexpr auto copy_lossy_utf8_bytes(
		std::basic_string_view<CharT> bytes,
		const Allocator& alloc) -> utf8_base_string<Allocator>
	{
		utf8_base_string<Allocator> result{ alloc };
		const auto [output_size, _] = analyze_lossy_utf8(bytes);
		result.resize_and_overwrite(output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				return write_lossy_utf8_bytes(bytes, buffer);
			});

		return result;
	}

	template <typename Allocator>
	inline constexpr void repair_utf8_bytes_inplace(utf8_base_string<Allocator>& bytes)
	{
		const auto original_size = bytes.size();
		const auto [output_size, has_expanding_sequence] = analyze_lossy_utf8(std::u8string_view{ bytes });
		if (!has_expanding_sequence)
		{
			bytes.resize_and_overwrite(original_size,
				[&](char8_t* buffer, std::size_t) noexcept
				{
					return write_lossy_utf8_bytes(std::u8string_view{ buffer, original_size }, buffer);
				});
			return;
		}

		if (output_size > original_size)
		{
			bytes.resize_and_overwrite(output_size,
				[&](char8_t* buffer, std::size_t) noexcept
				{
					auto* shifted_input = buffer + (output_size - original_size);
					std::char_traits<char8_t>::move(shifted_input, buffer, original_size);
					return write_lossy_utf8_bytes(std::u8string_view{ shifted_input, original_size }, buffer);
				});
			return;
		}

		bytes = copy_lossy_utf8_bytes(std::u8string_view{ bytes }, bytes.get_allocator());
	}

	template <typename Allocator>
	inline constexpr auto copy_lossy_utf16_code_units(
		std::u16string_view code_units,
		const Allocator& alloc) -> utf16_base_string<Allocator>
	{
		utf16_base_string<Allocator> result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				return write_lossy_utf16_code_units(code_units, buffer);
			});

		return result;
	}

	template <typename Allocator>
	inline constexpr void repair_utf16_code_units_inplace(utf16_base_string<Allocator>& code_units)
	{
		const auto original_size = code_units.size();
		code_units.resize_and_overwrite(original_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				return write_lossy_utf16_code_units(std::u16string_view{ buffer, original_size }, buffer);
			});
	}

	template <typename Allocator>
	inline constexpr auto copy_lossy_utf32_code_points(
		std::u32string_view code_points,
		const Allocator& alloc) -> utf32_base_string<Allocator>
	{
		utf32_base_string<Allocator> result{ alloc };
		result.resize_and_overwrite(code_points.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				return write_lossy_utf32_code_points(code_points, buffer);
			});

		return result;
	}

	template <typename Allocator>
	inline constexpr void repair_utf32_code_points_inplace(utf32_base_string<Allocator>& code_points)
	{
		const auto original_size = code_points.size();
		code_points.resize_and_overwrite(original_size,
			[&](char32_t* buffer, std::size_t) noexcept
			{
				return write_lossy_utf32_code_points(std::u32string_view{ buffer, original_size }, buffer);
			});
	}

	template <typename Allocator>
	inline constexpr auto copy_validated_utf8_bytes(
		std::string_view bytes,
		const Allocator& alloc) -> std::expected<utf8_base_string<Allocator>, utf8_error>
	{
		utf8_base_string<Allocator> result{ alloc };
		std::optional<utf8_error> error;
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						copy_ascii_bytes_to_utf8(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto sequence = validate_utf8_sequence_at_raw(bytes, read_index);
					if (sequence.size == 0) [[unlikely]]
					{
						error = utf8_error{
							.code = sequence.code,
							.first_invalid_byte_index = read_index
						};
						return std::size_t{ 0 };
					}

					for (std::size_t i = 0; i != sequence.size; ++i)
					{
						buffer[write_index + i] = static_cast<char8_t>(bytes[read_index + i]);
					}

					write_index += sequence.size;
					read_index += sequence.size;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <bool Validate, typename CharT>
	requires (sizeof(CharT) == 1)
	inline constexpr std::size_t transcode_utf8_to_utf16_runtime_kernel(
		std::basic_string_view<CharT> bytes,
		char16_t* buffer,
		std::optional<utf8_error>* error) noexcept
	{
		std::size_t write_index = 0;
		std::size_t read_index = 0;
		while (read_index < bytes.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ bytes.data() + read_index, bytes.size() - read_index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				if constexpr (std::is_same_v<CharT, char8_t>)
				{
					const auto ascii_bytes = std::u8string_view{ bytes.data() + read_index, ascii_run };
					if (ascii_run < 16) [[unlikely]]
					{
						copy_ascii_utf8_to_utf16_scalar(buffer + write_index, ascii_bytes);
					}
					else
					{
						copy_ascii_utf8_to_utf16(buffer + write_index, ascii_bytes);
					}
				}
				else
				{
					copy_ascii_bytes_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
				}
				write_index += ascii_run;
				read_index += ascii_run;
				continue;
			}

			std::size_t count = 0;
			if constexpr (Validate)
			{
				const auto sequence = validate_utf8_sequence_at_raw(bytes, read_index);
				if (sequence.size == 0) [[unlikely]]
				{
					UTF8_RANGES_DEBUG_ASSERT(error != nullptr);
					*error = utf8_error{
						.code = sequence.code,
						.first_invalid_byte_index = read_index
					};
					return std::size_t{ 0 };
				}

				count = sequence.size;
			}
			else
			{
				count = utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
			}

			const auto scalar = decode_valid_utf8_char(bytes.data() + read_index, count);
			write_index += encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
			read_index += count;
		}

		return write_index;
	}

	template <typename CharT, typename Allocator>
	requires (sizeof(CharT) == 1)
	inline constexpr auto transcode_valid_utf8_to_utf16_unchecked(
		std::basic_string_view<CharT> bytes,
		const Allocator& alloc) -> utf16_base_string<Allocator>
	{
		utf16_base_string<Allocator> result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				if (!std::is_constant_evaluated())
				{
					if constexpr (std::is_same_v<CharT, char8_t>)
					{
						if (internal::simdutf_haswell_runtime_available())
						{
							return internal::simdutf_haswell_utf8_to_utf16::convert_valid(
								std::u8string_view{ bytes.data(), bytes.size() },
								buffer);
						}
					}

					return transcode_utf8_to_utf16_runtime_kernel<false>(bytes, buffer, nullptr);
				}

				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::basic_string_view<CharT>{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (std::is_same_v<CharT, char8_t>)
						{
							const auto ascii_bytes = std::u8string_view{ bytes.data() + read_index, ascii_run };
							if (ascii_run < 16) [[unlikely]]
							{
								copy_ascii_utf8_to_utf16_scalar(buffer + write_index, ascii_bytes);
							}
							else
							{
								copy_ascii_utf8_to_utf16(buffer + write_index, ascii_bytes);
							}
						}
						else
						{
							copy_ascii_bytes_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
						}
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto count = utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = decode_valid_utf8_char(bytes.data() + read_index, count);
					write_index += encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_utf8_to_utf16_checked(
		std::string_view bytes,
		const Allocator& alloc) -> std::expected<utf16_base_string<Allocator>, utf8_error>
	{
		utf16_base_string<Allocator> result{ alloc };
		std::optional<utf8_error> error;
		result.resize_and_overwrite(bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				if (!std::is_constant_evaluated())
				{
					if (internal::simdutf_haswell_runtime_available())
					{
						if (const auto validation = validate_utf8(bytes); !validation) [[unlikely]]
						{
							error = validation.error();
							return std::size_t{ 0 };
						}

						return internal::simdutf_haswell_utf8_to_utf16::convert_valid(
							std::u8string_view{
								reinterpret_cast<const char8_t*>(bytes.data()),
								bytes.size()
							},
							buffer);
					}

					return transcode_utf8_to_utf16_runtime_kernel<true>(bytes, buffer, &error);
				}

				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						copy_ascii_bytes_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto sequence = validate_utf8_sequence_at_raw(bytes, read_index);
					if (sequence.size == 0) [[unlikely]]
					{
						error = utf8_error{
							.code = sequence.code,
							.first_invalid_byte_index = read_index
						};
						return std::size_t{ 0 };
					}

					const auto scalar = decode_valid_utf8_char(bytes.data() + read_index, sequence.size);
					write_index += encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += sequence.size;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_utf16_to_utf8_checked(
		std::wstring_view code_units,
		const Allocator& alloc) -> std::expected<utf8_base_string<Allocator>, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		utf8_base_string<Allocator> result{ alloc };
		std::optional<utf16_error> error;
		result.resize_and_overwrite(code_units.size() * encoding_constants::three_code_unit_count,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto remaining = std::wstring_view{ code_units.data() + read_index, code_units.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						copy_ascii_code_units_to_utf8(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto sequence_length = validate_utf16_sequence_at(code_units, read_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					const auto scalar = decode_valid_utf16_char(code_units.data() + read_index, *sequence_length);
					write_index += encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
					read_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto copy_validated_utf16_code_units(
		std::wstring_view code_units,
		const Allocator& alloc) -> std::expected<utf16_base_string<Allocator>, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		utf16_base_string<Allocator> result{ alloc };
		std::optional<utf16_error> error;
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				while (write_index < code_units.size())
				{
					const auto remaining = std::wstring_view{ code_units.data() + write_index, code_units.size() - write_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						for (std::size_t i = 0; i != ascii_run; ++i)
						{
							buffer[write_index + i] = static_cast<char16_t>(code_units[write_index + i]);
						}

						write_index += ascii_run;
						continue;
					}

					const auto sequence_length = validate_utf16_sequence_at(code_units, write_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					for (std::size_t i = 0; i != *sequence_length; ++i)
					{
						buffer[write_index + i] = static_cast<char16_t>(code_units[write_index + i]);
					}

					write_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <bool Validate, typename CharT>
	requires (sizeof(CharT) == 1)
	inline constexpr std::size_t transcode_utf8_to_utf32_runtime_kernel(
		std::basic_string_view<CharT> bytes,
		char32_t* buffer,
		std::optional<utf8_error>* error) noexcept
	{
		std::size_t write_index = 0;
		std::size_t read_index = 0;
		while (read_index < bytes.size())
		{
			const auto remaining = std::basic_string_view<CharT>{ bytes.data() + read_index, bytes.size() - read_index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				for (std::size_t i = 0; i != ascii_run; ++i)
				{
					buffer[write_index + i] = static_cast<char32_t>(static_cast<std::uint8_t>(remaining[i]));
				}

				write_index += ascii_run;
				read_index += ascii_run;
				continue;
			}

			std::size_t count = 0;
			if constexpr (Validate)
			{
				const auto sequence = validate_utf8_sequence_at_raw(bytes, read_index);
				if (sequence.size == 0) [[unlikely]]
				{
					UTF8_RANGES_DEBUG_ASSERT(error != nullptr);
					*error = utf8_error{
						.code = sequence.code,
						.first_invalid_byte_index = read_index
					};
					return std::size_t{ 0 };
				}

				count = sequence.size;
			}
			else
			{
				count = utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
			}

			buffer[write_index++] = static_cast<char32_t>(decode_valid_utf8_char(bytes.data() + read_index, count));
			read_index += count;
		}

		return write_index;
	}

	inline constexpr std::size_t transcode_valid_utf8_to_utf32_runtime_u8(
		std::u8string_view bytes,
		char32_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		std::size_t read_index = 0;
		while (read_index < bytes.size())
		{
			const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				copy_ascii_utf8_to_utf32_scalar(buffer + write_index, remaining.substr(0, ascii_run));
				write_index += ascii_run;
				read_index += ascii_run;
				continue;
			}

			const auto byte = [&](std::size_t offset) noexcept -> std::uint32_t
			{
				return static_cast<std::uint8_t>(bytes[read_index + offset]);
			};

			const auto lead = byte(0);
			if ((lead & encoding_constants::utf8_two_byte_prefix_mask) == encoding_constants::utf8_two_byte_prefix_value)
			{
				buffer[write_index++] =
					((lead & encoding_constants::utf8_two_byte_payload_mask) << encoding_constants::utf8_two_byte_lead_shift) |
					(byte(1) & encoding_constants::utf8_continuation_payload_mask);
				read_index += encoding_constants::two_code_unit_count;
				continue;
			}

			if ((lead & encoding_constants::utf8_three_byte_prefix_mask) == encoding_constants::utf8_three_byte_prefix_value)
			{
				buffer[write_index++] =
					((lead & encoding_constants::utf8_three_byte_payload_mask) << encoding_constants::utf8_three_byte_lead_shift) |
					((byte(1) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_continuation_payload_bits) |
					(byte(2) & encoding_constants::utf8_continuation_payload_mask);
				read_index += encoding_constants::three_code_unit_count;
				continue;
			}

			buffer[write_index++] =
				((lead & encoding_constants::utf8_four_byte_payload_mask) << encoding_constants::utf8_four_byte_lead_shift) |
				((byte(1) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_three_byte_lead_shift) |
				((byte(2) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_continuation_payload_bits) |
				(byte(3) & encoding_constants::utf8_continuation_payload_mask);
			read_index += encoding_constants::max_utf8_code_units;
		}

		return write_index;
	}

	template <typename CharT, typename Allocator>
	requires (sizeof(CharT) == 1)
	inline constexpr auto transcode_valid_utf8_to_utf32_unchecked(
		std::basic_string_view<CharT> bytes,
		const Allocator& alloc) -> utf32_base_string<Allocator>
	{
		utf32_base_string<Allocator> result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				if (!std::is_constant_evaluated())
				{
					if constexpr (std::is_same_v<CharT, char8_t>)
					{
						if (internal::simdutf_haswell_runtime_available())
						{
							return internal::simdutf_haswell_utf8_to_utf32::convert_valid(
								std::u8string_view{ bytes.data(), bytes.size() },
								buffer);
						}

						return transcode_valid_utf8_to_utf32_runtime_u8(
							std::u8string_view{ bytes.data(), bytes.size() },
							buffer);
					}

					return transcode_utf8_to_utf32_runtime_kernel<false>(bytes, buffer, nullptr);
				}

				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::basic_string_view<CharT>{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						for (std::size_t i = 0; i != ascii_run; ++i)
						{
							buffer[write_index + i] = static_cast<char32_t>(static_cast<std::uint8_t>(remaining[i]));
						}

						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto count = utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					buffer[write_index++] = static_cast<char32_t>(decode_valid_utf8_char(bytes.data() + read_index, count));
					read_index += count;
				}

				return write_index;
			});

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_utf8_to_utf32_checked(
		std::string_view bytes,
		const Allocator& alloc) -> std::expected<utf32_base_string<Allocator>, utf8_error>
	{
		utf32_base_string<Allocator> result{ alloc };
		std::optional<utf8_error> error;
		result.resize_and_overwrite(bytes.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				if (!std::is_constant_evaluated())
				{
					if (internal::simdutf_haswell_runtime_available())
					{
						if (const auto validation = validate_utf8(bytes); !validation) [[unlikely]]
						{
							error = validation.error();
							return std::size_t{ 0 };
						}

						return internal::simdutf_haswell_utf8_to_utf32::convert_valid(
							std::u8string_view{
								reinterpret_cast<const char8_t*>(bytes.data()),
								bytes.size()
							},
							buffer);
					}

					return transcode_utf8_to_utf32_runtime_kernel<true>(bytes, buffer, &error);
				}

				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						for (std::size_t i = 0; i != ascii_run; ++i)
						{
							buffer[write_index + i] = static_cast<char32_t>(static_cast<unsigned char>(remaining[i]));
						}

						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto sequence_length = validate_utf8_sequence_at(bytes, read_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					buffer[write_index++] = static_cast<char32_t>(decode_valid_utf8_char(bytes.data() + read_index, *sequence_length));
					read_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_utf16_to_utf32_checked(
		std::wstring_view code_units,
		const Allocator& alloc) -> std::expected<utf32_base_string<Allocator>, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		utf32_base_string<Allocator> result{ alloc };
		std::optional<utf16_error> error;
		result.resize_and_overwrite(code_units.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto remaining = std::wstring_view{ code_units.data() + read_index, code_units.size() - read_index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						for (std::size_t i = 0; i != ascii_run; ++i)
						{
							buffer[write_index + i] = static_cast<char32_t>(remaining[i]);
						}

						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto sequence_length = validate_utf16_sequence_at(code_units, read_index);
					if (!sequence_length) [[unlikely]]
					{
						error = sequence_length.error();
						return std::size_t{ 0 };
					}

					buffer[write_index++] = static_cast<char32_t>(decode_valid_utf16_char(code_units.data() + read_index, *sequence_length));
					read_index += *sequence_length;
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto copy_validated_utf32_code_points(
		std::u32string_view code_points,
		const Allocator& alloc) -> std::expected<utf32_base_string<Allocator>, utf32_error>
	{
		if (auto validation = validate_utf32(code_points); !validation)
		{
			return std::unexpected(validation.error());
		}

		return utf32_base_string<Allocator>{ code_points, alloc };
	}

	template <typename Allocator>
	inline constexpr auto transcode_unicode_scalars_to_utf8_checked(
		std::wstring_view scalars,
		const Allocator& alloc) -> std::expected<utf8_base_string<Allocator>, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		utf8_base_string<Allocator> result{ alloc };
		std::optional<unicode_scalar_error> error;
		result.resize_and_overwrite(scalars.size() * encoding_constants::max_utf8_code_units,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t read_index = 0; read_index != scalars.size(); ++read_index)
				{
					const auto scalar = static_cast<std::uint32_t>(scalars[read_index]);
					if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
					{
						error = unicode_scalar_error{
							.code = unicode_scalar_error_code::invalid_scalar,
							.first_invalid_element_index = read_index
						};
						return std::size_t{ 0 };
					}

					write_index += encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_unicode_scalars_to_utf16_checked(
		std::wstring_view scalars,
		const Allocator& alloc) -> std::expected<utf16_base_string<Allocator>, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		utf16_base_string<Allocator> result{ alloc };
		std::optional<unicode_scalar_error> error;
		result.resize_and_overwrite(scalars.size() * encoding_constants::utf16_surrogate_code_unit_count,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t read_index = 0; read_index != scalars.size(); ++read_index)
				{
					const auto scalar = static_cast<std::uint32_t>(scalars[read_index]);
					if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
					{
						error = unicode_scalar_error{
							.code = unicode_scalar_error_code::invalid_scalar,
							.first_invalid_element_index = read_index
						};
						return std::size_t{ 0 };
					}

					write_index += encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template <typename Allocator>
	inline constexpr auto transcode_unicode_scalars_to_utf32_checked(
		std::wstring_view scalars,
		const Allocator& alloc) -> std::expected<utf32_base_string<Allocator>, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		utf32_base_string<Allocator> result{ alloc };
		std::optional<unicode_scalar_error> error;
		result.resize_and_overwrite(scalars.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t read_index = 0; read_index != scalars.size(); ++read_index)
				{
					const auto scalar = static_cast<std::uint32_t>(scalars[read_index]);
					if (!is_valid_unicode_scalar(scalar)) [[unlikely]]
					{
						error = unicode_scalar_error{
							.code = unicode_scalar_error_code::invalid_scalar,
							.first_invalid_element_index = read_index
						};
						return std::size_t{ 0 };
					}

					buffer[write_index++] = static_cast<char32_t>(scalar);
				}

				return write_index;
			});

		if (error) [[unlikely]]
		{
			return std::unexpected(*error);
		}

		return result;
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(const CharT* ch, std::size_t size) noexcept
	{
		const auto byte = [ch](std::size_t index) noexcept -> std::uint8_t
		{
			return static_cast<std::uint8_t>(ch[index]);
		};

		if (size == encoding_constants::single_code_unit_count) [[likely]]
		{
			return byte(0);
		}

		if (size == encoding_constants::two_code_unit_count)
		{
			return (static_cast<std::uint32_t>(byte(0) & encoding_constants::utf8_two_byte_payload_mask) << encoding_constants::utf8_two_byte_lead_shift) |
				(static_cast<std::uint32_t>(byte(1) & encoding_constants::utf8_continuation_payload_mask));
		}

		if (size == encoding_constants::three_code_unit_count)
		{
			return (static_cast<std::uint32_t>(byte(0) & encoding_constants::utf8_three_byte_payload_mask) << encoding_constants::utf8_three_byte_lead_shift) |
				(static_cast<std::uint32_t>(byte(1) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_continuation_payload_bits) |
				(static_cast<std::uint32_t>(byte(2) & encoding_constants::utf8_continuation_payload_mask));
		}

		return (static_cast<std::uint32_t>(byte(0) & encoding_constants::utf8_four_byte_payload_mask) << encoding_constants::utf8_four_byte_lead_shift) |
			(static_cast<std::uint32_t>(byte(1) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_three_byte_lead_shift) |
			(static_cast<std::uint32_t>(byte(2) & encoding_constants::utf8_continuation_payload_mask) << encoding_constants::utf8_continuation_payload_bits) |
			(static_cast<std::uint32_t>(byte(3) & encoding_constants::utf8_continuation_payload_mask));
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf8_char(std::basic_string_view<CharT> ch) noexcept
	{
		return decode_valid_utf8_char(ch.data(), ch.size());
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(const CharT* ch, std::size_t size) noexcept
	{
		if (size == encoding_constants::single_code_unit_count) [[likely]]
		{
			return static_cast<std::uint16_t>(ch[0]);
		}

		const auto high = static_cast<std::uint16_t>(ch[0]) - encoding_constants::high_surrogate_min;
		const auto low = static_cast<std::uint16_t>(ch[1]) - encoding_constants::low_surrogate_min;
		return encoding_constants::supplementary_plane_base + (static_cast<std::uint32_t>(high) << encoding_constants::utf16_high_surrogate_shift) + low;
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf16_char(std::basic_string_view<CharT> ch) noexcept
	{
		return decode_valid_utf16_char(ch.data(), ch.size());
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf32_char(const CharT* ch) noexcept
	{
		return static_cast<std::uint32_t>(ch[0]);
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_valid_utf32_char(std::basic_string_view<CharT> ch) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(ch.size() == encoding_constants::single_code_unit_count);
		return decode_valid_utf32_char(ch.data());
	}

	struct decoded_scalar
	{
		std::uint32_t scalar = 0;
		std::size_t next_index = 0;
	};

	template<typename CharT>
	inline constexpr std::uint32_t decode_next_utf8_scalar(const CharT*& cursor) noexcept
	{
		const auto* const current = cursor;
		const auto length = utf8_byte_count_from_lead(static_cast<std::uint8_t>(*current));
		cursor += length;
		return decode_valid_utf8_char(current, length);
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_next_utf16_scalar(const CharT*& cursor) noexcept
	{
		const auto* const current = cursor;
		const auto length = is_utf16_high_surrogate(static_cast<std::uint16_t>(*current))
			? encoding_constants::utf16_surrogate_code_unit_count
			: encoding_constants::single_code_unit_count;
		cursor += length;
		return decode_valid_utf16_char(current, length);
	}

	template<typename CharT>
	inline constexpr std::uint32_t decode_next_utf32_scalar(const CharT*& cursor) noexcept
	{
		const auto* const current = cursor;
		cursor += encoding_constants::single_code_unit_count;
		return decode_valid_utf32_char(current);
	}

	inline constexpr decoded_scalar decode_next_scalar(std::u8string_view text, std::size_t index) noexcept
	{
		const auto* cursor = text.data() + index;
		const auto scalar = decode_next_utf8_scalar(cursor);
		return decoded_scalar{
			.scalar = scalar,
			.next_index = static_cast<std::size_t>(cursor - text.data())
		};
	}

	inline constexpr decoded_scalar decode_next_scalar(std::u16string_view text, std::size_t index) noexcept
	{
		const auto* cursor = text.data() + index;
		const auto scalar = decode_next_utf16_scalar(cursor);
		return decoded_scalar{
			.scalar = scalar,
			.next_index = static_cast<std::size_t>(cursor - text.data())
		};
	}

	inline constexpr decoded_scalar decode_next_scalar(std::u32string_view text, std::size_t index) noexcept
	{
		const auto* cursor = text.data() + index;
		const auto scalar = decode_next_utf32_scalar(cursor);
		return decoded_scalar{
			.scalar = scalar,
			.next_index = static_cast<std::size_t>(cursor - text.data())
		};
	}

		enum class grapheme_emoji_suffix_state
		{
			none,
			extended_pictographic,
			extended_pictographic_extend,
			extended_pictographic_extend_zwj
		};

		enum class grapheme_indic_suffix_state
		{
			none,
			consonant_no_linker,
			consonant_linker
		};

		struct grapheme_state
		{
			unicode::grapheme_cluster_break_property previous_break = unicode::grapheme_cluster_break_property::other;
			std::size_t trailing_regional_indicator_count = 0;
			grapheme_emoji_suffix_state emoji_suffix = grapheme_emoji_suffix_state::none;
			grapheme_indic_suffix_state indic_suffix = grapheme_indic_suffix_state::none;
		};

		struct grapheme_scalar_info
		{
			std::uint32_t scalar = 0;
			unicode::grapheme_cluster_break_property break_property = unicode::grapheme_cluster_break_property::other;
			unicode::indic_conjunct_break_property indic_property = unicode::indic_conjunct_break_property::none;
			bool extended_pictographic = false;
		};

		inline constexpr bool is_grapheme_control(unicode::grapheme_cluster_break_property value) noexcept
		{
			using enum unicode::grapheme_cluster_break_property;
			return value == cr || value == lf || value == control;
		}

		inline constexpr unicode::grapheme_cluster_break_property ascii_grapheme_break_property(
			std::uint32_t scalar) noexcept
		{
			using enum unicode::grapheme_cluster_break_property;
			if (scalar == static_cast<std::uint32_t>('\r'))
			{
				return cr;
			}

			if (scalar == static_cast<std::uint32_t>('\n'))
			{
				return lf;
			}

			return scalar < 0x20u || scalar == 0x7Fu ? control : other;
		}

		inline constexpr grapheme_scalar_info classify_grapheme_scalar(std::uint32_t scalar) noexcept
		{
			const auto properties = unicode::grapheme_properties(scalar);
			return grapheme_scalar_info{
				.scalar = scalar,
				.break_property = properties.break_property,
				.indic_property = properties.indic_property,
				.extended_pictographic = properties.extended_pictographic
			};
		}

		inline constexpr grapheme_scalar_info decode_next_grapheme_utf8_scalar(const char8_t*& cursor) noexcept
		{
			// Fold decode + classify into one helper so grapheme algorithms only branch
			// once per scalar, with a very cheap ASCII fast path.
			const auto scalar = decode_next_utf8_scalar(cursor);
			if (scalar < 0x80u) [[likely]]
			{
				return grapheme_scalar_info{
					.scalar = scalar,
					.break_property = ascii_grapheme_break_property(scalar),
					.indic_property = unicode::indic_conjunct_break_property::none,
					.extended_pictographic = false
				};
			}

			return classify_grapheme_scalar(scalar);
		}

		inline constexpr grapheme_scalar_info decode_next_grapheme_utf16_scalar(const char16_t*& cursor) noexcept
		{
			// Fold decode + classify into one helper so grapheme algorithms only branch
			// once per scalar, with a very cheap ASCII fast path.
			const auto scalar = decode_next_utf16_scalar(cursor);
			if (scalar < 0x80u) [[likely]]
			{
				return grapheme_scalar_info{
					.scalar = scalar,
					.break_property = ascii_grapheme_break_property(scalar),
					.indic_property = unicode::indic_conjunct_break_property::none,
					.extended_pictographic = false
				};
			}

			return classify_grapheme_scalar(scalar);
		}

		inline constexpr grapheme_scalar_info decode_next_grapheme_utf32_scalar(const char32_t*& cursor) noexcept
		{
			// Fold decode + classify into one helper so grapheme algorithms only branch
			// once per scalar, with a very cheap ASCII fast path.
			const auto scalar = decode_next_utf32_scalar(cursor);
			if (scalar < 0x80u) [[likely]]
			{
				return grapheme_scalar_info{
					.scalar = scalar,
					.break_property = ascii_grapheme_break_property(scalar),
					.indic_property = unicode::indic_conjunct_break_property::none,
					.extended_pictographic = false
				};
			}

			return classify_grapheme_scalar(scalar);
		}

		inline constexpr grapheme_emoji_suffix_state advance_emoji_suffix(
			grapheme_emoji_suffix_state state,
			const grapheme_scalar_info& scalar_info) noexcept
		{
			if (scalar_info.extended_pictographic)
			{
				return grapheme_emoji_suffix_state::extended_pictographic;
			}

			using enum unicode::grapheme_cluster_break_property;
			switch (scalar_info.break_property)
			{
			case extend:
				if (state == grapheme_emoji_suffix_state::extended_pictographic
					|| state == grapheme_emoji_suffix_state::extended_pictographic_extend)
				{
					return grapheme_emoji_suffix_state::extended_pictographic_extend;
				}
				return grapheme_emoji_suffix_state::none;
			case zwj:
				if (state == grapheme_emoji_suffix_state::extended_pictographic
					|| state == grapheme_emoji_suffix_state::extended_pictographic_extend)
				{
					return grapheme_emoji_suffix_state::extended_pictographic_extend_zwj;
				}
				return grapheme_emoji_suffix_state::none;
			default:
				return grapheme_emoji_suffix_state::none;
			}
		}

		inline constexpr grapheme_emoji_suffix_state advance_emoji_suffix(
			grapheme_emoji_suffix_state state,
			std::uint32_t scalar,
			unicode::grapheme_cluster_break_property break_property) noexcept
		{
			if (unicode::is_extended_pictographic(scalar))
			{
				return grapheme_emoji_suffix_state::extended_pictographic;
			}

			using enum unicode::grapheme_cluster_break_property;
			switch (break_property)
			{
			case extend:
				if (state == grapheme_emoji_suffix_state::extended_pictographic
					|| state == grapheme_emoji_suffix_state::extended_pictographic_extend)
				{
					return grapheme_emoji_suffix_state::extended_pictographic_extend;
				}
				return grapheme_emoji_suffix_state::none;
			case zwj:
				if (state == grapheme_emoji_suffix_state::extended_pictographic
					|| state == grapheme_emoji_suffix_state::extended_pictographic_extend)
				{
					return grapheme_emoji_suffix_state::extended_pictographic_extend_zwj;
				}
				return grapheme_emoji_suffix_state::none;
			default:
				return grapheme_emoji_suffix_state::none;
			}
		}

		inline constexpr grapheme_indic_suffix_state advance_indic_suffix(
			grapheme_indic_suffix_state state,
			const grapheme_scalar_info& scalar_info) noexcept
		{
			using enum unicode::indic_conjunct_break_property;
			switch (scalar_info.indic_property)
			{
			case consonant:
				return grapheme_indic_suffix_state::consonant_no_linker;
			case extend:
				if (state == grapheme_indic_suffix_state::consonant_no_linker
					|| state == grapheme_indic_suffix_state::consonant_linker)
				{
					return state;
				}
				return grapheme_indic_suffix_state::none;
			case linker:
				if (state == grapheme_indic_suffix_state::consonant_no_linker
					|| state == grapheme_indic_suffix_state::consonant_linker)
				{
					return grapheme_indic_suffix_state::consonant_linker;
				}
				return grapheme_indic_suffix_state::none;
			default:
				return grapheme_indic_suffix_state::none;
			}
		}

		inline constexpr grapheme_indic_suffix_state advance_indic_suffix(
			grapheme_indic_suffix_state state,
			std::uint32_t scalar) noexcept
		{
			using enum unicode::indic_conjunct_break_property;
			switch (unicode::indic_conjunct_break(scalar))
			{
			case consonant:
				return grapheme_indic_suffix_state::consonant_no_linker;
			case extend:
				if (state == grapheme_indic_suffix_state::consonant_no_linker
					|| state == grapheme_indic_suffix_state::consonant_linker)
				{
					return state;
				}
				return grapheme_indic_suffix_state::none;
			case linker:
				if (state == grapheme_indic_suffix_state::consonant_no_linker
					|| state == grapheme_indic_suffix_state::consonant_linker)
				{
					return grapheme_indic_suffix_state::consonant_linker;
				}
				return grapheme_indic_suffix_state::none;
			default:
				return grapheme_indic_suffix_state::none;
			}
		}

		inline constexpr grapheme_state make_initial_grapheme_state(std::uint32_t scalar) noexcept
		{
			if (scalar < 0x80u) [[likely]]
			{
				return grapheme_state{
					.previous_break = ascii_grapheme_break_property(scalar)
				};
			}

			const auto break_property = unicode::grapheme_cluster_break(scalar);
			return grapheme_state{
				.previous_break = break_property,
				.trailing_regional_indicator_count =
					break_property == unicode::grapheme_cluster_break_property::regional_indicator ? 1u : 0u,
				.emoji_suffix = advance_emoji_suffix(grapheme_emoji_suffix_state::none, scalar, break_property),
				.indic_suffix = advance_indic_suffix(grapheme_indic_suffix_state::none, scalar)
			};
		}

		inline constexpr grapheme_state make_initial_grapheme_state(const grapheme_scalar_info& scalar_info) noexcept
		{
			return grapheme_state{
				.previous_break = scalar_info.break_property,
				.trailing_regional_indicator_count =
					scalar_info.break_property == unicode::grapheme_cluster_break_property::regional_indicator ? 1u : 0u,
				.emoji_suffix = advance_emoji_suffix(grapheme_emoji_suffix_state::none, scalar_info),
				.indic_suffix = advance_indic_suffix(grapheme_indic_suffix_state::none, scalar_info)
			};
		}

		inline constexpr void consume_grapheme_scalar(grapheme_state& state, std::uint32_t scalar) noexcept
		{
			if (scalar < 0x80u) [[likely]]
			{
				state.previous_break = ascii_grapheme_break_property(scalar);
				state.trailing_regional_indicator_count = 0;
				state.emoji_suffix = grapheme_emoji_suffix_state::none;
				state.indic_suffix = grapheme_indic_suffix_state::none;
				return;
			}

			const auto break_property = unicode::grapheme_cluster_break(scalar);
			state.previous_break = break_property;
			if (break_property == unicode::grapheme_cluster_break_property::regional_indicator)
			{
				++state.trailing_regional_indicator_count;
			}
			else
			{
				state.trailing_regional_indicator_count = 0;
			}

			state.emoji_suffix = advance_emoji_suffix(state.emoji_suffix, scalar, break_property);
			state.indic_suffix = advance_indic_suffix(state.indic_suffix, scalar);
		}

		inline constexpr void consume_grapheme_scalar(
			grapheme_state& state,
			const grapheme_scalar_info& scalar_info) noexcept
		{
			state.previous_break = scalar_info.break_property;
			if (scalar_info.break_property == unicode::grapheme_cluster_break_property::regional_indicator)
			{
				++state.trailing_regional_indicator_count;
			}
			else
			{
				state.trailing_regional_indicator_count = 0;
			}

			state.emoji_suffix = advance_emoji_suffix(state.emoji_suffix, scalar_info);
			state.indic_suffix = advance_indic_suffix(state.indic_suffix, scalar_info);
		}

		inline constexpr bool should_continue_grapheme_cluster(
			const grapheme_state& state,
			std::uint32_t scalar) noexcept
		{
			if (scalar < 0x80u) [[likely]]
			{
				const auto current_break = ascii_grapheme_break_property(scalar);
				const auto previous_break = state.previous_break;

				if (previous_break == unicode::grapheme_cluster_break_property::cr
					&& current_break == unicode::grapheme_cluster_break_property::lf)
				{
					return true;
				}

				if (is_grapheme_control(previous_break) || is_grapheme_control(current_break))
				{
					return false;
				}

				return previous_break == unicode::grapheme_cluster_break_property::prepend;
			}

			using enum unicode::grapheme_cluster_break_property;

			const auto current_break = unicode::grapheme_cluster_break(scalar);
			const auto previous_break = state.previous_break;

			if (previous_break == cr && current_break == lf)
			{
				return true;
			}

			if (is_grapheme_control(previous_break) || is_grapheme_control(current_break))
			{
				return false;
			}

			if (previous_break == l && (current_break == l || current_break == v || current_break == lv || current_break == lvt))
			{
				return true;
			}

			if ((previous_break == lv || previous_break == v) && (current_break == v || current_break == t))
			{
				return true;
			}

			if ((previous_break == lvt || previous_break == t) && current_break == t)
			{
				return true;
			}

			if (state.indic_suffix == grapheme_indic_suffix_state::consonant_linker
				&& unicode::indic_conjunct_break(scalar) == unicode::indic_conjunct_break_property::consonant)
			{
				return true;
			}

			if (current_break == extend || current_break == zwj)
			{
				return true;
			}

			if (current_break == spacing_mark)
			{
				return true;
			}

			if (previous_break == prepend)
			{
				return true;
			}

			if (state.emoji_suffix == grapheme_emoji_suffix_state::extended_pictographic_extend_zwj
				&& unicode::is_extended_pictographic(scalar))
			{
				return true;
			}

			if (previous_break == regional_indicator
				&& current_break == regional_indicator
				&& (state.trailing_regional_indicator_count % 2u) == 1u)
			{
				return true;
			}

			return false;
		}

		inline constexpr bool should_continue_grapheme_cluster(
			const grapheme_state& state,
			const grapheme_scalar_info& scalar_info) noexcept
		{
			using enum unicode::grapheme_cluster_break_property;

			const auto current_break = scalar_info.break_property;
			const auto previous_break = state.previous_break;
			const auto has_tail_state =
				state.emoji_suffix != grapheme_emoji_suffix_state::none
				|| state.indic_suffix != grapheme_indic_suffix_state::none;

			if (!has_tail_state && previous_break == other) [[likely]]
			{
				return current_break == extend
					|| current_break == zwj
					|| current_break == spacing_mark;
			}

			if (!has_tail_state && previous_break == regional_indicator)
			{
				if (current_break == extend || current_break == zwj || current_break == spacing_mark)
				{
					return true;
				}

				return current_break == regional_indicator
					&& (state.trailing_regional_indicator_count % 2u) == 1u;
			}

			if (previous_break == cr && current_break == lf)
			{
				return true;
			}

			if (is_grapheme_control(previous_break) || is_grapheme_control(current_break))
			{
				return false;
			}

			if (previous_break == l && (current_break == l || current_break == v || current_break == lv || current_break == lvt))
			{
				return true;
			}

			if ((previous_break == lv || previous_break == v) && (current_break == v || current_break == t))
			{
				return true;
			}

			if ((previous_break == lvt || previous_break == t) && current_break == t)
			{
				return true;
			}

			if (state.indic_suffix == grapheme_indic_suffix_state::consonant_linker
				&& scalar_info.indic_property == unicode::indic_conjunct_break_property::consonant)
			{
				return true;
			}

			if (current_break == extend || current_break == zwj)
			{
				return true;
			}

			if (current_break == spacing_mark)
			{
				return true;
			}

			if (previous_break == prepend)
			{
				return true;
			}

			if (state.emoji_suffix == grapheme_emoji_suffix_state::extended_pictographic_extend_zwj
				&& scalar_info.extended_pictographic)
			{
				return true;
			}

			if (previous_break == regional_indicator
				&& current_break == regional_indicator
				&& (state.trailing_regional_indicator_count % 2u) == 1u)
			{
				return true;
			}

			return false;
		}

		inline constexpr std::size_t next_grapheme_boundary(std::u8string_view text, std::size_t index) noexcept
		{
			if (index >= text.size()) [[unlikely]]
			{
				return text.size();
			}

			const auto* const begin = text.data();
			const auto* const end = begin + text.size();
			auto position = begin + index;
			// The initial scalar seeds the rolling state; each following scalar asks
			// whether the cluster continues before mutating that state.
			auto state = make_initial_grapheme_state(decode_next_grapheme_utf8_scalar(position));

			while (position < end)
			{
				auto next_position = position;
				const auto next_scalar_info = decode_next_grapheme_utf8_scalar(next_position);
				if (!should_continue_grapheme_cluster(state, next_scalar_info))
				{
					return static_cast<std::size_t>(position - begin);
				}

				consume_grapheme_scalar(state, next_scalar_info);
				position = next_position;
			}

			return text.size();
		}

		inline constexpr std::size_t next_grapheme_boundary(std::u16string_view text, std::size_t index) noexcept
		{
			if (index >= text.size()) [[unlikely]]
			{
				return text.size();
			}

			const auto* const begin = text.data();
			const auto* const end = begin + text.size();
			auto position = begin + index;
			// The initial scalar seeds the rolling state; each following scalar asks
			// whether the cluster continues before mutating that state.
			auto state = make_initial_grapheme_state(decode_next_grapheme_utf16_scalar(position));

			while (position < end)
			{
				auto next_position = position;
				const auto next_scalar_info = decode_next_grapheme_utf16_scalar(next_position);
				if (!should_continue_grapheme_cluster(state, next_scalar_info))
				{
					return static_cast<std::size_t>(position - begin);
				}

				consume_grapheme_scalar(state, next_scalar_info);
				position = next_position;
			}

			return text.size();
		}

		inline constexpr std::size_t next_grapheme_boundary(std::u32string_view text, std::size_t index) noexcept
		{
			if (index >= text.size()) [[unlikely]]
			{
				return text.size();
			}

			const auto* const begin = text.data();
			const auto* const end = begin + text.size();
			auto position = begin + index;
			// The initial scalar seeds the rolling state; each following scalar asks
			// whether the cluster continues before mutating that state.
			auto state = make_initial_grapheme_state(decode_next_grapheme_utf32_scalar(position));

			while (position < end)
			{
				auto next_position = position;
				const auto next_scalar_info = decode_next_grapheme_utf32_scalar(next_position);
				if (!should_continue_grapheme_cluster(state, next_scalar_info))
				{
					return static_cast<std::size_t>(position - begin);
				}

				consume_grapheme_scalar(state, next_scalar_info);
				position = next_position;
			}

			return text.size();
		}

		template <typename CharT>
		inline constexpr std::size_t count_ascii_grapheme_run(
			std::basic_string_view<CharT> ascii_run,
			bool first_continues_previous) noexcept
		{
			// For ASCII, grapheme segmentation degenerates to "one code point per
			// grapheme" except for CRLF, so we can count runs without per-scalar table
			// lookups or full grapheme-state transitions.
			if (ascii_run.empty())
			{
				return 0;
			}

			if (ascii_run.size() == 1)
			{
				return first_continues_previous ? 0u : 1u;
			}

			if (ascii_run.find(static_cast<CharT>('\r')) == std::basic_string_view<CharT>::npos)
			{
				return ascii_run.size() - (first_continues_previous ? 1u : 0u);
			}

			std::size_t count = first_continues_previous ? 0u : 1u;
			for (std::size_t index = 1; index != ascii_run.size(); ++index)
			{
				const auto previous = static_cast<std::uint32_t>(ascii_run[index - 1]);
				const auto current = static_cast<std::uint32_t>(ascii_run[index]);
				if (previous != static_cast<std::uint32_t>('\r')
					|| current != static_cast<std::uint32_t>('\n'))
				{
					++count;
				}
			}

			return count;
		}

		inline constexpr std::size_t grapheme_count(std::u8string_view text) noexcept
		{
			if (text.empty())
			{
				return 0;
			}

			const auto* const begin = text.data();
			const auto* const end = begin + text.size();
			auto position = begin;
			grapheme_state state{};
			std::size_t count = 1;

			// Large ASCII spans are counted in bulk; non-ASCII falls back to the full
			// state machine only when needed.
			if (static_cast<std::uint8_t>(*position) <= encoding_constants::ascii_scalar_max) [[likely]]
			{
				const auto remaining = std::u8string_view{ position, static_cast<std::size_t>(end - position) };
				const auto ascii_run = ascii_prefix_length(remaining);
				const auto ascii_view = remaining.substr(0, ascii_run);
				count = count_ascii_grapheme_run(ascii_view, false);
				state = make_initial_grapheme_state(static_cast<std::uint8_t>(ascii_view.back()));
				position += ascii_run;
			}
			else
			{
				auto next_position = position;
				state = make_initial_grapheme_state(decode_next_grapheme_utf8_scalar(next_position));
				position = next_position;
			}

			while (position < end)
			{
				if (static_cast<std::uint8_t>(*position) <= encoding_constants::ascii_scalar_max) [[likely]]
				{
					const auto remaining = std::u8string_view{ position, static_cast<std::size_t>(end - position) };
					const auto ascii_run = ascii_prefix_length(remaining);
					const auto ascii_view = remaining.substr(0, ascii_run);
					count += count_ascii_grapheme_run(
						ascii_view,
						should_continue_grapheme_cluster(state, static_cast<std::uint8_t>(ascii_view.front())));
					state = make_initial_grapheme_state(static_cast<std::uint8_t>(ascii_view.back()));
					position += ascii_run;
					continue;
				}

				auto next_position = position;
				const auto scalar_info = decode_next_grapheme_utf8_scalar(next_position);
				if (!should_continue_grapheme_cluster(state, scalar_info))
				{
					++count;
					state = make_initial_grapheme_state(scalar_info);
				}
				else
				{
					consume_grapheme_scalar(state, scalar_info);
				}

				position = next_position;
			}

			return count;
		}

		inline constexpr std::size_t grapheme_count(std::u16string_view text) noexcept
		{
			if (text.empty())
			{
				return 0;
			}

			const auto* const begin = text.data();
			const auto* const end = begin + text.size();
			auto position = begin;
			grapheme_state state{};
			std::size_t count = 1;

			// Large ASCII spans are counted in bulk; non-ASCII falls back to the full
			// state machine only when needed.
			if (static_cast<std::uint16_t>(*position) <= encoding_constants::ascii_scalar_max) [[likely]]
			{
				const auto remaining = std::u16string_view{ position, static_cast<std::size_t>(end - position) };
				const auto ascii_run = ascii_prefix_length(remaining);
				const auto ascii_view = remaining.substr(0, ascii_run);
				count = count_ascii_grapheme_run(ascii_view, false);
				state = make_initial_grapheme_state(static_cast<std::uint16_t>(ascii_view.back()));
				position += ascii_run;
			}
			else
			{
				auto next_position = position;
				state = make_initial_grapheme_state(decode_next_grapheme_utf16_scalar(next_position));
				position = next_position;
			}

			while (position < end)
			{
				if (static_cast<std::uint16_t>(*position) <= encoding_constants::ascii_scalar_max) [[likely]]
				{
					const auto remaining = std::u16string_view{ position, static_cast<std::size_t>(end - position) };
					const auto ascii_run = ascii_prefix_length(remaining);
					const auto ascii_view = remaining.substr(0, ascii_run);
					count += count_ascii_grapheme_run(
						ascii_view,
						should_continue_grapheme_cluster(state, static_cast<std::uint16_t>(ascii_view.front())));
					state = make_initial_grapheme_state(static_cast<std::uint16_t>(ascii_view.back()));
					position += ascii_run;
					continue;
				}

				auto next_position = position;
				const auto scalar_info = decode_next_grapheme_utf16_scalar(next_position);
				if (!should_continue_grapheme_cluster(state, scalar_info))
				{
					++count;
					state = make_initial_grapheme_state(scalar_info);
				}
				else
				{
					consume_grapheme_scalar(state, scalar_info);
				}

				position = next_position;
			}

			return count;
		}

		inline constexpr std::size_t char_count_scalar(std::u16string_view text) noexcept
		{
			std::size_t count = 0;
			for (const auto value : text)
			{
				count += is_utf16_low_surrogate(static_cast<std::uint16_t>(value)) ? 0u : 1u;
			}

			return count;
		}

		inline std::size_t char_count_runtime(std::u16string_view text) noexcept
		{
			std::size_t count = 0;
			std::size_t index = 0;
			while (index < text.size())
			{
				const auto remaining = text.substr(index);
				const auto ascii_run = ascii_prefix_length(remaining);
				if (ascii_run != 0) [[likely]]
				{
					count += ascii_run;
					index += ascii_run;
					continue;
				}

				++count;
				index += is_utf16_high_surrogate(static_cast<std::uint16_t>(text[index])) ? 2u : 1u;
			}

			return count;
		}

		inline constexpr std::size_t char_count(std::u32string_view text) noexcept
		{
			return text.size();
		}

		inline constexpr std::size_t char_count(std::u16string_view text) noexcept
		{
			if (std::is_constant_evaluated())
			{
				return char_count_scalar(text);
			}

			return char_count_runtime(text);
		}

		template <typename CharT>
		inline constexpr std::size_t previous_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			index = (std::min)(text.size(), index);
			if (index == 0 || index == text.size()) [[unlikely]]
			{
				return index;
			}

			std::size_t current = 0;
			while (current < text.size())
			{
				const auto next = next_grapheme_boundary(text, current);
				if (next >= index)
				{
					return next == index ? next : current;
				}

				current = next;
			}

			return text.size();
		}

		inline constexpr std::size_t grapheme_count(std::u32string_view text) noexcept
		{
			if (text.empty())
			{
				return 0;
			}

			const auto* const begin = text.data();
			const auto* const end = begin + text.size();
			auto position = begin;
			grapheme_state state{};
			std::size_t count = 1;

			// Large ASCII spans are counted in bulk; non-ASCII falls back to the full
			// state machine only when needed.
			if (static_cast<std::uint32_t>(*position) <= encoding_constants::ascii_scalar_max) [[likely]]
			{
				const auto remaining = std::u32string_view{ position, static_cast<std::size_t>(end - position) };
				const auto ascii_run = ascii_prefix_length(remaining);
				const auto ascii_view = remaining.substr(0, ascii_run);
				count = count_ascii_grapheme_run(ascii_view, false);
				state = make_initial_grapheme_state(static_cast<std::uint32_t>(ascii_view.back()));
				position += ascii_run;
			}
			else
			{
				auto next_position = position;
				state = make_initial_grapheme_state(decode_next_grapheme_utf32_scalar(next_position));
				position = next_position;
			}

			while (position < end)
			{
				if (static_cast<std::uint32_t>(*position) <= encoding_constants::ascii_scalar_max) [[likely]]
				{
					const auto remaining = std::u32string_view{ position, static_cast<std::size_t>(end - position) };
					const auto ascii_run = ascii_prefix_length(remaining);
					const auto ascii_view = remaining.substr(0, ascii_run);
					count += count_ascii_grapheme_run(
						ascii_view,
						should_continue_grapheme_cluster(state, static_cast<std::uint32_t>(ascii_view.front())));
					state = make_initial_grapheme_state(static_cast<std::uint32_t>(ascii_view.back()));
					position += ascii_run;
					continue;
				}

				auto next_position = position;
				const auto scalar_info = decode_next_grapheme_utf32_scalar(next_position);
				if (!should_continue_grapheme_cluster(state, scalar_info))
				{
					++count;
					state = make_initial_grapheme_state(scalar_info);
				}
				else
				{
					consume_grapheme_scalar(state, scalar_info);
				}

				position = next_position;
			}

			return count;
		}

			template <typename CharT>
			inline constexpr std::size_t grapheme_count(std::basic_string_view<CharT> text) noexcept
			{
				if constexpr (std::same_as<CharT, char8_t>)
				{
					return grapheme_count(std::u8string_view{ text.data(), text.size() });
				}

				else if constexpr (std::same_as<CharT, char16_t>)
				{
					return grapheme_count(std::u16string_view{ text.data(), text.size() });
				}

				else if constexpr (std::same_as<CharT, char32_t>)
				{
					return grapheme_count(std::u32string_view{ text.data(), text.size() });
				}
				else
				{
					std::size_t count = 0;
					for (std::size_t index = 0; index < text.size(); index = next_grapheme_boundary(text, index))
					{
						++count;
					}

						return count;
					}
				}

			template <typename CharT>
			inline constexpr std::size_t floor_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
			{
			return previous_grapheme_boundary(text, index);
		}

		template <typename CharT>
		inline constexpr std::size_t ceil_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			index = (std::min)(text.size(), index);
			if (index == 0 || index == text.size()) [[unlikely]]
			{
				return index;
			}

			std::size_t current = 0;
			while (current < text.size())
			{
				if (current >= index)
				{
					return current;
				}

				const auto next = next_grapheme_boundary(text, current);
				if (next >= index)
				{
					return next;
				}

				current = next;
			}

			return text.size();
		}

		template <typename CharT>
		inline constexpr bool is_grapheme_boundary(std::basic_string_view<CharT> text, std::size_t index) noexcept
		{
			if (index > text.size()) [[unlikely]]
			{
				return false;
			}

			return floor_grapheme_boundary(text, index) == index;
		}

		template <typename CharT>
		inline constexpr bool grapheme_match_at(
			std::basic_string_view<CharT> text,
			std::basic_string_view<CharT> needle,
			std::size_t index) noexcept
		{
			if (needle.size() > text.size() - index)
			{
				return false;
			}

			for (std::size_t needle_index = 0; needle_index != needle.size(); ++needle_index)
			{
				if (text[index + needle_index] != needle[needle_index])
				{
					return false;
				}
			}

			return is_grapheme_boundary(text, index + needle.size());
		}

		template <typename CharT>
		inline constexpr std::size_t find_grapheme(
			std::basic_string_view<CharT> text,
			std::basic_string_view<CharT> needle,
			std::size_t pos = std::basic_string_view<CharT>::npos) noexcept
		{
			pos = ceil_grapheme_boundary(text, (std::min)(text.size(), pos));
			if (needle.empty())
			{
				return pos;
			}

			if (needle.size() > text.size() - pos)
			{
				return std::basic_string_view<CharT>::npos;
			}

			for (std::size_t current = pos; current + needle.size() <= text.size(); current = next_grapheme_boundary(text, current))
			{
				if (grapheme_match_at(text, needle, current))
				{
					return current;
				}
			}

			return std::basic_string_view<CharT>::npos;
		}

		template <typename CharT>
		inline constexpr std::size_t rfind_grapheme(
			std::basic_string_view<CharT> text,
			std::basic_string_view<CharT> needle,
			std::size_t pos = std::basic_string_view<CharT>::npos) noexcept
		{
			pos = floor_grapheme_boundary(text, (std::min)(text.size(), pos));
			if (needle.empty())
			{
				return pos;
			}

			if (needle.size() > text.size())
			{
				return std::basic_string_view<CharT>::npos;
			}

			const auto last_start = floor_grapheme_boundary(text, text.size() - needle.size());
			pos = (std::min)(pos, last_start);
			auto last_match = std::basic_string_view<CharT>::npos;
			for (std::size_t current = 0; current <= pos; current = next_grapheme_boundary(text, current))
			{
				if (grapheme_match_at(text, needle, current))
				{
					last_match = current;
				}

				if (current == pos)
				{
					break;
				}
			}

			return last_match;
		}

		namespace literals
		{
		template<typename CharT, std::size_t N>
		struct constexpr_utf8_character
		{
			CharT p[N]{};

			static constexpr std::size_t SIZE = N;
			using CHAR_T = CharT;

			consteval constexpr_utf8_character(CharT const(&pp)[N])
			{
				std::ranges::copy(pp, p);

				if (!details::is_single_valid_utf8_char(std::basic_string_view<CharT>{ &p[0], N - 1 }))
				{
					throw std::invalid_argument("literal must contain exactly one valid UTF-8 character");
				}
			}

			consteval const CharT* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf8_string
		{
			char8_t p[N]{};

			static constexpr std::size_t SIZE = N;

			consteval constexpr_utf8_string(CharT const(&pp)[N])
			{
				for (std::size_t i = 0; i < N; ++i)
				{
					p[i] = static_cast<char8_t>(pp[i]);
				}
			}

			consteval const char8_t* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf16_character
		{
			CharT p[N]{};

			static constexpr std::size_t SIZE = N;
			using CHAR_T = CharT;

			consteval constexpr_utf16_character(CharT const(&pp)[N])
			{
				std::ranges::copy(pp, p);

				if (!details::is_single_valid_utf16_char(std::basic_string_view<CharT>{ &p[0], N - 1 }))
				{
					throw std::invalid_argument("literal must contain exactly one valid UTF-16 character");
				}
			}

			consteval const CharT* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf16_string
		{
			char16_t p[N]{};

			static constexpr std::size_t SIZE = N;

			consteval constexpr_utf16_string(CharT const(&pp)[N])
			{
				for (std::size_t i = 0; i < N; ++i)
				{
					p[i] = static_cast<char16_t>(pp[i]);
				}
			}

			consteval const char16_t* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf32_character
		{
			CharT p[N]{};

			static constexpr std::size_t SIZE = N;
			using CHAR_T = CharT;

			consteval constexpr_utf32_character(CharT const(&pp)[N])
			{
				std::ranges::copy(pp, p);

				if (!details::is_single_valid_utf32_char(std::basic_string_view<CharT>{ &p[0], N - 1 }))
				{
					throw std::invalid_argument("literal must contain exactly one valid UTF-32 character");
				}
			}

			consteval const CharT* data() const noexcept
			{
				return &p[0];
			}
		};

		template<typename CharT, std::size_t N>
		struct constexpr_utf32_string
		{
			char32_t p[N]{};

			static constexpr std::size_t SIZE = N;

			consteval constexpr_utf32_string(CharT const(&pp)[N])
			{
				for (std::size_t i = 0; i < N; ++i)
				{
					p[i] = static_cast<char32_t>(pp[i]);
				}
			}

				consteval const char32_t* data() const noexcept
				{
					return &p[0];
				}
				};
			}
		}

		#if UTF8_RANGES_HAS_ICU
	namespace literals
		{
	consteval locale_id operator ""_locale(const char* name, std::size_t size)
	{
		for (std::size_t i = 0; i < size; ++i)
		{
			if (name[i] == '\0')
			{
				throw std::invalid_argument("locale literal must not contain embedded NUL");
			}
		}

		return locale_id{ name };
	}
}
#endif

}

#endif // UTF8_RANGES_CORE_HPP
