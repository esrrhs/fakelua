#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_sqlite, open_memory) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_open_memory", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, create_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_create_table", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, insert) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_insert", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, select) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_select", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, select_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_select_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, insert_return_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_insert_return_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_close", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, multiple_inserts) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_multiple_inserts", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, select_where) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_select_where", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, auto_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_auto_close", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
