#include "fakelua.h"
#include "native/mysql/mysql_field_convert.h"
#include "native/mysql/native_mysql.h"
#include "native/native_common.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;
using namespace fakelua::mysql;


// MySQL 模块测试

// 测试 1: 连接失败时 pcall 能捕获错误（无需真实 MySQL 服务器）
TEST(test_mysql, connect_failure_catchable) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_connect_fail.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_connect_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 2: 连接失败时错误信息包含 "connect"（验证错误传播）
TEST(test_mysql, connect_failure_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_error_message.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_error_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_mysql, close_in_connect_callback) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_connect_fail.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_close_in_connect_cb", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_mysql, connect_ssl_require_failure) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_connect_fail.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_connect_ssl_require", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：需要本地 MySQL 服务（root@127.0.0.1:3306, 密码 root, 数据库 test）

TEST(test_mysql, integration_callback_api) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_integration.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_mysql_integration", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：预处理语句
TEST(test_mysql, integration_prepared_statements) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_stmt.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_stmt", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：多结果集
TEST(test_mysql, integration_multi_result) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_multi_result.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_multi_result", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：连接池
TEST(test_mysql, integration_pool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_pool.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_pool", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：SQL 错误与连接恢复 (P0)
TEST(test_mysql, integration_query_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_query_error.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_query_error", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：数据类型转换与 NULL 列支持 (P0)
TEST(test_mysql, integration_datatypes_and_null) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_datatypes.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_datatypes", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：DML 写操作状态包与自增 ID / 受影响行 (P0)
TEST(test_mysql, integration_dml_status) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_dml.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_dml", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：预处理语句 NULL 参数绑定、类型转换与复用 (P0)
TEST(test_mysql, integration_stmt_params) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_stmt_params.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_stmt_params", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：连接池连接耗尽与通过 conn:close() 归还 (P1)
TEST(test_mysql, integration_pool_advanced) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_pool_adv.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_pool_advanced", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：连接生命周期、ping 心跳、重复关闭幂等与关闭后防护 (P1)
TEST(test_mysql, integration_ping_and_lifecycle) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_lifecycle.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_lifecycle", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_mysql, bad_port) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_connect_fail.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlTest.test_bad_port", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
// 回归（P0-1/P0-2/P1-4）：连接未就绪时用闭包发起 query，
// 连接失败后回调应恰好收到一次错误（曾被静默吞掉或报错丢失）。
TEST(test_mysql, query_closure_callback_exactly_once) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlContractTest.test_query_closure_exactly_once", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 回归（P0-1）：回调参数传非法类型必须响亮报错，而不是静默丢弃回调。
TEST(test_mysql, bad_callback_type_throws) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlContractTest.test_bad_callback_type", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 回归（P1-4）：pool:with 基础行为 —— 无可用连接返回 nil，非函数参数报错。
TEST(test_mysql, pool_with_lease_api) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);
    int64_t ret = 0;
    CallAll(s, "MysqlContractTest.test_pool_with", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 回归（P1-4）：pool:with 的 fn 抛错后连接必须归还，否则下一次 Acquire 拿不到。
// 不依赖真实 MySQL：测试入口把池里的连接标成已连接再走真正的 PoolWith。
TEST(test_mysql, pool_with_fn_throw_returns_connection) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);

    int arg_count = 0;
    bool is_vararg = false;
    void *addr = inter::GetFuncAddr(s, JIT_TCC, "MysqlContractTest.make_thrower", arg_count, is_vararg);
    ASSERT_NE(addr, nullptr);
    CVar fn = inter::DispatchCall(s, addr, nullptr, arg_count, JIT_TCC);
    ASSERT_EQ(fn.type_, static_cast<int>(VarType::Closure));

    const int released = mysql::TestPoolWithFnThrowReturnsConnection(s, fn);
    EXPECT_EQ(released, 1);
    FakeluaDeleteState(s);
}

