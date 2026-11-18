#include "fakelua.h"
#include "jit/vm.h"
#include "native/mysql/mysql_connection_pool.h"
#include "native/mysql/mysql_field_convert.h"
#include "native/mysql/native_mysql.h"
#include "native/native_common.h"
#include "native/net/net_event.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "state/state.h"
#include "test_jit.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

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

// 回归（P1-4）：pool:with 内部发起异步 query 时，连接不能在 fn 返回瞬间立即归还，
// 必须在 query 结果派发完毕后才自动释放回池。
TEST(test_mysql, pool_with_async_query_auto_release) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);

    int arg_count = 0;
    bool is_vararg = false;
    void *addr = inter::GetFuncAddr(s, JIT_TCC, "MysqlContractTest.make_query_runner", arg_count, is_vararg);
    ASSERT_NE(addr, nullptr);
    CVar fn = inter::DispatchCall(s, addr, nullptr, arg_count, JIT_TCC);
    ASSERT_EQ(fn.type_, static_cast<int>(VarType::Closure));

    const int ok = mysql::TestPoolWithAsyncQueryAutoRelease(s, fn);
    EXPECT_EQ(ok, 1);
    FakeluaDeleteState(s);
}

// 回归（P0-1）：异步回调的绑定参数（序列化字节串）跨过下一次顶层 Call 的临时
// arena Reset 后仍然能被反序列化并原样送达回调。
// 回调派回【登记时的引擎】：CallAll 三引擎各自 arm 出自己的连接，pump 帧 tick
// 时该连接的具名回调执行同引擎编译产物、写同引擎的文件级 local，三引擎各自闭环，
// 与旧内联闭包模型行为一致。
TEST(test_mysql, closure_callback_survives_frame_reset) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_contract.lua", config);

    // 各引擎各自 arm；jit_id 区分 global native holder，避免共享 State 上互相覆盖。
    for (const auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "arm jit=" << JitTypeName(jit_type));
        int64_t armed = 0;
        Call(s, jit_type, "MysqlContractTest.arm_cross_frame", armed, static_cast<int64_t>(jit_type));
        EXPECT_EQ(armed, 1);
    }
    // 顶层 Call 返回后显式再 Reset 一次，模拟宿主在帧末回收临时 arena。
    inter::Reset(s);

    for (const auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "pump jit=" << JitTypeName(jit_type));
        int64_t hits = 0;
        Call(s, jit_type, "MysqlContractTest.pump_cross_frame", hits, static_cast<int64_t>(jit_type));
        EXPECT_EQ(hits, 7);
    }
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

