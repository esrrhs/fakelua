#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

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

constexpr const char *kBubbleSortScript = R"(
function bench_bubble_sort(n)
    local t = {}
    for i = 1, n do t[i] = n - i + 1 end
    for i = 1, n do
        for j = 1, n - i do
            if t[j] > t[j + 1] then
                local tmp = t[j]
                t[j] = t[j + 1]
                t[j + 1] = tmp
            end
        end
    end
    return t[1]
end
)";

constexpr const char *kSieveScript = R"(
function bench_sieve(n)
    local is_prime = {}
    for i = 2, n do is_prime[i] = true end
    local i = 2
    while i * i <= n do
        if is_prime[i] then
            local j = i * i
            while j <= n do
                is_prime[j] = false
                j = j + i
            end
        end
        i = i + 1
    end
    local count = 0
    for k = 2, n do
        if is_prime[k] then
            count = count + 1
        end
    end
    return count
end
)";

constexpr const char *kBinarySearchScript = R"(
function bench_binary_search(n)
    local t = {}
    for i = 1, n do t[i] = i end
    local found = 0
    for target = 1, n do
        local lo = 1
        local hi = n
        while lo <= hi do
            local mid = (lo + hi) // 2
            if t[mid] == target then
                found = found + 1
                break
            elseif t[mid] < target then
                lo = mid + 1
            else
                hi = mid - 1
            end
        end
    end
    return found
end
)";

constexpr const char *kFastPowScript = R"(
function bench_fast_pow(base, exp, mod)
    local result = 1
    base = base % mod
    while exp > 0 do
        if (exp & 1) == 1 then
            result = (result * base) % mod
        end
        exp = exp >> 1
        base = (base * base) % mod
    end
    return result
end
)";

constexpr const char *kPopcountScript = R"(
function bench_popcount(n)
    local total = 0
    for i = 0, n - 1 do
        local x = i
        while x ~= 0 do
            x = x & (x - 1)
            total = total + 1
        end
    end
    return total
end
)";

constexpr const char *kInsertionSortScript = R"(
function bench_insertion_sort(n)
    local t = {}
    for i = 1, n do t[i] = n - i + 1 end
    for i = 2, n do
        local key = t[i]
        local j = i - 1
        while j >= 1 and t[j] > key do
            t[j + 1] = t[j]
            j = j - 1
        end
        t[j + 1] = key
    end
    return t[1]
end
)";

constexpr const char *kMatMulScript = R"(
function bench_matmul()
    local a = {1, 2, 3, 4, 5, 6, 7, 8, 9}
    local b = {9, 8, 7, 6, 5, 4, 3, 2, 1}
    local c = {0, 0, 0, 0, 0, 0, 0, 0, 0}
    for i = 1, 3 do
        for j = 1, 3 do
            local s = 0
            for k = 1, 3 do
                s = s + a[(i - 1) * 3 + k] * b[(k - 1) * 3 + j]
            end
            c[(i - 1) * 3 + j] = s
        end
    end
    return c[1] + c[5] + c[9]
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

// A global volatile zero prevents the compiler from constant-folding the 3x3
// matrix product in CppMatMul while keeping the numeric results correct.
volatile int64_t g_matmul_nonce = 0;

int64_t CppBubbleSort(const int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = n - i;
    }
    for (int64_t i = 1; i <= n; ++i) {
        for (int64_t j = 1; j <= n - i; ++j) {
            if (t[static_cast<size_t>(j - 1)] > t[static_cast<size_t>(j)]) {
                std::swap(t[static_cast<size_t>(j - 1)], t[static_cast<size_t>(j)]);
            }
        }
    }
    return t[0];
}

int64_t CppSieve(const int64_t n) {
    std::vector<bool> is_prime(static_cast<size_t>(n + 1), true);
    for (int64_t i = 2; i * i <= n; ++i) {
        if (is_prime[static_cast<size_t>(i)]) {
            for (int64_t j = i * i; j <= n; j += i) {
                is_prime[static_cast<size_t>(j)] = false;
            }
        }
    }
    int64_t count = 0;
    for (int64_t k = 2; k <= n; ++k) {
        if (is_prime[static_cast<size_t>(k)]) {
            ++count;
        }
    }
    return count;
}

int64_t CppBinarySearch(const int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = i + 1;
    }
    int64_t found = 0;
    for (int64_t target = 1; target <= n; ++target) {
        int64_t lo = 0;
        int64_t hi = n - 1;
        while (lo <= hi) {
            const int64_t mid = (lo + hi) / 2;
            if (t[static_cast<size_t>(mid)] == target) {
                ++found;
                break;
            } else if (t[static_cast<size_t>(mid)] < target) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
    }
    return found;
}

