#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_string, test_string_len) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_len.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_len", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_sub) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_sub.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_sub", res);
        EXPECT_EQ(res, 200);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_rep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_rep.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_rep", res);
        EXPECT_EQ(res, 300);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_case) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_case.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_case", res);
        EXPECT_EQ(res, 400);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_byte_char) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_byte_char.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_byte_char", res);
        EXPECT_EQ(res, 500);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_format) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_format.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_format", res);
        EXPECT_EQ(res, 600);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_dump) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_dump.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_dump", res);
        EXPECT_EQ(res, 700);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_find) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_find.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_find", res);
        EXPECT_EQ(res, 1000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_match) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_match.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_match", res);
        EXPECT_EQ(res, 2000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_gmatch) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_gmatch.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_gmatch", res);
        EXPECT_EQ(res, 3000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_gsub) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_gsub.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_gsub", res);
        EXPECT_EQ(res, 4000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_pack_unpack) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_pack_unpack.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_pack_unpack", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}


TEST(test_string, test_string_charpattern) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_charpattern.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_charpattern", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_loadfile) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_loadfile.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_loadfile", res);
        EXPECT_NEAR(res, 7000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_format_p) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_format_p.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_format_p", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_rep_bad_sep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_string_rep_bad_sep.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_rep_bad_sep", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_load_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_load_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_load_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_loadfile_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_loadfile_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_loadfile_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_pack_c_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_pack_c_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_pack_c_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_pack_i_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_pack_i_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_pack_i_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_pack_i16_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_pack_i_bad_arg.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_pack_i16_throw", res), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_string, test_pack_c_huge_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_pack_i_bad_arg.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_pack_c_huge_throw", res), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_string, test_format_huge_width) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_pack_i_bad_arg.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_huge_width", res), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_string, test_format_n_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_pack_i_bad_arg.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_n_throw", res), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_string, test_packsize_z_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_packsize_z_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_packsize_z_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_format_q_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_format_q_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_q_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_gsub_bad_repl_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_gsub_bad_repl_bool.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_gsub_bad_repl_bool", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_gsub_bad_table_value_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_gsub_bad_table_value_bool.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_gsub_bad_table_value_bool", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_gsub_bad_func_return_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_gsub_bad_func_return_bool.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_gsub_bad_func_return_bool", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_format_s_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_format_s_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_s_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_format_s_bad_arg_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_format_s_bad_arg_table.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_s_bad_arg_table", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_format_p_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_format_p_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_p_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_format_p_bad_arg_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_format_p_bad_arg_table.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_format_p_bad_tbl", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_pack_c_bad_arg_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_pack_c_bad_arg_table.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_pack_c_bad_tbl", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_len_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_string_len_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_len_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_len_bad_arg_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_string_len_bad_arg_table.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_len_bad_tbl", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_reverse) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_reverse.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_reverse", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_rep_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_rep_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_rep_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_rep_2pow63) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./string/test_string_rep_boundary.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_rep_2pow63", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_rep_nan", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_rep_frac", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_byte_multi) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_byte_multi.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_byte_multi", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_sub_undeclared_var) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    // fmod 与 C 标准库 math.h 中的 fmod 同名；未在 Lua 中声明时应求值为 nil，
    // 传给 string.sub 必须抛出异常（与 Lua 5.4 行为一致），不得被当成 C 函数指针调用。
    const std::string script = R"(
        function test_fmod_sub()
            local suA_ub = string.sub(fmod, 3)
            return 0
        end
    )";

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileString(s, script, config);
        int64_t res = 0;
        EXPECT_THROW(Call(s, jit_type, "test_fmod_sub", res), std::exception);
    }

    FakeluaDeleteState(s);
}

