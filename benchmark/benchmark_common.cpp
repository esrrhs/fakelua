#include "benchmark/benchmark.h"
#include "fakelua/fakelua.h"

using namespace fakelua;

static void BM_ini(benchmark::State &state) {
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            auto L = fakelua_newstate();
        }
    }
}

BENCHMARK(BM_ini)->Range(8, 8 << 10);
