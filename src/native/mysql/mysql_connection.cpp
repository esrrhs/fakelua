#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_result.h"
#include "native/native_common.h"

#include <cstring>

namespace fakelua::mysql {

MysqlConnection::MysqlConnection() = default;

MysqlConnection::~MysqlConnection() {
    close();
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::connect(const std::string &host, uint16_t port,
                              const std::string &user, const std::string &password,
                              const std::string &database) {
    user_ = user;
    password_ = password;
    database_ = database;

    net_config_.ip = host;
    net_config_.port = port;
    net_config_.non_blocking = true;
    net_config_.no_delay = true;
    net_config_.framer = net::FramerType::RawStream;
    net_config_.max_packet_len = static_cast<int>(MAX_PACKET_SIZE);
    net_config_.recv_buf_size = 256 * 1024;
    net_config_.send_buf_size = 64 * 1024;

    client_ = std::make_unique<net::TcpClient>(net_config_);
    state_ = State::Connecting;
    recv_buf_.clear();
    client_->connect();
}

void MysqlConnection::query(const std::string &sql) {
    if (state_ != State::Ready || !client_ || !client_->connected()) {
        dispatch_result({}, "connection not ready");
        return;
    }
    last_sql_ = sql;
    state_ = State::Querying;

    // Build COM_QUERY packet
    std::string payload;
    payload.push_back(static_cast<char>(COM_QUERY));
    payload.append(sql);
    send_packet(0, payload.data(), payload.size());
}

void MysqlConnection::close() {
    if (client_ && client_->connected() && state_ != State::Idle) {
        // Send COM_QUIT
        std::string payload(1, static_cast<char>(COM_QUIT));
        send_packet(0, payload.data(), payload.size());
    }
    client_.reset();
    state_ = State::Idle;
    ready_ = false;
}

void MysqlConnection::stmt_prepare(const std::string &sql) {
    if (state_ != State::Ready || !client_ || !client_->connected()) {
        dispatch_result({}, "connection not ready for prepare");
        return;
    }

    // Build COM_STMT_PREPARE packet
    std::string payload;
    payload.push_back(static_cast<char>(COM_STMT_PREPARE));
    payload.append(sql);
    send_packet(0, payload.data(), payload.size());

    state_ = State::Querying;  // Reuse Querying state for prepare
}

void MysqlConnection::stmt_execute(uint32_t stmt_id, const std::vector<std::string> &params) {
    if (state_ != State::Ready || !client_ || !client_->connected()) {
        dispatch_result({}, "connection not ready for execute");
        return;
    }

    std::string payload = build_stmt_execute(stmt_id, params);
    send_packet(0, payload.data(), payload.size());

    state_ = State::Querying;
}

void MysqlConnection::stmt_close(uint32_t stmt_id) {
    if (state_ != State::Ready || !client_ || !client_->connected()) return;

    std::string payload;
    payload.push_back(static_cast<char>(COM_STMT_CLOSE));
    write_uint32(payload, stmt_id);
    send_packet(0, payload.data(), payload.size());
}

void MysqlConnection::tick() {
    if (!client_) return;

    // Only tick when we expect data (not idle)
    if (state_ == State::Idle || state_ == State::Error) return;

    client_->tick(
        // on_recv: feed raw bytes into MySQL packet parser
        [this](const char *data, size_t len) {
            feed_bytes(data, len);
        },
        // on_close: connection lost
        [this]() {
            if (state_ == State::Connecting || state_ == State::Handshaking) {
                dispatch_connect("connection closed during handshake");
            } else if (state_ == State::Querying) {
                dispatch_result({}, "connection closed during query");
            } else if (state_ == State::Ready) {
                // Server closed connection (e.g. idle timeout)
            }
            state_ = State::Idle;
            ready_ = false;
        });
}

// ─────────────────────────────────────────────────────────────────────────────
// Packet I/O
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::send_packet(uint8_t seq, const char *payload, size_t len) {
    if (!client_) return;
    std::string pkt = make_packet(seq, payload, len);
    client_->send(pkt.data(), pkt.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// Byte buffer → MySQL packet parsing
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::feed_bytes(const char *data, size_t len) {
    recv_buf_.insert(recv_buf_.end(), data, data + len);

    // Parse all complete packets in the buffer
    while (true) {
        std::vector<uint8_t> payload;
        if (!try_parse_packet(payload)) break;

        // Dispatch based on protocol state
        switch (state_) {
            case State::Handshaking:
                handle_handshake_packet(payload);
                break;
            case State::Querying:
                handle_query_packet(payload);
                break;
            case State::Connecting:
                // Shouldn't receive data before TCP connect completes
                break;
            case State::Ready:
                // Unsolicited packet (shouldn't happen normally)
                break;
            default:
                break;
        }
    }
}

bool MysqlConnection::try_parse_packet(std::vector<uint8_t> &out_payload) {
    if (recv_buf_.size() < 4) return false;

    // MySQL packet header: 3-byte LE payload length + 1-byte sequence
    uint32_t payload_len = recv_buf_[0] | (static_cast<uint8_t>(recv_buf_[1]) << 8) |
                           (static_cast<uint8_t>(recv_buf_[2]) << 16);
    // uint8_t seq = recv_buf_[3];

    if (recv_buf_.size() < 4 + payload_len) return false;  // incomplete

    out_payload.assign(recv_buf_.begin() + 4, recv_buf_.begin() + 4 + payload_len);
    recv_buf_.erase(recv_buf_.begin(), recv_buf_.begin() + 4 + payload_len);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Protocol handlers
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::handle_handshake_packet(const std::vector<uint8_t> &payload) {
    // Convert to char vector for existing parse functions
    std::vector<char> char_payload(payload.begin(), payload.end());

    if (payload.empty()) {
        dispatch_connect("empty handshake packet");
        state_ = State::Error;
        return;
    }

    uint8_t type = payload[0];

    if (type == PACKET_ERR) {
        auto err = parse_err(char_payload);
        dispatch_connect(err.message.c_str());
        state_ = State::Error;
        return;
    }

    if (type == 0x0A) {
        // Handshake packet (protocol v10)
        HandshakeInfo info = parse_handshake(char_payload);

        // Check auth plugin
        if (!info.auth_plugin_name.empty() &&
            info.auth_plugin_name != "mysql_native_password") {
            dispatch_connect(std::format("unsupported auth plugin '{}'", info.auth_plugin_name).c_str());
            state_ = State::Error;
            return;
        }

        // Build and send handshake response
        std::string response = build_handshake_response(info, user_, password_, database_);
        send_packet(1, response.data(), response.size());

        capabilities_ = info.capabilities;
        charset_ = info.charset;
        // Stay in Handshaking state - wait for auth OK/ERR
        return;
    }

    // Auth switch request (0xFE) - not supported in v1
    if (type == PACKET_EOF) {
        dispatch_connect("auth switch / legacy auth not supported");
        state_ = State::Error;
        return;
    }

    // Could be OK after handshake response (some servers skip the handshake packet)
    if (type == PACKET_OK) {
        state_ = State::Ready;
        ready_ = true;
        seq_ = 0;
        dispatch_connect(nullptr);
        return;
    }

    dispatch_connect(std::format("unexpected handshake byte 0x{:02x}", type).c_str());
    state_ = State::Error;
}

void MysqlConnection::handle_query_packet(const std::vector<uint8_t> &payload) {
    if (payload.empty()) {
        dispatch_result({}, "empty query response");
        state_ = State::Ready;
        return;
    }

    std::vector<char> char_payload(payload.begin(), payload.end());
    uint8_t type = payload[0];

    if (type == PACKET_ERR) {
        auto err = parse_err(char_payload);
        state_ = State::Ready;
        dispatch_result({}, err.message.c_str());
        return;
    }

    if (type == PACKET_OK) {
        // OK response (INSERT/UPDATE/DELETE)
        MysqlResult result = parse_ok_to_result(char_payload);

        // Check for more results (SERVER_MORE_RESULTS_EXISTS = 0x00000008)
        if (result.status_flags & 0x00000008) {
            // Multi-result: save this result, signal more coming
            pending_results_.push_back(result);
            // Wait for next packet (don't dispatch yet)
            return;
        }

        state_ = State::Ready;
        dispatch_result(result, nullptr);
        return;
    }

    if (type == PACKET_EOF) {
        // EOF packet: could be end of result set or between columns/rows
        // Check if this is a multi-result separator
        uint16_t status = 0;
        if (char_payload.size() >= 5) {
            status = static_cast<uint8_t>(char_payload[3]) |
                    (static_cast<uint8_t>(char_payload[4]) << 8);
        }

        if (status & 0x00000008) {
            // More results coming
            return;
        }

        // Under CLIENT_DEPRECATE_EOF, 0xFE can be OK-as-EOF
        MysqlResult result;
        result.is_result_set = false;
        state_ = State::Ready;
        dispatch_result(result, nullptr);
        return;
    }

    // Otherwise: column count (length-encoded integer) → result set
    MysqlResult result;
    result.is_result_set = true;
    size_t pos = 0;
    uint64_t col_count = read_lenenc_int(char_payload, pos);

    // Read column definitions (each is a separate packet)
    result.columns.resize(static_cast<size_t>(col_count));
    for (uint64_t i = 0; i < col_count; ++i) {
        std::vector<uint8_t> col_pkt;
        if (!try_parse_packet(col_pkt)) {
            dispatch_result({}, "incomplete column definitions");
            state_ = State::Ready;
            return;
        }
        std::vector<char> col_char(col_pkt.begin(), col_pkt.end());
        result.columns[i] = parse_column_def(col_char);
    }

    // Read EOF/OK between columns and rows
    {
        std::vector<uint8_t> eof_pkt;
        if (!try_parse_packet(eof_pkt)) {
            dispatch_result({}, "incomplete EOF after columns");
            state_ = State::Ready;
            return;
        }
        if (!eof_pkt.empty()) {
            uint8_t h = eof_pkt[0];
            if (h != PACKET_EOF && h != PACKET_OK) {
                dispatch_result({}, "expected EOF after columns");
                state_ = State::Ready;
                return;
            }
        }
    }

    // Read rows until EOF/OK
    while (true) {
        std::vector<uint8_t> row_pkt;
        if (!try_parse_packet(row_pkt)) break;
        if (row_pkt.empty()) break;

        uint8_t h = row_pkt[0];
        if (h == PACKET_EOF || h == PACKET_OK) {
            // Check for more results
            if (row_pkt.size() >= 5) {
                uint16_t status = static_cast<uint8_t>(row_pkt[3]) |
                                 (static_cast<uint8_t>(row_pkt[4]) << 8);
                if (status & 0x00000008) {
                    // More results coming - save and continue
                    pending_results_.push_back(result);
                    return;
                }
            }
            break;
        }
        std::vector<char> row_char(row_pkt.begin(), row_pkt.end());
        result.rows.push_back(parse_row(row_char, result.columns.size()));
    }

    state_ = State::Ready;
    dispatch_result(result, nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// Lua callback dispatch (same mechanism as net module's call_lua_event)
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::dispatch_connect(const char *err_msg) {
    if (!lua_state_ || connect_cb_.empty()) return;

    auto func = lua_state_->GetVM().GetFunction(connect_cb_);
    if (func.Empty()) return;

    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) return;

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    args[1] = inter::NativeToFakeluaString(lua_state_, err_msg ? err_msg : "");
    args[2] = inter::NativeToFakeluaInt(lua_state_, err_msg ? 0 : 1);

    inter::DispatchCall(addr, args, 3, jit_type);
}

void MysqlConnection::dispatch_result(const MysqlResult &result, const char *err_msg) {
    if (!lua_state_ || result_cb_.empty()) return;

    auto func = lua_state_->GetVM().GetFunction(result_cb_);
    if (func.Empty()) return;

    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) return;

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, err_msg);
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[2] = nil;
    } else {
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = result_to_lua(lua_state_, result);
    }

    inter::DispatchCall(addr, args, 3, jit_type);
}

[[noreturn]] void MysqlConnection::net_error(const std::string &msg) {
    ThrowFakeluaException("mysql net: " + msg);
}

}  // namespace fakelua::mysql