// 回归（P0-1）：闭包回调跨过下一次顶层 Call 的临时 arena Reset 后仍然能读到捕获的表。
TEST(test_mysql, closure_callback_survives_frame_reset) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);

    int64_t armed = 0;
    CallAll(s, "MysqlContractTest.arm_cross_frame", armed);
    EXPECT_EQ(armed, 1);
    // 顶层 Call 返回后显式再 Reset 一次，模拟宿主在帧末回收临时 arena。
    inter::Reset(s);

    int64_t hits = 0;
    CallAll(s, "MysqlContractTest.pump_cross_frame", hits);
    EXPECT_EQ(hits, 7);
    FakeluaDeleteState(s);
}

// 单元测试（P2-7）：FieldCellToCVar 行值类型转换，无需真实 MySQL 连接。
// 验证整数列返回 Int、浮点/DECIMAL 列返回 Float、其他列返回 String、NULL 返回 nil。
// MYSQL_TYPE_* 常量取自 MariaDB/MySQL 协议规范（固定值，不随版本变化）：
//   LONG=3, LONGLONG=8, DOUBLE=5, NEWDECIMAL=246, VAR_STRING=253
TEST(test_mysql, field_cell_to_cvar_type_conversion) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    // 整数列 TINY（MYSQL_TYPE_TINY = 1）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "1";
        CVar v = FieldCellToCVar(s, fv, 1 /*MYSQL_TYPE_TINY*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Int));
        EXPECT_EQ(v.data_.i, 1);
    }
    // 整数列（MYSQL_TYPE_LONG = 3）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "42";
        CVar v = FieldCellToCVar(s, fv, 3 /*MYSQL_TYPE_LONG*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Int));
        EXPECT_EQ(v.data_.i, 42);
    }
    // 负整数（MYSQL_TYPE_LONGLONG = 8）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "-99";
        CVar v = FieldCellToCVar(s, fv, 8 /*MYSQL_TYPE_LONGLONG*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Int));
        EXPECT_EQ(v.data_.i, -99);
    }
    // 浮点列（MYSQL_TYPE_FLOAT = 4）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "1.5";
        CVar v = FieldCellToCVar(s, fv, 4 /*MYSQL_TYPE_FLOAT*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Float));
        EXPECT_NEAR(v.data_.f, 1.5, 1e-6);
    }
    // 浮点列（MYSQL_TYPE_DOUBLE = 5）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "3.14";
        CVar v = FieldCellToCVar(s, fv, 5 /*MYSQL_TYPE_DOUBLE*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Float));
        EXPECT_NEAR(v.data_.f, 3.14, 1e-9);
    }
    // DECIMAL 列（MYSQL_TYPE_DECIMAL = 0）与 NEWDECIMAL 都落成 double，精度见 README。
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "9.5";
        CVar v = FieldCellToCVar(s, fv, 0 /*MYSQL_TYPE_DECIMAL*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Float));
        EXPECT_NEAR(v.data_.f, 9.5, 1e-9);
    }
    // DECIMAL 列（MYSQL_TYPE_NEWDECIMAL = 246）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "123.456";
        CVar v = FieldCellToCVar(s, fv, 246 /*MYSQL_TYPE_NEWDECIMAL*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Float));
        EXPECT_NEAR(v.data_.f, 123.456, 1e-9);
    }
    // 字符串列（MYSQL_TYPE_VAR_STRING = 253）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "hello";
        CVar v = FieldCellToCVar(s, fv, 253 /*MYSQL_TYPE_VAR_STRING*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::String));
    }
    // NULL 字段（任意列类型）
    {
        FieldCell fv;
        fv.is_null = true;
        fv.value = "";
        CVar v = FieldCellToCVar(s, fv, 3 /*MYSQL_TYPE_LONG*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::Nil));
    }
    // 整数列但值为非数字 → 回退为 string（不崩溃）
    {
        FieldCell fv;
        fv.is_null = false;
        fv.value = "not_a_number";
        CVar v = FieldCellToCVar(s, fv, 3 /*MYSQL_TYPE_LONG*/);
        EXPECT_EQ(v.type_, static_cast<int>(VarType::String));
    }

    FakeluaDeleteState(s);
}