TEST(test_mysql, large_multi_results_and_queued_query_callbacks) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./mysql/test_mysql_streamed_results.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "MysqlStream.test_large_and_queued_results", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_mysql, pool_heartbeat_recovers_a_closed_idle_connection) {
    FakeluaStateGuard state;
    PoolConfig config;
    config.host = "127.0.0.1";
    config.port = 3306;
    config.user = "root";
    config.password = "root";
    config.database = "test";
    config.pool_size = 1;
    config.connect_timeout_ms = 1000;
    config.heartbeat_interval_ms = 1;
    config.retry_base_ms = 1;
    config.max_retries = 2;
    MysqlConnectionPool pool(config, state.GetState());
    pool.Initialize();
    auto wait_healthy = [&] {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        do {
            pool.Tick();
            if (pool.HealthyCount() == 1) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } while (std::chrono::steady_clock::now() < deadline);
        return false;
    };
    ASSERT_TRUE(wait_healthy());
    // Drive several real heartbeat request/response cycles before borrowing.
    for (int i = 0; i < 20; ++i) {
        pool.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    auto *connection = pool.Acquire();
    ASSERT_NE(connection, nullptr);
    connection->Close();
    pool.Release(connection);
    ASSERT_TRUE(wait_healthy());
    auto *reconnected = pool.Acquire();
    ASSERT_NE(reconnected, nullptr);
    EXPECT_TRUE(reconnected->Connected());
    pool.Release(reconnected);
    pool.Close();
    pool.Tick();
    EXPECT_EQ(pool.TotalCount(), 0u);
    EXPECT_EQ(pool.Acquire(), nullptr);
}

TEST(test_mysql, pool_close_inside_result_callback_defers_connection_destruction) {
    FakeluaStateGuard state;
    PoolConfig config;
    config.host = "127.0.0.1";
    config.port = 3306;
    config.user = "root";
    config.password = "root";
    config.database = "test";
    config.pool_size = 1;
    config.connect_timeout_ms = 1000;
    config.heartbeat_interval_ms = 0;
    MysqlConnectionPool pool(config, state.GetState());
    bool callback_ran = false;
    RegisterNativeFunction(state.GetState(), "close_pool_from_callback", 0, false, [&](State *s, CVar *, int) {
        callback_ran = true;
        pool.Close();
        // The active Tick frame still owns the connection until it unwinds.
        EXPECT_EQ(pool.TotalCount(), 1u);
        EXPECT_EQ(pool.Acquire(), nullptr);
        return inter::NativeToFakeluaNil(s);
    });
    CompileConfig compile;
    DisableAllJit(compile);
    compile.disable_jit[JIT_INTERP] = false;
    compile.debug_mode = false;
    CompileString(state.GetState(), R"(
        function pool_close_on_result(conn, err, result)
            assert(err == nil)
            assert(result[3][1][1] == 42)
            close_pool_from_callback()
        end
    )",
                  compile);
    pool.Initialize();
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (pool.HealthyCount() != 1 && std::chrono::steady_clock::now() < deadline) {
        pool.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    auto *connection = pool.Acquire();
    ASSERT_NE(connection, nullptr);
    ResultCallback callback;
    callback.name = "pool_close_on_result";
    callback.jit = JIT_INTERP;
    connection->Query("SELECT 42", callback);
    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!callback_ran && std::chrono::steady_clock::now() < deadline) {
        pool.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_TRUE(callback_ran);
    EXPECT_EQ(pool.TotalCount(), 0u);
    pool.Reap();
    pool.Close();
    EXPECT_EQ(pool.HealthyCount(), 0u);
}

TEST(test_mysql, pool_ssl_options_and_outstanding_wrapper_invalidation) {
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        FakeluaStateGuard state;
        CompileFile(state.GetState(), "./mysql/test_mysql_pool_lifecycle.lua", {});
        int64_t result = 0;
        Call(state.GetState(), backend, "test_pool_lifecycle_contracts", result);
        EXPECT_EQ(result, 1);
    }
}

TEST(test_mysql, statement_and_late_multi_result_errors_preserve_callback_and_connection) {
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        FakeluaStateGuard state;
        CompileFile(state.GetState(), "./mysql/test_mysql_streamed_results.lua", {});
        int64_t result = 0;
        Call(state.GetState(), backend, "MysqlStream.test_statement_and_late_multi_result_errors", result);
        EXPECT_EQ(result, 1);
    }
}

TEST(test_mysql, retryable_error_classification) {
    for (auto type: {MysqlErrorType::None, MysqlErrorType::Connection, MysqlErrorType::Authentication, MysqlErrorType::Syntax, MysqlErrorType::Timeout, MysqlErrorType::Protocol,
                     MysqlErrorType::Server, MysqlErrorType::Unknown}) {
        EXPECT_EQ(MysqlConnection::IsRetryable(type), type == MysqlErrorType::Connection || type == MysqlErrorType::Timeout);
    }
}

