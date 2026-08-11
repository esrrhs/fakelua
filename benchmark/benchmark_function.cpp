#include "benchmark_common.h"

#include <functional>

namespace {

// ---------------------------------------------------------------------------
// Scripts
// ---------------------------------------------------------------------------

constexpr const char *kEmptyCallScript = R"(
function bench_empty()
    return 0
end

function bench_call_empty(n)
    local total = 0
    for i = 1, n do
        total = total + bench_empty()
    end
    return total
end
)";

constexpr const char *kRecursionScript = R"(
function bench_fib(n)
    if n <= 1 then
        return n
    end
    return bench_fib(n - 1) + bench_fib(n - 2)
end
)";

constexpr const char *kVariadicScript = R"(
function bench_variadic(...)
    local n = select("#", ...)
    local s = 0
    for i = 1, n do
        s = s + select(i, ...)
    end
    return s
end
)";

constexpr const char *kMultiReturnScript = R"(
function bench_multi_return(n)
    local total = 0
    for i = 1, n do
        total = total + inner_multi(i)
    end
    return total
end

function inner_multi(x)
    return x + (x * 2) + (x * 3)
end
)";

constexpr const char *kClosureScript = R"(
function make_adder(x)
    return function(y)
        return x + y
    end
end
function bench_closure(n)
    local total = 0
    for i = 1, n do
        local inc = make_adder(i)
        total = total + inc(i)
    end
    return total
end
)";

constexpr const char *kTailRecursionScript = R"(
function bench_tail_sum_acc(x, acc)
    if x <= 0 then
        return acc
    end
    return bench_tail_sum_acc(x - 1, acc + x)
end

function bench_tail_sum(n)
    return bench_tail_sum_acc(n, 0)
end
)";

// ---------------------------------------------------------------------------
// C++ reference implementations
// ---------------------------------------------------------------------------

inline int64_t CppEmpty(int64_t n) {
    int64_t total = 0;
    for (int64_t i = 0; i < n; ++i) {
        total += 0; // empty inline equivalent
    }
    return total;
}

int64_t CppFib(int64_t n) {
    if (n <= 1) {
        return n;
    }
    return CppFib(n - 1) + CppFib(n - 2);
}

int64_t CppVariadic(int64_t n) {
    // Variadic equivalent: sum the numbers 1..n
    int64_t s = 0;
    for (int64_t i = 1; i <= n; ++i) {
        s += i;
    }
    return s;
}

int64_t CppMultiReturn(int64_t n) {
    int64_t total = 0;
    for (int64_t i = 1; i <= n; ++i) {
        int64_t a = i;
        int64_t b = i * 2;
        int64_t c = i * 3;
        total += a + b + c;
    }
    return total;
}

int64_t CppClosure(int64_t n) {
    int64_t total = 0;
    for (int64_t i = 1; i <= n; ++i) {
        auto inc = [x = i](int64_t y) { return x + y; };
        total += inc(i);
    }
    return total;
}

int64_t CppTailSum(int64_t n) {
    int64_t acc = 0;
    for (int64_t x = n; x > 0; --x) {
        acc += x;
    }
    return acc;
}

// ---------------------------------------------------------------------------
// Lua helpers
// ---------------------------------------------------------------------------

const char *const kFunctionScripts[] = {
            kEmptyCallScript, kRecursionScript,  kVariadicScript,
            kMultiReturnScript, kClosureScript, kTailRecursionScript,
        };
constexpr size_t kFunctionScriptCount = sizeof(kFunctionScripts) / sizeof(kFunctionScripts[0]);

struct Ctx : RuntimeContext {
    Ctx() {
        Init(kFunctionScripts, kFunctionScriptCount);
        // Warmup (TCC only)
        int64_t w = 0;
        Call(flua, JIT_TCC, "bench_call_empty", w, 10);
        Call(flua, JIT_TCC, "bench_fib", w, 10);
        Call(flua, JIT_TCC, "bench_variadic", w, 1, 2, 3, 4, 5);
        Call(flua, JIT_TCC, "bench_multi_return", w, 10);
        Call(flua, JIT_TCC, "bench_closure", w, 10);
        Call(flua, JIT_TCC, "bench_tail_sum", w, 10);
    }
    ~Ctx() { Destroy(); }
} g_ctx;

// ---------------------------------------------------------------------------
// Benchmarks: empty call
// ---------------------------------------------------------------------------

