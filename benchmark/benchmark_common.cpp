#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"
#include "util/rich_hashmap.h"

using namespace fakelua;

static void BM_normal_hashmap(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    std::unordered_map<int, int> map;
    map.reserve(8 << 10);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            map[i] = i;
        }
    }
}

BENCHMARK(BM_normal_hashmap)->Range(8, 8 << 10);

static void BM_normal_richmap(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    rich_hashmap<int, int, MAX_VAR_TABLE_HASHMAP_BUCKET_HEIGHT> map(8 << 10);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            map.set(i, i);
        }
    }
}

BENCHMARK(BM_normal_richmap)->Range(8, 8 << 10);
