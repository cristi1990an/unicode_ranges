#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_FRAMEWORK_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_FRAMEWORK_HPP

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "model.hpp"

namespace comparative_benchmarks
{

[[noreturn]] inline void benchmark_assert_fail(const char* expression, const char* file, int line)
{
	std::fprintf(stderr, "comparative benchmark assertion failed: %s (%s:%d)\n", expression, file, line);
	std::abort();
}

#define UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(expr) \
	do \
	{ \
		if (!(expr)) \
		{ \
			::comparative_benchmarks::benchmark_assert_fail(#expr, __FILE__, __LINE__); \
		} \
	} while (false)

inline volatile std::size_t benchmark_sink = 0;

struct benchmark_options
{
	std::chrono::milliseconds min_duration{ 250 };
	std::size_t min_iterations = 2;
	std::size_t sample_count = 3;
	std::string_view filter{};
	bool list_only = false;
};

struct benchmark_case
{
	std::string name{};
	std::size_t bytes_per_iteration = 0;
	std::size_t batch_size = 1;
	std::function<std::size_t()> run{};
};

struct benchmark_result
{
	std::string_view name{};
	double nanoseconds_per_iteration = 0.0;
	double mib_per_second = 0.0;
	std::size_t iterations = 0;
};

inline benchmark_options parse_options(int argc, char** argv)
{
	benchmark_options options{};
	for (int i = 1; i != argc; ++i)
	{
		const std::string_view arg{ argv[i] };
		if (arg == "--quick")
		{
			options.min_duration = std::chrono::milliseconds{ 60 };
			options.min_iterations = 1;
		}
		else if (arg == "--list")
		{
			options.list_only = true;
		}
		else if (arg.starts_with("--filter="))
		{
			options.filter = arg.substr(std::string_view{ "--filter=" }.size());
		}
		else if (arg.starts_with("--min-ms="))
		{
			options.min_duration = std::chrono::milliseconds{
				static_cast<std::chrono::milliseconds::rep>(std::strtoll(arg.substr(std::string_view{ "--min-ms=" }.size()).data(), nullptr, 10))
			};
		}
		else if (arg.starts_with("--samples="))
		{
			options.sample_count = (std::max)(std::size_t{ 1 }, static_cast<std::size_t>(
				std::strtoull(arg.substr(std::string_view{ "--samples=" }.size()).data(), nullptr, 10)));
		}
	}

	return options;
}

inline std::size_t checksum(std::u8string_view text) noexcept
{
	if (text.empty())
	{
		return 0;
	}

	return text.size() * 1315423911u
		^ static_cast<std::size_t>(text.front())
		^ (static_cast<std::size_t>(text.back()) << 8u);
}

inline std::size_t checksum(std::u16string_view text) noexcept
{
	if (text.empty())
	{
		return 0;
	}

	return text.size() * 1315423911u
		^ static_cast<std::size_t>(text.front())
		^ (static_cast<std::size_t>(text.back()) << 8u);
}

inline std::size_t checksum(std::u32string_view text) noexcept
{
	if (text.empty())
	{
		return 0;
	}

	return text.size() * 1315423911u
		^ static_cast<std::size_t>(text.front())
		^ (static_cast<std::size_t>(text.back()) << 8u);
}

inline benchmark_result run_case(const benchmark_case& c, const benchmark_options& options)
{
	using clock = std::chrono::steady_clock;

	std::vector<double> ns_samples{};
	std::vector<double> mib_samples{};
	std::vector<std::size_t> iteration_samples{};
	ns_samples.reserve(options.sample_count);
	mib_samples.reserve(options.sample_count);
	iteration_samples.reserve(options.sample_count);

	for (std::size_t sample = 0; sample != options.sample_count; ++sample)
	{
		std::size_t iterations = 0;
		const auto start = clock::now();
		do
		{
			for (std::size_t i = 0; i != c.batch_size; ++i)
			{
				benchmark_sink ^= c.run() + 0x9E3779B97F4A7C15ull;
			}
			iterations += c.batch_size;
		}
		while (iterations < options.min_iterations
			|| (clock::now() - start) < options.min_duration);

		const auto finish = clock::now();
		const auto elapsed = std::chrono::duration<double>(finish - start).count();

		ns_samples.push_back((elapsed * 1'000'000'000.0) / static_cast<double>(iterations));
		mib_samples.push_back(c.bytes_per_iteration == 0
			? 0.0
			: ((static_cast<double>(c.bytes_per_iteration) * static_cast<double>(iterations))
				/ (1024.0 * 1024.0)) / elapsed);
		iteration_samples.push_back(iterations);
	}

	const auto median_double = [](std::vector<double> values)
	{
		std::sort(values.begin(), values.end());
		return values[values.size() / 2];
	};

	const auto median_size_t = [](std::vector<std::size_t> values)
	{
		std::sort(values.begin(), values.end());
		return values[values.size() / 2];
	};

	return benchmark_result{
		.name = c.name,
		.nanoseconds_per_iteration = median_double(std::move(ns_samples)),
		.mib_per_second = median_double(std::move(mib_samples)),
		.iterations = median_size_t(std::move(iteration_samples))
	};
}

inline std::string make_case_name(const scenario& row, library_id library)
{
	std::string value = "compare.";
	value += to_string(row.family);
	value += '.';
	value += to_string(row.semantics);
	value += '.';
	value += to_string(row.output);
	value += '.';
	value += row.input->id;
	value += '.';
	value += row.operation;
	value += '.';
	value += to_string(library);
	return value;
}

inline std::vector<benchmark_case> expand_cases(const std::vector<scenario>& scenarios)
{
	std::vector<benchmark_case> cases{};
	for (const scenario& row : scenarios)
	{
		UTF8_RANGES_COMPARATIVE_BENCHMARK_ASSERT(row.input != nullptr);
		for (const implementation_case& implementation : row.implementations)
		{
			cases.push_back(benchmark_case{
				.name = make_case_name(row, implementation.library),
				.bytes_per_iteration = row.input->bytes.size(),
				.batch_size = row.batch_size,
				.run = implementation.run
			});
		}
	}

	return cases;
}

inline int run_suite(int argc, char** argv, const std::vector<scenario>& scenarios)
{
	const auto options = parse_options(argc, argv);
	const auto cases = expand_cases(scenarios);

	std::cout << "unicode_ranges comparative benchmark suite\n";
	std::cout << "min duration: " << options.min_duration.count() << " ms";
	std::cout << ", samples: " << options.sample_count << " (median)";
	if (!options.filter.empty())
	{
		std::cout << ", filter: " << options.filter;
	}
	std::cout << "\n\n";

	if (options.list_only)
	{
		for (const auto& c : cases)
		{
			std::cout << c.name << '\n';
		}
		return 0;
	}

	std::cout << std::left
		<< std::setw(60) << "case"
		<< std::right << std::setw(14) << "ns/op"
		<< std::setw(14) << "MiB/s"
		<< std::setw(12) << "iters/smp"
		<< '\n';
	std::cout << std::string(100, '-') << '\n';

	for (const auto& c : cases)
	{
		if (!options.filter.empty() && c.name.find(options.filter) == std::string::npos)
		{
			continue;
		}

		const auto result = run_case(c, options);
		std::cout << std::left << std::setw(60) << result.name
			<< std::right << std::fixed << std::setprecision(2)
			<< std::setw(14) << result.nanoseconds_per_iteration
			<< std::setw(14) << result.mib_per_second
			<< std::setw(12) << result.iterations
			<< '\n';
	}

	std::cout << "\nbenchmark sink: " << benchmark_sink << '\n';
	return 0;
}

} // namespace comparative_benchmarks

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_FRAMEWORK_HPP
