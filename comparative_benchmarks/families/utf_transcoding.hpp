#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_TRANSCODING_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_TRANSCODING_HPP

#include "../adapters/unicode_ranges.hpp"
#include "../corpora.hpp"

namespace comparative_benchmarks::families
{

inline std::vector<scenario> make_utf_transcoding_scenarios()
{
	std::vector<scenario> scenarios{};
	scenarios.reserve(utf8_corpora().size() * 2);

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
				}
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
				}
			}
		});
	}

	return scenarios;
}

} // namespace comparative_benchmarks::families

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_FAMILIES_UTF_TRANSCODING_HPP
