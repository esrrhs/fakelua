#include <gtest/gtest.h>
#include <string>

#include "fakelua.h"
#include "test_jit.h"
#include "compile/compiler.h"
#include "compile/preprocessor.h"
#include "compile/semantic_analysis.h"
#include "compile/register_limit.h"

using namespace fakelua;

TEST(test_basic, test_basic_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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
    CallThrow(s, "test_dofile_bad_arg", res);

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_version) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
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

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./basic/test_basic_tostring_cases.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_basic_tostring_edge", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_tonumber_lua54) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "jit=" << JitTypeName(jit_type));
        CompileFile(s, "./basic/test_basic_tonumber_lua54.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_basic_tonumber_lua54", res);
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
    CallThrow(s, "test_error_bad_arg", res);
    CallThrow(s, "test_assert_bad_msg", res);
    CallThrow(s, "test_select_bad_arg", res);
    CallThrow(s, "test_next_bad_arg", res);
    CallThrow(s, "test_pairs_bad_arg", res);
    CallThrow(s, "test_ipairs_bad_arg", res);
    CallThrow(s, "test_collectgarbage_bad_arg", res);
    CallThrow(s, "test_tonumber_bad_base", res);
    CallThrow(s, "test_call_too_many_args", res);

    FakeluaDeleteState(s);
}

