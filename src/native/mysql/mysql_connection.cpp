#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_result.h"
#include "native/native_common.h"

#include <chrono>
#include <cstddef>
#include <cstring>

namespace fakelua::mysql {

namespace {

int64_t now_ms() {
    auto tp = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

struct TickDepthGuard {
    int &depth;
    explicit TickDepthGuard(int &d) : depth(d) { ++depth; }
    ~TickDepthGuard() { if (depth > 0) --depth; }
};

constexpr uint64_t kMaxResultColumns = 4096;

}  // namespace

MysqlConnection::MysqlConnection() = default;

MysqlConnection::~MysqlConnection() {
    close();
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::connect(const std::string &host, uint16_t port,
                              const std::string &user, const std::string &password,
                              const std::string &database, int timeout_ms) {
    user_ = user;
    password_ = password;
    database_ = database;
    connect_timeout_ms_ = timeout_ms;
    connect_start_ms_ = now_ms();
    pending_connect_err_.clear();
    close_pending_ = false;
    compress_ = false;  // renegotiated on each fresh handshake

    net_config_.ip = host;
    net_config_.port = port;
    net_config_.non_blocking = true;
    net_config_.no_delay = true;
    net_config_.framer = net::FramerType::RawStream;
    net_config_.max_packet_len = static_cast<int>(MAX_PACKET_SIZE);
    net_config_.recv_buf_size = 256 * 1024;
    net_config_.send_buf_size = 256 * 1024;

    client_ = std::make_unique<net::TcpClient>(net_config_);
    state_ = State::Connecting;
    recv_buf_.clear();
    client_->connect();

    fprintf(stderr, "[mysql] connect: connected=%d connecting=%d\n",
            client_->connected(), client_->connecting());

    // Immediate TCP failure (e.g. refused): do not dispatch here — Lua close()
    // in a nested callback would delete *this while connect() is still on stack.
    if (!client_->connected() && !client_->connecting()) {
        fprintf(stderr, "[mysql] connect failed immediately\n");
        pending_connect_err_ = "connection failed";
        state_ = State::Error;
    }
}

void MysqlConnection::query(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !client_ || !client_->connected()) {
        dispatch_result({}, "connection not ready");
        return;
    }
    if (ping_inflight_) {
        dispatch_result({}, "connection busy (ping in progress)");
        return;
    }
    last_sql_ = sql;

    // Build COM_QUERY packet
    std::string payload;
    payload.push_back(static_cast<char>(COM_QUERY));
    payload.append(sql);
    if (!send_packet(0, payload.data(), payload.size())) {
        dispatch_result({}, "failed to send query");
        return;
    }
    state_ = State::Querying;
    query_type_ = QueryType::Query;
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
    ping_inflight_ = false;
    query_type_ = QueryType::None;
    prepare_eofs_remaining_ = 0;
    rs_parser_.reset();
}

void MysqlConnection::stmt_prepare(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !client_ || !client_->connected()) {
        dispatch_result({}, "connection not ready for prepare");
        return;
    }
    if (ping_inflight_) {
        dispatch_result({}, "connection busy (ping in progress)");
        return;
    }

