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

class MysqlConnection {
public:
    MysqlConnection();
    ~MysqlConnection();

    MysqlConnection(const MysqlConnection &) = delete;
    MysqlConnection &operator=(const MysqlConnection &) = delete;

    // Start async TCP connect. on_connect(conn, err) called when done.
    void connect(const std::string &host, uint16_t port,
                 const std::string &user, const std::string &password,
                 const std::string &database);

    // Send a query. on_result(result, err) called when response arrives.
    void query(const std::string &sql);

    // Prepared statement API
    void stmt_prepare(const std::string &sql);
    void stmt_execute(uint32_t stmt_id, const std::vector<std::string> &params);
    void stmt_close(uint32_t stmt_id);

    // Close connection
    void close();

    // Pump network events (call periodically from game loop)
    void tick();

    // Set Lua callback function names (called by native_mysql.cpp)
    void set_connect_callback(const std::string &name) { connect_cb_ = name; }
    void set_result_callback(const std::string &name) { result_cb_ = name; }
    void set_state(::fakelua::State *state) { lua_state_ = state; }
    void set_native_object(::fakelua::NativeObject *obj) { native_obj_ = obj; }

    bool connected() const { return ready_; }

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

    // Auth info (saved during connect, used in handshake)
    std::string user_;
    std::string password_;
    std::string database_;

    // Lua callback function names
    std::string connect_cb_;
    std::string result_cb_;

    // Pending query info
    std::string last_sql_;

    // Multi-result support
    std::vector<MysqlResult> pending_results_;

    // ── packet I/O ──
    void send_packet(uint8_t seq, const char *payload, size_t len);

    // ── byte buffer → MySQL packet parsing ──
    void feed_bytes(const char *data, size_t len);
    bool try_parse_packet(std::vector<uint8_t> &out_payload);

    // ── protocol handlers ──
    void handle_handshake_packet(const std::vector<uint8_t> &payload);
    void handle_query_packet(const std::vector<uint8_t> &payload);

    // ── Lua callback dispatch ──
    void dispatch_connect(const char *err_msg);
    void dispatch_result(const MysqlResult &result, const char *err_msg);

    // ── helpers ──
    [[noreturn]] static void net_error(const std::string &msg);
};

}  // namespace fakelua::mysql
