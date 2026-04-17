#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

using namespace fakelua;

namespace {

constexpr const char *kFibScript = R"(
local function fib(n)
    if n <= 1 then
        return n
    end
    return fib(n - 1) + fib(n - 2)
end

function bench_fib(n)
    return fib(n)
end
)";

constexpr const char *kGcdScript = R"(
function bench_gcd(a, b)
    while b ~= 0 do
        a, b = b, a % b
    end
    return a
end
)";

constexpr const char *kPowScript = R"(
function bench_powmod(base, exp, mod)
    local result = 1
    base = base % mod
    while exp > 0 do
        if exp % 2 == 1 then
            result = (result * base) % mod
        end
        base = (base * base) % mod
        exp = exp // 2
    end
    return result
end
)";

constexpr const char *kSumScript = R"(
function bench_sum(n)
    local s = 0
    for i = 1, n do
        s = s + i
    end
    return s
end
)";

int64_t CppFib(const int64_t n) {
    if (n <= 1) {
        return n;
    }
    return CppFib(n - 1) + CppFib(n - 2);
}

int64_t CppGcd(int64_t a, int64_t b) {
    while (b != 0) {
        const int64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int64_t CppPowMod(int64_t base, int64_t exp, const int64_t mod) {
    int64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if ((exp & 1) != 0) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

int64_t CppSum(const int64_t n) {
    int64_t s = 0;
    for (int64_t i = 1; i <= n; ++i) {
        s += i;
    }
    return s;
}

void PushLuaArg(lua_State *L, const int64_t value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void PushLuaArgs(lua_State *) {
}

template<typename T, typename... Args>
void PushLuaArgs(lua_State *L, T first, Args... args) {
    PushLuaArg(L, static_cast<int64_t>(first));
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

    PushLuaArgs(L, args...);

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

struct RuntimeContext {
    RuntimeContext() {
        lua = luaL_newstate();
        luaL_openlibs(lua);

        if (luaL_dostring(lua, kFibScript) != LUA_OK || luaL_dostring(lua, kGcdScript) != LUA_OK ||
            luaL_dostring(lua, kPowScript) != LUA_OK || luaL_dostring(lua, kSumScript) != LUA_OK) {
            const char *err = lua_tostring(lua, -1);
            throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
        }

        flua = FakeluaNewState();
        CompileString(flua, kFibScript, {.debug_mode = false});
        CompileString(flua, kGcdScript, {.debug_mode = false});
        CompileString(flua, kPowScript, {.debug_mode = false});
        CompileString(flua, kSumScript, {.debug_mode = false});
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

void VerifyEqual(const int64_t got, const int64_t expected, const char *name) {
    if (got != expected) {
        throw std::runtime_error(std::string(name) + " wrong result: got " + std::to_string(got) +
                                 ", expected " + std::to_string(expected));
    }
}

static void BM_CPP_Fibonacci(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppFib(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ fib");
    }
}

static void BM_Lua_Fibonacci(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_fib", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua fib");
    }
}

static void BM_FakeLua_Fibonacci_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_fib", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fib");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_Fibonacci_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_fib", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fib");
    }
}
#endif // !_WIN32

static void BM_CPP_GCD(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for (auto _: state) {
        int64_t aa = a;
        int64_t bb = b;
        benchmark::DoNotOptimize(aa);
        benchmark::DoNotOptimize(bb);
        int64_t ret = CppGcd(aa, bb);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ gcd");
    }
}

static void BM_Lua_GCD(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_gcd", a, b);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua gcd");
    }
}

static void BM_FakeLua_GCD_TCC(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_gcd", ret, a, b);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua gcd");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_GCD_GCC(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_gcd", ret, a, b);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua gcd");
    }
}
#endif // !_WIN32

static void BM_CPP_PowMod(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for (auto _: state) {
        int64_t bb = base;
        int64_t ee = exp;
        int64_t mm = mod;
        benchmark::DoNotOptimize(bb);
        benchmark::DoNotOptimize(ee);
        benchmark::DoNotOptimize(mm);
        int64_t ret = CppPowMod(bb, ee, mm);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ powmod");
    }
}

static void BM_Lua_PowMod(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_powmod", base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua powmod");
    }
}

static void BM_FakeLua_PowMod_TCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_powmod", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua powmod");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_PowMod_GCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_powmod", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua powmod");
    }
}
#endif // !_WIN32

static void BM_CPP_Sum(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppSum(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ sum");
    }
}

static void BM_Lua_Sum(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_sum", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua sum");
    }
}

static void BM_FakeLua_Sum_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_sum", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sum");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_Sum_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_sum", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sum");
    }
}
#endif // !_WIN32

}// namespace

#define FIB_ARGS ->Arg(20)->Arg(25)->Arg(30)->Arg(32)
#define GCD_ARGS ->Args({832040, 514229})->Args({123456789, 987654321})->Args({2147483647, 1073741823})
#define POWMOD_ARGS                                                                                                         \
    ->Args({2, 1000, 1000000007})->Args({7, 1000000, 1000000007})->Args({1234567, 7654321, 1000000007})
#define SUM_ARGS ->Arg(10000)->Arg(100000)->Arg(1000000)->Arg(5000000)

BENCHMARK(BM_CPP_Fibonacci) FIB_ARGS;
BENCHMARK(BM_Lua_Fibonacci) FIB_ARGS;
BENCHMARK(BM_FakeLua_Fibonacci_TCC) FIB_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_Fibonacci_GCC) FIB_ARGS;
#endif

BENCHMARK(BM_CPP_GCD) GCD_ARGS;
BENCHMARK(BM_Lua_GCD) GCD_ARGS;
BENCHMARK(BM_FakeLua_GCD_TCC) GCD_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_GCD_GCC) GCD_ARGS;
#endif

BENCHMARK(BM_CPP_PowMod) POWMOD_ARGS;
BENCHMARK(BM_Lua_PowMod) POWMOD_ARGS;
BENCHMARK(BM_FakeLua_PowMod_TCC) POWMOD_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_PowMod_GCC) POWMOD_ARGS;
#endif

BENCHMARK(BM_CPP_Sum) SUM_ARGS;
BENCHMARK(BM_Lua_Sum) SUM_ARGS;
BENCHMARK(BM_FakeLua_Sum_TCC) SUM_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_Sum_GCC) SUM_ARGS;
#endif
