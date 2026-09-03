#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_result.h"
#include "native/native_common.h"
#include "util/logging.h"

#include <chrono>
#include <cstring>
#include <string>

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

}  // namespace

MysqlConnection::MysqlConnection()
    : work_(boost::asio::make_work_guard(io_ctx_))
{
}

MysqlConnection::~MysqlConnection() {
    close();
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::connect(const std::string &host, uint16_t port,
                              const std::string &user, const std::string &password,
                              const std::string &database, int timeout_ms) {
    // Store connection parameters
    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    timeout_ms_ = timeout_ms;
    connect_start_ms_ = now_ms();
    pending_connect_err_.clear();
    close_pending_ = false;
    state_ = State::Connecting;

    // Reset connection state
    conn_.close();
    pending_results_.clear();
    prepared_statements_.clear();
    next_stmt_id_ = 1;
    last_error_ = {};

    // Start asynchronous connect
    boost::asio::ip::tcp::resolver resolver(io_ctx_);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    boost::asio::async_connect(conn_.lowest_layer(), endpoints,
        [this](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
            if (ec) {
                pending_connect_err_ = ec.message();
                state_ = State::Error;
                return;
            }

            // Start MySQL handshake
            boost::mysql::ssl_mode ssl_mode = boost::mysql::ssl_mode::disable;
            boost::mysql::handshake_params params(user_, password_, database_);
            if (timeout_ms_ > 0) {
                params.expires_after(std::chrono::milliseconds(timeout_ms_));
            }

            conn_.async_handshake(ssl_mode, params,
                [this](const boost::system::error_code& ec, boost::mysql::handshake_output) {
                    if (ec) {
                        pending_connect_err_ = ec.message();
                        state_ = State::Error;
                        return;
                    }
                    state_ = State::Ready;
                    ready_ = true;
                });
        });
}

void MysqlConnection::query(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !conn_.lowest_layer().is_open()) {
        dispatch_result({}, "connection not ready");
        return;
    }

    last_sql_ = sql;
    state_ = State::Querying;
    query_type_ = QueryType::Query;

    // Execute query asynchronously
    conn_.async_query(sql,
        [this](const boost::system::error_code& err, boost::mysql::results result) {
            if (err) {
                pending_result_err_ = err.message();
                pending_result_ = true;
                state_ = State::Ready;
                return;
            }
            pending_result_data_ = std::move(result);
            pending_result_ = true;
            state_ = State::Ready;
        });
}

void MysqlConnection::stmt_prepare(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !conn_.lowest_layer().is_open()) {
        dispatch_result({}, "connection not ready for prepare");
        return;
    }

    state_ = State::Querying;
    query_type_ = QueryType::StmtPrepare;

    // Prepare statement asynchronously
    conn_.async_prepare(sql,
        [this](const boost::system::error_code& err, boost::mysql::prepared_statement stmt) {
            if (err) {
                pending_result_err_ = err.message();
                pending_result_ = true;
                state_ = State::Ready;
                return;
            }

            // Store prepared statement and return statement ID
            uint32_t stmt_id = next_stmt_id_++;
            prepared_statements_[stmt_id] = std::move(stmt);

            // Create result with statement ID
            boost::mysql::results result;
            result.emplace_back();
            result.front().affected_rows = 0; // Not used for prepare
            result.front().last_insert_id = 0;
            result.front().stmt_id = stmt_id;

            pending_result_data_ = std::move(result);
            pending_result_ = true;
            state_ = State::Ready;
        });
}

void MysqlConnection::stmt_execute(uint32_t stmt_id, const std::vector<StmtParam> &params) {
    if (close_pending_) return;
    if (state_ != State::Ready || !conn_.lowest_layer().is_open()) {
        dispatch_result({}, "connection not ready for execute");
        return;
    }

    // Find prepared statement
    auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end()) {
        dispatch_result({}, "statement not prepared");
        return;
    }

    state_ = State::Querying;
    query_type_ = QueryType::StmtExecute;

    // Convert StmtParam to boost::mysql::tuple
    boost::mysql::tuple tuple_params;
    for (const auto& param : params) {
        if (param.is_null) {
            tuple_params.push_back(nullptr);
        } else {
            tuple_params.push_back(param.value);
        }
    }

    // Execute prepared statement asynchronously
    conn_.async_execute(it->second, tuple_params,
        [this](const boost::system::error_code& err, boost::mysql::results result) {
            if (err) {
                pending_result_err_ = err.message();
                pending_result_ = true;
                state_ = State::Ready;
                return;
            }
            pending_result_data_ = std::move(result);
            pending_result_ = true;
            state_ = State::Ready;
        });
}

void MysqlConnection::stmt_close(uint32_t stmt_id) {
    if (close_pending_) return;
    if (state_ != State::Ready || !conn_.lowest_layer().is_open()) return;

    // Remove prepared statement from cache
    prepared_statements_.erase(stmt_id);

    // Note: Boost.MySQL doesn't require explicit statement close when destroying
    // the prepared_statement object, which we do by erasing from the map
}

bool MysqlConnection::ping() {
    if (close_pending_) return false;
    if (state_ != State::Ready || !conn_.lowest_layer().is_open()) return false;

    // Ping asynchronously
    conn_.async_ping(
        [this](const boost::system::error_code& err) {
            if (err) {
                pending_result_err_ = err.message();
                pending_result_ = true;
                state_ = State::Error;
            } else {
                pending_result_ = true;
                state_ = State::Ready;
            }
        });

    return true;
}

