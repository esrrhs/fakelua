#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var.h"
#include "var/var_table.h"
#include <lua.hpp>
#include <unordered_map>
#include <vector>

using namespace fakelua;

// 生成不连续的 key 序列（使用质数步长来确保分散）
static std::vector<int64_t> GenerateSparseKeys(int n) {
    std::vector<int64_t> keys;
    keys.reserve(n);
    // 使用质数步长，确保 key 不连续且分散
    int64_t key = 0;
    for (int i = 0; i < n; ++i) {
        constexpr int64_t prime_step = 7919;
        keys.push_back(key);
        key += prime_step;
    }
    return keys;
}

// ==================== Set 操作对比 ====================

static void BM_VarTable_Set(benchmark::State &state) {
    auto *s = FakeluaNewState();
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);
    std::vector<Var> var_keys;
    std::vector<Var> var_vals;
    for (int i = 0; i < n; ++i) {
        var_keys.emplace_back(static_cast<int64_t>(keys[i]));
        var_vals.emplace_back(static_cast<int64_t>(keys[i] * 2));
    }

    for (auto _: state) {
        Var table_var;
        table_var.SetTable(s);
        auto *table = table_var.GetTable();
        for (int i = 0; i < n; ++i) {
            table->Set(s, var_keys[i], var_vals[i], false);
        }
        benchmark::DoNotOptimize(table);
    }
    state.SetComplexityN(state.range(0));
    FakeluaDeleteState(s);
}

