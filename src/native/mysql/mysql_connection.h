#pragma once

// mysql_connection.h — async MySQL client using libmysqlclient (MariaDB Connector/C)
// MYSQL_OPT_NONBLOCK + libevent wait on mysql_get_socket().

#include "native/native_io_context.h"

#ifdef __cplusplus
#ifndef HAVE_BOOL
#define HAVE_BOOL 1
#endif
#endif
#include <mysql.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct event;

namespace fakelua {
struct CVar;
class State;
class NativeObject;
class VarClosure;
}// namespace fakelua

namespace fakelua::mysql {

struct StmtParam {
    bool is_null = false;
    std::string value;
};

// C++ → Lua 回调：支持全局函数名（字符串）或闭包（VarClosure*）。
// 闭包由 arena 分配，生命周期覆盖整个 State，裸指针持有是安全的。
struct ResultCallback {
    std::string name;
    VarClosure *closure = nullptr;

    [[nodiscard]] bool Empty() const { return name.empty() && closure == nullptr; }
};

enum class SslMode {
    Disable = 0,
    Enable,
    Require,
};

enum class MysqlErrorType {
    None = 0,
    Connection,
    Authentication,
    Syntax,
    Timeout,
    Protocol,
    Server,
    Unknown
};

struct MysqlError {
    MysqlErrorType type = MysqlErrorType::None;
    int code = 0;
    std::string message;
    std::string sql_state;
};

struct FieldCell {
    bool is_null = false;
    std::string value;
};

struct ResultsetData {
    bool is_resultset = false;
    std::vector<std::pair<std::string, int>> columns;
    std::vector<std::vector<FieldCell>> rows;
    uint64_t affected_rows = 0;
    uint64_t last_insert_id = 0;
    std::string info;
};

class MysqlConnection {
public:
    explicit MysqlConnection(::fakelua::State *state);
    ~MysqlConnection();

    MysqlConnection(const MysqlConnection &) = delete;
    MysqlConnection &operator=(const MysqlConnection &) = delete;

    void Connect(const std::string &host, uint16_t port, const std::string &user, const std::string &password, const std::string &database, int timeout_ms = 0, SslMode ssl = SslMode::Disable, std::string ssl_ca = {});

    void Query(const std::string &sql);
    void StmtPrepare(const std::string &sql);
    void StmtExecute(uint32_t stmt_id, const std::vector<StmtParam> &params);
    void StmtClose(uint32_t stmt_id);
    bool Ping();
    void Close();
    void Tick();

    MysqlError LastError() const;
    static bool IsRetryable(MysqlErrorType type);

    void SetConnectCallback(const ResultCallback &cb);
    void SetResultCallback(const ResultCallback &cb);
    void SetState(::fakelua::State *state);
    void SetNativeObject(::fakelua::NativeObject *obj);

    bool Connected() const;
    bool Connecting() const;

    int TickDepth() const;
    bool ClosePending() const;
    void RequestClose();

private:
    native::IoContext &io_;
    MYSQL *mysql_ = nullptr;
    event *wait_ev_ = nullptr;

    enum class ConnState { Idle, Connecting, Handshaking, Ready, Querying, Error };
    ConnState state_ = ConnState::Idle;
    ::fakelua::State *lua_state_ = nullptr;
    ::fakelua::NativeObject *native_obj_ = nullptr;

    ResultCallback connect_cb_;
    ResultCallback result_cb_;
    std::string last_sql_;

    // 同连接飞行中再次发起的 query：排队而不是报 "connection not ready"，
    // 由 Tick() 在上一条结果派发完毕、连接回到 Ready 后依次启动。
    struct QueuedQuery {
        std::string sql;
        ResultCallback cb;
    };
    std::vector<QueuedQuery> queued_queries_;

    enum class QueryType { None, Query, StmtPrepare, StmtExecute, Ping };
    QueryType query_type_ = QueryType::None;

    enum class WaitOp { None, Connect, Query, Store, Next, Ping, StmtPrepare, StmtExecute, StmtStore };
    WaitOp wait_op_ = WaitOp::None;

    std::unordered_map<uint32_t, MYSQL_STMT *> prepared_statements_;
    uint32_t next_stmt_id_ = 1;
    uint32_t pending_exec_stmt_id_ = 0;
    MYSQL_STMT *pending_stmt_ = nullptr;
    std::vector<StmtParam> pending_stmt_params_;
    std::vector<MYSQL_BIND> pending_binds_;
    std::vector<unsigned long> pending_bind_lens_;

    MysqlError last_error_;

    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
    uint16_t port_ = 3306;
    int timeout_ms_ = 0;
    SslMode ssl_mode_ = SslMode::Disable;
    std::string ssl_ca_;

    int64_t connect_start_ms_ = 0;

    bool pending_connect_ = false;
    std::string pending_connect_err_;
    bool pending_result_ = false;
    std::string pending_result_err_;
    std::vector<ResultsetData> pending_results_;
    bool has_pending_stmt_id_ = false;
    uint32_t pending_stmt_id_ = 0;
    uint32_t dispatch_stmt_id_ = 0;

    bool ready_ = false;
    int tick_depth_ = 0;
    bool close_pending_ = false;

    native::LifeToken life_;

    void DispatchConnect(const char *err_msg);
    void DispatchResult(const char *err_msg);
    void DispatchCallbackWithResult(const ResultCallback &cb, const char *err_msg);
    void SetError(MysqlErrorType type, uint16_t code, const std::string &msg, const std::string &sql_state);

    // 统一的回调调用入口：闭包走 DispatchCallClosure，函数名走 VM 查表。
    // 回调缺失或不可调用时返回空 CVar（调用方无需关心返回值）。
    void InvokeCallback(const ResultCallback &cb, CVar *args, int n);
    // 连接可用时启动下一条排队 query；连接进入终态（Error/Idle）时
    // 逐条给排队的 query 回调派发错误，保证"每条 query 恰好一次回调"。
    void DrainQueryQueue();

    void Teardown();
    void ApplySsl();
    void ArmWait(int status);
    void ClearWait();
    void Continue(int ready);
    void FinishConnect(MYSQL *ret);
    void StartStore();
    void FinishQueryOk();
    ResultsetData ConsumeResult(MYSQL_RES *res, bool is_resultset);
    ResultsetData ConsumeStmtResult(MYSQL_STMT *stmt);

    static CVar ResultsetToLua(::fakelua::State *s, const ResultsetData &result);
};

}// namespace fakelua::mysql
