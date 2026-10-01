#include "unicode_ranges_tests.hpp"
#include "tests/edge_cases.hpp"
#include "tests/conformance.hpp"

int main(int argc, char** argv)
{
	if (argc == 3 && std::string_view{argv[1]} == "--conformance-dir")
	{
		try { unicode_ranges_edge_tests::conformance(argv[2]); }
		catch (const std::exception& error)
		{
			std::fprintf(stderr, "%s\n", error.what());
			return 1;
		}
		return 0;
	}
	if (argc == 1)
	{
		run_unicode_ranges_tests();
		unicode_ranges_edge_tests::run();
		return 0;
	}
	if (argc == 3 && std::string_view{argv[1]} == "--suite")
	{
		if (std::string_view{argv[2]} == "existing")
		{
			run_unicode_ranges_tests();
			return 0;
		}
		if (unicode_ranges_edge_tests::run(argv[2])) return 0;
	}
	std::fprintf(stderr, "Usage: unicode_ranges_tests [--suite existing");
	for (const auto& test : unicode_ranges_edge_tests::suites) std::fprintf(stderr, "|%s", test.name);
	std::fprintf(stderr, "]\n       unicode_ranges_tests --conformance-dir <ucd-directory>\n");
	return 2;
}
