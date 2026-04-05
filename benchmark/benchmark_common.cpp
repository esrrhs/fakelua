#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var.h"
#include "var/var_table.h"
#include <unordered_map>
#include <vector>

using namespace fakelua;

static void BM_VarTable_Set(benchmark::State &state) {
    auto *s = FakeluaNewState();
    const int n = static_cast<int>(state.range(0));
    std::vector<Var> keys;
    std::vector<Var> vals;
    for (int i = 0; i < n; ++i) {
        keys.emplace_back(static_cast<int64_t>(i));
        vals.emplace_back(static_cast<int64_t>(i * 2));
    }

    for (auto _: state) {
        Var table_var;
        table_var.SetTable(s);
        auto *table = table_var.GetTable();
        for (int i = 0; i < n; ++i) {
            table->Set(s, keys[i], vals[i], false);
        }
        benchmark::DoNotOptimize(table);
    }
    state.SetComplexityN(state.range(0));
    FakeluaDeleteState(s);
}

static void BM_StdUnorderedMap_Set(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    std::vector<int64_t> keys;
    std::vector<int64_t> vals;
    for (int i = 0; i < n; ++i) {
        keys.emplace_back(i);
        vals.emplace_back(i * 2);
    }

    for (auto _: state) {
        std::unordered_map<int64_t, int64_t> m;
        for (int i = 0; i < n; ++i) {
            m[keys[i]] = vals[i];
        }
        benchmark::DoNotOptimize(m);
    }
    state.SetComplexityN(state.range(0));
}

static void BM_VarTable_Get(benchmark::State &state) {
    auto *s = FakeluaNewState();
    const int n = static_cast<int>(state.range(0));
    Var table_var;
    table_var.SetTable(s);
    auto *table = table_var.GetTable();
    std::vector<Var> keys;
    for (int i = 0; i < n; ++i) {
        Var key(static_cast<int64_t>(i));
        keys.push_back(key);
        table->Set(s, key, Var(static_cast<int64_t>(i * 2)), false);
    }

    for (auto _: state) {
        for (int i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(table->Get(keys[i]));
        }
    }
    state.SetComplexityN(state.range(0));
    FakeluaDeleteState(s);
}

static void BM_StdUnorderedMap_Get(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    std::unordered_map<int64_t, int64_t> m;
    std::vector<int64_t> keys;
    for (int i = 0; i < n; ++i) {
        keys.push_back(i);
        m[i] = i * 2;
    }

    for (auto _: state) {
        for (int i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(m[keys[i]]);
        }
    }
    state.SetComplexityN(state.range(0));
}

// 注册测试，覆盖 quick_data (<=4) 和 hash table (>4) 两种情况
#define ARGS ->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(128)->Arg(256)->Arg(512)->Arg(1024)

BENCHMARK(BM_VarTable_Set) ARGS;
BENCHMARK(BM_StdUnorderedMap_Set) ARGS;
BENCHMARK(BM_VarTable_Get) ARGS;
BENCHMARK(BM_StdUnorderedMap_Get) ARGS;