int64_t CppFastPow(int64_t base, int64_t exp, const int64_t mod) {
    int64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if ((exp & 1) != 0) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

int64_t CppPopcount(const int64_t n) {
    int64_t total = 0;
    for (int64_t i = 0; i < n; ++i) {
        int64_t x = i;
        while (x != 0) {
            x &= (x - 1);
            ++total;
        }
    }
    return total;
}

int64_t CppInsertionSort(const int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = n - i;
    }
    for (int64_t i = 1; i < n; ++i) {
        const int64_t key = t[static_cast<size_t>(i)];
        int64_t j = i - 1;
        while (j >= 0 && t[static_cast<size_t>(j)] > key) {
            t[static_cast<size_t>(j + 1)] = t[static_cast<size_t>(j)];
            --j;
        }
        t[static_cast<size_t>(j + 1)] = key;
    }
    return t[0];
}

int64_t CppMatMul() {
    // a[0] depends on a volatile global (always 1 at runtime) so the compiler
    // cannot precompute the matrix product at compile time.
    int64_t a[9] = {g_matmul_nonce + 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int64_t b[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    int64_t c[9] = {};
    for (int64_t i = 0; i < 3; ++i) {
        for (int64_t j = 0; j < 3; ++j) {
            int64_t s = 0;
            for (int64_t k = 0; k < 3; ++k) {
                s += a[i * 3 + k] * b[k * 3 + j];
            }
            c[i * 3 + j] = s;
        }
    }
    return c[0] + c[4] + c[8];
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

        const char *lua_scripts[] = {
                kFibScript,    kGcdScript,    kPowScript,         kSumScript,        kBubbleSortScript,
                kSieveScript,  kBinarySearchScript, kFastPowScript, kPopcountScript,
                kInsertionSortScript, kMatMulScript};
        for (const char *script : lua_scripts) {
            if (luaL_dostring(lua, script) != LUA_OK) {
                const char *err = lua_tostring(lua, -1);
                throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
            }
        }

        flua = FakeluaNewState();
        for (const char *script : lua_scripts) {
            CompileString(flua, script, {.debug_mode = false});
        }
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

static void BM_CPP_BubbleSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = 1;
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppBubbleSort(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ bubble_sort");
    }
}

static void BM_Lua_BubbleSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_bubble_sort", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "Lua bubble_sort");
    }
}

static void BM_FakeLua_BubbleSort_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_bubble_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua bubble_sort");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_BubbleSort_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_bubble_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua bubble_sort");
    }
}
#endif // !_WIN32

static void BM_CPP_Sieve(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppSieve(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ sieve");
    }
}

static void BM_Lua_Sieve(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_sieve", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua sieve");
    }
}

static void BM_FakeLua_Sieve_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_sieve", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sieve");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_Sieve_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_sieve", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sieve");
    }
}
#endif // !_WIN32

static void BM_CPP_BinarySearch(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppBinarySearch(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "C++ binary_search");
    }
}

static void BM_Lua_BinarySearch(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_binary_search", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "Lua binary_search");
    }
}

static void BM_FakeLua_BinarySearch_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_binary_search", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "FakeLua binary_search");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_BinarySearch_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_binary_search", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "FakeLua binary_search");
    }
}
#endif // !_WIN32

static void BM_CPP_FastPow(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for (auto _: state) {
        int64_t bb = base;
        int64_t ee = exp;
        int64_t mm = mod;
        benchmark::DoNotOptimize(bb);
        benchmark::DoNotOptimize(ee);
        benchmark::DoNotOptimize(mm);
        int64_t ret = CppFastPow(bb, ee, mm);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ fast_pow");
    }
}

static void BM_Lua_FastPow(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_fast_pow", base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua fast_pow");
    }
}

static void BM_FakeLua_FastPow_TCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_fast_pow", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fast_pow");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_FastPow_GCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_fast_pow", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fast_pow");
    }
}
#endif // !_WIN32

static void BM_CPP_Popcount(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppPopcount(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ popcount");
    }
}

static void BM_Lua_Popcount(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_popcount", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua popcount");
    }
}

static void BM_FakeLua_Popcount_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_popcount", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua popcount");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_Popcount_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_popcount", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua popcount");
    }
}
#endif // !_WIN32

