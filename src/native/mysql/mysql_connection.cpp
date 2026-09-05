#include "native/mysql/mysql_connection.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/logging.h"
#include "var/var.h"

#include <chrono>
#include <cstring>
#include <stdexcept>
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
    : work_(boost::asio::make_work_guard(io_ctx_)),
      conn_(io_ctx_) {
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
    pending_connect_ = false;
    close_pending_ = false;
    state_ = State::Connecting;
    ready_ = false;

    // Reset connection state
    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;
    conn_.close(ec, diag);  // idempotent: closes any prior session
    pending_results_.clear();
    prepared_statements_.clear();
    next_stmt_id_ = 1;
    last_error_ = {};

    // Build connect_params (stored as member so it outlives the async callback lambda).
    pending_connect_params_.emplace();
    pending_connect_params_->server_address.emplace_host_and_port(host, port);
    pending_connect_params_->username = user;
    pending_connect_params_->password = password;
    pending_connect_params_->database = database;
    pending_connect_params_->ssl = boost::mysql::ssl_mode::disable;
    pending_connect_params_->multi_queries = true;  // preserve legacy multi-statement behavior

    // Start asynchronous connect
    conn_.async_connect(*pending_connect_params_, async_diag_, [this](boost::mysql::error_code ec) {
        pending_connect_params_.reset();
        if (ec) {
            pending_connect_err_ = ec.message();
            state_ = State::Error;
            pending_connect_ = true;
            return;
        }
        state_ = State::Ready;
        ready_ = true;
        pending_connect_ = true;
    });
}

void MysqlConnection::query(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !ready_) {
        dispatch_result({}, "connection not ready");
        return;
    }

    last_sql_ = sql;
    state_ = State::Querying;
    query_type_ = QueryType::Query;

    // Execute query asynchronously (with diagnostics for error reporting)
    conn_.async_execute(sql, pending_result_data_,
                        [this](boost::mysql::error_code err) {
                            if (err) {
                                pending_result_err_ = err.message();
                                pending_result_data_ = {};
                                pending_result_ = true;
                                state_ = State::Ready;
                                return;
                            }
                            pending_result_err_.clear();
                            pending_result_ = true;
                            state_ = State::Ready;
                        });
}

void MysqlConnection::stmt_prepare(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !ready_) {
        dispatch_result({}, "connection not ready for prepare");
        return;
    }

    state_ = State::Querying;
    query_type_ = QueryType::StmtPrepare;

    // Prepare statement asynchronously
    conn_.async_prepare_statement(sql, [this](boost::mysql::error_code err, boost::mysql::statement stmt) {
        if (err) {
            pending_result_err_ = err.message();
            pending_result_data_ = {};
            pending_result_ = true;
            state_ = State::Ready;
            return;
        }

        // Allocate a Lua-side statement ID and remember the boost statement.
        uint32_t stmt_id = next_stmt_id_++;
        prepared_statements_[stmt_id] = std::move(stmt);

        // Build a results-shaped reply that carries the statement id.
        pending_stmt_id_ = stmt_id;
        has_pending_stmt_id_ = true;
        pending_result_data_ = {};
        pending_result_err_.clear();
        pending_result_ = true;
        state_ = State::Ready;
    });
}

void MysqlConnection::stmt_execute(uint32_t stmt_id, const std::vector<StmtParam> &params) {
    if (close_pending_) return;
    if (state_ != State::Ready || !ready_) {
        dispatch_result({}, "connection not ready for execute");
        return;
    }

    // Find prepared statement
    auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end() || !it->second.valid()) {
        dispatch_result({}, "statement not prepared");
        return;
    }

    state_ = State::Querying;
    query_type_ = QueryType::StmtExecute;

    // Build a tuple of boost::mysql::field parameters. NULLs become nullptr_t
    // (boost::optional-like empty optional). Non-null values become strings.
    // Store as member so iterators passed to bound_statement remain valid
    // until the async callback fires (otherwise UAF).
    pending_stmt_fields_.clear();
    pending_stmt_fields_.reserve(params.size());
    for (const auto &p : params) {
        if (p.is_null) {
            pending_stmt_fields_.emplace_back(nullptr);
        } else {
            pending_stmt_fields_.emplace_back(p.value);
        }
    }

    conn_.async_execute(it->second.bind(pending_stmt_fields_.begin(), pending_stmt_fields_.end()),
                        pending_result_data_,
                        [this](boost::mysql::error_code err) {
                            pending_stmt_fields_.clear();
                            if (err) {
                                pending_result_err_ = err.message();
                                pending_result_data_ = {};
                                pending_result_ = true;
                                state_ = State::Ready;
                                return;
                            }
                            pending_result_err_.clear();
                            pending_result_ = true;
                            state_ = State::Ready;
                        });
}

