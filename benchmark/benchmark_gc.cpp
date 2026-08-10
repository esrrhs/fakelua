#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace fakelua;

namespace {

// ---------------------------------------------------------------------------
// Scripts
// ---------------------------------------------------------------------------

constexpr const char *kTableChurnScript = R"(
function bench_table_churn(n)
    local total = 0
    for i = 1, n do
        local t = {}
        t[1] = i
        t[2] = i * 2
        t[3] = i * 3
        total = total + t[1] + t[2] + t[3]
    end
    return total
end
)";

constexpr const char *kStringChurnScript = R"(
function bench_string_churn(n)
    local total = 0
    for i = 1, n do
        local s = tostring(i) .. "-" .. tostring(i * 2) .. "-data"
        total = total + #s
    end
    return total
end
)";

constexpr const char *kMixedAllocScript = R"(
function bench_mixed_alloc(n)
    local total = 0
    for i = 1, n do
        local t = {x = i, y = i * 2, name = tostring(i)}
        local s = t.name .. "-extra"
        t.extra = s
        total = total + t.x + t.y + #s
    end
    return total
end
)";

// ---------------------------------------------------------------------------
// C++ reference implementations
// ---------------------------------------------------------------------------

int64_t CppTableChurn(int64_t n) {
    int64_t total = 0;
    for (int64_t i = 1; i <= n; ++i) {
        std::vector<int64_t> t = {i, i * 2, i * 3};
        total += t[0] + t[1] + t[2];
    }
    return total;
}

int64_t CppStringChurn(int64_t n) {
    int64_t total = 0;
    for (int64_t i = 1; i <= n; ++i) {
        std::string s = std::to_string(i) + "-" + std::to_string(i * 2) + "-data";
        total += static_cast<int64_t>(s.size());
    }
    return total;
}

int64_t CppMixedAlloc(int64_t n) {
    int64_t total = 0;
    for (int64_t i = 1; i <= n; ++i) {
        struct {
            int64_t x, y;
            std::string name, extra;
        } obj;
        obj.x = i;
        obj.y = i * 2;
        obj.name = std::to_string(i);
        obj.extra = obj.name + "-extra";
        total += obj.x + obj.y + static_cast<int64_t>(obj.extra.size());
    }
    return total;
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

        const char *scripts[] = {kTableChurnScript, kStringChurnScript, kMixedAllocScript};
        for (const char *script: scripts) {
            if (luaL_dostring(lua, script) != LUA_OK) {
                const char *err = lua_tostring(lua, -1);
                throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
            }
        }

        flua = FakeluaNewState();
        for (const char *script: scripts) {
            CompileString(flua, script, {.debug_mode = false});
        }

        // Warmup (TCC only)
        int64_t w = 0;
        Call(flua, JIT_TCC, "bench_table_churn", w, 10);
        Call(flua, JIT_TCC, "bench_string_churn", w, 10);
        Call(flua, JIT_TCC, "bench_mixed_alloc", w, 10);
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
// Benchmarks: table churn
// ---------------------------------------------------------------------------

static void BM_CPP_TableChurn(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppTableChurn(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_TableChurn(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_table_churn", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableChurn_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_table_churn", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TableChurn_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_table_churn", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string churn
// ---------------------------------------------------------------------------

static void BM_CPP_StringChurn(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppStringChurn(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_StringChurn(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_string_churn", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringChurn_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_string_churn", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringChurn_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_string_churn", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: mixed allocation
// ---------------------------------------------------------------------------

static void BM_CPP_MixedAlloc(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppMixedAlloc(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MixedAlloc(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_mixed_alloc", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MixedAlloc_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_mixed_alloc", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MixedAlloc_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_mixed_alloc", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Benchmark registrations
// ---------------------------------------------------------------------------

#define TABLE_CHURN_ARGS  ->Arg(100)->Arg(500)->Arg(1000)
#define STRING_CHURN_ARGS ->Arg(100)->Arg(500)->Arg(1000)
#define MIXED_ALLOC_ARGS  ->Arg(100)->Arg(500)->Arg(1000)

BENCHMARK(BM_CPP_TableChurn) TABLE_CHURN_ARGS;
BENCHMARK(BM_Lua_TableChurn) TABLE_CHURN_ARGS;
BENCHMARK(BM_FakeLua_TableChurn_TCC) TABLE_CHURN_ARGS;
BENCHMARK(BM_FakeLua_TableChurn_GCC) TABLE_CHURN_ARGS;

BENCHMARK(BM_CPP_StringChurn) STRING_CHURN_ARGS;
BENCHMARK(BM_Lua_StringChurn) STRING_CHURN_ARGS;
BENCHMARK(BM_FakeLua_StringChurn_TCC) STRING_CHURN_ARGS;
BENCHMARK(BM_FakeLua_StringChurn_GCC) STRING_CHURN_ARGS;

BENCHMARK(BM_CPP_MixedAlloc) MIXED_ALLOC_ARGS;
BENCHMARK(BM_Lua_MixedAlloc) MIXED_ALLOC_ARGS;
BENCHMARK(BM_FakeLua_MixedAlloc_TCC) MIXED_ALLOC_ARGS;
BENCHMARK(BM_FakeLua_MixedAlloc_GCC) MIXED_ALLOC_ARGS;
