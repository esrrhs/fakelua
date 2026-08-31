#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_io_edge, test_io_open_read) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_read", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_open_write) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_write", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_open_append) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_append", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_read_formats) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_read_formats", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_write_multi) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_write_multi", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_flush) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_flush", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_type", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_type_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_type_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_file_read_line) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_read_line", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_file_read_all) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_read_all", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_file_seek) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_seek", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_file_lines) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_file_lines", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_tmpfile) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_tmpfile", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_close", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_io_edge, test_io_open_nonexistent) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./io/test_io_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "IoTest.test_io_open_nonexistent", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
