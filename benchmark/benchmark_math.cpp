#include "benchmark_common.h"

#include <cmath>

namespace {

// ---------------------------------------------------------------------------
// Scripts
// ---------------------------------------------------------------------------

constexpr const char *kMathTrigScript = R"(
function bench_math_trig(n)
    local sum = 0.0
    for i = 1, n do
        sum = sum + math.sin(i) + math.cos(i)
    end
    return sum
end
)";

constexpr const char *kMathSqrtScript = R"(
function bench_math_sqrt(n)
    local sum = 0.0
    for i = 1, n do
        sum = sum + math.sqrt(i)
    end
    return sum
end
)";

constexpr const char *kMathExpLogScript = R"(
function bench_math_exp_log(n)
    local sum = 0.0
    for i = 1, n do
        sum = sum + math.exp(i * 0.0001) + math.log(i)
    end
    return sum
end
)";

constexpr const char *kMathAtan2Script = R"(
function bench_math_atan2(n)
    local sum = 0.0
    for i = 1, n do
        sum = sum + math.atan2(i, n - i + 1)
    end
    return sum
end
)";

constexpr const char *kMathPowScript = R"(
function bench_math_pow(n)
    local sum = 0.0
    for i = 1, n do
        sum = sum + math.pow(i, 0.5)
    end
    return sum
end
)";

constexpr const char *kMathMinMaxScript = R"(
function bench_math_minmax(n)
    local min_v = math.huge
    local max_v = -math.huge
    for i = 1, n do
        local v = math.sin(i) * 100.0
        min_v = math.min(min_v, v)
        max_v = math.max(max_v, v)
    end
    return min_v + max_v
end
)";

// ---------------------------------------------------------------------------
// C++ reference implementations
// ---------------------------------------------------------------------------

double CppMathTrig(const int64_t n) {
    double sum = 0.0;
    for (int64_t i = 1; i <= n; ++i) {
        sum += std::sin(static_cast<double>(i)) + std::cos(static_cast<double>(i));
    }
    return sum;
}

double CppMathSqrt(const int64_t n) {
    double sum = 0.0;
    for (int64_t i = 1; i <= n; ++i) {
        sum += std::sqrt(static_cast<double>(i));
    }
    return sum;
}

double CppMathExpLog(const int64_t n) {
    double sum = 0.0;
    for (int64_t i = 1; i <= n; ++i) {
        sum += std::exp(static_cast<double>(i) * 0.0001) + std::log(static_cast<double>(i));
    }
    return sum;
}

double CppMathAtan2(const int64_t n) {
    double sum = 0.0;
    for (int64_t i = 1; i <= n; ++i) {
        sum += std::atan2(static_cast<double>(i), static_cast<double>(n - i + 1));
    }
    return sum;
}

double CppMathPow(const int64_t n) {
    double sum = 0.0;
    for (int64_t i = 1; i <= n; ++i) {
        sum += std::pow(static_cast<double>(i), 0.5);
    }
    return sum;
}

double CppMathMinMax(const int64_t n) {
    double min_v = std::numeric_limits<double>::infinity();
    double max_v = -std::numeric_limits<double>::infinity();
    for (int64_t i = 1; i <= n; ++i) {
        const double v = std::sin(static_cast<double>(i)) * 100.0;
        min_v = std::min(min_v, v);
        max_v = std::max(max_v, v);
    }
    return min_v + max_v;
}

// ---------------------------------------------------------------------------
// Lua helpers
// ---------------------------------------------------------------------------

const char *const kMathScripts[] = {
    kMathTrigScript, kMathSqrtScript, kMathExpLogScript,
    kMathAtan2Script, kMathPowScript, kMathMinMaxScript,
};
constexpr size_t kMathScriptCount = sizeof(kMathScripts) / sizeof(kMathScripts[0]);

