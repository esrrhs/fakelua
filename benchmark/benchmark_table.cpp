#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace fakelua;

namespace {

// ---------------------------------------------------------------------------
// Scripts
// ---------------------------------------------------------------------------

constexpr const char *kTableInsertScript = R"(
function bench_table_insert(n)
    local t = {}
    for i = 1, n do
        table.insert(t, i)
    end
    return #t
end
)";

constexpr const char *kTableRemoveScript = R"(
function bench_table_remove(n)
    local t = {}
    for i = 1, n do
        t[i] = i
    end
    for i = n, 1, -1 do
        table.remove(t)
    end
    return #t
end
)";

constexpr const char *kTableConcatScript = R"(
function bench_table_concat(n)
    local t = {}
    for i = 1, n do
        t[i] = tostring(i)
    end
    local s = table.concat(t)
    return #s
end
)";

constexpr const char *kTablePackScript = R"(
function bench_table_pack(n)
    local t = {}
    for i = 1, 10 do
        t[i] = i
    end
    t.n = 10
    return t.n
end
)";

constexpr const char *kTableMoveScript = R"(
function bench_table_move(n)
    local src = {}
    for i = 1, n do
        src[i] = i
    end
    local dst = {}
    table.move(src, 1, n, 1, dst)
    return #dst
end
)";

constexpr const char *kTableSortScript = R"(
function bench_table_sort(n)
    local t = {}
    for i = 1, n do
        t[i] = (i * 12345) % (n * 10)
    end
    table.sort(t)
    return t[1]
end
)";

constexpr const char *kTableCreateScript = R"(
function bench_table_create(n)
    local t = {}
    for i = 1, n do
        t[i] = i * 2
    end
    return #t
end
)";

constexpr const char *kHashInsertScript = R"(
function bench_hash_insert(n)
    local t = {}
    for i = 1, n do
        local key = "k" .. tostring(i)
        t[key] = i
    end
    return #t or n
end
)";

constexpr const char *kHashLookupScript = R"(
local hash_t = {}
for i = 1, 1000 do
    hash_t["k" .. tostring(i)] = i
end

function bench_hash_lookup(n)
    local sum = 0
    for i = 1, n do
        sum = sum + hash_t["k" .. tostring(i)]
    end
    return sum
end
)";

constexpr const char *kNestedTableScript = R"(
local deep = {}
local cur = deep
for i = 1, 10 do
    cur.a = {}
    cur = cur.a
end
cur.value = 42

function bench_nested_table(n)
    local sum = 0
    for i = 1, n do
        local cur2 = deep
        while cur2.a do
            cur2 = cur2.a
        end
        sum = sum + cur2.value
    end
    return sum
end
)";

// ---------------------------------------------------------------------------
// C++ reference implementations
// ---------------------------------------------------------------------------

int64_t CppTableInsert(int64_t n) {
    std::vector<int64_t> t;
    t.reserve(static_cast<size_t>(n));
    for (int64_t i = 1; i <= n; ++i) {
        t.push_back(i);
    }
    return static_cast<int64_t>(t.size());
}

int64_t CppTableRemove(int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = i + 1;
    }
    for (int64_t i = 0; i < n; ++i) {
        t.pop_back();
    }
    return static_cast<int64_t>(t.size());
}

int64_t CppTableConcat(int64_t n) {
    std::string s;
    for (int64_t i = 1; i <= n; ++i) {
        s += std::to_string(i);
    }
    return static_cast<int64_t>(s.size());
}

int64_t CppTableMove(int64_t n) {
    std::vector<int64_t> src(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        src[static_cast<size_t>(i)] = i + 1;
    }
    std::vector<int64_t> dst(src.size());
    std::copy(src.begin(), src.end(), dst.begin());
    return static_cast<int64_t>(dst.size());
}

int64_t CppTableSort(int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = ((i + 1) * 12345) % (n * 10); // pseudo-random
    }
    std::sort(t.begin(), t.end());
    return t[0];
}

int64_t CppTableCreate(int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = (i + 1) * 2;
    }
    return static_cast<int64_t>(t.size());
}

int64_t CppHashInsert(int64_t n) {
    std::unordered_map<std::string, int64_t> t;
    for (int64_t i = 1; i <= n; ++i) {
        t["k" + std::to_string(i)] = i;
    }
    return static_cast<int64_t>(t.size());
}

