#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_VALIDATION_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_VALIDATION_HPP

#include "../config.hpp"
#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF
#include "../adapters/simdutf.hpp"
#endif
#if UTF8_RANGES_COMPARATIVE_WITH_UTFCPP
#include "../adapters/utfcpp.hpp"
#endif
#if UTF8_RANGES_COMPARATIVE_WITH_UNI_ALGO
#include "../adapters/uni_algo.hpp"
#endif
#include "../adapters/unicode_ranges.hpp"
#include "../corpora.hpp"

namespace comparative_benchmarks::families
{

inline std::vector<scenario> make_utf_validation_scenarios()
{
	std::vector<scenario> scenarios{};
	scenarios.reserve(utf8_corpora().size());

	for (const corpus& input : utf8_corpora())
	{
		scenarios.push_back(scenario{
			.family = benchmark_family::utf_validation,
			.semantics = benchmark_semantics::strict_validation,
			.output = output_model::none,
			.operation = "public_factory",
			.input = &input,
			.batch_size = 4,
			.implementations = {
				implementation_case{
					.library = library_id::unicode_ranges,
					.run = [&input]() -> std::size_t
					{
						return adapters::validate_utf8_public_factory(input);
					}
				},
#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF
				implementation_case{
					.library = library_id::simdutf,
					.run = [&input]() -> std::size_t
					{
						return adapters::validate_utf8_simdutf(input);
					}
				}
#else
				make_unsupported_case(
					library_id::simdutf,
					"simdutf dependency was not fetched for this runner")
#endif
#if UTF8_RANGES_COMPARATIVE_WITH_UTFCPP
				,
				implementation_case{
					.library = library_id::utfcpp,
					.run = [&input]() -> std::size_t
					{
						return adapters::validate_utf8_utfcpp(input);
					}
				}
#else
				,
				make_unsupported_case(
					library_id::utfcpp,
					"utfcpp dependency was not fetched for this runner")
#endif
#if UTF8_RANGES_COMPARATIVE_WITH_UNI_ALGO
				,
				implementation_case{
					.library = library_id::uni_algo,
					.run = [&input]() -> std::size_t
					{
						return adapters::validate_utf8_uni_algo(input);
					}
				}
#else
				,
				make_unsupported_case(
					library_id::uni_algo,
					"uni-algo dependency was not fetched for this runner")
#endif
			}
		});
	}

	return scenarios;
}

} // namespace comparative_benchmarks::families

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_VALIDATION_HPP
