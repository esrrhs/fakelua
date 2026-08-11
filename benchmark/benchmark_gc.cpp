#include "benchmark_common.h"

#include <vector>

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

const char *const kGcScripts[] = {kTableChurnScript, kStringChurnScript, kMixedAllocScript};
constexpr size_t kGcScriptCount = sizeof(kGcScripts) / sizeof(kGcScripts[0]);

struct Ctx : RuntimeContext {
    Ctx() {
        Init(kGcScripts, kGcScriptCount);
        // Warmup (TCC only)
        int64_t w = 0;
        Call(flua, JIT_TCC, "bench_table_churn", w, 10);
        Call(flua, JIT_TCC, "bench_string_churn", w, 10);
        Call(flua, JIT_TCC, "bench_mixed_alloc", w, 10);
    }
    ~Ctx() { Destroy(); }
} g_ctx;

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
