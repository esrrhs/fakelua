#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_basic, test_basic_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_type.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_type", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_tostring) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_tostring.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_tostring", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_tonumber) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_tonumber.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_tonumber", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_select) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_select.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_select", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_assert) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_assert.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_assert", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_next) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_next.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_next", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_pairs) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_pairs.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_pairs", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_ipairs) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_ipairs.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_ipairs", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_print) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_print.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_print", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_dofile) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_dofile.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_dofile", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_dofile_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./basic/test_dofile_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_dofile_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_version) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_version.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_version", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_collectgarbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_collectgarbage.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_collectgarbage", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_continue) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_continue.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_continue", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_for_dynamic_continue) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_for_dynamic_continue.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_for_dynamic_continue", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_pcall) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./basic/test_basic_pcall.lua", config);

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        double res = 0;
        Call(s, jit_type, "test_pcall_success", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_pcall_multi_return", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_pcall_with_args", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_pcall_non_function", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_pcall_upvalue", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_pcall_missing_args", res);
        EXPECT_NEAR(res, 5000, 0.5);

        // error() 在 JIT 代码里触发运行时错误，pcall 必须能接住它：TCC 的代码页没有
        // DWARF 展开表，错误靠 jit_error_boundary 的跳转边界回到 C++ 而不是异常展开。
        res = 0;
        Call(s, jit_type, "test_pcall_error", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_xpcall) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./basic/test_basic_pcall.lua", config);

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        double res = 0;
        Call(s, jit_type, "test_xpcall_success", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_xpcall_non_function", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_xpcall_error", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_xpcall_many_args", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_type_closure) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_type_closure.lua", config);
        double res = 0;
        Call(s, jit_type, "test_type_closure", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_type_nil_multi_table", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_tonumber_with_base) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_type_closure.lua", config);
        double res = 0;
        Call(s, jit_type, "test_tonumber_with_base", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_tonumber_with_base_negative", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_tonumber_with_base_invalid", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_tonumber_edge) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_tonumber_edge.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_basic_tonumber_edge", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_tostring_edge) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_tostring_edge.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_basic_tostring_edge", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./basic/test_basic_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_error_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_assert_bad_msg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_select_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_next_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_pairs_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_ipairs_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_collectgarbage_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_tonumber_bad_base", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_call_too_many_args", res), std::exception);

    FakeluaDeleteState(s);
}
