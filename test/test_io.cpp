#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_io, test_io_open_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_open_close.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_open_close", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_read_write) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_read_write.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_read_write", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_seek) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_seek.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_seek", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_type.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_type", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_tmpfile) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_tmpfile.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_tmpfile", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_popen) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_popen.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_popen", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_long_line) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_long_line.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_long_line", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}
