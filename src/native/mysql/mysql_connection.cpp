#include "native/mysql/mysql_connection.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/logging.h"
#include "var/var.h"

#include <chrono>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/ssl.hpp>

#include "native/tls_util.h"

namespace fakelua::mysql {

namespace {

int64_t NowMs() {
    auto tp = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

struct TickDepthGuard {
    int &depth;

    explicit TickDepthGuard(int &d) : depth(d) {
        ++depth;
    }

    ~TickDepthGuard() {
        if (depth > 0) --depth;
    }
};

}// namespace

MysqlConnection::MysqlConnection(::fakelua::State *state) : io_(state->GetIoContext()) {
    lua_state_ = state;
    EnsureConn();
}

MysqlConnection::~MysqlConnection() {
    Close();
}

void MysqlConnection::EnsureConn() {
    if (!conn_) {
        boost::mysql::any_connection_params params;
        if (ssl_ctx_) {
            params.ssl_context = ssl_ctx_.get();
        }
        conn_ = std::make_unique<boost::mysql::any_connection>(io_.Get(), params);
    }
}

void MysqlConnection::TeardownTransport() {
    if (cancel_signal_) {
        cancel_signal_->emit(boost::asio::cancellation_type::all);
    }

    // Let the cancelled operation complete WHILE conn_ is still alive. The State's
    // io_context outlives this connection, so an operation left in flight really is
    // resumed later, and Boost.MySQL would resume it on a destroyed connection. In
    // practice the cancellation completes in well under a millisecond.
    if (op_in_progress_) {
        const int wait_ms = native::kWindowsAsio ? 1000 : 5000;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(wait_ms);
        while (op_in_progress_ && std::chrono::steady_clock::now() < deadline) {
            if (io_.Poll() == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    // Transport-level close via destructor (no blocking COM_QUIT). Windows: if
    // cancel did not finish, leak — destroying any_connection on the poll()
    // thread deadlocks. POSIX: always destroy so we do not leak.
    if (op_in_progress_ && native::kWindowsAsio) {
        LOG_ERROR(lua_state_, "mysql", "cancellation did not complete, leaking the connection to stay safe");
        (void) conn_.release();
    } else {
        conn_.reset();
    }
    io_.Poll();
    cancel_signal_.reset();
    op_in_progress_ = false;
    ready_ = false;
}

// Public API

void MysqlConnection::Connect(const std::string &host, uint16_t port, const std::string &user, const std::string &password, const std::string &database, int timeout_ms, boost::mysql::ssl_mode ssl, std::string ssl_ca) {
    // Store connection parameters
    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    timeout_ms_ = timeout_ms;
    ssl_mode_ = ssl;
    ssl_ca_ = std::move(ssl_ca);
    connect_start_ms_ = NowMs();
    pending_connect_err_.clear();
    pending_connect_ = false;
    close_pending_ = false;

    TeardownTransport();
    ssl_ctx_.reset();
    if (ssl_mode_ != boost::mysql::ssl_mode::disable && !ssl_ca_.empty()) {
        ssl_ctx_ = std::make_unique<boost::asio::ssl::context>(tls::MakeClientContext(true, ssl_ca_));
    }
    EnsureConn();
    ready_ = false;
    state_ = State::Connecting;
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
    pending_connect_params_->ssl = ssl_mode_;
    pending_connect_params_->multi_queries = true;// preserve legacy multi-statement behavior

    // Start asynchronous connect
    cancel_signal_ = std::make_unique<boost::asio::cancellation_signal>();
    op_in_progress_ = true;
    conn_->async_connect(*pending_connect_params_, async_diag_, boost::asio::bind_cancellation_slot(cancel_signal_->slot(), [this, alive = life_.GetWatch()](boost::mysql::error_code ec) {
        if (!alive.Alive()) return;
        op_in_progress_ = false;
        pending_connect_params_.reset();
        if (ec) {
            // Prefix a stable English token: Boost.Asio's ec.message() is
            // localized on Windows (e.g. WSAECONNREFUSED -> 中文系统文案).
            if (pending_connect_err_.empty()) {
                pending_connect_err_ = "connect failed: " + ec.message();
            }
            state_ = State::Error;
            pending_connect_ = true;
            return;
        }
        state_ = State::Ready;
        ready_ = true;
        pending_connect_ = true;
    }));
}

void MysqlConnection::Query(const std::string &sql) {
    if (close_pending_ || !conn_) return;
    if (state_ != State::Ready || !ready_) {
        DispatchResult({}, "connection not ready");
        return;
    }

    last_sql_ = sql;
    state_ = State::Querying;
    query_type_ = QueryType::Query;

    // Execute query asynchronously (with diagnostics for error reporting)
    cancel_signal_ = std::make_unique<boost::asio::cancellation_signal>();
    op_in_progress_ = true;
    conn_->async_execute(sql, pending_result_data_, boost::asio::bind_cancellation_slot(cancel_signal_->slot(), [this, alive = life_.GetWatch()](boost::mysql::error_code err) {
                             if (!alive.Alive()) return;
                             op_in_progress_ = false;
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
                         }));
}

void MysqlConnection::StmtPrepare(const std::string &sql) {
    if (close_pending_ || !conn_) return;
    if (state_ != State::Ready || !ready_) {
        DispatchResult({}, "connection not ready for prepare");
        return;
    }

    state_ = State::Querying;
    query_type_ = QueryType::StmtPrepare;

    // Prepare statement asynchronously
    cancel_signal_ = std::make_unique<boost::asio::cancellation_signal>();
    op_in_progress_ = true;
    conn_->async_prepare_statement(sql, boost::asio::bind_cancellation_slot(cancel_signal_->slot(), [this, alive = life_.GetWatch()](boost::mysql::error_code err, boost::mysql::statement stmt) {
                                       if (!alive.Alive()) return;
                                       op_in_progress_ = false;
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
                                   }));
}

void MysqlConnection::StmtExecute(uint32_t stmt_id, const std::vector<StmtParam> &params) {
    if (close_pending_ || !conn_) return;
    if (state_ != State::Ready || !ready_) {
        DispatchResult({}, "connection not ready for execute");
        return;
    }

    // Find prepared statement
    auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end() || !it->second.valid()) {
        DispatchResult({}, "statement not prepared");
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
    for (const auto &p: params) {
        if (p.is_null) {
            pending_stmt_fields_.emplace_back(nullptr);
        } else {
            pending_stmt_fields_.emplace_back(p.value);
        }
    }

    cancel_signal_ = std::make_unique<boost::asio::cancellation_signal>();
    op_in_progress_ = true;
    conn_->async_execute(it->second.bind(pending_stmt_fields_.begin(), pending_stmt_fields_.end()), pending_result_data_,
                         boost::asio::bind_cancellation_slot(cancel_signal_->slot(), [this, alive = life_.GetWatch()](boost::mysql::error_code err) {
                             if (!alive.Alive()) return;
                             op_in_progress_ = false;
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
                         }));
}

void MysqlConnection::StmtClose(uint32_t stmt_id) {
    if (close_pending_ || !conn_) return;
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

    // Drop the local handle only. close_statement() is synchronous impl_.run()
    // on this State's io_context: Windows deadlocks the poll() thread, POSIX
    // would nest run() inside tick(). COM_STMT_CLOSE goes out with the connection.
    prepared_statements_.erase(it);
}

bool MysqlConnection::Ping() {
    if (close_pending_ || !conn_) return false;
    if (state_ != State::Ready || !ready_) return false;

    cancel_signal_ = std::make_unique<boost::asio::cancellation_signal>();
    op_in_progress_ = true;
    conn_->async_ping(boost::asio::bind_cancellation_slot(cancel_signal_->slot(), [this, alive = life_.GetWatch()](boost::mysql::error_code err) {
        if (!alive.Alive()) return;
        op_in_progress_ = false;
        if (err) {
            pending_result_err_ = err.message();
            pending_result_ = true;
            state_ = State::Error;
        } else {
            pending_result_ = true;
            state_ = State::Ready;
        }
    }));

    return true;
}

void MysqlConnection::Close() {
    if (!conn_ && state_ == State::Idle && !ready_ && prepared_statements_.empty() && !op_in_progress_) {
        return;
    }

    TeardownTransport();

    state_ = State::Idle;
    pending_connect_ = false;
    pending_result_ = false;
    pending_results_.clear();
    prepared_statements_.clear();
    next_stmt_id_ = 1;
}

// Network event pumping

void MysqlConnection::Tick() {
    if (tick_depth_ > 0) return;
    TickDepthGuard guard(tick_depth_);

    // Handle connection timeout. Cancel only — tearing down here would block the
    // tick waiting for the cancelled ConnectEx to come back. The completion is
    // picked up by the poll below, or by a later tick.
    if (state_ == State::Connecting && timeout_ms_ > 0) {
        if (NowMs() - connect_start_ms_ >= static_cast<int64_t>(timeout_ms_)) {
            if (pending_connect_err_.empty()) {
                pending_connect_err_ = "connect timeout";
            }
            state_ = State::Error;
            if (cancel_signal_) {
                cancel_signal_->emit(boost::asio::cancellation_type::all);
            }
            pending_connect_ = true;
        }
    }

    // Runs the handlers that are already ready, including those of other objects on
    // this State's context. Each one only records state; callbacks reach Lua from
    // the owner's own tick.
    io_.Poll();

    // Handle pending connection result
    if (pending_connect_) {
        pending_connect_ = false;
        if (!pending_connect_err_.empty()) {
            DispatchConnect(pending_connect_err_.c_str());
        } else {
            DispatchConnect(nullptr);
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
            DispatchResult({}, pending_result_err_.c_str());
        } else if (had_stmt_id) {
            // COM_STMT_PREPARE response: pass stmt_id through a synthetic result.
            dispatch_stmt_id_ = stmt_id_for_dispatch;
            DispatchResult(pending_result_data_, nullptr);
            dispatch_stmt_id_ = 0;
        } else {
            DispatchResult(pending_result_data_, nullptr);
        }
        pending_result_err_.clear();
        pending_result_data_ = {};
    }
}

// Error handling

MysqlError MysqlConnection::LastError() const {
    return last_error_;
}

bool MysqlConnection::IsRetryable(MysqlErrorType type) {
    switch (type) {
        case MysqlErrorType::Connection:
        case MysqlErrorType::Timeout:
            return true;// network issues, can retry
        case MysqlErrorType::Authentication:
        case MysqlErrorType::Syntax:
        case MysqlErrorType::Protocol:
            return false;// logic errors, retry won't help
        case MysqlErrorType::Server:
        case MysqlErrorType::Unknown:
        default:
            return false;// server errors, don't retry by default
    }
}

void MysqlConnection::SetConnectCallback(const std::string &name) {
    connect_cb_ = name;
}

void MysqlConnection::SetResultCallback(const std::string &name) {
    result_cb_ = name;
}

void MysqlConnection::SetState(::fakelua::State *state) {
    lua_state_ = state;
}

void MysqlConnection::SetNativeObject(::fakelua::NativeObject *obj) {
    native_obj_ = obj;
}

bool MysqlConnection::Connected() const {
    return ready_;
}

bool MysqlConnection::Connecting() const {
    return state_ == State::Connecting || state_ == State::Handshaking;
}

int MysqlConnection::TickDepth() const {
    return tick_depth_;
}

bool MysqlConnection::ClosePending() const {
    return close_pending_;
}

void MysqlConnection::RequestClose() {
    close_pending_ = true;
}

// Helper methods

void MysqlConnection::DispatchConnect(const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    // Nothing on this State's context may run handlers while Lua is on the stack:
    // a nested poll would overwrite the state this dispatch is reading.
    native::IoContext::DispatchScope dispatch_scope(io_);
    if (close_pending_) return;
    LOG_DEBUG(lua_state_, "mysql", "dispatch_connect: err_msg={} cb={}", err_msg ? err_msg : "(null)", connect_cb_.c_str());

    if (!lua_state_ || connect_cb_.empty()) {
        LOG_DEBUG(lua_state_, "mysql", "dispatch_connect: no state or no callback");
        return;
    }

    auto func = lua_state_->GetVM().GetFunction(connect_cb_);
    if (func.Empty()) {
        LOG_DEBUG(lua_state_, "mysql", "dispatch_connect: function not found");
        return;
    }

    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) {
        LOG_DEBUG(lua_state_, "mysql", "dispatch_connect: no JIT address");
        return;
    }

    // Ensure we always have a valid error message (never empty string with failure)
    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg && err_msg[0]) {
        args[1] = inter::NativeToFakeluaString(lua_state_, err_msg);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 0);
        LOG_DEBUG(lua_state_, "mysql", "dispatch_connect: calling callback with msg={} success=0", err_msg);
    } else {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 1);
        LOG_DEBUG(lua_state_, "mysql", "dispatch_connect: calling callback success=1");
    }

    inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
}

void MysqlConnection::DispatchResult(const boost::mysql::results &result, const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    native::IoContext::DispatchScope dispatch_scope(io_);
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

    LOG_DEBUG(lua_state_, "mysql", "dispatch_result: err_msg={} cb={} stmt_id_dispatch={}", err_msg ? err_msg : "(null)", result_cb_.c_str(), dispatch_stmt_id_);

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);

    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[2] = nil;
        inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
    } else if (dispatch_stmt_id_ != 0) {
        // COM_STMT_PREPARE response: surface the statement id as a number.
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = inter::NativeToFakeluaInt(lua_state_, static_cast<int64_t>(dispatch_stmt_id_));
        inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
    } else {
        if (result.empty()) {
            CVar nil{};
            nil.type_ = static_cast<int>(VarType::Nil);
            args[1] = nil;
            args[2] = table::TableHelper::CreateTable(lua_state_);
            inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
        } else {
            for (size_t i = 0; i < result.size(); ++i) {
                CVar nil{};
                nil.type_ = static_cast<int>(VarType::Nil);
                args[1] = nil;
                args[2] = ResultsetToLua(lua_state_, result[i]);
                inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
                if (close_pending_) break;
            }
        }
    }
}

void MysqlConnection::SetError(MysqlErrorType type, uint16_t code, const std::string &msg, const std::string &sql_state) {
    last_error_.type = type;
    last_error_.code = code;
    last_error_.message = msg;
    last_error_.sql_state = sql_state;
}