void MysqlConnection::close() {
    if (!conn_.lowest_layer().is_open()) return;

    // Close the connection
    boost::system::error_code ec;
    conn_.close(ec);
    if (ec) {
        LOG_DEBUG("mysql", "Error closing connection: {}", ec.message());
    }

    state_ = State::Idle;
    ready_ = false;
    conn_.close();
}

// ─────────────────────────────────────────────────────────────────────────────
// Network event pumping
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::tick() {
    if (tick_depth_ > 0) return;
    TickDepthGuard guard(tick_depth_);

    // Handle connection timeout
    if (state_ == State::Connecting && timeout_ms_ > 0) {
        if (now_ms() - connect_start_ms_ >= static_cast<int64_t>(timeout_ms_)) {
            pending_connect_err_ = "connect timeout";
            state_ = State::Error;
            close();
            return;
        }
    }

    // Process any pending async operations by running io_context for a short time
    // We use poll_one to process exactly one handler (if available) to avoid blocking
    io_ctx_.poll_one();

    // Handle pending connection result
    if (pending_connect_) {
        pending_connect_ = false;
        if (!pending_connect_err_.empty()) {
            dispatch_connect(pending_connect_err_.c_str());
        } else {
            dispatch_connect(nullptr);
        }
        pending_connect_err_.clear();
    }

    // Handle pending query result
    if (pending_result_) {
        pending_result_ = false;
        if (!pending_result_err_.empty()) {
            dispatch_result({}, pending_result_err_.c_str());
        } else {
            dispatch_result(pending_result_data_, nullptr);
        }
        pending_result_err_.clear();
    }

    // Handle connection state transitions based on lowest layer state
    if (!conn_.lowest_layer().is_open()) {
        if (state_ != State::Idle && state_ != State::Error) {
            if (state_ == State::Connecting || state_ == State::Handshaking) {
                pending_connect_err_ = "connection closed during handshake";
                state_ = State::Error;
            } else if (state_ == State::Querying) {
                pending_result_err_ = "connection closed during query";
                state_ = State::Error;
            } else {
                state_ = State::Error;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Error handling
// ─────────────────────────────────────────────────────────────────────────────

MysqlError MysqlConnection::last_error() const { return last_error_; }

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

void MysqlConnection::set_connect_callback(const std::string &name) { connect_cb_ = name; }
void MysqlConnection::set_result_callback(const std::string &name) { result_cb_ = name; }
void MysqlConnection::set_state(::fakelua::State *state) { lua_state_ = state; }
void MysqlConnection::set_native_object(::fakelua::NativeObject *obj) { native_obj_ = obj; }

bool MysqlConnection::connected() const { return ready_; }
bool MysqlConnection::connecting() const {
    return state_ == State::Connecting || state_ == State::Handshaking;
}

int MysqlConnection::tick_depth() const { return tick_depth_; }
bool MysqlConnection::close_pending() const { return close_pending_; }
void MysqlConnection::request_close() { close_pending_ = true; }

// ─────────────────────────────────────────────────────────────────────────────
// Helper methods
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::dispatch_connect(const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    if (close_pending_) return;
    LOG_DEBUG("mysql", "dispatch_connect: err_msg={} cb={}",
              err_msg ? err_msg : "(null)", connect_cb_.c_str());

    if (!lua_state_ || connect_cb_.empty()) {
        LOG_DEBUG("mysql", "dispatch_connect: no state or no callback");
        return;
    }

    auto func = lua_state_->GetVM().GetFunction(connect_cb_);
    if (func.Empty()) {
        LOG_DEBUG("mysql", "dispatch_connect: function not found");
        return;
    }

    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) {
        LOG_DEBUG("mysql", "dispatch_connect: no JIT address");
        return;
    }

    // Ensure we always have a valid error message (never empty string with failure)
    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg && err_msg[0]) {
        args[1] = inter::NativeToFakeluaString(lua_state_, err_msg);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 0);
        LOG_DEBUG("mysql", "dispatch_connect: calling callback with msg={} success=0", err_msg);
    } else {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 1);
        LOG_DEBUG("mysql", "dispatch_connect: calling callback success=1");
    }

    inter::DispatchCall(addr, args, 3, jit_type);
}

void MysqlConnection::dispatch_result(const boost::mysql::results &result, const char *err_msg) {
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

    LOG_DEBUG("mysql", "dispatch_result: err_msg={} cb={} result.rows={}",
              err_msg ? err_msg : "(null)", result_cb_.c_str(),
              result.size() > 0 ? result.front().rows.size() : 0);

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[2] = nil;
    } else if (!result.empty() && result.front().stmt_id != 0) {
        // COM_STMT_PREPARE response: pass statement_id as number
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = inter::NativeToFakeluaInt(lua_state_, static_cast<int64_t>(result.front().stmt_id));
    } else {
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = result_to_lua(lua_state_, result);
    }

    inter::DispatchCall(addr, args, 3, jit_type);
}

void MysqlConnection::set_error(MysqlErrorType type, uint16_t code,
                                const std::string &msg, const std::string &sql_state) {
    last_error_.type = type;
    last_error_.code = code;
    last_error_.message = msg;
    last_error_.sql_state = sql_state;
}

}  // namespace fakelua::mysql