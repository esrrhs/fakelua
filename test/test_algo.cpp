#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

#include <lua.hpp>

using namespace fakelua;

// Helper: compile the given file in both debug and non-debug mode and run f each time.
// Matches the JitterRunHelper pattern: the lambda is responsible for calling CompileFile.
static void AlgoRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: {JIT_TCC, JIT_GCC}) {
        f(s, type, true);
        f(s, type, false);
    }
    FakeluaDeleteState(s);
}

// Push helpers for the Lua C API.
static void LuaPush(lua_State *L, int v) { lua_pushinteger(L, v); }
static void LuaPush(lua_State *L, double v) { lua_pushnumber(L, v); }

// Call a global Lua function and return its single result.
// Ret must be int or double; Args must be int or double.
template<typename Ret, typename... Args>
static Ret LuaCall(lua_State *L, const char *func, Args... args) {
    lua_getglobal(L, func);
    (LuaPush(L, args), ...);
    const int rc = lua_pcall(L, static_cast<int>(sizeof...(args)), 1, 0);
    if (rc != LUA_OK) {
        ADD_FAILURE() << "LuaCall(" << func << ") failed: " << lua_tostring(L, -1);
        lua_pop(L, 1);
        return Ret{};
    }
    Ret ret{};
    if constexpr (std::is_same_v<Ret, double>) {
        ret = static_cast<double>(lua_tonumber(L, -1));
    } else {
        ret = static_cast<Ret>(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    return ret;
}

// Cross-validate algo results against the embedded Lua interpreter.
// Creates a fresh lua_State, loads lua_file, runs f(L), then closes the state.
static void LuaAlgoRunHelper(const std::string &lua_file, const std::function<void(lua_State *)> &f) {
    lua_State *L = luaL_newstate();
    ASSERT_NE(L, nullptr);
    luaL_openlibs(L);
    const int err = luaL_dofile(L, lua_file.c_str());
    if (err != LUA_OK) {
        const std::string msg = lua_tostring(L, -1);
        lua_close(L);
        FAIL() << "luaL_dofile(" << lua_file << ") failed: " << msg;
        return;
    }
    f(L);
    lua_close(L);
}

TEST(algo, fibonacci) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/fibonacci.lua", {.debug_mode = debug_mode});
        int i = 0;
        Call(s, type, "test", i, 30);
        ASSERT_EQ(i, 832040);
    });

    LuaAlgoRunHelper("./algo/fibonacci.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test", 30), 832040);
    });
}

// Bubble sort: sort a 9-element array and verify element positions and sum.
// Exercises: local functions, nested for loops, if/else, table get/set, swap.
TEST(algo, bubble_sort) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/bubble_sort.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test", r, 1);
        ASSERT_EQ(r, 1);
        Call(s, type, "test", r, 5);
        ASSERT_EQ(r, 5);
        Call(s, type, "test", r, 9);
        ASSERT_EQ(r, 9);
        Call(s, type, "test_sum", r);
        ASSERT_EQ(r, 45);
    });

    LuaAlgoRunHelper("./algo/bubble_sort.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test", 1), 1);
        ASSERT_EQ(LuaCall<int>(L, "test", 5), 5);
        ASSERT_EQ(LuaCall<int>(L, "test", 9), 9);
        ASSERT_EQ(LuaCall<int>(L, "test_sum"), 45);
    });
}

// GCD (Euclidean, recursive) and derived LCM; also iterative step counter.
// Exercises: recursion, modulo, floor division, while loop, multi-assignment.
TEST(algo, gcd) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/gcd.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test_gcd", r, 48, 18);
        ASSERT_EQ(r, 6);
        Call(s, type, "test_gcd", r, 100, 75);
        ASSERT_EQ(r, 25);
        Call(s, type, "test_gcd", r, 7, 13);
        ASSERT_EQ(r, 1);

        Call(s, type, "test_lcm", r, 12, 18);
        ASSERT_EQ(r, 36);
        Call(s, type, "test_lcm", r, 5, 7);
        ASSERT_EQ(r, 35);

        Call(s, type, "test_gcd_steps", r, 48, 18);
        ASSERT_EQ(r, 3);
    });

    LuaAlgoRunHelper("./algo/gcd.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test_gcd", 48, 18), 6);
        ASSERT_EQ(LuaCall<int>(L, "test_gcd", 100, 75), 25);
        ASSERT_EQ(LuaCall<int>(L, "test_gcd", 7, 13), 1);
        ASSERT_EQ(LuaCall<int>(L, "test_lcm", 12, 18), 36);
        ASSERT_EQ(LuaCall<int>(L, "test_lcm", 5, 7), 35);
        ASSERT_EQ(LuaCall<int>(L, "test_gcd_steps", 48, 18), 3);
    });
}

// Sieve of Eratosthenes: count primes <= n; also find n-th prime.
// Exercises: table with boolean values, nested while loops, repeat-until.
TEST(algo, sieve) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/sieve.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test", r, 10);
        ASSERT_EQ(r, 4);
        Call(s, type, "test", r, 50);
        ASSERT_EQ(r, 15);
        Call(s, type, "test", r, 2);
        ASSERT_EQ(r, 1);

        Call(s, type, "test_nth_prime", r, 1);
        ASSERT_EQ(r, 2);
        Call(s, type, "test_nth_prime", r, 5);
        ASSERT_EQ(r, 11);
        Call(s, type, "test_nth_prime", r, 10);
        ASSERT_EQ(r, 29);
    });

    LuaAlgoRunHelper("./algo/sieve.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test", 10), 4);
        ASSERT_EQ(LuaCall<int>(L, "test", 50), 15);
        ASSERT_EQ(LuaCall<int>(L, "test", 2), 1);
        ASSERT_EQ(LuaCall<int>(L, "test_nth_prime", 1), 2);
        ASSERT_EQ(LuaCall<int>(L, "test_nth_prime", 5), 11);
        ASSERT_EQ(LuaCall<int>(L, "test_nth_prime", 10), 29);
    });
}

