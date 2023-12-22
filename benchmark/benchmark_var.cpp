#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var.h"

using namespace fakelua;

static void BM_var_set_int(benchmark::State &state) {
    var v;
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            v.set_int((int64_t) 1234567890);
        }
    }
}

BENCHMARK(BM_var_set_int)->Range(8, 8 << 10);

static void BM_var_set_string(benchmark::State &bstate) {
    auto s = std::make_shared<state>();
    var v;
    for (auto _: bstate) {
        for (int i = 0; i < bstate.range(0); ++i) {
            v.set_string(s, "test");
        }
    }
}

BENCHMARK(BM_var_set_string)->Range(8, 8 << 10);

static void BM_var_get_int(benchmark::State &state) {
    var v;
    v.set_int((int64_t) 1234567890);
    for (auto _: state) {
        for (int i = 0; i < state.range(0); ++i) {
            v.get_int();
        }
    }
}

BENCHMARK(BM_var_get_int)->Range(8, 8 << 10);

static void BM_var_get_string(benchmark::State &bstate) {
    auto s = std::make_shared<state>();
    var v;
    v.set_string(s, "test");
    for (auto _: bstate) {
        for (int i = 0; i < bstate.range(0); ++i) {
            v.get_string();
        }
    }
}

BENCHMARK(BM_var_get_string)->Range(8, 8 << 10);
