#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

#include <cstdio>

using namespace fakelua;

// Helper: compile the given file in both debug and non-debug mode and run f each time.
// Matches the JitterRunHelper pattern: the lambda is responsible for calling CompileFile.
static void AlgoRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    f(s, JIT_TCC, true);
    f(s, JIT_TCC, false);
    FakeluaDeleteState(s);
}

// Detect the available Lua 5.4 binary once and cache the result.
static const char *GetLuaBin() {
    static const char *lua_bin = []() -> const char * {
        if (system("which lua5.4 > /dev/null 2>&1") == 0) return "lua5.4";
        if (system("which lua > /dev/null 2>&1") == 0) return "lua";
        return nullptr;
    }();
    return lua_bin;
}

// Execute a Lua expression by loading lua_file with dofile() then printing the result.
// Returns the raw stdout output (no trailing newline).
// lua_file and call_expr must be hardcoded test-only values (no user input).
static std::string LuaEval(const std::string &lua_file, const std::string &call_expr) {
    const char *lua_bin = GetLuaBin();
    if (!lua_bin) return "";
    const std::string cmd = std::string(lua_bin) + " -e 'dofile(\"" + lua_file +
                            "\"); io.write(tostring(" + call_expr + "))' 2>/dev/null";
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) return "";
    char buf[512] = {};
    const size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    pclose(fp);
    return std::string(buf, n);
}

static int LuaEvalInt(const std::string &lua_file, const std::string &call_expr) {
    const auto s = LuaEval(lua_file, call_expr);
    return s.empty() ? 0 : std::stoi(s);
}

static double LuaEvalDouble(const std::string &lua_file, const std::string &call_expr) {
    const auto s = LuaEval(lua_file, call_expr);
    return s.empty() ? 0.0 : std::stod(s);
}

// Cross-validate algo results against the reference Lua interpreter.
// Skips silently if neither lua5.4 nor lua is available.
static void LuaAlgoRunHelper(const std::function<void()> &f) {
    if (GetLuaBin() != nullptr) {
        f();
    }
}

TEST(algo, fibonacci) {
    AlgoRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./algo/fibonacci.lua", {.debug_mode = debug_mode});
        int i = 0;
        Call(s, type, "test", i, 30);
        ASSERT_EQ(i, 832040);
    });

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/fibonacci.lua", "test(30)"), 832040);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/bubble_sort.lua", "test(1)"), 1);
        ASSERT_EQ(LuaEvalInt("./algo/bubble_sort.lua", "test(5)"), 5);
        ASSERT_EQ(LuaEvalInt("./algo/bubble_sort.lua", "test(9)"), 9);
        ASSERT_EQ(LuaEvalInt("./algo/bubble_sort.lua", "test_sum()"), 45);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/gcd.lua", "test_gcd(48,18)"), 6);
        ASSERT_EQ(LuaEvalInt("./algo/gcd.lua", "test_gcd(100,75)"), 25);
        ASSERT_EQ(LuaEvalInt("./algo/gcd.lua", "test_gcd(7,13)"), 1);
        ASSERT_EQ(LuaEvalInt("./algo/gcd.lua", "test_lcm(12,18)"), 36);
        ASSERT_EQ(LuaEvalInt("./algo/gcd.lua", "test_lcm(5,7)"), 35);
        ASSERT_EQ(LuaEvalInt("./algo/gcd.lua", "test_gcd_steps(48,18)"), 3);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/sieve.lua", "test(10)"), 4);
        ASSERT_EQ(LuaEvalInt("./algo/sieve.lua", "test(50)"), 15);
        ASSERT_EQ(LuaEvalInt("./algo/sieve.lua", "test(2)"), 1);
        ASSERT_EQ(LuaEvalInt("./algo/sieve.lua", "test_nth_prime(1)"), 2);
        ASSERT_EQ(LuaEvalInt("./algo/sieve.lua", "test_nth_prime(5)"), 11);
        ASSERT_EQ(LuaEvalInt("./algo/sieve.lua", "test_nth_prime(10)"), 29);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/binary_search.lua", "test(7)"), 4);
        ASSERT_EQ(LuaEvalInt("./algo/binary_search.lua", "test(1)"), 1);
        ASSERT_EQ(LuaEvalInt("./algo/binary_search.lua", "test(19)"), 10);
        ASSERT_EQ(LuaEvalInt("./algo/binary_search.lua", "test(10)"), 0);
        ASSERT_EQ(LuaEvalInt("./algo/binary_search.lua", "test_steps(7)"), 4);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test(2,10,1000)"), 24);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test(3,5,100)"), 43);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test(7,1,1000000007)"), 7);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test_is_pow2(8)"), 1);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test_is_pow2(7)"), 0);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test_is_pow2(0)"), 0);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test_popcount(7)"), 3);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test_popcount(255)"), 8);
        ASSERT_EQ(LuaEvalInt("./algo/fast_pow.lua", "test_popcount(12)"), 2);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/insertion_sort.lua", "test(1)"), 3);
        ASSERT_EQ(LuaEvalInt("./algo/insertion_sort.lua", "test(4)"), 28);
        ASSERT_EQ(LuaEvalInt("./algo/insertion_sort.lua", "test(8)"), 99);
        ASSERT_EQ(LuaEvalInt("./algo/insertion_sort.lua", "test_median()"), 5);
        ASSERT_NEAR(LuaEvalDouble("./algo/insertion_sort.lua", "test_float_sum()"), 11.6, 0.001);
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

    LuaAlgoRunHelper([]() {
        ASSERT_EQ(LuaEvalInt("./algo/matrix.lua", "test_trace()"), 189);
        ASSERT_EQ(LuaEvalInt("./algo/matrix.lua", "test_cell(1,1)"), 30);
        ASSERT_EQ(LuaEvalInt("./algo/matrix.lua", "test_cell(2,3)"), 54);
        ASSERT_EQ(LuaEvalInt("./algo/matrix.lua", "test_cell(3,2)"), 114);
    });
}