// Binary search in a fixed sorted array; returns 1-based index or 0 if absent.
// Exercises: while loop, floor division, table access, comparison chain.
TEST(algo, binary_search) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/binary_search.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test", r, 7);
        ASSERT_EQ(r, 4);
        Call(s, type, "test", r, 1);
        ASSERT_EQ(r, 1);
        Call(s, type, "test", r, 19);
        ASSERT_EQ(r, 10);
        Call(s, type, "test", r, 10);
        ASSERT_EQ(r, 0);

        Call(s, type, "test_steps", r, 7);
        ASSERT_EQ(r, 4);
    });

    LuaAlgoRunHelper("./algo/binary_search.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test", 7), 4);
        ASSERT_EQ(LuaCall<int>(L, "test", 1), 1);
        ASSERT_EQ(LuaCall<int>(L, "test", 19), 10);
        ASSERT_EQ(LuaCall<int>(L, "test", 10), 0);
        ASSERT_EQ(LuaCall<int>(L, "test_steps", 7), 4);
    });
}

// Fast modular exponentiation using bitwise AND and right shift.
// Exercises: while loop, &, >>, %, integer arithmetic.
// Also: is-power-of-two check and popcount via Brian Kernighan.
TEST(algo, fast_pow) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/fast_pow.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test", r, 2, 10, 1000);
        ASSERT_EQ(r, 24);
        Call(s, type, "test", r, 3, 5, 100);
        ASSERT_EQ(r, 43);
        Call(s, type, "test", r, 7, 1, 1000000007);
        ASSERT_EQ(r, 7);

        Call(s, type, "test_is_pow2", r, 8);
        ASSERT_EQ(r, 1);
        Call(s, type, "test_is_pow2", r, 7);
        ASSERT_EQ(r, 0);
        Call(s, type, "test_is_pow2", r, 0);
        ASSERT_EQ(r, 0);

        Call(s, type, "test_popcount", r, 7);
        ASSERT_EQ(r, 3);
        Call(s, type, "test_popcount", r, 255);
        ASSERT_EQ(r, 8);
        Call(s, type, "test_popcount", r, 12);
        ASSERT_EQ(r, 2);
    });

    LuaAlgoRunHelper("./algo/fast_pow.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test", 2, 10, 1000), 24);
        ASSERT_EQ(LuaCall<int>(L, "test", 3, 5, 100), 43);
        ASSERT_EQ(LuaCall<int>(L, "test", 7, 1, 1000000007), 7);
        ASSERT_EQ(LuaCall<int>(L, "test_is_pow2", 8), 1);
        ASSERT_EQ(LuaCall<int>(L, "test_is_pow2", 7), 0);
        ASSERT_EQ(LuaCall<int>(L, "test_is_pow2", 0), 0);
        ASSERT_EQ(LuaCall<int>(L, "test_popcount", 7), 3);
        ASSERT_EQ(LuaCall<int>(L, "test_popcount", 255), 8);
        ASSERT_EQ(LuaCall<int>(L, "test_popcount", 12), 2);
    });
}

// Insertion sort: sort an 8-element integer array; verify specific positions.
// Also: median of 5 elements, and float array sum after sort.
// Exercises: for loop, inner while with compound condition (and), table get/set.
TEST(algo, insertion_sort) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/insertion_sort.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test", r, 1);
        ASSERT_EQ(r, 3);
        Call(s, type, "test", r, 4);
        ASSERT_EQ(r, 28);
        Call(s, type, "test", r, 8);
        ASSERT_EQ(r, 99);

        Call(s, type, "test_median", r);
        ASSERT_EQ(r, 5);

        double f = 0;
        Call(s, type, "test_float_sum", f);
        ASSERT_NEAR(f, 11.6, 0.001);
    });

    LuaAlgoRunHelper("./algo/insertion_sort.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test", 1), 3);
        ASSERT_EQ(LuaCall<int>(L, "test", 4), 28);
        ASSERT_EQ(LuaCall<int>(L, "test", 8), 99);
        ASSERT_EQ(LuaCall<int>(L, "test_median"), 5);
        ASSERT_NEAR(LuaCall<double>(L, "test_float_sum"), 11.6, 0.001);
    });
}

// 3x3 matrix multiplication (flat row-major tables).
// Exercises: triple-nested for loops, arithmetic index expressions, table get/set.
TEST(algo, matrix) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/matrix.lua", {.debug_mode = debug_mode});
        int r = 0;
        Call(s, type, "test_trace", r);
        ASSERT_EQ(r, 189);

        Call(s, type, "test_cell", r, 1, 1);
        ASSERT_EQ(r, 30);
        Call(s, type, "test_cell", r, 2, 3);
        ASSERT_EQ(r, 54);
        Call(s, type, "test_cell", r, 3, 2);
        ASSERT_EQ(r, 114);
    });

    LuaAlgoRunHelper("./algo/matrix.lua", [](lua_State *L) {
        ASSERT_EQ(LuaCall<int>(L, "test_trace"), 189);
        ASSERT_EQ(LuaCall<int>(L, "test_cell", 1, 1), 30);
        ASSERT_EQ(LuaCall<int>(L, "test_cell", 2, 3), 54);
        ASSERT_EQ(LuaCall<int>(L, "test_cell", 3, 2), 114);
    });
}
