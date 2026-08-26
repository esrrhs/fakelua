#pragma once

// mysql_connection.h — async MySQL client (callback-based, non-blocking I/O).
// Built on top of net::TcpClient (RawStream) for event-driven TCP.

#include "native/mysql/mysql_protocol.h"
#include "native/mysql/mysql_result.h"
#include "native/net/net_socket.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
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

    // Send raw packet (for pool heartbeat)
    bool send_packet(uint8_t seq, const char *payload, size_t len);

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
    // TCP client from net module (RawStream framing for raw bytes)
    std::unique_ptr<net::TcpClient> client_;
    net::NetConfig net_config_;

    // Protocol state
    enum class State { Idle, Connecting, Handshaking, Ready, Querying, Error };
    State state_ = State::Idle;
    ::fakelua::State *lua_state_ = nullptr;  // fakelua State for callback dispatch
    ::fakelua::NativeObject *native_obj_ = nullptr;  // NativeObject wrapper (for passing to Lua)

    // Raw byte buffer for MySQL packet parsing
    std::vector<uint8_t> recv_buf_;

    // Protocol state
    uint8_t seq_ = 0;
    uint32_t capabilities_ = 0;
    uint8_t charset_ = 0;
    bool ready_ = false;
    bool compress_ = false;  // true after handshake negotiates CLIENT_COMPRESS

    // Auth info (saved during connect, used in handshake)
    std::string user_;
    std::string password_;
    std::string database_;

    // Lua callback function names
    std::string connect_cb_;
    std::string result_cb_;

    // Pending query info
    std::string last_sql_;

    // Query type for differentiating query vs stmt_prepare responses
    enum class QueryType { None, Query, StmtPrepare, StmtExecute };
    QueryType query_type_ = QueryType::None;

    // Multi-result support
    std::vector<MysqlResult> pending_results_;

    // Incremental result set parser state (for large result sets across ticks)
    enum class ParsePhase { Columns, Rows, Done };
    struct ResultSetParser {
        ParsePhase phase = ParsePhase::Columns;
        MysqlResult result;
        uint64_t col_count = 0;
        uint64_t cols_read = 0;
        bool in_result_set = false;
        bool binary_rows = false;
    };
    std::unique_ptr<ResultSetParser> rs_parser_;

    // COM_PING in flight — next OK must not be treated as a query result
    bool ping_inflight_ = false;

    int tick_depth_ = 0;
    bool close_pending_ = false;
    std::string pending_connect_err_;
    int connect_timeout_ms_ = 0;
    int64_t connect_start_ms_ = 0;

    // COM_STMT_PREPARE 之后还要丢掉 param/column definition + EOF
    uint16_t prepare_eofs_remaining_ = 0;
    MysqlResult pending_prepare_result_;

    // Error tracking
    MysqlError last_error_;

    // ── byte buffer → MySQL packet parsing ──
    void feed_bytes(const char *data, size_t len);
    bool try_parse_packet(std::vector<uint8_t> &out_payload);

    // ── protocol handlers ──
    void handle_handshake_packet(const std::vector<uint8_t> &payload);
    void handle_query_packet(const std::vector<uint8_t> &payload);

    // ── Lua callback dispatch ──
    void dispatch_connect(const char *err_msg);
    void dispatch_result(const MysqlResult &result, const char *err_msg);

    // ── error handling ──
    void set_error(MysqlErrorType type, uint16_t code,
                   const std::string &msg, const std::string &sql_state);

    // ── helpers ──
    [[noreturn]] static void net_error(const std::string &msg);
};

}  // namespace fakelua::mysql
