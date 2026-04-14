#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// Helper: compile the given file twice (debug and non-debug) and run f(s) each time.
static void AlgoRunHelper(const std::string &lua_file, const std::function<void(State *, JITType)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileFile(s, lua_file, {.debug_mode = true});
    f(s, JIT_TCC);
    CompileFile(s, lua_file, {.debug_mode = false});
    f(s, JIT_TCC);
    FakeluaDeleteState(s);
}

TEST(algo, fibonacci) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    int i = 0;
    CompileFile(s, "./algo/fibonacci.lua", {});
    Call(s, JIT_TCC, "test", i, 30);
    ASSERT_EQ(i, 832040);

    i = 0;
    CompileFile(s, "./algo/fibonacci.lua", {.debug_mode = false});
    Call(s, JIT_TCC, "test", i, 30);
    ASSERT_EQ(i, 832040);
}

// Bubble sort: sort a 9-element array and verify element positions and sum.
// Exercises: local functions, nested for loops, if/else, table get/set, swap.
TEST(algo, bubble_sort) {
    AlgoRunHelper("./algo/bubble_sort.lua", [](State *s, JITType type) {
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
}

// GCD (Euclidean, recursive) and derived LCM; also iterative step counter.
// Exercises: recursion, modulo, floor division, while loop, multi-assignment.
TEST(algo, gcd) {
    AlgoRunHelper("./algo/gcd.lua", [](State *s, JITType type) {
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
}

// Sieve of Eratosthenes: count primes <= n; also find n-th prime.
// Exercises: table with boolean values, nested while loops, repeat-until.
TEST(algo, sieve) {
    AlgoRunHelper("./algo/sieve.lua", [](State *s, JITType type) {
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
}

// Binary search in a fixed sorted array; returns 1-based index or 0 if absent.
// Exercises: while loop, floor division, table access, comparison chain.
TEST(algo, binary_search) {
    AlgoRunHelper("./algo/binary_search.lua", [](State *s, JITType type) {
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
}

// Fast modular exponentiation using bitwise AND and right shift.
// Exercises: while loop, &, >>, %, integer arithmetic.
// Also: is-power-of-two check and popcount via Brian Kernighan.
TEST(algo, fast_pow) {
    AlgoRunHelper("./algo/fast_pow.lua", [](State *s, JITType type) {
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
}

// Insertion sort: sort an 8-element integer array; verify specific positions.
// Also: median of 5 elements, and float array sum after sort.
// Exercises: for loop, inner while with compound condition (and), table get/set.
TEST(algo, insertion_sort) {
    AlgoRunHelper("./algo/insertion_sort.lua", [](State *s, JITType type) {
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
}

// 3x3 matrix multiplication (flat row-major tables).
// Exercises: triple-nested for loops, arithmetic index expressions, table get/set.
TEST(algo, matrix) {
    AlgoRunHelper("./algo/matrix.lua", [](State *s, JITType type) {
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
}
