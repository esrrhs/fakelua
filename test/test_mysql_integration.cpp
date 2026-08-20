#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// MySQL 集成测试 — 需要本地 MySQL 服务
// 环境：root@127.0.0.1:3306，密码 root，数据库 test

TEST(test_mysql_integration, connect_query_insert) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_integration.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_mysql_integration", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_mysql_integration, null_mapping) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_integration.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_mysql_null", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
