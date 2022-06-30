#include "benchmark/benchmark.h"
#include "fakelua/fakelua.h"

using namespace fakelua;

static void BM_ini(benchmark::State &state) {
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            fakelua_state *L = fakelua_newstate();
            fakelua_close(L);
        }
    }
}

BENCHMARK(BM_ini)->Range(8, 8 << 10);
