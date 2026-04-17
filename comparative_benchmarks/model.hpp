#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_MODEL_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_MODEL_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace comparative_benchmarks
{

enum class benchmark_family
{
	utf_validation,
	utf_transcoding,
	normalization,
	case_mapping,
	segmentation,
	boundary_encoding
};

enum class benchmark_semantics
{
	strict_validation,
	strict_transcoding,
	replacement_transcoding
};

enum class output_model
{
	none,
	owned_result,
	bounded_sink,
	growable_container
};

enum class library_id
{
	unicode_ranges,
	simdutf,
	icu,
	boost_text,
	uni_algo,
	utf8proc,
	utfcpp,
	libiconv
};

constexpr std::string_view to_string(benchmark_family value) noexcept
{
	switch (value)
	{
	case benchmark_family::utf_validation: return "utf_validation";
	case benchmark_family::utf_transcoding: return "utf_transcoding";
	case benchmark_family::normalization: return "normalization";
	case benchmark_family::case_mapping: return "case_mapping";
	case benchmark_family::segmentation: return "segmentation";
	case benchmark_family::boundary_encoding: return "boundary_encoding";
	}

	return "unknown_family";
}

constexpr std::string_view to_string(benchmark_semantics value) noexcept
{
	switch (value)
	{
	case benchmark_semantics::strict_validation: return "strict";
	case benchmark_semantics::strict_transcoding: return "strict";
	case benchmark_semantics::replacement_transcoding: return "replacement";
	}

	return "unknown_semantics";
}

constexpr std::string_view to_string(output_model value) noexcept
{
	switch (value)
	{
	case output_model::none: return "none";
	case output_model::owned_result: return "owned_result";
	case output_model::bounded_sink: return "bounded_sink";
	case output_model::growable_container: return "growable_container";
	}

	return "unknown_output";
}

constexpr std::string_view to_string(library_id value) noexcept
{
	switch (value)
	{
	case library_id::unicode_ranges: return "unicode_ranges";
	case library_id::simdutf: return "simdutf";
	case library_id::icu: return "icu";
	case library_id::boost_text: return "boost_text";
	case library_id::uni_algo: return "uni_algo";
	case library_id::utf8proc: return "utf8proc";
	case library_id::utfcpp: return "utfcpp";
	case library_id::libiconv: return "libiconv";
	}

	return "unknown_library";
}

struct corpus
{
	std::string id{};
	std::string description{};
	std::u8string bytes{};
	bool valid_utf8 = false;
};

struct implementation_case
{
	library_id library = library_id::unicode_ranges;
	std::function<std::size_t()> run{};
	std::string unsupported_reason{};

	constexpr bool supported() const noexcept
	{
		return static_cast<bool>(run);
	}
};

inline implementation_case make_unsupported_case(library_id library, std::string reason)
{
	return implementation_case{
		.library = library,
		.run = {},
		.unsupported_reason = std::move(reason)
	};
}

struct scenario
{
	benchmark_family family = benchmark_family::utf_validation;
	benchmark_semantics semantics = benchmark_semantics::strict_validation;
	output_model output = output_model::none;
	std::string operation{};
	const corpus* input = nullptr;
	std::size_t batch_size = 1;
	std::vector<implementation_case> implementations{};
};

} // namespace comparative_benchmarks

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_MODEL_HPP