static void BM_CPP_InsertionSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = 1;
    for (auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppInsertionSort(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ insertion_sort");
    }
}

static void BM_Lua_InsertionSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_insertion_sort", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "Lua insertion_sort");
    }
}

static void BM_FakeLua_InsertionSort_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_insertion_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua insertion_sort");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_InsertionSort_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_insertion_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua insertion_sort");
    }
}
#endif // !_WIN32

static void BM_CPP_MatMul(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for (auto _: state) {
        int64_t ret = CppMatMul();
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ matmul");
    }
}

static void BM_Lua_MatMul(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_matmul");
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua matmul");
    }
}

static void BM_FakeLua_MatMul_TCC(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_matmul", ret);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua matmul");
    }
}

#if !defined(_WIN32)
static void BM_FakeLua_MatMul_GCC(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_matmul", ret);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua matmul");
    }
}
#endif // !_WIN32

}// namespace

#define FIB_ARGS ->Arg(20)->Arg(25)->Arg(30)->Arg(32)
#define GCD_ARGS ->Args({832040, 514229})->Args({123456789, 987654321})->Args({2147483647, 1073741823})
#define POWMOD_ARGS                                                                                                         \
    ->Args({2, 1000, 1000000007})->Args({7, 1000000, 1000000007})->Args({1234567, 7654321, 1000000007})
#define SUM_ARGS ->Arg(10000)->Arg(100000)->Arg(1000000)->Arg(5000000)
#define BUBBLE_SORT_ARGS ->Arg(50)->Arg(100)->Arg(200)
#define SIEVE_ARGS ->Arg(100)->Arg(500)->Arg(1000)->Arg(5000)
#define BINARY_SEARCH_ARGS ->Arg(100)->Arg(500)->Arg(1000)
#define FAST_POW_ARGS ->Args({2, 1000, 1000000007})->Args({7, 1000000, 1000000007})->Args({1234567, 7654321, 1000000007})
#define POPCOUNT_ARGS ->Arg(1000)->Arg(10000)->Arg(100000)
#define INSERTION_SORT_ARGS ->Arg(50)->Arg(100)->Arg(200)

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

BENCHMARK(BM_CPP_BubbleSort) BUBBLE_SORT_ARGS;
BENCHMARK(BM_Lua_BubbleSort) BUBBLE_SORT_ARGS;
BENCHMARK(BM_FakeLua_BubbleSort_TCC) BUBBLE_SORT_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_BubbleSort_GCC) BUBBLE_SORT_ARGS;
#endif

BENCHMARK(BM_CPP_Sieve) SIEVE_ARGS;
BENCHMARK(BM_Lua_Sieve) SIEVE_ARGS;
BENCHMARK(BM_FakeLua_Sieve_TCC) SIEVE_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_Sieve_GCC) SIEVE_ARGS;
#endif

BENCHMARK(BM_CPP_BinarySearch) BINARY_SEARCH_ARGS;
BENCHMARK(BM_Lua_BinarySearch) BINARY_SEARCH_ARGS;
BENCHMARK(BM_FakeLua_BinarySearch_TCC) BINARY_SEARCH_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_BinarySearch_GCC) BINARY_SEARCH_ARGS;
#endif

BENCHMARK(BM_CPP_FastPow) FAST_POW_ARGS;
BENCHMARK(BM_Lua_FastPow) FAST_POW_ARGS;
BENCHMARK(BM_FakeLua_FastPow_TCC) FAST_POW_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_FastPow_GCC) FAST_POW_ARGS;
#endif

BENCHMARK(BM_CPP_Popcount) POPCOUNT_ARGS;
BENCHMARK(BM_Lua_Popcount) POPCOUNT_ARGS;
BENCHMARK(BM_FakeLua_Popcount_TCC) POPCOUNT_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_Popcount_GCC) POPCOUNT_ARGS;
#endif

BENCHMARK(BM_CPP_InsertionSort) INSERTION_SORT_ARGS;
BENCHMARK(BM_Lua_InsertionSort) INSERTION_SORT_ARGS;
BENCHMARK(BM_FakeLua_InsertionSort_TCC) INSERTION_SORT_ARGS;
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_InsertionSort_GCC) INSERTION_SORT_ARGS;
#endif

BENCHMARK(BM_CPP_MatMul);
BENCHMARK(BM_Lua_MatMul);
BENCHMARK(BM_FakeLua_MatMul_TCC);
#if !defined(_WIN32)
BENCHMARK(BM_FakeLua_MatMul_GCC);
#endif