// Basic edge case tests
TEST(test_basic, select_number) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_select_number", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_hash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_select_hash", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_out_of_range) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_select_out_of_range", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, error_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_error_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, error_with_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_error_with_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, assert_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_assert_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, assert_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_assert_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, assert_no_msg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_assert_no_msg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pcall_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_pcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pcall_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_pcall_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, xpcall_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_xpcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, xpcall_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_xpcall_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, type_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_type_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tostring_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_tostring_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_tonumber_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, print_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_print_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pairs_iter) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_pairs_iter", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_iter) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_ipairs_iter", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_func) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_next_func", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_edge) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTest.test_collectgarbage", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Extra basic edge case tests
TEST(test_basic, error_non_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_error.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicError.test_error_non_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, error_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_error.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicError.test_error_table", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, assert_bad_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_assert_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicAssertCases.test_assert_bad_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, assert_table_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_assert_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicAssertCases.test_assert_table_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, assert_success_multi) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_assert_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicAssertCases.test_assert_success_multi", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pcall_non_function) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pcall_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPcallCases.test_pcall_non_function", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pcall_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pcall_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPcallCases.test_pcall_error", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, extra_pcall_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pcall_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPcallCases.test_pcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, xpcall_handler) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_xpcall.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicXpcall.test_xpcall_handler", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, extra_xpcall_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_xpcall.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicXpcall.test_xpcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, xpcall_non_function) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_xpcall.lua", config);
    int64_t ret = 0;
    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    CallAll(s, "BasicXpcall.test_xpcall_non_function", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_with_index) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_next_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicNextCases.test_next_with_index", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_no_index) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_next_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicNextCases.test_next_no_index", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_next_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicNextCases.test_next_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_end) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_next_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicNextCases.test_next_end", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_positive) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_select_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicSelectCases.test_select_positive", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_select_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicSelectCases.test_select_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_count) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_select_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicSelectCases.test_select_count", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, extra_select_out_of_range) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_select_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicSelectCases.test_select_out_of_range", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_hex) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_hex", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_octal) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_octal", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_auto_hex) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_auto_hex", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_float", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_invalid", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_whitespace) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_whitespace", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_bad_base) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_bad_base", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, tonumber_base_too_small) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_tonumber_extra.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTonumberExtra.test_tonumber_base_too_small", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, type_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_type_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicTypeCases.test_type_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_count) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_count", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_collect) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_collect", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_stop) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_stop", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_restart) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_restart", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_step) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_step", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_setpause) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_setpause", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_setstepmul) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_setstepmul", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, collectgarbage_default) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_collectgarbage_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicCollectgarbageCases.test_collectgarbage_default", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pairs_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPairsCases.test_pairs_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPairsCases.test_ipairs_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_non_continuous) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPairsCases.test_ipairs_non_continuous", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_local_shadow) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPairsCases.test_ipairs_local_shadow", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, file_level_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_file_level_table.lua", config);
    int64_t ret = 0;
    CallAll(s, "FileLevelTable.test_file_level_table", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, init_load_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_init_load_table.lua", config);
    int64_t ret = 0;
    CallAll(s, "InitLoadTable.test_init_load_table", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pairs_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicPairsCases.test_pairs_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Basic iterator tests (GCC backend for coverage)
TEST(test_basic, pairs_basic_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_pairs_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pairs_empty_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_pairs_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, pairs_array_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_pairs_array", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_basic_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_ipairs_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_empty_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_ipairs_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, ipairs_index_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_ipairs_index", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_basic_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_next_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, next_empty_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_next_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_basic_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_select_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, select_negative_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_iterators.lua", config);
    int64_t ret = 0;
    CallAll(s, "BasicIterators.test_select_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 pairs/ipairs 迭代器覆盖
// Normal tests use TCC backend
TEST(test_basic, pairs_ipairs_iterators) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_coverage.lua", config);
    int64_t ret = 0;
    // All normal tests use TCC
    CallAll(s, "BasicPairsCoverage.test_pairs_iterator_basic", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_pairs_iterator_empty", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_pairs_iterator_array", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_ipairs_iterator_basic", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_ipairs_iterator_empty", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_ipairs_iterator_index", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_next_basic", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_next_empty", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_select_basic", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_select_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 tonumber/totype/type 边界情况
// Normal tests use TCC backend
TEST(test_basic, tonumber_tostring_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_coverage.lua", config);
    int64_t ret = 0;
    // All normal tests use TCC
    CallAll(s, "BasicPairsCoverage.test_tonumber_non_string_non_number", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tonumber_string_number", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tonumber_invalid_string", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tonumber_hex", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tonumber_invalid_digit_for_base", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tonumber_float", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tostring_already_string", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_tostring_number", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_type_userdata", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_type_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 xpcall
// All normal tests use TCC backend (xpcall catches errors in Lua, no C++ exception)
TEST(test_basic, xpcall_coverage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_pairs_coverage.lua", config);
    int64_t ret = 0;
    // All normal tests use TCC
    CallAll(s, "BasicPairsCoverage.test_xpcall_with_handler", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "BasicPairsCoverage.test_xpcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic, test_basic_select_range) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./basic/test_basic_select_range.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_basic_select_range", res);
        EXPECT_EQ(res, 5000LL);
    }

    FakeluaDeleteState(s);
}

// Regression: FlFastAlloc/HeapAllocator offset sync (tables/strings via JIT then next/pairs).
TEST(test_basic, heap_offset_sync_many_tables_next) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_heap_offset_sync_many_tables_next.lua", config);
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_heap_offset_sync_many_tables_next", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }
    FakeluaDeleteState(s);
}

TEST(test_basic, heap_offset_sync_pairs_under_alloc_pressure) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_heap_offset_sync_pairs_under_alloc_pressure.lua", config);
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_heap_offset_sync_pairs_under_alloc_pressure", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }
    FakeluaDeleteState(s);
}

TEST(test_basic, heap_offset_sync_nested_tables_next) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_heap_offset_sync_nested_tables_next.lua", config);
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_heap_offset_sync_nested_tables_next", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }
    FakeluaDeleteState(s);
}

TEST(test_basic, heap_offset_sync_string_keys_and_rehash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_heap_offset_sync_string_keys_and_rehash.lua", config);
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_heap_offset_sync_string_keys_and_rehash", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }
    FakeluaDeleteState(s);
}


// 寄存器高峰门禁：语义分析阶段按共享 FunctionRegisterUsage 规则检查，与 JIT_INTERP 的 Inst::a/b/c(uint8) 对齐。
// 共享计量 = 高峰并发槽（形参 + AllocLocal + AllocReg 临时）。
// 包含语义分析/预处理插入的临时 local（如 __fakelua_tmp_*、__fakelua_lhs_*），
// 不是「源码里用户写的 local 个数」，更不是 C 后端临时 C 变量个数。
// 复杂相邻夹具 pad=234 OK / pad=235 over（table/调用/多赋值+下标 LHS）；
// exact 夹具另证 256 槽 vs 第 257 次 AllocLocal。
TEST(test_basic, reg_limit_ok_all_backends) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    ASSERT_NO_THROW(CompileFile(s, "./basic/test_reg_limit_ok.lua", {}));
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_reg_limit", res);
        // sum(1..234) + a + b + t[1] = 27495 + 1 + 2 + 2
        EXPECT_NEAR(res, 27500.0, 0.5) << "jit=" << JitTypeName(jit_type);
    }
    FakeluaDeleteState(s);
}

static void ExpectRegLimitThrow(CompileConfig cfg, const char *label) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr) << label;
    try {
        CompileFile(s, "./basic/test_reg_limit_over.lua", cfg);
        FAIL() << label << ": expected compile to throw too many registers";
    } catch (const std::exception &e) {
        EXPECT_NE(std::string(e.what()).find("too many registers"), std::string::npos) << label << " what=" << e.what();
    }
    FakeluaDeleteState(s);
}