static void BM_StdUnorderedMap_Set(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);
    std::vector<int64_t> vals;
    for (int i = 0; i < n; ++i) {
        vals.emplace_back(keys[i] * 2);
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

static void BM_LuaTable_Set(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    // 预先准备好 lua_State，避免计入初始化时间
    lua_State *L = luaL_newstate();
    // 关闭 GC 避免干扰
    lua_gc(L, LUA_GCSTOP, 0);

    for (auto _: state) {
        state.PauseTiming();
        lua_newtable(L);// 创建一个新表，放在栈顶
        state.ResumeTiming();

        for (int i = 0; i < n; ++i) {
            lua_pushinteger(L, keys[i]);    // 压入 key
            lua_pushinteger(L, keys[i] * 2);// 压入 value
            lua_settable(L, -3);            // t[key] = value
        }

        benchmark::DoNotOptimize(lua_topointer(L, -1));
        state.PauseTiming();
        lua_pop(L, 1);// 弹出表
        state.ResumeTiming();
    }

    lua_close(L);
    state.SetComplexityN(state.range(0));
}

// ==================== Get 操作对比 ====================

static void BM_VarTable_Get(benchmark::State &state) {
    auto *s = FakeluaNewState();
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    Var table_var;
    table_var.SetTable(s);
    auto *table = table_var.GetTable();
    std::vector<Var> var_keys;
    for (int i = 0; i < n; ++i) {
        Var key(static_cast<int64_t>(keys[i]));
        var_keys.push_back(key);
        table->Set(s, key, Var(static_cast<int64_t>(keys[i] * 2)), false);
    }

    for (auto _: state) {
        for (int i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(table->Get(var_keys[i]));
        }
    }
    state.SetComplexityN(state.range(0));
    FakeluaDeleteState(s);
}

static void BM_StdUnorderedMap_Get(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    std::unordered_map<int64_t, int64_t> m;
    for (int i = 0; i < n; ++i) {
        m[keys[i]] = keys[i] * 2;
    }

    for (auto _: state) {
        for (int i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(m[keys[i]]);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_LuaTable_Get(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    // 预先创建并填充表
    lua_State *L = luaL_newstate();
    lua_gc(L, LUA_GCSTOP, 0);// 关闭 GC
    lua_newtable(L);
    for (int i = 0; i < n; ++i) {
        lua_pushinteger(L, keys[i]);
        lua_pushinteger(L, keys[i] * 2);
        lua_settable(L, -3);
    }

    for (auto _: state) {
        for (int i = 0; i < n; ++i) {
            lua_pushinteger(L, keys[i]);// 压入 key
            lua_gettable(L, -2);        // 获取 value
            benchmark::DoNotOptimize(lua_tointeger(L, -1));
            lua_pop(L, 1);// 弹出 value
        }
    }

    lua_close(L);
    state.SetComplexityN(state.range(0));
}

// ==================== Iterate 操作对比 ====================

static void BM_VarTable_Iter(benchmark::State &state) {
    auto *s = FakeluaNewState();
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    Var table_var;
    table_var.SetTable(s);
    auto *table = table_var.GetTable();
    for (int i = 0; i < n; ++i) {
        table->Set(s, Var(static_cast<int64_t>(keys[i])), Var(static_cast<int64_t>(keys[i] * 2)), false);
    }

    // 预先判定模式，模拟 JIT 高性能访问
    const auto count = static_cast<uint32_t>(table->Size());
    const VarTable::VarEntry *quick_data = table->GetQuickData();
    const auto *nodes = table->GetNodes();
    const uint32_t *active_list = table->GetActiveList();

    for (auto _: state) {
        if (active_list == nullptr) {// Quick Path
            for (uint32_t i = 0; i < count; ++i) {
                const auto &entry = quick_data[i];
                benchmark::DoNotOptimize(entry.key);
                benchmark::DoNotOptimize(entry.val);
            }
        } else {// Hash Table Path
            for (uint32_t i = 0; i < count; ++i) {
                const auto &entry = nodes[active_list[i]].entry;
                benchmark::DoNotOptimize(entry.key);
                benchmark::DoNotOptimize(entry.val);
            }
        }
    }
    state.SetComplexityN(state.range(0));
    FakeluaDeleteState(s);
}

static void BM_StdUnorderedMap_Iter(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    std::unordered_map<int64_t, int64_t> m;
    for (int i = 0; i < n; ++i) {
        m[keys[i]] = keys[i] * 2;
    }

    for (auto _: state) {
        for (auto &[fst, snd]: m) {
            benchmark::DoNotOptimize(fst);
            benchmark::DoNotOptimize(snd);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_LuaTable_Iter(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    // 预先创建并填充表
    lua_State *L = luaL_newstate();
    lua_gc(L, LUA_GCSTOP, 0);// 关闭 GC
    lua_newtable(L);
    for (int i = 0; i < n; ++i) {
        lua_pushinteger(L, keys[i]);
        lua_pushinteger(L, keys[i] * 2);
        lua_settable(L, -3);
    }

    for (auto _: state) {
        lua_pushnil(L);// 第一个 key
        while (lua_next(L, -2) != 0) {
            // key 在 -2，value 在 -1
            benchmark::DoNotOptimize(lua_tointeger(L, -2));// key
            benchmark::DoNotOptimize(lua_tointeger(L, -1));// value
            lua_pop(L, 1);                                 // 弹出 value，保留 key 用于下次迭代
        }
    }

    lua_close(L);
    state.SetComplexityN(state.range(0));
}

// ==================== Delete 操作对比 ====================

static void BM_VarTable_Del(benchmark::State &state) {
    auto *s = FakeluaNewState();
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    std::vector<Var> var_keys;
    std::vector<Var> var_vals;
    for (int i = 0; i < n; ++i) {
        var_keys.emplace_back(static_cast<int64_t>(keys[i]));
        var_vals.emplace_back(static_cast<int64_t>(keys[i] * 2));
    }

    for (auto _: state) {
        state.PauseTiming();
        Var table_var;
        table_var.SetTable(s);
        auto *table = table_var.GetTable();
        for (int i = 0; i < n; ++i) {
            table->Set(s, var_keys[i], var_vals[i], false);
        }
        state.ResumeTiming();

        for (int i = 0; i < n; ++i) {
            table->Set(s, var_keys[i], Var(), false);
        }
        benchmark::DoNotOptimize(table);
    }
    state.SetComplexityN(state.range(0));
    FakeluaDeleteState(s);
}

static void BM_StdUnorderedMap_Del(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    std::vector<int64_t> vals;
    for (int i = 0; i < n; ++i) {
        vals.emplace_back(keys[i] * 2);
    }

    for (auto _: state) {
        state.PauseTiming();
        std::unordered_map<int64_t, int64_t> m;
        for (int i = 0; i < n; ++i) {
            m[keys[i]] = vals[i];
        }
        state.ResumeTiming();

        for (int i = 0; i < n; ++i) {
            m.erase(keys[i]);
        }
        benchmark::DoNotOptimize(m);
    }
    state.SetComplexityN(state.range(0));
}

static void BM_LuaTable_Del(benchmark::State &state) {
    const int n = static_cast<int>(state.range(0));
    const auto keys = GenerateSparseKeys(n);

    // 预先准备好 lua_State，避免计入初始化时间
    lua_State *L = luaL_newstate();
    lua_gc(L, LUA_GCSTOP, 0);// 关闭 GC

    for (auto _: state) {
        state.PauseTiming();
        lua_newtable(L);
        for (int i = 0; i < n; ++i) {
            lua_pushinteger(L, keys[i]);
            lua_pushinteger(L, keys[i] * 2);
            lua_settable(L, -3);
        }
        state.ResumeTiming();

        for (int i = 0; i < n; ++i) {
            lua_pushinteger(L, keys[i]);
            lua_pushnil(L);
            lua_settable(L, -3);// t[key] = nil 即删除
        }
        benchmark::DoNotOptimize(lua_topointer(L, -1));

        state.PauseTiming();
        lua_pop(L, 1);// 弹出表
        state.ResumeTiming();
    }

    lua_close(L);
    state.SetComplexityN(state.range(0));
}

// 注册测试，覆盖 quick_data (<=4) 和 hash table (>4) 两种情况
#define ARGS ->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(128)->Arg(256)->Arg(512)->Arg(1024)

// Set 操作
BENCHMARK(BM_VarTable_Set) ARGS;
BENCHMARK(BM_StdUnorderedMap_Set) ARGS;
BENCHMARK(BM_LuaTable_Set) ARGS;

// Get 操作
BENCHMARK(BM_VarTable_Get) ARGS;
BENCHMARK(BM_StdUnorderedMap_Get) ARGS;
BENCHMARK(BM_LuaTable_Get) ARGS;

// Iterate 操作
BENCHMARK(BM_VarTable_Iter) ARGS;
BENCHMARK(BM_StdUnorderedMap_Iter) ARGS;
BENCHMARK(BM_LuaTable_Iter) ARGS;

// Delete 操作
BENCHMARK(BM_VarTable_Del) ARGS;
BENCHMARK(BM_StdUnorderedMap_Del) ARGS;
BENCHMARK(BM_LuaTable_Del) ARGS;
