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

TEST(test_sqlite, prepare_bind_step) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_prepare_bind_step", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, prepare_select_where) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_prepare_select_where", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, prepare_nil_bind) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_prepare_nil_bind", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, prepare_reset) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_prepare_reset", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, columns) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_columns", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, last_insert_rowid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_last_insert_rowid", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, changes) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_changes", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, transaction) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_transaction", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, prepare_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_prepare_error", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, prepare_multi_rows) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_prepare_multi_rows", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, blob) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_blob", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, bind_embedded_nul) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_bind_embedded_nul", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, delete_state_without_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_delete_state_without_close", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_sqlite, stmt_close_after_db_close) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./sqlite/test_sqlite_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "SqliteTest.test_stmt_close_after_db_close", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
