#include "unicode_ranges.hpp"

#if defined(__has_include)
#if !__has_include("third_party/simdutf/simdutf.h") || !__has_include("third_party/simdutf/simdutf.cpp")
#error "unicode_ranges.cpp requires the vendored third_party/simdutf/simdutf.h and simdutf.cpp files"
#endif
#endif

#include "third_party/simdutf/simdutf.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include "third_party/simdutf/simdutf.cpp"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace unicode_ranges::details
{

namespace
{
	[[nodiscard]]
	auto map_simdutf_utf8_error(simdutf::result result) noexcept -> utf8_error
	{
		switch (result.error)
		{
		case simdutf::SUCCESS:
			return utf8_error{};

		case simdutf::HEADER_BITS:
		case simdutf::TOO_LONG:
			return utf8_error{
				.code = utf8_error_code::invalid_lead_byte,
				.first_invalid_byte_index = result.count
			};

		case simdutf::TOO_SHORT:
			return utf8_error{
				.code = utf8_error_code::truncated_sequence,
				.first_invalid_byte_index = result.count
			};

		case simdutf::OVERLONG:
		case simdutf::TOO_LARGE:
		case simdutf::SURROGATE:
			return utf8_error{
				.code = utf8_error_code::invalid_sequence,
				.first_invalid_byte_index = result.count
			};

		case simdutf::OUTPUT_BUFFER_TOO_SMALL:
			UTF8_RANGES_DEBUG_ASSERT(false && "simdutf output buffer unexpectedly too small");
			[[fallthrough]];

		default:
			return utf8_error{
				.code = utf8_error_code::invalid_sequence,
				.first_invalid_byte_index = result.count
			};
		}
	}

	[[nodiscard]]
	auto validate_utf8_runtime_impl(const char* bytes, std::size_t size) noexcept
		-> std::expected<void, utf8_error>
	{
		const auto result = simdutf::validate_utf8_with_errors(bytes, size);
		if (result.error == simdutf::SUCCESS) [[likely]]
		{
			return {};
		}

		return std::unexpected(map_simdutf_utf8_error(result));
	}
}

#if UTF8_RANGES_HAS_ICU

auto checked_icu_locale_name(locale_id locale) -> const char*
{
	if (locale.name == nullptr)
	{
		throw std::invalid_argument("locale_id must not be null");
	}

	return locale.name;
}

[[noreturn]] void throw_icu_error(const char* operation, UErrorCode error)
{
	throw std::runtime_error(std::string{ operation } + " failed: " + u_errorName(error));
}

auto normalized_icu_locale_name(locale_id locale) -> std::string
{
	const auto locale_name = checked_icu_locale_name(locale);
	if (*locale_name == '\0')
	{
		return {};
	}

	std::array<char, 160> buffer{};
	UErrorCode error = U_ZERO_ERROR;
	auto written = uloc_getName(locale_name, buffer.data(), static_cast<int32_t>(buffer.size()), &error);
	if (error == U_BUFFER_OVERFLOW_ERROR)
	{
		std::string expanded(static_cast<std::size_t>(written), '\0');
		error = U_ZERO_ERROR;
		written = uloc_getName(locale_name, expanded.data(), static_cast<int32_t>(expanded.size() + 1), &error);
		if (U_FAILURE(error))
		{
			throw_icu_error("uloc_getName", error);
		}

		expanded.resize(static_cast<std::size_t>(written));
		return expanded;
	}

	if (U_FAILURE(error))
	{
		throw_icu_error("uloc_getName", error);
	}

	return std::string{ buffer.data(), static_cast<std::size_t>(written) };
}

auto checked_icu_length(std::size_t size, const char* what) -> int32_t
{
	if (size > static_cast<std::size_t>((std::numeric_limits<int32_t>::max)()))
	{
		throw std::length_error(std::string{ what } + " is too large for ICU");
	}

	return static_cast<int32_t>(size);
}

auto make_icu_case_map(locale_id locale, uint32_t options)
	-> std::unique_ptr<UCaseMap, ucasemap_closer>
{
	const auto locale_name = checked_icu_locale_name(locale);
	UErrorCode error = U_ZERO_ERROR;
	auto map = std::unique_ptr<UCaseMap, ucasemap_closer>{ ucasemap_open(locale_name, options, &error) };
	if (U_FAILURE(error) || map == nullptr)
	{
		throw_icu_error("ucasemap_open", error);
	}

	return map;
}

bool icu_locale_uses_turkic_case_behavior(locale_id locale)
{
	const auto locale_name = checked_icu_locale_name(locale);
	if (*locale_name == '\0')
	{
		return false;
	}

	std::array<char, 32> language{};
	UErrorCode error = U_ZERO_ERROR;
	const auto written = uloc_getLanguage(
		locale_name,
		language.data(),
		static_cast<int32_t>(language.size()),
		&error);
	if (U_FAILURE(error))
	{
		throw_icu_error("uloc_getLanguage", error);
	}

	const auto language_name = std::string_view{ language.data(), static_cast<std::size_t>(written) };
	return language_name == "tr" || language_name == "az";
}

auto icu_case_fold_options(locale_id locale) -> uint32_t
{
	return icu_locale_uses_turkic_case_behavior(locale) ? U_FOLD_CASE_EXCLUDE_SPECIAL_I : U_FOLD_CASE_DEFAULT;
}

namespace
{
	template <typename Transform>
	auto icu_utf8_case_map_copy_impl(
		std::u8string_view bytes,
		locale_id locale,
		uint32_t options,
		const char* operation,
		Transform&& transform) -> std::u8string
	{
		if (bytes.empty())
		{
			return {};
		}

		auto case_map = make_icu_case_map(locale, options);
		const auto input_size = checked_icu_length(bytes.size(), "UTF-8 input");
		std::u8string result(bytes.size(), u8'\0');

		UErrorCode error = U_ZERO_ERROR;
		const auto written = transform(
			case_map.get(),
			reinterpret_cast<char*>(result.data()),
			input_size,
			reinterpret_cast<const char*>(bytes.data()),
			input_size,
			&error);

		if (error == U_BUFFER_OVERFLOW_ERROR)
		{
			result.resize(static_cast<std::size_t>(written));
			error = U_ZERO_ERROR;
			const auto rerun_written = transform(
				case_map.get(),
				reinterpret_cast<char*>(result.data()),
				written,
				reinterpret_cast<const char*>(bytes.data()),
				input_size,
				&error);
			if (U_FAILURE(error))
			{
				throw_icu_error(operation, error);
			}

			result.resize(static_cast<std::size_t>(rerun_written));
		}
		else if (U_FAILURE(error))
		{
			throw_icu_error(operation, error);
		}
		else
		{
			result.resize(static_cast<std::size_t>(written));
		}

		return result;
	}

	template <typename Transform>
	auto icu_utf16_case_map_copy_impl(
		std::u16string_view code_units,
		const char* operation,
		Transform&& transform) -> std::u16string
	{
		static_assert(sizeof(UChar) == sizeof(char16_t));

		if (code_units.empty())
		{
			return {};
		}

		const auto input_size = checked_icu_length(code_units.size(), "UTF-16 input");
		const auto* source = reinterpret_cast<const UChar*>(code_units.data());
		std::u16string result(code_units.size(), u'\0');

		UErrorCode error = U_ZERO_ERROR;
		const auto written = transform(
			reinterpret_cast<UChar*>(result.data()),
			input_size,
			source,
			input_size,
			&error);

		if (error == U_BUFFER_OVERFLOW_ERROR)
		{
			result.resize(static_cast<std::size_t>(written));
			error = U_ZERO_ERROR;
			const auto rerun_written = transform(
				reinterpret_cast<UChar*>(result.data()),
				written,
				source,
				input_size,
				&error);
			if (U_FAILURE(error))
			{
				throw_icu_error(operation, error);
			}

			result.resize(static_cast<std::size_t>(rerun_written));
		}
		else if (U_FAILURE(error))
		{
			throw_icu_error(operation, error);
		}
		else
		{
			result.resize(static_cast<std::size_t>(written));
		}

		return result;
	}
}

auto icu_lowercase_utf8_runtime_copy(std::u8string_view bytes, locale_id locale) -> std::u8string
{
	return icu_utf8_case_map_copy_impl(bytes, locale, 0, "ucasemap_utf8ToLower",
		[](UCaseMap* case_map, char* dest, int32_t dest_capacity, const char* source, int32_t source_length, UErrorCode* error)
		{
			return ucasemap_utf8ToLower(case_map, dest, dest_capacity, source, source_length, error);
		});
}

auto icu_uppercase_utf8_runtime_copy(std::u8string_view bytes, locale_id locale) -> std::u8string
{
	return icu_utf8_case_map_copy_impl(bytes, locale, 0, "ucasemap_utf8ToUpper",
		[](UCaseMap* case_map, char* dest, int32_t dest_capacity, const char* source, int32_t source_length, UErrorCode* error)
		{
			return ucasemap_utf8ToUpper(case_map, dest, dest_capacity, source, source_length, error);
		});
}

auto icu_lowercase_utf16_runtime_copy(std::u16string_view code_units, locale_id locale) -> std::u16string
{
	const auto locale_name = checked_icu_locale_name(locale);
	return icu_utf16_case_map_copy_impl(code_units, "u_strToLower",
		[&](UChar* dest, int32_t dest_capacity, const UChar* source, int32_t source_length, UErrorCode* error)
		{
			return u_strToLower(dest, dest_capacity, source, source_length, locale_name, error);
		});
}

auto icu_uppercase_utf16_runtime_copy(std::u16string_view code_units, locale_id locale) -> std::u16string
{
	const auto locale_name = checked_icu_locale_name(locale);
	return icu_utf16_case_map_copy_impl(code_units, "u_strToUpper",
		[&](UChar* dest, int32_t dest_capacity, const UChar* source, int32_t source_length, UErrorCode* error)
		{
			return u_strToUpper(dest, dest_capacity, source, source_length, locale_name, error);
		});
}

auto icu_titlecase_utf8_runtime_copy(std::u8string_view bytes, locale_id locale) -> std::u8string
{
	return icu_utf8_case_map_copy_impl(bytes, locale, 0, "ucasemap_utf8ToTitle",
		[](UCaseMap* case_map, char* dest, int32_t dest_capacity, const char* source, int32_t source_length, UErrorCode* error)
		{
			return ucasemap_utf8ToTitle(case_map, dest, dest_capacity, source, source_length, error);
		});
}

auto icu_titlecase_utf16_runtime_copy(std::u16string_view code_units, locale_id locale) -> std::u16string
{
	const auto locale_name = checked_icu_locale_name(locale);
	return icu_utf16_case_map_copy_impl(code_units, "u_strToTitle",
		[&](UChar* dest, int32_t dest_capacity, const UChar* source, int32_t source_length, UErrorCode* error)
		{
			return u_strToTitle(dest, dest_capacity, source, source_length, nullptr, locale_name, error);
		});
}

auto icu_case_fold_utf8_runtime_copy(std::u8string_view bytes, locale_id locale) -> std::u8string
{
	return icu_utf8_case_map_copy_impl(bytes, locale, icu_case_fold_options(locale), "ucasemap_utf8FoldCase",
		[](UCaseMap* case_map, char* dest, int32_t dest_capacity, const char* source, int32_t source_length, UErrorCode* error)
		{
			return ucasemap_utf8FoldCase(case_map, dest, dest_capacity, source, source_length, error);
		});
}

auto icu_case_fold_utf16_runtime_copy(std::u16string_view code_units, locale_id locale) -> std::u16string
{
	const auto options = icu_case_fold_options(locale);
	return icu_utf16_case_map_copy_impl(code_units, "u_strFoldCase",
		[&](UChar* dest, int32_t dest_capacity, const UChar* source, int32_t source_length, UErrorCode* error)
		{
			return u_strFoldCase(dest, dest_capacity, source, source_length, options, error);
		});
}

auto icu_lowercase_utf32_runtime_copy(std::u32string_view code_points, locale_id locale) -> std::u32string
{
	basic_utf16_string<> utf16;
	utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
	auto lowered = icu_lowercase_utf16_runtime_copy(std::u16string_view{ utf16.base() }, locale);
	auto result = basic_utf32_string<>{ utf16_string_view::from_code_units_unchecked(std::u16string_view{ lowered }) };
	return std::move(result).base();
}

auto icu_uppercase_utf32_runtime_copy(std::u32string_view code_points, locale_id locale) -> std::u32string
{
	basic_utf16_string<> utf16;
	utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
	auto uppered = icu_uppercase_utf16_runtime_copy(std::u16string_view{ utf16.base() }, locale);
	auto result = basic_utf32_string<>{ utf16_string_view::from_code_units_unchecked(std::u16string_view{ uppered }) };
	return std::move(result).base();
}

auto icu_titlecase_utf32_runtime_copy(std::u32string_view code_points, locale_id locale) -> std::u32string
{
	basic_utf16_string<> utf16;
	utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
	auto titled = icu_titlecase_utf16_runtime_copy(std::u16string_view{ utf16.base() }, locale);
	auto result = basic_utf32_string<>{ utf16_string_view::from_code_units_unchecked(std::u16string_view{ titled }) };
	return std::move(result).base();
}

auto icu_case_fold_utf32_runtime_copy(std::u32string_view code_points, locale_id locale) -> std::u32string
{
	basic_utf16_string<> utf16;
	utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
	auto folded = icu_case_fold_utf16_runtime_copy(std::u16string_view{ utf16.base() }, locale);
	auto result = basic_utf32_string<>{ utf16_string_view::from_code_units_unchecked(std::u16string_view{ folded }) };
	return std::move(result).base();
}

#endif

auto simdutf_validate_utf8_runtime(std::string_view bytes) noexcept
	-> std::expected<void, utf8_error>
{
	return validate_utf8_runtime_impl(bytes.data(), bytes.size());
}

auto simdutf_validate_utf8_runtime(std::u8string_view bytes) noexcept
	-> std::expected<void, utf8_error>
{
	return validate_utf8_runtime_impl(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

std::size_t simdutf_utf16_length_from_valid_utf8_runtime(std::u8string_view bytes) noexcept
{
	return simdutf::utf16_length_from_utf8(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size());
}

std::size_t simdutf_utf32_length_from_valid_utf8_runtime(std::u8string_view bytes) noexcept
{
	return simdutf::utf32_length_from_utf8(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size());
}

std::size_t simdutf_convert_valid_utf8_to_utf16_runtime(
	std::u8string_view bytes,
	char16_t* output) noexcept
{
	const auto count = simdutf::convert_valid_utf8_to_utf16(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size(),
		output);
	UTF8_RANGES_DEBUG_ASSERT(count == simdutf_utf16_length_from_valid_utf8_runtime(bytes));
	return count;
}

std::size_t simdutf_convert_valid_utf8_to_utf32_runtime(
	std::u8string_view bytes,
	char32_t* output) noexcept
{
	const auto count = simdutf::convert_valid_utf8_to_utf32(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size(),
		output);
	UTF8_RANGES_DEBUG_ASSERT(count == simdutf_utf32_length_from_valid_utf8_runtime(bytes));
	return count;
}

auto simdutf_convert_utf8_to_utf16_checked_runtime(
	std::string_view bytes,
	char16_t* output) noexcept
	-> std::expected<std::size_t, utf8_error>
{
	const auto result = simdutf::convert_utf8_to_utf16_with_errors(bytes.data(), bytes.size(), output);
	if (result.error == simdutf::SUCCESS) [[likely]]
	{
		return result.count;
	}

	return std::unexpected(map_simdutf_utf8_error(result));
}

auto simdutf_convert_utf8_to_utf32_checked_runtime(
	std::string_view bytes,
	char32_t* output) noexcept
	-> std::expected<std::size_t, utf8_error>
{
	const auto result = simdutf::convert_utf8_to_utf32_with_errors(bytes.data(), bytes.size(), output);
	if (result.error == simdutf::SUCCESS) [[likely]]
	{
		return result.count;
	}

	return std::unexpected(map_simdutf_utf8_error(result));
}

utf32_parallel_plan make_utf32_parallel_plan(std::size_t code_point_count) noexcept
{
	return make_utf32_parallel_plan(code_point_count, std::thread::hardware_concurrency());
}

void run_parallel_jobs_runtime(
	std::size_t worker_count,
	void* context,
	utf32_parallel_job_fn job)
{
	if (worker_count <= 1)
	{
		job(context, 0);
		return;
	}

	// The calling thread participates as the last worker to avoid spinning up an
	// extra thread just to sit idle waiting for joins.
	const auto background_worker_count = worker_count - 1;
	static_assert(runtime_parallel_max_worker_count >= 2);
#if UTF8_RANGES_HAS_JTHREAD
	std::array<std::jthread, runtime_parallel_max_worker_count - 1> workers{};
	for (std::size_t worker_index = 0; worker_index != background_worker_count; ++worker_index)
	{
		workers[worker_index] = std::jthread([=] noexcept
			{
				job(context, worker_index);
			});
	}
	job(context, background_worker_count);
#else
	std::array<std::thread, runtime_parallel_max_worker_count - 1> workers{};
	std::size_t started_workers = 0;
	try
	{
		for (std::size_t worker_index = 0; worker_index != background_worker_count; ++worker_index)
		{
			workers[worker_index] = std::thread([=]
				{
					job(context, worker_index);
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
	job(context, background_worker_count);
	for (std::size_t worker_index = 0; worker_index != started_workers; ++worker_index)
	{
		if (workers[worker_index].joinable())
		{
			workers[worker_index].join();
		}
	}
#endif
}

}

#if UTF8_RANGES_HAS_ICU

namespace unicode_ranges
{

bool is_available_locale(locale_id locale) noexcept
{
	try
	{
		const auto normalized = details::normalized_icu_locale_name(locale);
		if (normalized.empty())
		{
			return false;
		}

		const auto available_count = uloc_countAvailable();
		for (int32_t index = 0; index < available_count; ++index)
		{
			const char* candidate = uloc_getAvailable(index);
			if (candidate == nullptr)
			{
				continue;
			}

			const auto candidate_length = static_cast<int32_t>(std::char_traits<char>::length(candidate));
			if (normalized.size() == static_cast<std::size_t>(candidate_length)
				&& std::char_traits<char>::compare(normalized.data(), candidate, static_cast<std::size_t>(candidate_length)) == 0)
			{
				return true;
			}
		}

		return false;
	}
	catch (...)
	{
		return false;
	}
}

}

#endif