// result_to_lua: convert Boost.MySQL results to the same Lua-table shape that
// the legacy COM_STMT_PREPARE-free result format produced:
//   {is_result_set, columns, rows, affected_rows, last_insert_id, info}
// columns: { {name, type}, ... }    (1-indexed)
// rows:    { {col1, col2, ...}, ... }  (NULL → nil, others → string)

// static
std::pair<bool, std::string> MysqlConnection::FieldToString(const boost::mysql::field_view &fv) {
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
            return {false, std::string(reinterpret_cast<const char *>(fv.as_blob().data()), fv.as_blob().size())};
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
                std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u.%06u", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), dt.microsecond());
            } else {
                std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u", dt.year(), dt.month(), dt.day());
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
            std::snprintf(buf, sizeof(buf), "%s%02lld:%02lld:%02lld.%06lld", negative ? "-" : "", hh, mm, ss, us);
            return {false, std::string(buf)};
        }
    }
    return {false, ""};
}

CVar MysqlConnection::ResultToLua(::fakelua::State *s, const boost::mysql::results &result) {
    if (result.empty()) {
        return table::TableHelper::CreateTable(s);
    }
    return ResultsetToLua(s, result[0]);
}

CVar MysqlConnection::ResultsetToLua(::fakelua::State *s, const boost::mysql::resultset_view &result) {
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
        // Result set: {true, columns, rows}
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, true));

        // columns at key 2
        CVar cols_tbl = table::TableHelper::CreateTable(s);
        int64_t col_idx = 1;
        for (const auto &col: first_meta) {
            CVar col_tbl = table::TableHelper::CreateTable(s);
            table::TableHelper::SetTableInt(s, col_tbl, 1, inter::NativeToFakeluaString(s, std::string(col.column_name())));
            // column_type() returns a column_type enum; expose its underlying value.
            table::TableHelper::SetTableInt(s, col_tbl, 2, inter::NativeToFakeluaInt(s, static_cast<int64_t>(col.type())));
            table::TableHelper::SetTableInt(s, cols_tbl, col_idx++, col_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 2, cols_tbl);

        // rows at key 3
        CVar rows_tbl = table::TableHelper::CreateTable(s);
        int64_t row_idx = 1;
        for (auto row_view: result.rows()) {
            CVar row_tbl = table::TableHelper::CreateTable(s);
            int64_t col_pos = 1;
            for (const auto &fv: row_view) {
                auto [is_null, value] = FieldToString(fv);
                if (is_null) {
                    table::TableHelper::SetTableInt(s, row_tbl, col_pos, inter::NativeToFakeluaNil(s));
                } else {
                    table::TableHelper::SetTableInt(s, row_tbl, col_pos, inter::NativeToFakeluaString(s, value));
                }
                ++col_pos;
            }
            table::TableHelper::SetTableInt(s, rows_tbl, row_idx++, row_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 3, rows_tbl);
    } else {
        // Status reply: {false, _, _, affected_rows, last_insert_id, info}
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, false));
        table::TableHelper::SetTableInt(s, tbl, 4, inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.affected_rows())));
        table::TableHelper::SetTableInt(s, tbl, 5, inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.last_insert_id())));
        table::TableHelper::SetTableInt(s, tbl, 6, inter::NativeToFakeluaString(s, std::string(result.info())));
    }

    return tbl;
}

}// namespace fakelua::mysql