TEST(test_mysql, closed_cpp_statement_apis_report_errors_through_gcc_callback_fallback) {
    FakeluaStateGuard state;
    State *s = state.GetState();
    int callbacks = 0;
    RegisterNativeFunction(s, "record_closed_mysql", 3, false, [&](State *st, CVar *args, int n) {
        EXPECT_EQ(n, 3);
        EXPECT_EQ(args[0].type_, static_cast<int>(VarType::Nil));
        EXPECT_EQ(inter::CVarToString(args[1], ""), "connection closed");
        EXPECT_EQ(args[2].type_, static_cast<int>(VarType::Nil));
        ++callbacks;
        return inter::NativeToFakeluaNil(st);
    });
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileString(s, "function closed_mysql_result(conn, err, result) record_closed_mysql(conn, err, result) end", config);
    MysqlConnection conn(s);
    conn.SetResultCallback(ResultCallback{"closed_mysql_result"});
    conn.Query("SELECT 1");
    EXPECT_TRUE(conn.HasPendingWork());
    conn.Tick();
    conn.StmtPrepare("SELECT ?");
    EXPECT_TRUE(conn.HasPendingWork());
    conn.Tick();
    conn.StmtExecute(1, {});
    EXPECT_TRUE(conn.HasPendingWork());
    conn.Tick();
    EXPECT_EQ(callbacks, 3);
    EXPECT_FALSE(conn.HasPendingWork());
    EXPECT_FALSE(conn.Connected());
}

TEST(test_mysql, streamed_rows_continue_between_packets_and_preserve_all_results) {
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        FakeluaStateGuard state;
        CompileFile(state.GetState(), "./mysql/test_mysql_streamed_results.lua", {});
        int64_t result = 0;
        Call(state.GetState(), backend, "MysqlStream.test_row_stream_continuations", result);
        EXPECT_EQ(result, 1);
    }
}

TEST(test_mysql, configuration_statement_parameters_and_single_stream) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./mysql/test_mysql_streamed_results.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "MysqlStream.test_configuration_and_statement_parameter_contracts", result);
    EXPECT_EQ(result, 1);
}

TEST(test_mysql, explicit_group_destruction_unregisters_live_wrapper) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(function live_wrapper_factory() return mysql.connect({host = "127.0.0.1", user = "root"}, "unused_callback") end)", {});
    CVar value = CallByNameArgs(state.GetState(), JIT_INTERP, "live_wrapper_factory", 0, nullptr);
    auto *object = NativeObject::Unwrap(value);
    ASSERT_NE(object, nullptr);
    const auto group = object->GetGroupId();
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 1u);
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 0u);
}