void MysqlConnection::stmt_close(uint32_t stmt_id) {
    if (close_pending_) return;
    if (state_ != State::Ready || !ready_) {
        prepared_statements_.erase(stmt_id);
        return;
    }

    auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end()) return;
    if (!it->second.valid()) {
        prepared_statements_.erase(it);
        return;
    }

    // Close server-side statement. Note: this is synchronous; the legacy
    // implementation also did not network-roundtrip here.
    boost::mysql::statement stmt = std::move(it->second);
    prepared_statements_.erase(it);
    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;
    conn_.close_statement(stmt, ec, diag);
    if (ec) {
        LOG_DEBUG("mysql", "close_statement error: {}", ec.message());
    }
}

bool MysqlConnection::ping() {
    if (close_pending_) return false;
    if (state_ != State::Ready || !ready_) return false;

    conn_.async_ping([this](boost::mysql::error_code err) {
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
    if (state_ == State::Idle && !ready_ && prepared_statements_.empty()) {
        // already closed
        return;
    }

    // Close the connection (best effort).
    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;
    conn_.close(ec, diag);
    if (ec) {
        LOG_DEBUG("mysql", "Error closing connection: {}", ec.message());
    }

    state_ = State::Idle;
    ready_ = false;
    pending_results_.clear();
    prepared_statements_.clear();
    next_stmt_id_ = 1;
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
            pending_connect_ = true;
        }
    }

    // Process all ready async operations by running io_context.
    // poll() handles currently-ready handlers. If an async operation is in progress,
    // wait up to 1ms (matching net::TcpClient's 1ms wait_timeout_ms) so tight Lua loops
    // don't starve async I/O.
    io_ctx_.poll();
    if (!pending_connect_ && !pending_result_ && (state_ == State::Connecting || state_ == State::Querying)) {
        io_ctx_.run_for(std::chrono::milliseconds(1));
    }

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
        bool had_stmt_id = has_pending_stmt_id_;
        uint32_t stmt_id_for_dispatch = pending_stmt_id_;
        has_pending_stmt_id_ = false;
        pending_stmt_id_ = 0;
        if (!pending_result_err_.empty()) {
            dispatch_result({}, pending_result_err_.c_str());
        } else if (had_stmt_id) {
            // COM_STMT_PREPARE response: pass stmt_id through a synthetic result.
            dispatch_stmt_id_ = stmt_id_for_dispatch;
            dispatch_result(pending_result_data_, nullptr);
            dispatch_stmt_id_ = 0;
        } else {
            dispatch_result(pending_result_data_, nullptr);
        }
        pending_result_err_.clear();
        pending_result_data_ = {};
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

    LOG_DEBUG("mysql", "dispatch_result: err_msg={} cb={} stmt_id_dispatch={}",
              err_msg ? err_msg : "(null)", result_cb_.c_str(), dispatch_stmt_id_);

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);

    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[2] = nil;
    } else if (dispatch_stmt_id_ != 0) {
        // COM_STMT_PREPARE response: surface the statement id as a number.
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = inter::NativeToFakeluaInt(lua_state_,
                                            static_cast<int64_t>(dispatch_stmt_id_));
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

// ─────────────────────────────────────────────────────────────────────────────
// result_to_lua: convert Boost.MySQL results to the same Lua-table shape that
// the legacy COM_STMT_PREPARE-free result format produced:
//   {is_result_set, columns, rows, affected_rows, last_insert_id, info}
// columns: { {name, type}, ... }    (1-indexed)
// rows:    { {col1, col2, ...}, ... }  (NULL → nil, others → string)
// ─────────────────────────────────────────────────────────────────────────────

// static
std::pair<bool, std::string> MysqlConnection::field_to_string(const boost::mysql::field_view &fv) {
    using boost::mysql::field_kind;
    switch (fv.kind()) {
        case field_kind::null:
            return {true, ""};
        case field_kind::int64:
            return {false, std::to_string(fv.as_int64())};
        case field_kind::uint64:
            return {false, std::to_string(fv.as_uint64())};
        case field_kind::string:
            return {false, std::string(fv.as_string().data(), fv.as_string().size())};
        case field_kind::blob:
            return {false, std::string(reinterpret_cast<const char *>(fv.as_blob().data()),
                                       fv.as_blob().size())};
        case field_kind::float_:
            return {false, std::to_string(fv.as_float())};
        case field_kind::double_:
            return {false, std::to_string(fv.as_double())};
        case field_kind::date: {
            auto d = fv.as_date();
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u", d.year(), d.month(), d.day());
            return {false, std::string(buf)};
        }
        case field_kind::datetime: {
            auto dt = fv.as_datetime();
            char buf[64];
            if (dt.hour() || dt.minute() || dt.second() || dt.microsecond()) {
                std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u.%06u",
                              dt.year(), dt.month(), dt.day(),
                              dt.hour(), dt.minute(), dt.second(), dt.microsecond());
            } else {
                std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u",
                              dt.year(), dt.month(), dt.day());
            }
            return {false, std::string(buf)};
        }
        case field_kind::time: {
            // boost::mysql::time is std::chrono::microseconds.
            auto t = fv.as_time();
            auto total_us = t.count();
            bool negative = (total_us < 0);
            auto abs_us = negative ? -total_us : total_us;
            auto us = static_cast<long long>(abs_us % 1000000);
            auto total_s = abs_us / 1000000;
            auto hh = static_cast<long long>(total_s / 3600);
            auto mm = static_cast<long long>((total_s / 60) % 60);
            auto ss = static_cast<long long>(total_s % 60);
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%s%02lld:%02lld:%02lld.%06lld",
                          negative ? "-" : "", hh, mm, ss, us);
            return {false, std::string(buf)};
        }
    }
    return {false, ""};
}

