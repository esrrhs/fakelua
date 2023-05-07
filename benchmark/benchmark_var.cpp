#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/concurrent_hashmap.h"
#include "var/var.h"

using namespace fakelua;

static void BM_var_set_int(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    var v;
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            v.set((int64_t) 1234567890);
        }
    }
}

BENCHMARK(BM_var_set_int)->Range(8, 8 << 10);

static void BM_var_set_string(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    class state s;
    var v;
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            v.set(&s, "test");
        }
    }
}

BENCHMARK(BM_var_set_string)->Range(8, 8 << 10);

static void BM_var_get_int(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    var v;
    v.set((int64_t) 1234567890);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            v.get_int();
        }
    }
}

BENCHMARK(BM_var_get_int)->Range(8, 8 << 10);

static void BM_var_get_string(benchmark::State &state) {
    FLAGS_minloglevel = 3;
    class state s;
    var v;
    v.set(&s, "test");
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            v.get_string_view(&s);
        }
    }
}

BENCHMARK(BM_var_get_string)->Range(8, 8 << 10);
