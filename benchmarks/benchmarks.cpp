#include <benchmark/benchmark.h>
#pragma comment(lib, "Shlwapi.lib")

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <json17/json.h>
#pragma comment(lib, "Shlwapi.lib")

static const std::string simple_json = R"({ "null_value": null, "string" : "some_string", "numeric" : 10, "integer" : -10, "bool" : true, "float" : -1.04e6, "array": [1, true, "some_string"]})";

namespace
{
	static void test_json17_parse(benchmark::State &state)
	{
		while (state.KeepRunning())
		{
			auto j1 = json17::from_string(simple_json);
		}
	}

	static void test_nlohmann_json(benchmark::State &state)
	{
		while (state.KeepRunning())
		{
			auto j1 = json::parse(simple_json);
		}
	}
}

BENCHMARK(test_json17_parse);

BENCHMARK(test_nlohmann_json);

BENCHMARK_MAIN();