static void BM_CPP_EmptyCall(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppEmpty(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_EmptyCall(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_call_empty", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_EmptyCall_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_call_empty", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_EmptyCall_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_call_empty", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: recursion (fib)
// ---------------------------------------------------------------------------

static void BM_CPP_Recursion(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for (auto _: state) {
        int64_t ret = CppFib(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ fib recourse");
    }
}

static void BM_Lua_Recursion(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_fib", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_Recursion_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_fib", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_Recursion_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_fib", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: variadic
// ---------------------------------------------------------------------------

static void BM_CPP_Variadic(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppVariadic(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_Variadic(benchmark::State &state) {
    for (auto _: state) {
        // Pass 1..5 as variadic args
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_variadic", 1, 2, 3, 4, 5);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_Variadic_TCC(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_variadic", ret, 1, 2, 3, 4, 5);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_Variadic_GCC(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_variadic", ret, 1, 2, 3, 4, 5);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: multi-return
// ---------------------------------------------------------------------------

static void BM_CPP_MultiReturn(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppMultiReturn(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_MultiReturn(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_multi_return", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MultiReturn_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_multi_return", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_MultiReturn_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_multi_return", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: closure
// ---------------------------------------------------------------------------

static void BM_CPP_Closure(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppClosure(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_Closure(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_closure", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_Closure_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_closure", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_Closure_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_closure", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: tail recursion
// ---------------------------------------------------------------------------

static void BM_CPP_TailRecursion(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CppTailSum(n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_TailRecursion(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_tail_sum", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TailRecursion_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_tail_sum", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_TailRecursion_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_tail_sum", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Benchmark registrations
// ---------------------------------------------------------------------------

#define EMPTY_CALL_ARGS    ->Arg(10000)->Arg(100000)
#define RECURSION_ARGS     ->Arg(10)->Arg(20)->Arg(25)
#define VARIADIC_ARGS      ->Arg(1)
#define MULTI_RETURN_ARGS  ->Arg(1000)->Arg(10000)
#define CLOSURE_ARGS       ->Arg(100)->Arg(1000)
#define TAIL_RECURSION_ARGS ->Arg(100)->Arg(1000)->Arg(5000)

BENCHMARK(BM_CPP_EmptyCall) EMPTY_CALL_ARGS;
BENCHMARK(BM_Lua_EmptyCall) EMPTY_CALL_ARGS;
BENCHMARK(BM_FakeLua_EmptyCall_TCC) EMPTY_CALL_ARGS;
BENCHMARK(BM_FakeLua_EmptyCall_GCC) EMPTY_CALL_ARGS;

BENCHMARK(BM_CPP_Recursion) RECURSION_ARGS;
BENCHMARK(BM_Lua_Recursion) RECURSION_ARGS;
BENCHMARK(BM_FakeLua_Recursion_TCC) RECURSION_ARGS;
BENCHMARK(BM_FakeLua_Recursion_GCC) RECURSION_ARGS;

BENCHMARK(BM_CPP_Variadic) VARIADIC_ARGS;
BENCHMARK(BM_Lua_Variadic) VARIADIC_ARGS;
BENCHMARK(BM_FakeLua_Variadic_TCC) VARIADIC_ARGS;
BENCHMARK(BM_FakeLua_Variadic_GCC) VARIADIC_ARGS;

BENCHMARK(BM_CPP_MultiReturn) MULTI_RETURN_ARGS;
BENCHMARK(BM_Lua_MultiReturn) MULTI_RETURN_ARGS;
BENCHMARK(BM_FakeLua_MultiReturn_TCC) MULTI_RETURN_ARGS;
BENCHMARK(BM_FakeLua_MultiReturn_GCC) MULTI_RETURN_ARGS;

BENCHMARK(BM_CPP_Closure) CLOSURE_ARGS;
BENCHMARK(BM_Lua_Closure) CLOSURE_ARGS;
BENCHMARK(BM_FakeLua_Closure_TCC) CLOSURE_ARGS;
BENCHMARK(BM_FakeLua_Closure_GCC) CLOSURE_ARGS;

BENCHMARK(BM_CPP_TailRecursion) TAIL_RECURSION_ARGS;
BENCHMARK(BM_Lua_TailRecursion) TAIL_RECURSION_ARGS;
BENCHMARK(BM_FakeLua_TailRecursion_TCC) TAIL_RECURSION_ARGS;
BENCHMARK(BM_FakeLua_TailRecursion_GCC) TAIL_RECURSION_ARGS;
