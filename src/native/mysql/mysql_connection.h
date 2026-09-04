#pragma once

// mysql_connection.h — async MySQL client using Boost.MySQL
// Built on top of boost::mysql::any_connection for asynchronous MySQL operations.

#include <boost/mysql.hpp>
#include <boost/asio.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for CVar
namespace fakelua {
struct CVar;
class State;
class NativeObject;
}

namespace fakelua::mysql {

// COM_STMT_EXECUTE 参数。is_null 时写入 null-bitmap，不附带值。
struct StmtParam {
    bool is_null = false;
    std::string value;
};

}  // namespace fakelua::mysql

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
    MysqlError last_error() const;

    // Check if error is retryable (network issues, not auth/syntax)
    static bool is_retryable(MysqlErrorType type);

    // Set Lua callback function names (called by native_mysql.cpp)
    void set_connect_callback(const std::string &name);
    void set_result_callback(const std::string &name);
    void set_state(::fakelua::State *state);
    void set_native_object(::fakelua::NativeObject *obj);

    bool connected() const;
    bool connecting() const;

    // Lua :close() during a callback/tick must not delete *this until the
    // outer native call returns (same pattern as net deferred close).
    int tick_depth() const;
    bool close_pending() const;
    void request_close();

private:
    // Boost.Asio I/O context for asynchronous operations
    boost::asio::io_context io_ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;

    // Boost.MySQL connection (modern any_connection API)
    boost::mysql::any_connection conn_;

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
    std::unordered_map<uint32_t, boost::mysql::statement> prepared_statements_;
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
    // When stmt_prepare completes, the callback receives (err, stmt_id)
    // rather than a row set. We carry that id through tick() to dispatch_result.
    bool has_pending_stmt_id_ = false;
    uint32_t pending_stmt_id_ = 0;
    // Set transiently in tick() right before dispatching, read by dispatch_result.
    uint32_t dispatch_stmt_id_ = 0;

    // Connection state
    bool ready_ = false;
    int tick_depth_ = 0;
    bool close_pending_ = false;

    // Persistent diagnostics object for async operations. Boost.MySQL stores
    // a reference to this object internally and writes to it during async
    // completion, so it MUST outlive the async operation (member, not stack-local).
    boost::mysql::diagnostics async_diag_;

    // Helpers
    void dispatch_connect(const char *err_msg);
    void dispatch_result(const boost::mysql::results &result, const char *err_msg);
    void set_error(MysqlErrorType type, uint16_t code,
                   const std::string &msg, const std::string &sql_state);

    // Convert Boost.MySQL results to Lua table
    static CVar result_to_lua(::fakelua::State *s, const boost::mysql::results &result);
    // Convert one Boost.MySQL field to (is_null, string_value).
    static std::pair<bool, std::string> field_to_string(const boost::mysql::field_view &fv);

private:
    // Async connect handler signature expects (error_code, diagnostics)
    // but we use a lambda that only captures error_code. This is OK since
    // Boost.Asio's completion token machinery supports it.
    // When we need to pass diagnostics for error reporting, we use the
    // overload that takes diagnostics& as a parameter.
};

}  // namespace fakelua::mysql