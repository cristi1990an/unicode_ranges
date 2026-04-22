#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_TRANSCODING_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_TRANSCODING_HPP

#include <memory>
#include <span>
#include <vector>

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

inline std::vector<scenario> make_utf_transcoding_scenarios()
{
	std::vector<scenario> scenarios{};
	scenarios.reserve(utf8_corpora().size() * 4);

	for (const corpus& input : utf8_corpora())
	{
		if (!input.valid_utf8)
		{
			continue;
		}

		scenarios.push_back(scenario{
			.family = benchmark_family::utf_transcoding,
			.semantics = benchmark_semantics::strict_transcoding,
			.output = output_model::owned_result,
			.operation = "utf8_to_utf16_owned",
			.input = &input,
			.batch_size = 2,
			.implementations = {
				implementation_case{
					.library = library_id::unicode_ranges,
					.run = [&input]() -> std::size_t
					{
						return adapters::utf8_to_utf16_owned(input);
					}
				},
#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF
				implementation_case{
					.library = library_id::simdutf,
					.run = [&input]() -> std::size_t
					{
						return adapters::utf8_to_utf16_owned_simdutf(input);
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
						return adapters::utf8_to_utf16_owned_utfcpp(input);
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
						return adapters::utf8_to_utf16_owned_uni_algo(input);
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

		scenarios.push_back(scenario{
			.family = benchmark_family::utf_transcoding,
			.semantics = benchmark_semantics::strict_transcoding,
			.output = output_model::owned_result,
			.operation = "utf8_to_utf32_owned",
			.input = &input,
			.batch_size = 2,
			.implementations = {
				implementation_case{
					.library = library_id::unicode_ranges,
					.run = [&input]() -> std::size_t
					{
						return adapters::utf8_to_utf32_owned(input);
					}
				},
#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF
				implementation_case{
					.library = library_id::simdutf,
					.run = [&input]() -> std::size_t
					{
						return adapters::utf8_to_utf32_owned_simdutf(input);
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
						return adapters::utf8_to_utf32_owned_utfcpp(input);
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
						return adapters::utf8_to_utf32_owned_uni_algo(input);
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

		auto utf16_buffer = std::make_shared<std::vector<char16_t>>(input.bytes.size());
		scenarios.push_back(scenario{
			.family = benchmark_family::utf_transcoding,
			.semantics = benchmark_semantics::strict_transcoding,
			.output = output_model::bounded_sink,
			.operation = "utf8_to_utf16_buffer",
			.input = &input,
			.batch_size = 2,
			.implementations = {
				make_unsupported_case(
					library_id::unicode_ranges,
					"no public caller-buffer UTF-8 to UTF-16 transcoding API"),
#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF
				implementation_case{
					.library = library_id::simdutf,
					.run = [&input, utf16_buffer]() -> std::size_t
					{
						return adapters::utf8_to_utf16_buffer_simdutf(
							input,
							std::span<char16_t>{ utf16_buffer->data(), utf16_buffer->size() });
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
					.run = [&input, utf16_buffer]() -> std::size_t
					{
						return adapters::utf8_to_utf16_buffer_utfcpp(
							input,
							std::span<char16_t>{ utf16_buffer->data(), utf16_buffer->size() });
					}
				}
#else
				,
				make_unsupported_case(
					library_id::utfcpp,
					"utfcpp dependency was not fetched for this runner")
#endif
				,
				make_unsupported_case(
					library_id::uni_algo,
					"no public caller-buffer UTF-8 to UTF-16 transcoding API")
			}
		});

		auto utf32_buffer = std::make_shared<std::vector<char32_t>>(input.bytes.size());
		scenarios.push_back(scenario{
			.family = benchmark_family::utf_transcoding,
			.semantics = benchmark_semantics::strict_transcoding,
			.output = output_model::bounded_sink,
			.operation = "utf8_to_utf32_buffer",
			.input = &input,
			.batch_size = 2,
			.implementations = {
				make_unsupported_case(
					library_id::unicode_ranges,
					"no public caller-buffer UTF-8 to UTF-32 transcoding API"),
#if UTF8_RANGES_COMPARATIVE_WITH_SIMDUTF
				implementation_case{
					.library = library_id::simdutf,
					.run = [&input, utf32_buffer]() -> std::size_t
					{
						return adapters::utf8_to_utf32_buffer_simdutf(
							input,
							std::span<char32_t>{ utf32_buffer->data(), utf32_buffer->size() });
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
					.run = [&input, utf32_buffer]() -> std::size_t
					{
						return adapters::utf8_to_utf32_buffer_utfcpp(
							input,
							std::span<char32_t>{ utf32_buffer->data(), utf32_buffer->size() });
					}
				}
#else
				,
				make_unsupported_case(
					library_id::utfcpp,
					"utfcpp dependency was not fetched for this runner")
#endif
				,
				make_unsupported_case(
					library_id::uni_algo,
					"no public caller-buffer UTF-8 to UTF-32 transcoding API")
			}
		});
	}

	return scenarios;
}

} // namespace comparative_benchmarks::families

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_TRANSCODING_HPP