int64_t CppHashLookup(int64_t n) {
    // Build map first
    std::unordered_map<std::string, int64_t> m;
    for (int64_t i = 1; i <= 1000; ++i) {
        m["k" + std::to_string(i)] = i;
    }
    int64_t sum = 0;
    for (int64_t i = 1; i <= n; ++i) {
        sum += m.at("k" + std::to_string(i));
    }
    return sum;
}

int64_t CppNestedTable(int64_t n) {
    struct Node {
        Node *a = nullptr;
        int64_t value = 0;
    };
    Node deep;
    Node *cur = &deep;
    for (int i = 0; i < 10; ++i) {
        cur->a = new Node();
        cur = cur->a;
    }
    cur->value = 42;

    int64_t sum = 0;
    for (int64_t i = 0; i < n; ++i) {
        Node *cur2 = &deep;
        while (cur2->a) {
            cur2 = cur2->a;
        }
        sum += cur2->value;
    }

    // Clean up
    cur = deep.a;
    while (cur) {
        Node *next = cur->a;
        delete cur;
        cur = next;
    }
    return sum;
}

// ---------------------------------------------------------------------------
// Lua helpers
// ---------------------------------------------------------------------------

void PushLuaArg(lua_State *L, int64_t value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void PushLuaArg(lua_State *L, const std::string &value) {
    lua_pushlstring(L, value.c_str(), value.size());
}

void PushLuaArgs(lua_State *) {}

template<typename T, typename... Args>
void PushLuaArgs(lua_State *L, T first, Args... args) {
    PushLuaArg(L, first);
    PushLuaArgs(L, args...);
}

template<typename... Args>
int64_t CallLuaInt(lua_State *L, const char *func_name, Args... args) {
    const int top = lua_gettop(L);
    lua_getglobal(L, func_name);
    if (!lua_isfunction(L, -1)) {
        lua_settop(L, top);
        throw std::runtime_error(std::string("Lua function not found: ") + func_name);
    }
    PushLuaArgs(L, std::forward<Args>(args)...);
    constexpr int nargs = sizeof...(Args);
    if (const int code = lua_pcall(L, nargs, 1, 0); code != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        std::string msg = err ? err : "unknown lua error";
        lua_settop(L, top);
        throw std::runtime_error("Lua call failed: " + msg);
    }
    const auto ret = static_cast<int64_t>(lua_tointeger(L, -1));
    lua_settop(L, top);
    return ret;
}

// ---------------------------------------------------------------------------
// RuntimeContext
// ---------------------------------------------------------------------------

struct RuntimeContext {
    RuntimeContext() {
        lua = luaL_newstate();
        luaL_openlibs(lua);

        const char *lua_scripts[] = {
            kTableInsertScript, kTableRemoveScript, kTableConcatScript,
            kTablePackScript,   kTableMoveScript,   kTableSortScript,
            kTableCreateScript, kHashInsertScript,  kHashLookupScript,
            kNestedTableScript,
        };
        for (const char *script: lua_scripts) {
            if (luaL_dostring(lua, script) != LUA_OK) {
                const char *err = lua_tostring(lua, -1);
                throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
            }
        }

        flua = FakeluaNewState();
        for (const char *script: lua_scripts) {
            CompileString(flua, script, {.debug_mode = false});
        }

        // Warmup (TCC only)
        int64_t warmup = 0;
        Call(flua, JIT_TCC, "bench_table_insert", warmup, 10);
        Call(flua, JIT_TCC, "bench_table_remove", warmup, 10);
        Call(flua, JIT_TCC, "bench_table_concat", warmup, 10);
        Call(flua, JIT_TCC, "bench_table_pack", warmup, 10);
        Call(flua, JIT_TCC, "bench_table_move", warmup, 10);
        Call(flua, JIT_TCC, "bench_table_sort", warmup, 10);
        Call(flua, JIT_TCC, "bench_table_create", warmup, 10);
        Call(flua, JIT_TCC, "bench_hash_insert", warmup, 10);
        Call(flua, JIT_TCC, "bench_hash_lookup", warmup, 10);
        Call(flua, JIT_TCC, "bench_nested_table", warmup, 10);
    }

    ~RuntimeContext() {
        if (lua) {
            lua_close(lua);
            lua = nullptr;
        }
        if (flua) {
            FakeluaDeleteState(flua);
            flua = nullptr;
        }
    }

    lua_State *lua = nullptr;
    State *flua = nullptr;
};

RuntimeContext g_ctx;

void VerifyEqual(int64_t got, int64_t expected, const char *name) {
    if (got != expected) {
        throw std::runtime_error(std::string(name) + " wrong result: got " + std::to_string(got) + ", expected " + std::to_string(expected));
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.insert
// ---------------------------------------------------------------------------

static void BM_CPP_TableInsert(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = n;
    for (auto _: state) {
        int64_t ret = CppTableInsert(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ table.insert");
    }
}

static void BM_Lua_TableInsert(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_insert", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableInsert_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_insert", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableInsert_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_insert", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.remove
// ---------------------------------------------------------------------------

static void BM_CPP_TableRemove(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = 0;
    for (auto _: state) {
        int64_t ret = CppTableRemove(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ table.remove");
    }
}

static void BM_Lua_TableRemove(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_remove", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableRemove_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_remove", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableRemove_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_remove", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.concat
// ---------------------------------------------------------------------------

static void BM_CPP_TableConcat(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppTableConcat(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_TableConcat(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_concat", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableConcat_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_concat", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableConcat_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_concat", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.pack (fixed-size pack/unpack)
// ---------------------------------------------------------------------------

static void BM_CPP_TablePack(benchmark::State &state) {
    // C++ doesn't have a direct equivalent; just measure a struct pack
    for (auto _: state) {
        int64_t values[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        int64_t ret = static_cast<int64_t>(sizeof(values) / sizeof(values[0]));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_TablePack(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_pack", 10);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TablePack_TCC(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_pack", ret, 10);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TablePack_GCC(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_pack", ret, 10);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.move
// ---------------------------------------------------------------------------

static void BM_CPP_TableMove(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = n;
    for (auto _: state) {
        int64_t ret = CppTableMove(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ table.move");
    }
}

static void BM_Lua_TableMove(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_move", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableMove_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_move", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableMove_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_move", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.sort
// ---------------------------------------------------------------------------

static void BM_CPP_TableSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppTableSort(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_TableSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_sort", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableSort_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_sort", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableSort_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_sort", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: table.create
// ---------------------------------------------------------------------------

static void BM_CPP_TableCreate(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = n;
    for (auto _: state) {
        int64_t ret = CppTableCreate(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ table.create");
    }
}

static void BM_Lua_TableCreate(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_create", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableCreate_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_create", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableCreate_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_create", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: hash insert
// ---------------------------------------------------------------------------

static void BM_CPP_HashInsert(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = n;
    for (auto _: state) {
        int64_t ret = CppHashInsert(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ hash_insert");
    }
}

static void BM_Lua_HashInsert(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_hash_insert", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_HashInsert_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_hash_insert", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_HashInsert_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_hash_insert", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: hash lookup
// ---------------------------------------------------------------------------

static void BM_CPP_HashLookup(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppHashLookup(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_HashLookup(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_hash_lookup", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_HashLookup_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_hash_lookup", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_HashLookup_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_hash_lookup", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: nested table access
// ---------------------------------------------------------------------------

static void BM_CPP_NestedTable(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppNestedTable(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_NestedTable(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_nested_table", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_NestedTable_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_nested_table", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_NestedTable_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_nested_table", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Benchmark registrations
// ---------------------------------------------------------------------------

#define TABLE_INSERT_ARGS  ->Arg(100)->Arg(500)->Arg(1000)->Arg(5000)
#define TABLE_REMOVE_ARGS  ->Arg(100)->Arg(500)->Arg(1000)->Arg(5000)
#define TABLE_CONCAT_ARGS  ->Arg(100)->Arg(500)->Arg(1000)
#define TABLE_PACK_ARGS    ->Arg(1)
#define TABLE_MOVE_ARGS    ->Arg(100)->Arg(500)->Arg(1000)->Arg(5000)
#define TABLE_SORT_ARGS    ->Arg(100)->Arg(500)->Arg(1000)
#define TABLE_CREATE_ARGS  ->Arg(1000)->Arg(3000)->Arg(5000)
#define HASH_INSERT_ARGS   ->Arg(100)->Arg(500)->Arg(1000)
#define HASH_LOOKUP_ARGS   ->Arg(100)->Arg(500)->Arg(1000)
#define NESTED_TABLE_ARGS  ->Arg(1000)->Arg(10000)

BENCHMARK(BM_CPP_TableInsert) TABLE_INSERT_ARGS;
BENCHMARK(BM_Lua_TableInsert) TABLE_INSERT_ARGS;
BENCHMARK(BM_FakeLua_TableInsert_TCC) TABLE_INSERT_ARGS;
BENCHMARK(BM_FakeLua_TableInsert_GCC) TABLE_INSERT_ARGS;

BENCHMARK(BM_CPP_TableRemove) TABLE_REMOVE_ARGS;
BENCHMARK(BM_Lua_TableRemove) TABLE_REMOVE_ARGS;
BENCHMARK(BM_FakeLua_TableRemove_TCC) TABLE_REMOVE_ARGS;
BENCHMARK(BM_FakeLua_TableRemove_GCC) TABLE_REMOVE_ARGS;

BENCHMARK(BM_CPP_TableConcat) TABLE_CONCAT_ARGS;
BENCHMARK(BM_Lua_TableConcat) TABLE_CONCAT_ARGS;
BENCHMARK(BM_FakeLua_TableConcat_TCC) TABLE_CONCAT_ARGS;
BENCHMARK(BM_FakeLua_TableConcat_GCC) TABLE_CONCAT_ARGS;

BENCHMARK(BM_CPP_TablePack) TABLE_PACK_ARGS;
BENCHMARK(BM_Lua_TablePack) TABLE_PACK_ARGS;
BENCHMARK(BM_FakeLua_TablePack_TCC) TABLE_PACK_ARGS;
BENCHMARK(BM_FakeLua_TablePack_GCC) TABLE_PACK_ARGS;

BENCHMARK(BM_CPP_TableMove) TABLE_MOVE_ARGS;
BENCHMARK(BM_Lua_TableMove) TABLE_MOVE_ARGS;
BENCHMARK(BM_FakeLua_TableMove_TCC) TABLE_MOVE_ARGS;
BENCHMARK(BM_FakeLua_TableMove_GCC) TABLE_MOVE_ARGS;

BENCHMARK(BM_CPP_TableSort) TABLE_SORT_ARGS;
BENCHMARK(BM_Lua_TableSort) TABLE_SORT_ARGS;
BENCHMARK(BM_FakeLua_TableSort_TCC) TABLE_SORT_ARGS;
BENCHMARK(BM_FakeLua_TableSort_GCC) TABLE_SORT_ARGS;

BENCHMARK(BM_CPP_TableCreate) TABLE_CREATE_ARGS;
BENCHMARK(BM_Lua_TableCreate) TABLE_CREATE_ARGS;
BENCHMARK(BM_FakeLua_TableCreate_TCC) TABLE_CREATE_ARGS;
BENCHMARK(BM_FakeLua_TableCreate_GCC) TABLE_CREATE_ARGS;

BENCHMARK(BM_CPP_HashInsert) HASH_INSERT_ARGS;
BENCHMARK(BM_Lua_HashInsert) HASH_INSERT_ARGS;
BENCHMARK(BM_FakeLua_HashInsert_TCC) HASH_INSERT_ARGS;
BENCHMARK(BM_FakeLua_HashInsert_GCC) HASH_INSERT_ARGS;

BENCHMARK(BM_CPP_HashLookup) HASH_LOOKUP_ARGS;
BENCHMARK(BM_Lua_HashLookup) HASH_LOOKUP_ARGS;
BENCHMARK(BM_FakeLua_HashLookup_TCC) HASH_LOOKUP_ARGS;
BENCHMARK(BM_FakeLua_HashLookup_GCC) HASH_LOOKUP_ARGS;

BENCHMARK(BM_CPP_NestedTable) NESTED_TABLE_ARGS;
BENCHMARK(BM_Lua_NestedTable) NESTED_TABLE_ARGS;
BENCHMARK(BM_FakeLua_NestedTable_TCC) NESTED_TABLE_ARGS;
BENCHMARK(BM_FakeLua_NestedTable_GCC) NESTED_TABLE_ARGS;
