#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static void BM_ini(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            auto L = fakelua_newstate();
        }
    }
}

BENCHMARK(BM_ini)->Range(8, 8 << 10);