struct Ctx : RuntimeContext {
    Ctx() {
        Init(kMathScripts, kMathScriptCount);
        // Warmup (TCC only)
        double w = 0;
        Call(flua, JIT_TCC, "bench_math_trig", w, 10);
        Call(flua, JIT_TCC, "bench_math_sqrt", w, 10);
        Call(flua, JIT_TCC, "bench_math_exp_log", w, 10);
        Call(flua, JIT_TCC, "bench_math_atan2", w, 10);
        Call(flua, JIT_TCC, "bench_math_pow", w, 10);
        Call(flua, JIT_TCC, "bench_math_minmax", w, 10);
    }
    ~Ctx() { Destroy(); }
} g_ctx;

// ---------------------------------------------------------------------------
// Benchmarks: math trig
// ---------------------------------------------------------------------------

static void BM_CPP_MathTrig(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CppMathTrig(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MathTrig(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CallLuaDouble(g_ctx.lua, "bench_math_trig", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathTrig_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_math_trig", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathTrig_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_math_trig", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: math sqrt
// ---------------------------------------------------------------------------

static void BM_CPP_MathSqrt(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CppMathSqrt(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MathSqrt(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CallLuaDouble(g_ctx.lua, "bench_math_sqrt", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathSqrt_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_math_sqrt", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathSqrt_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_math_sqrt", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: math exp/log
// ---------------------------------------------------------------------------

static void BM_CPP_MathExpLog(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CppMathExpLog(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MathExpLog(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CallLuaDouble(g_ctx.lua, "bench_math_exp_log", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathExpLog_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_math_exp_log", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathExpLog_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_math_exp_log", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: math atan2
// ---------------------------------------------------------------------------

static void BM_CPP_MathAtan2(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CppMathAtan2(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MathAtan2(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CallLuaDouble(g_ctx.lua, "bench_math_atan2", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathAtan2_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_math_atan2", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathAtan2_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_math_atan2", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: math pow
// ---------------------------------------------------------------------------

static void BM_CPP_MathPow(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CppMathPow(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MathPow(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CallLuaDouble(g_ctx.lua, "bench_math_pow", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathPow_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_math_pow", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathPow_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_math_pow", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: math minmax
// ---------------------------------------------------------------------------

static void BM_CPP_MathMinMax(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CppMathMinMax(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MathMinMax(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        const double ret = CallLuaDouble(g_ctx.lua, "bench_math_minmax", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathMinMax_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_math_minmax", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MathMinMax_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        double ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_math_minmax", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Benchmark registrations
// ---------------------------------------------------------------------------

#define MATH_ARGS ->Arg(100000)

BENCHMARK(BM_CPP_MathTrig) MATH_ARGS;
BENCHMARK(BM_Lua_MathTrig) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathTrig_TCC) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathTrig_GCC) MATH_ARGS;

BENCHMARK(BM_CPP_MathSqrt) MATH_ARGS;
BENCHMARK(BM_Lua_MathSqrt) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathSqrt_TCC) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathSqrt_GCC) MATH_ARGS;

BENCHMARK(BM_CPP_MathExpLog) MATH_ARGS;
BENCHMARK(BM_Lua_MathExpLog) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathExpLog_TCC) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathExpLog_GCC) MATH_ARGS;

BENCHMARK(BM_CPP_MathAtan2) MATH_ARGS;
BENCHMARK(BM_Lua_MathAtan2) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathAtan2_TCC) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathAtan2_GCC) MATH_ARGS;

BENCHMARK(BM_CPP_MathPow) MATH_ARGS;
BENCHMARK(BM_Lua_MathPow) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathPow_TCC) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathPow_GCC) MATH_ARGS;

BENCHMARK(BM_CPP_MathMinMax) MATH_ARGS;
BENCHMARK(BM_Lua_MathMinMax) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathMinMax_TCC) MATH_ARGS;
BENCHMARK(BM_FakeLua_MathMinMax_GCC) MATH_ARGS;