CVar MysqlConnection::result_to_lua(::fakelua::State *s, const boost::mysql::results &result) {
    using namespace fakelua;

    CVar tbl = table::TableHelper::CreateTable(s);

    if (!result.has_value()) {
        // Empty / uninitialized: surface as status-only with zero affected.
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, false));
        table::TableHelper::SetTableInt(s, tbl, 4, inter::NativeToFakeluaLonglong(s, 0));
        table::TableHelper::SetTableInt(s, tbl, 5, inter::NativeToFakeluaLonglong(s, 0));
        return tbl;
    }

    // Inspect metadata to decide
    auto first_meta = result.meta();
    bool has_meta = first_meta.begin() != first_meta.end();

    if (has_meta) {
        // ── Result set: {true, columns, rows} ──
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, true));

        // columns at key 2
        CVar cols_tbl = table::TableHelper::CreateTable(s);
        int64_t col_idx = 1;
        for (const auto &col : first_meta) {
            CVar col_tbl = table::TableHelper::CreateTable(s);
            table::TableHelper::SetTableInt(s, col_tbl, 1,
                inter::NativeToFakeluaString(s, std::string(col.column_name())));
            // column_type() returns a column_type enum; expose its underlying value.
            table::TableHelper::SetTableInt(s, col_tbl, 2,
                inter::NativeToFakeluaInt(s, static_cast<int64_t>(col.type())));
            table::TableHelper::SetTableInt(s, cols_tbl, col_idx++, col_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 2, cols_tbl);

        // rows at key 3
        CVar rows_tbl = table::TableHelper::CreateTable(s);
        int64_t row_idx = 1;
        for (auto row_view : result.rows()) {
            CVar row_tbl = table::TableHelper::CreateTable(s);
            int64_t col_pos = 1;
            for (const auto &fv : row_view) {
                auto [is_null, value] = field_to_string(fv);
                if (is_null) {
                    table::TableHelper::SetTableInt(s, row_tbl, col_pos,
                                                    inter::NativeToFakeluaNil(s));
                } else {
                    table::TableHelper::SetTableInt(s, row_tbl, col_pos,
                                                    inter::NativeToFakeluaString(s, value));
                }
                ++col_pos;
            }
            table::TableHelper::SetTableInt(s, rows_tbl, row_idx++, row_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 3, rows_tbl);
    } else {
        // ── Status reply: {false, _, _, affected_rows, last_insert_id, info} ──
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, false));
        table::TableHelper::SetTableInt(s, tbl, 4,
            inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.affected_rows())));
        table::TableHelper::SetTableInt(s, tbl, 5,
            inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.last_insert_id())));
        table::TableHelper::SetTableInt(s, tbl, 6,
            inter::NativeToFakeluaString(s, std::string(result.info())));
    }

    return tbl;
}

}  // namespace fakelua::mysql