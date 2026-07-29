#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_table, test_table_insert_remove) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_insert_remove.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_insert_remove", res);
        EXPECT_NEAR(res, 77.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_concat) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_concat.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_concat", res);
        EXPECT_NEAR(res, 100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_pack_unpack) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_pack_unpack.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_pack_unpack", res);
        EXPECT_NEAR(res, 123.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort", res);
        EXPECT_NEAR(res, 100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}
