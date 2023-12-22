#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static void BM_normal_hashmap(benchmark::State &state) {
    std::unordered_map<int, int> map;
    map.reserve(8 << 10);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            map[i] = i;
        }
    }
}

BENCHMARK(BM_normal_hashmap)->Range(8, 8 << 10);
