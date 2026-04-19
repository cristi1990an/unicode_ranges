#include <vector>

#include "comparative_benchmarks/families/utf_transcoding.hpp"
#include "comparative_benchmarks/families/utf_validation.hpp"
#include "comparative_benchmarks/framework.hpp"

int main(int argc, char** argv)
{
	using namespace comparative_benchmarks;

	std::vector<scenario> scenarios{};

	{
		auto validation = families::make_utf_validation_scenarios();
		scenarios.insert(scenarios.end(), validation.begin(), validation.end());
	}
	{
		auto transcoding = families::make_utf_transcoding_scenarios();
		scenarios.insert(scenarios.end(), transcoding.begin(), transcoding.end());
	}

	return run_suite(argc, argv, scenarios);
}
