#pragma once

// mysql_connection.h — async MySQL client using Boost.MySQL
// Built on top of boost::mysql::connection for asynchronous MySQL operations.

#include <boost/mysql.hpp>
#include <boost/asio.hpp>

// Keep the existing protocol and result headers for StmtParam and compatibility
#include "native/mysql/mysql_protocol.h"
#include "native/mysql/mysql_result.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {
class State;
class NativeObject;
}

namespace fakelua::mysql {

// ── Error classification ──
enum class MysqlErrorType {
    None = 0,
    Connection,     // TCP connect failed, connection lost
    Authentication, // auth failed (wrong password, unsupported plugin)
    Syntax,         // SQL syntax error
    Timeout,        // read/connect timeout
    Protocol,       // protocol parsing error
    Server,         // server-side error (table not found, duplicate key, etc.)
    Unknown         // unclassified
};

struct MysqlError {
    MysqlErrorType type = MysqlErrorType::None;
    int code = 0;           // MySQL error code (e.g. 1045, 1064)
    std::string message;    // error message
    std::string sql_state;  // 5-char SQL state
};

}  // namespace fakelua::mysql

namespace fakelua::mysql {

class MysqlConnection {
public:
    MysqlConnection();
    ~MysqlConnection();

    MysqlConnection(const MysqlConnection &) = delete;
    MysqlConnection &operator=(const MysqlConnection &) = delete;

    // Start async TCP connect. on_connect(conn, err) called when done.
    // timeout_ms <= 0 means no client-side connect/handshake timeout.
    void connect(const std::string &host, uint16_t port,
                 const std::string &user, const std::string &password,
                 const std::string &database, int timeout_ms = 0);

    // Send a query. on_result(result, err) called when response arrives.
    void query(const std::string &sql);

    // Prepared statement API
    void stmt_prepare(const std::string &sql);
    void stmt_execute(uint32_t stmt_id, const std::vector<StmtParam> &params);
    void stmt_close(uint32_t stmt_id);

    // Heartbeat (COM_PING) — for connection pool keepalive.
    // Returns false if the ping was not sent (busy / not ready).
    bool ping();

    // Close connection
    void close();

    // Pump network events (call periodically from game loop)
    void tick();

    // Error info from last operation
    MysqlError last_error() const { return last_error_; }

    // Check if error is retryable (network issues, not auth/syntax)
    static bool is_retryable(MysqlErrorType type);

    // Set Lua callback function names (called by native_mysql.cpp)
    void set_connect_callback(const std::string &name) { connect_cb_ = name; }
    void set_result_callback(const std::string &name) { result_cb_ = name; }
    void set_state(::fakelua::State *state) { lua_state_ = state; }
    void set_native_object(::fakelua::NativeObject *obj) { native_obj_ = obj; }

    bool connected() const { return ready_; }
    bool connecting() const {
        return state_ == State::Connecting || state_ == State::Handshaking;
    }

    // Lua :close() during a callback/tick must not delete *this until the
    // outer native call returns (same pattern as net deferred close).
    int tick_depth() const { return tick_depth_; }
    bool close_pending() const { return close_pending_; }
    void request_close() { close_pending_ = true; }

private:
    // Boost.Asio I/O context for asynchronous operations
    boost::asio::io_context io_ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;

    // Boost.MySQL connection
    boost::mysql::connection conn_;

    // State tracking
    enum class State { Idle, Connecting, Handshaking, Ready, Querying, Error };
    State state_ = State::Idle;
    ::fakelua::State *lua_state_ = nullptr;  // fakelua State for callback dispatch
    ::fakelua::NativeObject *native_obj_ = nullptr;  // NativeObject wrapper (for passing to Lua)

    // Lua callback function names
    std::string connect_cb_;
    std::string result_cb_;

    // Pending query info
    std::string last_sql_;

    // Query type for differentiating query vs stmt_prepare responses
    enum class QueryType { None, Query, StmtPrepare, StmtExecute };
    QueryType query_type_ = QueryType::None;

    // Multi-result support
    std::vector<boost::mysql::results> pending_results_;

    // Prepared statement cache: map from statement ID to prepared statement
    std::unordered_map<uint32_t, boost::mysql::prepared_statement> prepared_statements_;
    uint32_t next_stmt_id_ = 1;

    // Error tracking
    MysqlError last_error_;

    // Connection parameters for reconnect
    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
    uint16_t port_ = 3306;
    int timeout_ms_ = 0;

    // Timing
    int64_t connect_start_ms_ = 0;

    // Lua callback dispatch flags (to avoid reentrancy)
    bool pending_connect_ = false;
    std::string pending_connect_err_;
    bool pending_result_ = false;
    boost::mysql::results pending_result_data_;
    std::string pending_result_err_;

    // Helpers
    void dispatch_connect(const char *err_msg);
    void dispatch_result(const boost::mysql::results &result, const char *err_msg);
    void set_error(MysqlErrorType type, uint16_t code,
                   const std::string &msg, const std::string &sql_state);
};

}  // namespace fakelua::mysql