    // Build COM_STMT_PREPARE packet
    std::string payload;
    payload.push_back(static_cast<char>(COM_STMT_PREPARE));
    payload.append(sql);
    if (!send_packet(0, payload.data(), payload.size())) {
        dispatch_result({}, "failed to send prepare");
        return;
    }
    state_ = State::Querying;
    query_type_ = QueryType::StmtPrepare;
}

void MysqlConnection::stmt_execute(uint32_t stmt_id, const std::vector<StmtParam> &params) {
    if (close_pending_) return;
    if (state_ != State::Ready || !client_ || !client_->connected()) {
        dispatch_result({}, "connection not ready for execute");
        return;
    }
    if (ping_inflight_) {
        dispatch_result({}, "connection busy (ping in progress)");
        return;
    }

    std::string payload = build_stmt_execute(stmt_id, params);
    if (!send_packet(0, payload.data(), payload.size())) {
        dispatch_result({}, "failed to send execute");
        return;
    }

    state_ = State::Querying;
    query_type_ = QueryType::StmtExecute;
}

void MysqlConnection::stmt_close(uint32_t stmt_id) {
    if (close_pending_) return;
    if (state_ != State::Ready || !client_ || !client_->connected()) return;
    if (ping_inflight_) return;

    std::string payload;
    payload.push_back(static_cast<char>(COM_STMT_CLOSE));
    write_uint32(payload, stmt_id);
    send_packet(0, payload.data(), payload.size());
}

bool MysqlConnection::ping() {
    if (close_pending_) return false;
    if (state_ != State::Ready || query_type_ != QueryType::None || ping_inflight_) return false;
    if (!client_ || !client_->connected()) return false;

    std::string payload(1, static_cast<char>(0x0E));
    if (!send_packet(0, payload.data(), payload.size())) return false;
    ping_inflight_ = true;
    return true;
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
    if (tick_depth_ > 0) return;
    TickDepthGuard guard(tick_depth_);

    if (!pending_connect_err_.empty()) {
        std::string msg = std::move(pending_connect_err_);
        pending_connect_err_.clear();
        dispatch_connect(msg.c_str());
        return;
    }

    if (!client_) return;

    // Only tick when we expect data (not idle / terminal error)
    if (state_ == State::Idle || state_ == State::Error) return;

    if ((state_ == State::Connecting || state_ == State::Handshaking) && connect_timeout_ms_ > 0) {
        if (now_ms() - connect_start_ms_ >= connect_timeout_ms_) {
            dispatch_connect("connect timeout");
            close();
            state_ = State::Error;
            return;
        }
    }

    // TCP connect may already be complete (localhost often connects immediately).
    if (state_ == State::Connecting && client_->connected()) {
        state_ = State::Handshaking;
        fprintf(stderr, "[mysql] TCP connect completed, now handshaking\n");
    }

    client_->tick(
        // on_recv: feed raw bytes into MySQL packet parser
        [this](const char *data, size_t len) {
            // Connect can complete in the same wait as the first handshake bytes.
            if (state_ == State::Connecting && client_ && client_->connected()) {
                state_ = State::Handshaking;
            }
            fprintf(stderr, "[mysql] recv: %zu bytes (state=%d)\n", len, static_cast<int>(state_));
            feed_bytes(data, len);
        },
        // on_close: connection lost
        [this]() {
            fprintf(stderr, "[mysql] connection closed (state=%d)\n", static_cast<int>(state_));
            if (!close_pending_) {
                if (state_ == State::Connecting || state_ == State::Handshaking) {
                    dispatch_connect("connection closed during handshake");
                } else if (state_ == State::Querying) {
                    dispatch_result({}, "connection closed during query");
                }
            }
            state_ = State::Idle;
            ready_ = false;
        });

    if (client_ && state_ == State::Connecting && client_->connected()) {
        state_ = State::Handshaking;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Packet I/O
// ─────────────────────────────────────────────────────────────────────────────

bool MysqlConnection::send_packet(uint8_t seq, const char *payload, size_t len) {
    if (!client_) return false;
    std::string pkt = compress_ ? make_compressed_packet(seq, payload, len)
                                : make_packet(seq, payload, len);
    return client_->send(pkt.data(), pkt.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// Byte buffer → MySQL packet parsing
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::feed_bytes(const char *data, size_t len) {
    recv_buf_.insert(recv_buf_.end(), data, data + len);

    if (state_ == State::Connecting && client_ && client_->connected()) {
        state_ = State::Handshaking;
    }

    // Parse all complete packets in the buffer
    while (true) {
        if (close_pending_) break;
        // Handshake bytes can arrive in the same wait as TCP connect completion.
        // Keep them buffered until we have entered Handshaking.
        if (state_ == State::Connecting) break;

        std::vector<uint8_t> payload;
        if (!try_parse_packet(payload)) break;

        try {
            // Dispatch based on protocol state
            switch (state_) {
                case State::Handshaking:
                    handle_handshake_packet(payload);
                    break;
                case State::Querying:
                    handle_query_packet(payload);
                    break;
                case State::Connecting:
                    break;
                case State::Ready:
                    if (ping_inflight_ && query_type_ == QueryType::None && !rs_parser_ &&
                        !payload.empty() && payload[0] == PACKET_OK) {
                        ping_inflight_ = false;
                    } else if (ping_inflight_ && !payload.empty() && payload[0] == PACKET_ERR) {
                        ping_inflight_ = false;
                    }
                    break;
                default:
                    break;
            }
        } catch (const std::exception &e) {
            // Protocol parsing errors (e.g. unexpected packet type) should not crash.
            // Report as connection/auth failure instead.
            fprintf(stderr, "[mysql] feed_bytes exception (state=%d): %s\n",
                    static_cast<int>(state_), e.what());
            if (state_ == State::Handshaking || state_ == State::Connecting) {
                dispatch_connect(e.what());
            } else if (state_ == State::Querying) {
                dispatch_result({}, e.what());
            }
            state_ = State::Error;
            break;
        }
    }
}

bool MysqlConnection::try_parse_packet(std::vector<uint8_t> &out_payload) {
    size_t consumed = 0;
    uint8_t seq = 0;
    bool ok = compress_ ? consume_compressed_packet(recv_buf_.data(), recv_buf_.size(), consumed, out_payload, seq)
                        : consume_logical_packet(recv_buf_.data(), recv_buf_.size(), consumed, out_payload, seq);
    if (!ok) return false;
    recv_buf_.erase(recv_buf_.begin(), recv_buf_.begin() + static_cast<std::ptrdiff_t>(consumed));
    seq_ = seq;
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
        if (!send_packet(static_cast<uint8_t>(seq_ + 1), response.data(), response.size())) {
            dispatch_connect("failed to send handshake response");
            state_ = State::Error;
            return;
        }

        capabilities_ = info.capabilities;
        charset_ = info.charset;
        // Remember whether compression was negotiated. It is enabled only after
        // the handshake completes (see the PACKET_OK handler below) because the
        // server sends the auth switch / OK response to the handshake response
        // uncompressed.
        if ((kMyCapabilities & info.capabilities) & CLIENT_COMPRESS) {
            fprintf(stderr, "[mysql] compression negotiated, enabling after handshake\n");
        }
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
        if (password_.empty()) {
            // 空密码：初始握手与 AuthSwitch 都发空 token，不要再 hash。
            auth_response.clear();
        } else if (plugin_name == "mysql_native_password") {
            // mysql_native_password expects 20-byte scramble
            std::string scramble = auth_data;
            if (scramble.size() > 20) scramble.resize(20);
            auto hash = native_password_hash(password_, scramble);
            auth_response.assign(hash.begin(), hash.end());
        } else if (plugin_name == "caching_sha2_password" || plugin_name == "_sha2_password") {
            // AuthSwitchRequest remaining data is the scramble (optional trailing NUL), not a type byte.
            std::string scramble = auth_data;
            if (!scramble.empty() && scramble.back() == '\0') scramble.pop_back();
            if (scramble.size() > 20) scramble.resize(20);
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
        if (!send_packet(static_cast<uint8_t>(seq_ + 1),
                    reinterpret_cast<const char *>(auth_response.data()),
                    auth_response.size())) {
            dispatch_connect("failed to send auth response");
            state_ = State::Error;
            return;
        }
        // Stay in Handshaking state — wait for OK/ERR
        return;
    }

    // Could be OK after handshake response (some servers skip the handshake packet)
    if (type == PACKET_OK) {
        state_ = State::Ready;
        ready_ = true;
        seq_ = 0;
        // Handshake is complete — enable compressed protocol now. The server
        // sends the handshake-response reply (auth switch / this OK) uncompressed,
        // but all packets after the handshake are compressed.
        if (capabilities_ & CLIENT_COMPRESS) {
            compress_ = true;
            fprintf(stderr, "[mysql] compressed protocol enabled (handshake done)\n");
        }
        dispatch_connect(nullptr);
        return;
    }

    // caching_sha2_password AuthMoreData (0x01)
    // 0x03 = fast auth success, wait for OK; 0x04 = full auth (RSA/SSL)，本模块不支持。
    if (type == 0x01) {
        if (payload.size() > 1 && payload[1] == 0x04) {
            dispatch_connect("caching_sha2_password full authentication (RSA/SSL) is not supported");
            state_ = State::Error;
            return;
        }
        fprintf(stderr, "[mysql] caching_sha2_password fast auth complete, waiting for OK\n");
        // Stay in Handshaking state — wait for OK/ERR
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

    if (query_type_ == QueryType::StmtPrepare) {
        if (prepare_eofs_remaining_ > 0) {
            if (type == PACKET_ERR) {
                auto err = parse_err(char_payload);
                set_error(classify_error_code(err.error_code), err.error_code, err.message, err.sql_state);
                prepare_eofs_remaining_ = 0;
                query_type_ = QueryType::None;
                state_ = State::Ready;
                dispatch_result({}, err.message.c_str());
                return;
            }
            if (type == PACKET_EOF) {
                --prepare_eofs_remaining_;
                if (prepare_eofs_remaining_ == 0) {
                    query_type_ = QueryType::None;
                    state_ = State::Ready;
                    dispatch_result(pending_prepare_result_, nullptr);
                }
                return;
            }
            // param / column definition packet — discard
            return;
        }
        if (type == PACKET_ERR) {
            auto err = parse_err(char_payload);
            set_error(classify_error_code(err.error_code), err.error_code, err.message, err.sql_state);
            query_type_ = QueryType::None;
            state_ = State::Ready;
            dispatch_result({}, err.message.c_str());
            return;
        }
        auto prepare_result = parse_prepare_response(char_payload);
        pending_prepare_result_ = MysqlResult{};
        pending_prepare_result_.is_result_set = false;
        pending_prepare_result_.stmt_id = prepare_result.statement_id;
        if (!prepare_result.valid) {
            query_type_ = QueryType::None;
            state_ = State::Ready;
            dispatch_result({}, "prepare failed");
            return;
        }
        prepare_eofs_remaining_ = static_cast<uint16_t>(
            (prepare_result.num_params > 0 ? 1 : 0) + (prepare_result.num_columns > 0 ? 1 : 0));
        if (prepare_eofs_remaining_ == 0) {
            query_type_ = QueryType::None;
            state_ = State::Ready;
            dispatch_result(pending_prepare_result_, nullptr);
        }
        return;
    }

    if (type == PACKET_ERR) {
        auto err = parse_err(char_payload);
        auto err_type = classify_error_code(err.error_code);
        set_error(err_type, err.error_code, err.message, err.sql_state);
        ping_inflight_ = false;
        query_type_ = QueryType::None;
        state_ = State::Ready;
        rs_parser_.reset();
        dispatch_result({}, err.message.c_str());
        return;
    }

    // Binary rows start with 0x00; do not treat them as OK while reading rows.
    if (rs_parser_ && rs_parser_->phase == ParsePhase::Rows && type != PACKET_EOF) {
        try {
            if (rs_parser_->binary_rows) {
                std::vector<ColType> types;
                std::vector<uint16_t> flags;
                types.reserve(rs_parser_->result.columns.size());
                flags.reserve(rs_parser_->result.columns.size());
                for (const auto &c : rs_parser_->result.columns) {
                    types.push_back(c.type);
                    flags.push_back(c.flags);
                }
                rs_parser_->result.rows.push_back(parse_binary_row(char_payload, types, flags));
            } else {
                rs_parser_->result.rows.push_back(parse_row(char_payload, rs_parser_->result.columns.size()));
            }
        } catch (const std::exception &e) {
            fprintf(stderr, "[mysql] parse_row exception: %s\n", e.what());
            state_ = State::Ready;
            query_type_ = QueryType::None;
            rs_parser_.reset();
            dispatch_result({}, e.what());
        }
        return;
    }

    if (type == PACKET_OK) {
        if (ping_inflight_ && query_type_ == QueryType::None && !rs_parser_) {
            ping_inflight_ = false;
            return;
        }
        MysqlResult result = parse_ok_to_result(char_payload);
        if (result.status_flags & 0x00000008) {
            dispatch_result(result, nullptr);
            return;
        }
        query_type_ = QueryType::None;
        state_ = State::Ready;
        rs_parser_.reset();
        dispatch_result(result, nullptr);
        return;
    }

    if (type == PACKET_EOF) {
        if (rs_parser_ && rs_parser_->in_result_set) {
            if (rs_parser_->phase == ParsePhase::Rows) {
                MysqlResult result = std::move(rs_parser_->result);
                bool more_results = false;
                if (char_payload.size() >= 5) {
                    uint16_t status = static_cast<uint8_t>(char_payload[3]) |
                                     (static_cast<uint8_t>(char_payload[4]) << 8);
                    more_results = (status & 0x00000008);
                }
                rs_parser_.reset();
                dispatch_result(result, nullptr);
                if (more_results) {
                    state_ = State::Querying;
                } else {
                    query_type_ = QueryType::None;
                    state_ = State::Ready;
                }
                return;
            }
        }

        if (rs_parser_ && rs_parser_->in_result_set &&
            rs_parser_->phase == ParsePhase::Columns) {
            rs_parser_->phase = ParsePhase::Rows;
            return;
        }

        MysqlResult result;
        result.is_result_set = false;
        query_type_ = QueryType::None;
        state_ = State::Ready;
        rs_parser_.reset();
        dispatch_result(result, nullptr);
        return;
    }

    if (!rs_parser_) {
        rs_parser_ = std::make_unique<ResultSetParser>();
        rs_parser_->result.is_result_set = true;
        rs_parser_->in_result_set = true;
        rs_parser_->phase = ParsePhase::Columns;
        rs_parser_->binary_rows = (query_type_ == QueryType::StmtExecute);
        size_t pos = 0;
        rs_parser_->col_count = read_lenenc_int(char_payload, pos);
        if (rs_parser_->col_count > kMaxResultColumns) {
            rs_parser_.reset();
            query_type_ = QueryType::None;
            state_ = State::Error;
            dispatch_result({}, "too many columns in result set");
            return;
        }
        rs_parser_->cols_read = 0;
        rs_parser_->result.columns.resize(static_cast<size_t>(rs_parser_->col_count));
        return;
    }

    if (rs_parser_->phase == ParsePhase::Columns) {
        try {
            if (rs_parser_->cols_read < rs_parser_->col_count) {
                rs_parser_->result.columns[rs_parser_->cols_read] = parse_column_def(char_payload);
                ++rs_parser_->cols_read;
            }
        } catch (const std::exception &e) {
            fprintf(stderr, "[mysql] parse_column_def exception: %s\n", e.what());
            state_ = State::Error;
            query_type_ = QueryType::None;
            rs_parser_.reset();
            dispatch_result({}, e.what());
        }
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Lua callback dispatch (same mechanism as net module's call_lua_event)
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::dispatch_connect(const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    if (close_pending_) return;
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
    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg && err_msg[0]) {
        args[1] = inter::NativeToFakeluaString(lua_state_, err_msg);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 0);
        fprintf(stderr, "[mysql] dispatch_connect: calling callback with msg=%s success=0\n", err_msg);
    } else {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 1);
        fprintf(stderr, "[mysql] dispatch_connect: calling callback success=1\n");
    }

    inter::DispatchCall(addr, args, 3, jit_type);
}

void MysqlConnection::dispatch_result(const MysqlResult &result, const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    if (close_pending_) return;
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

    fprintf(stderr, "[mysql] dispatch_result: err_msg=%s cb=%s result.is_result_set=%d rows=%zu cols=%zu\n",
            err_msg ? err_msg : "(null)", result_cb_.c_str(),
            result.is_result_set, result.rows.size(), result.columns.size());

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[2] = nil;
    } else if (result.stmt_id != 0) {
        // COM_STMT_PREPARE response: pass statement_id as number
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = inter::NativeToFakeluaInt(lua_state_, static_cast<int64_t>(result.stmt_id));
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
