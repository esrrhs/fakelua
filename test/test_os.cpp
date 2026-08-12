#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_os, test_os_clock) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_clock.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_clock", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_date) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_date.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_date", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_difftime) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_difftime.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_difftime", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_execute) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_execute.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_execute", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_date_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_date_table.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_date_table", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_execute_triple) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_execute_triple.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_execute_triple", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_getenv) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_getenv.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_getenv", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_time) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_time.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_time", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_tmpname) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_tmpname.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_tmpname", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_remove_rename) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_remove_rename.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_remove_rename", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_setlocale) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_setlocale.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_setlocale", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_exit) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_exit.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_exit", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_date_format) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_date_format.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_date_format", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_time_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_time_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_time_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_date_utc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_date_utc.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_date_utc", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_os_date_numeric_ts", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_os_date_string_ts", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_os_date_table_utc", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_setlocale_extended) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_date_utc.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_setlocale_query", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_os_setlocale_category", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_execute_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_execute_empty.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_execute_empty", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./os/test_os_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_os_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_os, test_os_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./os/test_os_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    {
        double res = 0;
        EXPECT_THROW(Call(s, JIT_GCC, "test_os_boundary_error", res), std::exception);
    }
    {
        double res = 0;
        EXPECT_THROW(Call(s, JIT_GCC, "test_os_boundary_error2", res), std::exception);
    }
    {
        double res = 0;
        EXPECT_THROW(Call(s, JIT_GCC, "test_os_boundary_error3", res), std::exception);
    }

    FakeluaDeleteState(s);
}
