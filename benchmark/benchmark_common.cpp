#include "benchmark/benchmark.h"
#include "types.h"
#include "fakelua.h"
#include "location.h"

enum TestEnum {
    One, Two, Three
};

static void BM_enum_type_name(benchmark::State &state) {
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            std::string_view n = enum_type_name<TestEnum>();
            USE(n);
        }
    }
}

BENCHMARK(BM_enum_type_name)
->Range(8, 8<<10);
