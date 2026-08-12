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

TEST(test_table, test_table_sort_invalid_comparator) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./table/test_table_sort_bad_cmp.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_table_sort_bad_cmp", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_create) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_create.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_create", res);
        EXPECT_NEAR(res, 100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_move.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_move", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_create_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_create_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_create_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_insert_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_insert_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_insert_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_remove_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_remove_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_remove_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_concat_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_concat_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_concat_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_custom_cmp) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort_custom_cmp", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move_overlap) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_move_overlap", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move_no_overlap) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_move_no_overlap", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_concat_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_concat_float", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_single) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort_single", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./table/test_table_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./table/test_table_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_table_boundary_error", res), std::exception);

    FakeluaDeleteState(s);
}
