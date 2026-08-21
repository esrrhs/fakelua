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

    fprintf(stderr, "[mysql] connect: connected=%d connecting=%d\n",
            client_->connected(), client_->connecting());

    // If connect() failed immediately (e.g. connection refused), link_ is nullptr
    // and tick() will never fire on_close. Dispatch the error now.
    if (!client_->connected() && !client_->connecting()) {
        fprintf(stderr, "[mysql] connect failed immediately\n");
        dispatch_connect("connection failed");
        state_ = State::Error;
    }
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

void MysqlConnection::ping() {
    if (!client_ || !client_->connected()) return;

    // COM_PING = 0x0E
    std::string payload(1, static_cast<char>(0x0E));
    send_packet(0, payload.data(), payload.size());
}

bool MysqlConnection::is_retryable(MysqlErrorType type) {
    switch (type) {
        case MysqlErrorType::Connection:
        case MysqlErrorType::Timeout:
            return true;   // network issues, can retry
        case MysqlErrorType::Authentication:
        case MysqlErrorType::Syntax:
        case MysqlErrorType::Protocol:
            return false;  // logic errors, retry won't help
        case MysqlErrorType::Server:
        case MysqlErrorType::Unknown:
        default:
            return false;  // server errors, don't retry by default
    }
}

void MysqlConnection::tick() {
    if (!client_) return;

    // Only tick when we expect data (not idle)
    if (state_ == State::Idle || state_ == State::Error) return;

    // Check if TCP connect just completed — transition from Connecting to Handshaking
    if (state_ == State::Connecting && client_->connected()) {
        state_ = State::Handshaking;
        fprintf(stderr, "[mysql] TCP connect completed, now handshaking\n");
    }

    client_->tick(
        // on_recv: feed raw bytes into MySQL packet parser
        [this](const char *data, size_t len) {
            fprintf(stderr, "[mysql] recv: %zu bytes (state=%d)\n", len, static_cast<int>(state_));
            feed_bytes(data, len);
        },
        // on_close: connection lost
        [this]() {
            fprintf(stderr, "[mysql] connection closed (state=%d)\n", static_cast<int>(state_));
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

        // Check auth plugin — support mysql_native_password, caching_sha2_password, and _sha2_password (MySQL 8 variant)
        if (!info.auth_plugin_name.empty() &&
            info.auth_plugin_name != "mysql_native_password" &&
            info.auth_plugin_name != "caching_sha2_password" &&
            info.auth_plugin_name != "_sha2_password") {
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

    // Auth switch request (0xFE) — server wants us to switch auth plugin
    if (type == PACKET_EOF) {
        // Parse auth switch request: 0xFE + NUL-terminated plugin name + auth data
        std::string plugin_name;
        std::string auth_data;
        size_t pos = 1;
        if (pos < payload.size()) {
            // Read NUL-terminated plugin name
            while (pos < payload.size() && payload[pos] != 0) {
                plugin_name.push_back(static_cast<char>(payload[pos]));
                ++pos;
            }
            if (pos < payload.size()) ++pos; // skip NUL
            // Read remaining auth data
            if (pos < payload.size()) {
                auth_data.assign(char_payload.begin() + static_cast<ssize_t>(pos), char_payload.end());
            }
        }

        fprintf(stderr, "[mysql] auth switch: plugin='%s' auth_data_len=%zu\n",
                plugin_name.c_str(), auth_data.size());

        // Build auth response using the requested plugin
        std::vector<uint8_t> auth_response;
        if (plugin_name == "mysql_native_password") {
            // mysql_native_password expects 20-byte scramble
            std::string scramble = auth_data;
            if (scramble.size() > 20) scramble.resize(20);
            auto hash = native_password_hash(password_, scramble);
            auth_response.assign(hash.begin(), hash.end());
        } else if (plugin_name == "caching_sha2_password" || plugin_name == "_sha2_password") {
            // caching_sha2_password auth data: 1-byte fast auth type + 20-byte scramble
            std::string scramble;
            if (auth_data.size() > 1) {
                scramble = auth_data.substr(1); // skip fast auth type byte
            } else {
                scramble = auth_data;
            }
            if (scramble.size() != 20) {
                dispatch_connect(std::format("auth switch: caching_sha2_password scramble must be 20 bytes, got {}", scramble.size()).c_str());
                state_ = State::Error;
                return;
            }
            auth_response = caching_sha2_password_hash(password_, scramble);
        } else {
            dispatch_connect(std::format("auth switch: unsupported plugin '{}'", plugin_name).c_str());
            state_ = State::Error;
            return;
        }

        // Send auth response
        send_packet(3, reinterpret_cast<const char *>(auth_response.data()), auth_response.size());
        // Stay in Handshaking state — wait for OK/ERR
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

    set_error(MysqlErrorType::Protocol, 0,
              std::format("unexpected handshake byte 0x{:02x}", type), "");
    dispatch_connect(last_error_.message.c_str());
    state_ = State::Error;
}

// ─────────────────────────────────────────────────────────────────────────────
// Error classification (must be before handle_query_packet)
// ─────────────────────────────────────────────────────────────────────────────

static MysqlErrorType classify_error_code(uint16_t code) {
    // MySQL error code ranges:
    // 1000-1999: server errors (ER_HASHCHK, ER_NISAMCHK, etc.)
    // 2000-2999: client errors (connection, protocol)
    // 1045: ER_ACCESS_DENIED_ERROR (auth)
    // 1064: ER_PARSE_ERROR (syntax)
    // 1205: ER_LOCK_WAIT_TIMEOUT (timeout)
    // 1213: ER_LOCK_DEADLOCK (server)
    // 2003: CR_CONN_HOST_ERROR (connection)
    // 2006: CR_SERVER_GONE_ERROR (connection lost)
    // 2013: CR_SERVER_LOST (connection lost during query)
    switch (code) {
        case 1045:  // ER_ACCESS_DENIED_ERROR
        case 1698:  // ER_ACCESS_ACCESS_DENIED_ERROR
            return MysqlErrorType::Authentication;
        case 1064:  // ER_PARSE_ERROR
        case 1149:  // ER_SYNTAX_ERROR
            return MysqlErrorType::Syntax;
        case 1205:  // ER_LOCK_WAIT_TIMEOUT
        case 1213:  // ER_LOCK_DEADLOCK
            return MysqlErrorType::Timeout;
        case 2003:  // CR_CONN_HOST_ERROR
        case 2006:  // CR_SERVER_GONE_ERROR
        case 2013:  // CR_SERVER_LOST
            return MysqlErrorType::Connection;
        default:
            if (code >= 1000 && code < 2000) return MysqlErrorType::Server;
            if (code >= 2000 && code < 3000) return MysqlErrorType::Connection;
            return MysqlErrorType::Unknown;
    }
}

void MysqlConnection::set_error(MysqlErrorType type, uint16_t code,
                                const std::string &msg, const std::string &sql_state) {
    last_error_.type = type;
    last_error_.code = code;
    last_error_.message = msg;
    last_error_.sql_state = sql_state;
}

void MysqlConnection::handle_query_packet(const std::vector<uint8_t> &payload) {
    if (payload.empty()) {
        dispatch_result({}, "empty query response");
        state_ = State::Ready;
        rs_parser_.reset();
        return;
    }

    std::vector<char> char_payload(payload.begin(), payload.end());
    uint8_t type = payload[0];

    if (type == PACKET_ERR) {
        auto err = parse_err(char_payload);
        auto err_type = classify_error_code(err.error_code);
        set_error(err_type, err.error_code, err.message, err.sql_state);
        state_ = State::Ready;
        rs_parser_.reset();
        dispatch_result({}, err.message.c_str());
        return;
    }

    if (type == PACKET_OK) {
        // OK response (INSERT/UPDATE/DELETE)
        MysqlResult result = parse_ok_to_result(char_payload);

        // Check for more results (SERVER_MORE_RESULTS_EXISTS = 0x00000008)
        if (result.status_flags & 0x00000008) {
            // Multi-result: save this result, wait for next result set
            pending_results_.push_back(result);
            return;
        }

        state_ = State::Ready;
        rs_parser_.reset();
        dispatch_result(result, nullptr);
        return;
    }

    if (type == PACKET_EOF) {
        // EOF packet: could be end of result set or between columns/rows
        if (rs_parser_ && rs_parser_->in_result_set) {
            if (rs_parser_->phase == ParsePhase::Rows) {
                // End of rows — result set complete
                state_ = State::Ready;
                MysqlResult result = std::move(rs_parser_->result);
                bool more_results = false;

                // Check for more results
                if (char_payload.size() >= 5) {
                    uint16_t status = static_cast<uint8_t>(char_payload[3]) |
                                     (static_cast<uint8_t>(char_payload[4]) << 8);
                    more_results = (status & 0x00000008);
                }

                rs_parser_.reset();

                if (more_results) {
                    // Dispatch this result but keep state for more
                    // Note: multi-result with result sets is complex;
                    // we dispatch each result as it completes
                    dispatch_result(result, nullptr);
                    state_ = State::Querying;  // still expecting more
                } else {
                    dispatch_result(result, nullptr);
                }
                return;
            }
        }

        // Under CLIENT_DEPRECATE_EOF, 0xFE can be OK-as-EOF
        MysqlResult result;
        result.is_result_set = false;
        state_ = State::Ready;
        rs_parser_.reset();
        dispatch_result(result, nullptr);
        return;
    }

    // Otherwise: column count (length-encoded integer) → result set
    if (!rs_parser_) {
        // Start new result set parser
        rs_parser_ = std::make_unique<ResultSetParser>();
        rs_parser_->result.is_result_set = true;
        rs_parser_->in_result_set = true;
        rs_parser_->phase = ParsePhase::Columns;

        size_t pos = 0;
        rs_parser_->col_count = read_lenenc_int(char_payload, pos);
        rs_parser_->cols_read = 0;
        rs_parser_->result.columns.resize(static_cast<size_t>(rs_parser_->col_count));
    }

    // Feed the packet to the incremental parser
    if (rs_parser_ && rs_parser_->phase == ParsePhase::Columns) {
        // This packet is a column definition
        std::vector<char> col_char(payload.begin(), payload.end());
        if (rs_parser_->cols_read < rs_parser_->col_count) {
            rs_parser_->result.columns[rs_parser_->cols_read] = parse_column_def(col_char);
            ++rs_parser_->cols_read;
        }

        if (rs_parser_->cols_read >= rs_parser_->col_count) {
            // All columns read, move to rows phase
            rs_parser_->phase = ParsePhase::Rows;
        }
        // Wait for more packets (column defs or EOF/rows)
        return;
    }

    if (rs_parser_ && rs_parser_->phase == ParsePhase::Rows) {
        // This packet is a row (not EOF/OK, those are handled above)
        std::vector<char> row_char(payload.begin(), payload.end());
        rs_parser_->result.rows.push_back(parse_row(row_char, rs_parser_->result.columns.size()));
        // Wait for more rows or EOF
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Lua callback dispatch (same mechanism as net module's call_lua_event)
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::dispatch_connect(const char *err_msg) {
    fprintf(stderr, "[mysql] dispatch_connect: err_msg=%s cb=%s\n",
            err_msg ? err_msg : "(null)", connect_cb_.c_str());

    if (!lua_state_ || connect_cb_.empty()) {
        fprintf(stderr, "[mysql] dispatch_connect: no state or no callback\n");
        return;
    }

    auto func = lua_state_->GetVM().GetFunction(connect_cb_);
    if (func.Empty()) {
        fprintf(stderr, "[mysql] dispatch_connect: function not found\n");
        return;
    }

    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) {
        fprintf(stderr, "[mysql] dispatch_connect: no JIT address\n");
        return;
    }

    // Ensure we always have a valid error message (never empty string with failure)
    const char *msg = err_msg && err_msg[0] ? err_msg : "connection failed";

    fprintf(stderr, "[mysql] dispatch_connect: calling callback with msg=%s success=%d\n",
            msg, err_msg ? 0 : 1);

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    args[1] = inter::NativeToFakeluaString(lua_state_, msg);
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

    // Ensure we always have a valid error message (never empty string with failure)
    const char *msg = err_msg && err_msg[0] ? err_msg : "query failed";

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
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
