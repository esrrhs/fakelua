#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"
#include "util/concurrent_hashmap.h"
#include "util/concurrent_vector.h"

using namespace fakelua;

static void BM_concurrent_hashmap(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    concurrent_hashmap<std::string, std::string> map(STRING_HEAP_INIT_BUCKET_SIZE);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            map.set("hello", "world");
        }
    }
}

BENCHMARK(BM_concurrent_hashmap)->Range(8, 8 << 10);

static void BM_normal_hashmap(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    std::unordered_map<std::string, std::string> map;
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            map["hello"] = "world";
        }
    }
}

BENCHMARK(BM_normal_hashmap)->Range(8, 8 << 10);

static void BM_concurrent_vector(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    concurrent_vector<std::string> v(STRING_HEAP_INIT_BUCKET_SIZE);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            for (int j = 0; j < 100; ++j) {
                v.set(j, "hello");
            }
        }
    }
}

BENCHMARK(BM_concurrent_vector)->Range(8, 8 << 10);

static void BM_normal_vector(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    std::vector<std::string> v;
    v.resize(STRING_HEAP_INIT_BUCKET_SIZE);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            for (int j = 0; j < 100; ++j) {
                v[j] = "hello";
            }
        }
    }
}

BENCHMARK(BM_normal_vector)->Range(8, 8 << 10);