TEST(test_basic, reg_limit_over_consistent_across_backend_gates) {
    CompileConfig all{};
    ExpectRegLimitThrow(all, "all backends enabled");

    CompileConfig no_interp{};
    no_interp.disable_jit[JIT_INTERP] = true;
    ExpectRegLimitThrow(no_interp, "INTERP disabled (TCC/GCC only)");

    CompileConfig only_interp{};
    only_interp.disable_jit[JIT_TCC] = true;
    only_interp.disable_jit[JIT_GCC] = true;
    ExpectRegLimitThrow(only_interp, "INTERP only");
}

TEST(test_basic, reg_limit_nested_over) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    try {
        CompileFile(s, "./basic/test_reg_limit_nested_over.lua", {});
        FAIL() << "expected nested over-limit function to throw";
    } catch (const std::exception &e) {
        EXPECT_NE(std::string(e.what()).find("too many registers"), std::string::npos) << e.what();
    }
    FakeluaDeleteState(s);
}

TEST(test_basic, reg_limit_many_constants_allowed) {
    // 常量池走 sbx(int32)，>256 个互异常量在寄存器高峰不高时应可通过。
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    ASSERT_NO_THROW(CompileFile(s, "./basic/test_reg_limit_many_constants.lua", {}));
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_reg_limit_many_constants", res);
        EXPECT_NEAR(res, 301.0, 0.5) << "jit=" << JitTypeName(jit_type);
    }
    FakeluaDeleteState(s);
}


// Exact 256 vs 257 AllocLocal slots (indices 0..255 vs 第 257 次 AllocLocal 失败)。
// 无公开 introspection：靠「256 个 local + return u256」与「257 个 local」相邻证明槽位上限。
TEST(test_basic, reg_limit_exact_256_ok) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    ASSERT_NO_THROW(CompileFile(s, "./basic/test_reg_limit_exact_ok.lua", {}));
    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        Call(s, jit_type, "test_reg_limit_exact", res);
        EXPECT_NEAR(res, 256.0, 0.5) << "jit=" << JitTypeName(jit_type);
    }
    FakeluaDeleteState(s);
}

TEST(test_basic, reg_limit_exact_257_over_all_gates) {
    auto expect_throw = [](CompileConfig cfg, const char *label) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr) << label;
        try {
            CompileFile(s, "./basic/test_reg_limit_exact_over.lua", cfg);
            FAIL() << label << ": expected too many registers";
        } catch (const std::exception &e) {
            EXPECT_NE(std::string(e.what()).find("too many registers"), std::string::npos) << label << " what=" << e.what();
        }
        FakeluaDeleteState(s);
    };
    expect_throw({}, "all backends");
    CompileConfig no_interp{};
    no_interp.disable_jit[JIT_INTERP] = true;
    expect_throw(no_interp, "INTERP disabled");
    CompileConfig only_interp{};
    only_interp.disable_jit[JIT_TCC] = true;
    only_interp.disable_jit[JIT_GCC] = true;
    expect_throw(only_interp, "INTERP only");
}