TEST(test_mysql, cpp_legacy_queued_queries_and_connecting_statement_guards) {
    FakeluaStateGuard state;
    std::vector<std::string> errors;
    std::vector<int64_t> rows;
    RegisterNativeFunction(state.GetState(), "record_cpp_mysql_result", 2, false, [&](State *s, CVar *args, int) {
        if (args[0].type_ != static_cast<int>(VarType::Nil)) {
            errors.push_back(inter::CVarToString(args[0], ""));
        } else {
            auto all_rows = table::TableHelper::GetTableInt(s, args[1], 3);
            auto row = table::TableHelper::GetTableInt(s, all_rows, 1);
            rows.push_back(inter::CVarToInteger(table::TableHelper::GetTableInt(s, row, 1)));
        }
        return inter::NativeToFakeluaNil(s);
    });
    CompileConfig config;
    DisableAllJit(config);
    config.disable_jit[JIT_INTERP] = false;
    CompileString(state.GetState(), R"(function cpp_mysql_result(conn, err, result) record_cpp_mysql_result(err, result) end)", config);
    ResultCallback callback;
    callback.name = "cpp_mysql_result";
    callback.jit = JIT_INTERP;
    for (bool prepare: {true, false}) {
        MysqlConnection opening(state.GetState());
        opening.SetResultCallback(callback);
        opening.Connect("127.0.0.1", 3306, "root", "root", "test", 1000);
        ASSERT_TRUE(opening.Connecting());
        if (prepare) opening.StmtPrepare("SELECT 1");
        else
            opening.StmtExecute(1, {});
        const auto expected = errors.size() + 1;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (errors.size() < expected && std::chrono::steady_clock::now() < deadline) {
            opening.Tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ASSERT_EQ(errors.size(), expected);
        EXPECT_NE(errors.back().find(prepare ? "not ready for prepare" : "not ready for execute"), std::string::npos);
        opening.Close();
    }
    MysqlConnection connection(state.GetState());
    connection.SetResultCallback(callback);
    connection.Connect("127.0.0.1", 3306, "root", "root", "test", 1000);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!connection.Connected() && std::chrono::steady_clock::now() < deadline) {
        connection.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_TRUE(connection.Connected());
    connection.Query("SELECT 41 AS value, SLEEP(0.02)");
    connection.Query("SELECT 42 AS value");
    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (rows.size() < 2 && std::chrono::steady_clock::now() < deadline) {
        connection.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_EQ(rows, (std::vector<int64_t>{41, 42}));
    connection.RequestClose();
    connection.Query("SELECT 43");
    connection.Tick();
    ASSERT_EQ(errors.size(), 3u);
    EXPECT_NE(errors.back().find("closing"), std::string::npos);
    connection.Close();
}

TEST(test_mysql, handshake_deadline_with_silent_local_tcp_peer) {
    FakeluaStateGuard state;
    net::NetConfig peer_config;
    peer_config.ip = "127.0.0.1";
    peer_config.port = 19990;
    peer_config.framer = net::FramerType::RawStream;
    net::TcpServer peer(peer_config, state.GetState());
    peer.Start();
    ASSERT_TRUE(peer.Running());
    std::string error;
    RegisterNativeFunction(state.GetState(), "record_mysql_deadline", 1, false, [&](State *s, CVar *args, int) {
        error = inter::CVarToString(args[0], "");
        return inter::NativeToFakeluaNil(s);
    });
    CompileConfig config;
    DisableAllJit(config);
    config.disable_jit[JIT_INTERP] = false;
    CompileString(state.GetState(), "function mysql_deadline_callback(conn, err, success) record_mysql_deadline(err) end", config);
    MysqlConnection connection(state.GetState());
    ResultCallback callback;
    callback.name = "mysql_deadline_callback";
    callback.jit = JIT_INTERP;
    connection.SetConnectCallback(callback);
    connection.Connect("127.0.0.1", peer_config.port, "root", "root", "test", 10);
    ASSERT_TRUE(connection.Connecting());
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    connection.Tick();
    EXPECT_FALSE(connection.Connected());
    EXPECT_FALSE(connection.Connecting());
    EXPECT_NE(error.find("timeout"), std::string::npos);
    connection.Close();
    peer.Stop();
}

TEST(test_mysql, destroying_pool_group_invalidates_borrowed_wrapper_before_finalizer) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(
        function pool_group_factory()
            local pool = mysql_pool.create({host = "127.0.0.1", port = 3306,
                user = "root", password = "root", db = "test", pool_size = 1, heartbeat_ms = 0})
            local conn = nil
            for i = 1, 3000 do
                runtime.tick()
                conn = pool:acquire()
                if conn then break end
                os.sleep(1)
            end
            assert(conn ~= nil)
            return pool, conn
        end
        function borrowed_after_pool_destruction(conn)
            assert(conn:error() == nil)
            assert(not pcall(function() conn:query("SELECT 1", "unused_pool_callback") end))
            conn:close()
            return 1
        end
    )",
                  {});
    CVar result = CallByNameArgs(state.GetState(), JIT_INTERP, "pool_group_factory", 0, nullptr);
    auto *pool = NativeObject::Unwrap(inter::GetMultiCVarElement(result, 0));
    CVar borrowed = inter::GetMultiCVarElement(result, 1);
    ASSERT_NE(pool, nullptr);
    ASSERT_NE(NativeObject::Unwrap(borrowed), nullptr);
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(pool->GetGroupId()), 1u);
    CVar success = CallByNameArgs(state.GetState(), JIT_INTERP, "borrowed_after_pool_destruction", 1, &borrowed);
    EXPECT_EQ(inter::CVarToInteger(success), 1);
}
