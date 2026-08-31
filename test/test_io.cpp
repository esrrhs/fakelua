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

TEST(test_io, test_io_file_lines) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_file_lines.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_file_lines", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_lines_after_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_file_lines.lua", config);
        double res = 0;
        Call(s, jit_type, "test_lines_after_close", res);
        EXPECT_NEAR(res, 6000, 0.5);
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

TEST(test_io, test_io_read_multi) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_read_multi.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_read_multi", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_file_setvbuf) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_file_setvbuf.lua", config);
        double res = 0;
        Call(s, jit_type, "test_file_setvbuf", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_flush) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_flush.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_flush", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_input_output) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_input_output.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_input_output", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, stdin_survives_other_state) {
    CompileConfig config;
    State *a = FakeluaNewState();
    State *b = FakeluaNewState();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    CompileFile(a, "./io/test_io_input_output.lua", config);
    CompileFile(b, "./io/test_io_input_output.lua", config);
    double res = 0;
    Call(a, JIT_TCC, "test_io_input_output", res);
    EXPECT_NEAR(res, 5000, 0.5);
    FakeluaDeleteState(a);
    res = 0;
    Call(b, JIT_TCC, "test_io_input_output", res);
    EXPECT_NEAR(res, 5000, 0.5);
    FakeluaDeleteState(b);
}

TEST(test_io, test_io_lines_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_lines_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_lines_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./io/test_io_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_io_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./io/test_io_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_io_boundary_error", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_io, test_io_error_paths) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./io/test_io_error_paths.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_io_close_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_file_read_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_file_write_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_file_setvbuf_bad_mode", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_file_setvbuf_size_too_large", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_io_open_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_io_open_bad_mode", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_io_popen_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_io, delete_state_without_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileString(s, R"(
function test_io_delete_state_without_close()
    local f = io.tmpfile()
    if f == nil then return 0 end
    f:write("hello")
    return 6000
end
)",
                  config);
    double res = 0;
    Call(s, JIT_TCC, "test_io_delete_state_without_close", res);
    EXPECT_NEAR(res, 6000, 0.5);
    FakeluaDeleteState(s);
}

// IO edge case tests
TEST(test_io, io_open_read) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_read", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_open_write) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_write", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_open_append) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_append", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_read_formats) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_read_formats", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_write_multi) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_write_multi", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_flush) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_flush", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_type", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_type_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_type_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, file_read_line) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_read_line", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, file_read_all) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_read_all", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, file_seek) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_seek", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, file_lines) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_lines", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_tmpfile) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_tmpfile", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_close", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io, io_open_nonexistent) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_nonexistent", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