// 高峰检查在 SemanticAnalysis::Analyze 中完成，且 CheckFunctionRegisterLimits
// 本身不需要 State（不 Alloc 常量串、不注册 VM、不跑 init）。
TEST(test_basic, reg_limit_check_without_state) {
    const char *accepted[] = {
        "./basic/test_reg_limit_ok.lua",
        "./basic/test_reg_limit_exact_ok.lua",
        "./basic/test_reg_limit_many_constants.lua",
        "./basic/test_init_load_table.lua",
        "./closure/test_closure_nested.lua",
        "./basic/test_basic_pairs.lua",
    };
    for (const char *file: accepted) {
        SCOPED_TRACE(file);
        FakeluaStateGuard guard;
        Compiler compiler(guard.GetState());
        CompileConfig parse_only{};
        parse_only.skip_jit = true;
        auto pr = compiler.CompileFile(file, parse_only);
        PreProcessor pp(guard.GetState());
        pp.Process(pr, {});
        SemanticAnalysis semantic(guard.GetState());
        AnalysisResult ar;
        ASSERT_NO_THROW(ar = semantic.Analyze(pr, {}));
        // Analyze 已做过检查；再直接调用证明该入口无需 State。
        EXPECT_NO_THROW(CheckFunctionRegisterLimits(pr, ar));
    }
    const char *rejected[] = {
        "./basic/test_reg_limit_over.lua",
        "./basic/test_reg_limit_exact_over.lua",
        "./basic/test_reg_limit_nested_over.lua",
    };
    for (const char *file: rejected) {
        SCOPED_TRACE(file);
        FakeluaStateGuard guard;
        Compiler compiler(guard.GetState());
        CompileConfig parse_only{};
        parse_only.skip_jit = true;
        auto pr = compiler.CompileFile(file, parse_only);
        PreProcessor pp(guard.GetState());
        pp.Process(pr, {});
        SemanticAnalysis semantic(guard.GetState());
        try {
            semantic.Analyze(pr, {});
            FAIL() << "expected register limit failure in semantic analysis";
        } catch (const std::exception &e) {
            EXPECT_NE(std::string(e.what()).find("too many registers"), std::string::npos) << e.what();
        }
    }
}

TEST(test_basic, reg_limit_jit_only_execution) {
    struct Case { const char *file; const char *fn; double expected; };
    const Case cases[] = {
        {"./basic/test_reg_limit_ok.lua", "test_reg_limit", 27500},
        {"./basic/test_reg_limit_exact_ok.lua", "test_reg_limit_exact", 256},
        {"./basic/test_reg_limit_many_constants.lua", "test_reg_limit_many_constants", 301},
        {"./basic/test_init_load_table.lua", "InitLoadTable.test_init_load_table", 1},
    };
    CompileConfig cfg{};
    cfg.disable_jit[JIT_INTERP] = true;
    for (const auto &c: cases) {
        SCOPED_TRACE(c.file);
        FakeluaStateGuard guard;
        auto s = guard.GetState();
        ASSERT_NO_THROW(CompileFile(s, c.file, cfg));
        for (auto jit: {JIT_TCC, JIT_GCC}) {
            double result = 0;
            ASSERT_NO_THROW(Call(s, jit, c.fn, result));
            EXPECT_EQ(result, c.expected) << JitTypeName(jit);
        }
    }
    FakeluaStateGuard guard;
    EXPECT_THROW(CompileFile(guard.GetState(), "./basic/test_reg_limit_nested_over.lua", cfg), std::exception);
}
