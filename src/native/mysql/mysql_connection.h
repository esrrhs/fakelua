#pragma once

#include "native/mysql/mysql_protocol.h"
#include "native/mysql/mysql_result.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {
class State;
class NativeObject;
}

namespace fakelua::mysql {

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

class MysqlConnection {
public:
    MysqlConnection();
    ~MysqlConnection();

    MysqlConnection(const MysqlConnection &) = delete;
    MysqlConnection &operator=(const MysqlConnection &) = delete;

    void connect(const std::string &host, uint16_t port,
                 const std::string &user, const std::string &password,
                 const std::string &database, int timeout_ms = 0);

    void query(const std::string &sql);
    void stmt_prepare(const std::string &sql);
    void stmt_execute(uint32_t stmt_id, const std::vector<StmtParam> &params);
    void stmt_close(uint32_t stmt_id);
    bool ping();
    void close();
    void tick();

    MysqlError last_error() const { return last_error_; }
    static bool is_retryable(MysqlErrorType type);

    void set_connect_callback(const std::string &name) { connect_cb_ = name; }
    void set_result_callback(const std::string &name) { result_cb_ = name; }
    void set_state(::fakelua::State *state) { lua_state_ = state; }
    void set_native_object(::fakelua::NativeObject *obj) { native_obj_ = obj; }

    bool connected() const { return ready_; }
    bool connecting() const { return state_ == State::Connecting; }

    int tick_depth() const { return tick_depth_; }
    bool close_pending() const { return close_pending_; }
    void request_close() { close_pending_ = true; }

private:
    struct PreparedStatement {
        std::string sql;
        size_t param_count = 0;
    };

    enum class State { Idle, Connecting, Ready, Busy, Error };
    enum class OperationType { None, Query, Heartbeat, Prepare };
    enum class AsyncPhase { None, Start, StoreResult, NextResult, DispatchPrepare };

    void *mysql_ = nullptr;
    State state_ = State::Idle;
    OperationType operation_ = OperationType::None;
    AsyncPhase phase_ = AsyncPhase::None;

    ::fakelua::State *lua_state_ = nullptr;
    ::fakelua::NativeObject *native_obj_ = nullptr;

    std::string host_;
    uint16_t port_ = 3306;
    std::string user_;
    std::string password_;
    std::string database_;
    int connect_timeout_ms_ = 0;
    int64_t connect_start_ms_ = 0;

    bool ready_ = false;
    bool close_pending_ = false;
    int tick_depth_ = 0;
    std::string pending_connect_err_;

    std::string connect_cb_;
    std::string result_cb_;
    std::string pending_sql_;
    uint32_t pending_stmt_id_ = 0;
    MysqlResult pending_result_;
    std::unordered_map<uint32_t, PreparedStatement> prepared_statements_;
    uint32_t next_stmt_id_ = 1;

    MysqlError last_error_;

    void tick_connect();
    void tick_operation();

    void dispatch_connect(const char *err_msg);
    void dispatch_result(const MysqlResult &result, const char *err_msg);

    void set_error(MysqlErrorType type, uint16_t code,
                   const std::string &msg, const std::string &sql_state);
};

}  // namespace fakelua::mysql
