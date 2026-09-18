#include "native/mysql/mysql_connection.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/logging.h"
#include "var/var.h"

#include <event2/event.h>
#include <event2/util.h>

#include <chrono>
#include <cstring>
#include <string>
#include <vector>

namespace fakelua::mysql {

namespace {

int64_t NowMs() {
    auto tp = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

struct TickDepthGuard {
    int &depth;
    explicit TickDepthGuard(int &d) : depth(d) { ++depth; }
    ~TickDepthGuard() {
        if (depth > 0) --depth;
    }
};

int WaitReady(short what) {
    int ready = 0;
    if (what & EV_READ) ready |= MYSQL_WAIT_READ;
    if (what & EV_WRITE) ready |= MYSQL_WAIT_WRITE;
    if (what & EV_TIMEOUT) ready |= MYSQL_WAIT_TIMEOUT;
    return ready ? ready : (MYSQL_WAIT_READ | MYSQL_WAIT_WRITE);
}

}// namespace

MysqlConnection::MysqlConnection(::fakelua::State *state) : io_(state->GetIoContext()) {
    lua_state_ = state;
}

MysqlConnection::~MysqlConnection() {
    Close();
}

void MysqlConnection::ClearWait() {
    if (wait_ev_) {
        event_del(wait_ev_);
        event_free(wait_ev_);
        wait_ev_ = nullptr;
    }
}

void MysqlConnection::Teardown() {
    ClearWait();
    for (auto &kv: prepared_statements_) {
        if (kv.second) mysql_stmt_close(kv.second);
    }
    prepared_statements_.clear();
    if (mysql_) {
        mysql_close(mysql_);
        mysql_ = nullptr;
    }
    ready_ = false;
}

void MysqlConnection::ApplySsl() {
    if (!mysql_ || ssl_mode_ == SslMode::Disable) return;
    const char *ca = ssl_ca_.empty() ? nullptr : ssl_ca_.c_str();
    mysql_ssl_set(mysql_, nullptr, nullptr, ca, nullptr, nullptr);
#ifdef MYSQL_OPT_SSL_ENFORCE
    if (ssl_mode_ == SslMode::Require) {
        my_bool enforce = 1;
        mysql_options(mysql_, MYSQL_OPT_SSL_ENFORCE, &enforce);
    }
#endif
#ifdef MYSQL_OPT_SSL_VERIFY_SERVER_CERT
    if (!ssl_ca_.empty()) {
        my_bool verify = 1;
        mysql_options(mysql_, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify);
    }
#endif
}

void MysqlConnection::ArmWait(int status) {
    ClearWait();
    if (!mysql_ || status == 0) return;
    short ev = 0;
    if (status & MYSQL_WAIT_READ) ev |= EV_READ;
    if (status & MYSQL_WAIT_WRITE) ev |= EV_WRITE;
    if (status & MYSQL_WAIT_EXCEPT) ev |= EV_READ;
    timeval tv{};
    timeval *ptv = nullptr;
    if (status & MYSQL_WAIT_TIMEOUT) {
        tv.tv_sec = mysql_get_timeout_value(mysql_);
        tv.tv_usec = 0;
        if (tv.tv_sec == 0) {
            tv.tv_sec = 0;
            tv.tv_usec = 1000;
        }
        ptv = &tv;
    }
    my_socket fd = mysql_get_socket(mysql_);
    wait_ev_ = event_new(io_.Get(), fd, ev, [](evutil_socket_t, short what, void *ctx) {
        static_cast<MysqlConnection *>(ctx)->Continue(WaitReady(what));
    }, this);
    event_add(wait_ev_, ptv);
}

void MysqlConnection::Connect(const std::string &host, uint16_t port, const std::string &user, const std::string &password, const std::string &database, int timeout_ms, SslMode ssl, std::string ssl_ca) {
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
    Teardown();

    mysql_ = mysql_init(nullptr);
    if (!mysql_) {
        pending_connect_err_ = "connect failed: mysql_init failed";
        pending_connect_ = true;
        state_ = ConnState::Error;
        return;
    }
    mysql_options(mysql_, MYSQL_OPT_NONBLOCK, 0);
    mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    if (timeout_ms_ > 0) {
        unsigned int sec = static_cast<unsigned int>((timeout_ms_ + 999) / 1000);
        if (sec == 0) sec = 1;
        mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &sec);
    }
    ApplySsl();
    ready_ = false;
    state_ = ConnState::Connecting;
    pending_results_.clear();
    next_stmt_id_ = 1;
    last_error_ = {};

    MYSQL *ret = nullptr;
    unsigned long flags = CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS;
    int st = mysql_real_connect_start(&ret, mysql_, host_.c_str(), user_.c_str(), password_.c_str(), database_.c_str(), port_, nullptr, flags);
    wait_op_ = WaitOp::Connect;
    if (st) {
        ArmWait(st);
        return;
    }
    FinishConnect(ret);
}

void MysqlConnection::FinishConnect(MYSQL *ret) {
    wait_op_ = WaitOp::None;
    ClearWait();
    if (!ret) {
        std::string msg = mysql_error(mysql_);
        if (pending_connect_err_.empty()) pending_connect_err_ = "connect failed: " + (msg.empty() ? std::string("connect failed") : msg);
        state_ = ConnState::Error;
        pending_connect_ = true;
        return;
    }
    state_ = ConnState::Ready;
    ready_ = true;
    pending_connect_ = true;
}

void MysqlConnection::Query(const std::string &sql) {
    if (close_pending_ || !mysql_) return;
    if (state_ != ConnState::Ready || !ready_) {
        pending_result_err_ = "connection not ready";
        pending_result_ = true;
        return;
    }
    last_sql_ = sql;
    state_ = ConnState::Querying;
    query_type_ = QueryType::Query;
    pending_results_.clear();
    int err = 0;
    int st = mysql_real_query_start(&err, mysql_, sql.c_str(), sql.size());
    wait_op_ = WaitOp::Query;
    if (st) {
        ArmWait(st);
        return;
    }
    if (err) {
        pending_result_err_ = mysql_error(mysql_);
        pending_result_ = true;
        state_ = ConnState::Ready;
        wait_op_ = WaitOp::None;
        return;
    }
    StartStore();
}

void MysqlConnection::StartStore() {
    MYSQL_RES *res = nullptr;
    int st = mysql_store_result_start(&res, mysql_);
    wait_op_ = WaitOp::Store;
    if (st) {
        ArmWait(st);
        return;
    }
    if (mysql_errno(mysql_)) {
        pending_result_err_ = mysql_error(mysql_);
        pending_result_ = true;
        state_ = ConnState::Ready;
        wait_op_ = WaitOp::None;
        return;
    }
    pending_results_.push_back(ConsumeResult(res, res != nullptr));
    if (mysql_more_results(mysql_)) {
        int err = 0;
        int nst = mysql_next_result_start(&err, mysql_);
        wait_op_ = WaitOp::Next;
        if (nst) {
            ArmWait(nst);
            return;
        }
        if (err) {
            pending_result_err_ = mysql_error(mysql_);
            pending_result_ = true;
            state_ = ConnState::Ready;
            wait_op_ = WaitOp::None;
            return;
        }
        StartStore();
        return;
    }
    FinishQueryOk();
}

void MysqlConnection::FinishQueryOk() {
    wait_op_ = WaitOp::None;
    ClearWait();
    pending_result_err_.clear();
    pending_result_ = true;
    state_ = ConnState::Ready;
}

ResultsetData MysqlConnection::ConsumeResult(MYSQL_RES *res, bool is_resultset) {
    ResultsetData out;
    if (!is_resultset || !res) {
        out.is_resultset = false;
        out.affected_rows = mysql_affected_rows(mysql_);
        out.last_insert_id = mysql_insert_id(mysql_);
        const char *info = mysql_info(mysql_);
        if (info) out.info = info;
        return out;
    }
    out.is_resultset = true;
    unsigned int n = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    for (unsigned int i = 0; i < n; ++i) {
        out.columns.emplace_back(fields[i].name ? fields[i].name : "", static_cast<int>(fields[i].type));
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        unsigned long *lens = mysql_fetch_lengths(res);
        std::vector<FieldCell> cells;
        cells.reserve(n);
        for (unsigned int i = 0; i < n; ++i) {
            FieldCell c;
            if (!row[i]) {
                c.is_null = true;
            } else {
                c.value.assign(row[i], lens ? lens[i] : std::strlen(row[i]));
            }
            cells.push_back(std::move(c));
        }
        out.rows.push_back(std::move(cells));
    }
    mysql_free_result(res);
    return out;
}

void MysqlConnection::StmtPrepare(const std::string &sql) {
    if (close_pending_ || !mysql_) return;
    if (state_ != ConnState::Ready || !ready_) {
        pending_result_err_ = "connection not ready for prepare";
        pending_result_ = true;
        return;
    }
    state_ = ConnState::Querying;
    query_type_ = QueryType::StmtPrepare;
    MYSQL_STMT *stmt = mysql_stmt_init(mysql_);
    if (!stmt) {
        pending_result_err_ = mysql_error(mysql_);
        pending_result_ = true;
        state_ = ConnState::Ready;
        return;
    }
    pending_stmt_ = stmt;
    int err = 0;
    int st = mysql_stmt_prepare_start(&err, stmt, sql.c_str(), static_cast<unsigned long>(sql.size()));
    wait_op_ = WaitOp::StmtPrepare;
    if (st) {
        ArmWait(st);
        return;
    }
    if (err) {
        pending_result_err_ = mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        pending_stmt_ = nullptr;
        pending_result_ = true;
        state_ = ConnState::Ready;
        wait_op_ = WaitOp::None;
        return;
    }
    uint32_t id = next_stmt_id_++;
    prepared_statements_[id] = stmt;
    pending_stmt_ = nullptr;
    pending_stmt_id_ = id;
    has_pending_stmt_id_ = true;
    pending_result_err_.clear();
    pending_result_ = true;
    state_ = ConnState::Ready;
    wait_op_ = WaitOp::None;
}

void MysqlConnection::StmtExecute(uint32_t stmt_id, const std::vector<StmtParam> &params) {
    if (close_pending_ || !mysql_) return;
    if (state_ != ConnState::Ready || !ready_) {
        pending_result_err_ = "connection not ready for execute";
        pending_result_ = true;
        return;
    }
    auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end() || !it->second) {
        pending_result_err_ = "statement not prepared";
        pending_result_ = true;
        return;
    }
    state_ = ConnState::Querying;
    query_type_ = QueryType::StmtExecute;
    pending_results_.clear();
    MYSQL_STMT *stmt = it->second;
    pending_stmt_params_ = params;
    pending_binds_.assign(pending_stmt_params_.size(), MYSQL_BIND{});
    pending_bind_lens_.assign(pending_stmt_params_.size(), 0);
    for (size_t i = 0; i < pending_stmt_params_.size(); ++i) {
        if (pending_stmt_params_[i].is_null) {
            pending_binds_[i].buffer_type = MYSQL_TYPE_NULL;
        } else {
            pending_binds_[i].buffer_type = MYSQL_TYPE_STRING;
            pending_binds_[i].buffer = const_cast<char *>(pending_stmt_params_[i].value.data());
            pending_binds_[i].buffer_length = static_cast<unsigned long>(pending_stmt_params_[i].value.size());
            pending_bind_lens_[i] = static_cast<unsigned long>(pending_stmt_params_[i].value.size());
            pending_binds_[i].length = &pending_bind_lens_[i];
        }
    }
    if (!pending_stmt_params_.empty() && mysql_stmt_bind_param(stmt, pending_binds_.data()) != 0) {
        pending_result_err_ = mysql_stmt_error(stmt);
        pending_result_ = true;
        state_ = ConnState::Ready;
        return;
    }
    pending_exec_stmt_id_ = stmt_id;
    int err = 0;
    int st = mysql_stmt_execute_start(&err, stmt);
    wait_op_ = WaitOp::StmtExecute;
    if (st) {
        ArmWait(st);
        return;
    }
    if (err) {
        pending_result_err_ = mysql_stmt_error(stmt);
        pending_result_ = true;
        state_ = ConnState::Ready;
        wait_op_ = WaitOp::None;
        return;
    }
    pending_results_.push_back(ConsumeStmtResult(stmt));
    FinishQueryOk();
}

ResultsetData MysqlConnection::ConsumeStmtResult(MYSQL_STMT *stmt) {
    ResultsetData out;
    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    if (!meta) {
        out.is_resultset = false;
        out.affected_rows = mysql_stmt_affected_rows(stmt);
        out.last_insert_id = mysql_stmt_insert_id(stmt);
        return out;
    }
    out.is_resultset = true;
    unsigned int n = mysql_num_fields(meta);
    MYSQL_FIELD *fields = mysql_fetch_fields(meta);
    for (unsigned int i = 0; i < n; ++i) {
        out.columns.emplace_back(fields[i].name ? fields[i].name : "", static_cast<int>(fields[i].type));
    }
    mysql_stmt_store_result(stmt);
    std::vector<MYSQL_BIND> binds(n);
    std::vector<std::vector<char>> bufs(n);
    std::vector<unsigned long> lens(n);
    std::vector<my_bool> nulls(n);
    std::memset(binds.data(), 0, n * sizeof(MYSQL_BIND));
    for (unsigned int i = 0; i < n; ++i) {
        unsigned long sz = fields[i].length ? fields[i].length + 1 : 256;
        if (sz > 1024 * 1024) sz = 1024 * 1024;
        bufs[i].assign(sz, 0);
        binds[i].buffer_type = MYSQL_TYPE_STRING;
        binds[i].buffer = bufs[i].data();
        binds[i].buffer_length = static_cast<unsigned long>(bufs[i].size() - 1);
        binds[i].length = &lens[i];
        binds[i].is_null = &nulls[i];
    }
    mysql_stmt_bind_result(stmt, binds.data());
    int fetch_rc = 0;
    while ((fetch_rc = mysql_stmt_fetch(stmt)) == 0 || fetch_rc == MYSQL_DATA_TRUNCATED) {
        std::vector<FieldCell> cells;
        cells.reserve(n);
        for (unsigned int i = 0; i < n; ++i) {
            FieldCell c;
            if (nulls[i]) {
                c.is_null = true;
            } else {
                unsigned long ncopy = lens[i];
                if (ncopy > binds[i].buffer_length) ncopy = binds[i].buffer_length;
                c.value.assign(bufs[i].data(), ncopy);
            }
            cells.push_back(std::move(c));
        }
        out.rows.push_back(std::move(cells));
    }
    mysql_free_result(meta);
    mysql_stmt_free_result(stmt);
    return out;
}

void MysqlConnection::StmtClose(uint32_t stmt_id) {
    auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end()) return;
    if (it->second) mysql_stmt_close(it->second);
    prepared_statements_.erase(it);
}

bool MysqlConnection::Ping() {
    if (close_pending_ || !mysql_) return false;
    if (state_ != ConnState::Ready || !ready_) return false;
    query_type_ = QueryType::Ping;
    int err = 0;
    int st = mysql_ping_start(&err, mysql_);
    wait_op_ = WaitOp::Ping;
    if (st) {
        ArmWait(st);
        return true;
    }
    wait_op_ = WaitOp::None;
    if (err) {
        state_ = ConnState::Error;
        ready_ = false;
    }
    return true;
}

void MysqlConnection::Close() {
    Teardown();
    state_ = ConnState::Idle;
    pending_connect_ = false;
    pending_result_ = false;
    pending_results_.clear();
    next_stmt_id_ = 1;
}

void MysqlConnection::Continue(int ready) {
    if (!mysql_) return;
    switch (wait_op_) {
        case WaitOp::Connect: {
            MYSQL *ret = nullptr;
            int st = mysql_real_connect_cont(&ret, mysql_, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            FinishConnect(ret);
            break;
        }
        case WaitOp::Query: {
            int err = 0;
            int st = mysql_real_query_cont(&err, mysql_, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            if (err) {
                pending_result_err_ = mysql_error(mysql_);
                pending_result_ = true;
                state_ = ConnState::Ready;
                wait_op_ = WaitOp::None;
                ClearWait();
                return;
            }
            StartStore();
            break;
        }
        case WaitOp::Store: {
            MYSQL_RES *res = nullptr;
            int st = mysql_store_result_cont(&res, mysql_, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            if (mysql_errno(mysql_)) {
                pending_result_err_ = mysql_error(mysql_);
                pending_result_ = true;
                state_ = ConnState::Ready;
                wait_op_ = WaitOp::None;
                ClearWait();
                return;
            }
            pending_results_.push_back(ConsumeResult(res, res != nullptr));
            if (mysql_more_results(mysql_)) {
                int err = 0;
                int nst = mysql_next_result_start(&err, mysql_);
                wait_op_ = WaitOp::Next;
                if (nst) {
                    ArmWait(nst);
                    return;
                }
                if (err) {
                    pending_result_err_ = mysql_error(mysql_);
                    pending_result_ = true;
                    state_ = ConnState::Ready;
                    wait_op_ = WaitOp::None;
                    ClearWait();
                    return;
                }
                StartStore();
                return;
            }
            FinishQueryOk();
            break;
        }
        case WaitOp::Next: {
            int err = 0;
            int st = mysql_next_result_cont(&err, mysql_, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            if (err) {
                pending_result_err_ = mysql_error(mysql_);
                pending_result_ = true;
                state_ = ConnState::Ready;
                wait_op_ = WaitOp::None;
                ClearWait();
                return;
            }
            StartStore();
            break;
        }
        case WaitOp::Ping: {
            int err = 0;
            int st = mysql_ping_cont(&err, mysql_, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            wait_op_ = WaitOp::None;
            ClearWait();
            if (err) {
                state_ = ConnState::Error;
                ready_ = false;
            }
            break;
        }
        case WaitOp::StmtPrepare: {
            MYSQL_STMT *stmt = pending_stmt_;
            if (!stmt) return;
            int err = 0;
            int st = mysql_stmt_prepare_cont(&err, stmt, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            wait_op_ = WaitOp::None;
            ClearWait();
            if (err) {
                pending_result_err_ = mysql_stmt_error(stmt);
                mysql_stmt_close(stmt);
                pending_stmt_ = nullptr;
                pending_result_ = true;
                state_ = ConnState::Ready;
                return;
            }
            uint32_t id = next_stmt_id_++;
            prepared_statements_[id] = stmt;
            pending_stmt_ = nullptr;
            pending_stmt_id_ = id;
            has_pending_stmt_id_ = true;
            pending_result_err_.clear();
            pending_result_ = true;
            state_ = ConnState::Ready;
            break;
        }
        case WaitOp::StmtExecute: {
            auto it = prepared_statements_.find(pending_exec_stmt_id_);
            MYSQL_STMT *stmt = it != prepared_statements_.end() ? it->second : nullptr;
            if (!stmt) return;
            int err = 0;
            int st = mysql_stmt_execute_cont(&err, stmt, ready);
            if (st) {
                ArmWait(st);
                return;
            }
            wait_op_ = WaitOp::None;
            ClearWait();
            if (err) {
                pending_result_err_ = mysql_stmt_error(stmt);
                pending_result_ = true;
                state_ = ConnState::Ready;
                return;
            }
            pending_results_.push_back(ConsumeStmtResult(stmt));
            FinishQueryOk();
            break;
        }
        default:
            break;
    }
}

void MysqlConnection::Tick() {
    if (tick_depth_ > 0) return;
    TickDepthGuard guard(tick_depth_);

    if (state_ == ConnState::Connecting && timeout_ms_ > 0) {
        if (NowMs() - connect_start_ms_ >= static_cast<int64_t>(timeout_ms_)) {
            if (pending_connect_err_.empty()) pending_connect_err_ = "connect timeout";
            state_ = ConnState::Error;
            pending_connect_ = true;
            Teardown();
        }
    }

    io_.Poll();

    if (pending_connect_) {
        pending_connect_ = false;
        if (!pending_connect_err_.empty()) DispatchConnect(pending_connect_err_.c_str());
        else DispatchConnect(nullptr);
        pending_connect_err_.clear();
    }

    if (pending_result_) {
        pending_result_ = false;
        bool had_stmt_id = has_pending_stmt_id_;
        uint32_t stmt_id_for_dispatch = pending_stmt_id_;
        has_pending_stmt_id_ = false;
        pending_stmt_id_ = 0;
        if (!pending_result_err_.empty()) {
            DispatchResult(pending_result_err_.c_str());
        } else if (had_stmt_id) {
            dispatch_stmt_id_ = stmt_id_for_dispatch;
            DispatchResult(nullptr);
            dispatch_stmt_id_ = 0;
        } else {
            DispatchResult(nullptr);
        }
        pending_result_err_.clear();
        pending_results_.clear();
    }
}

MysqlError MysqlConnection::LastError() const {
    return last_error_;
}

bool MysqlConnection::IsRetryable(MysqlErrorType type) {
    switch (type) {
        case MysqlErrorType::Connection:
        case MysqlErrorType::Timeout:
            return true;
        default:
            return false;
    }
}

void MysqlConnection::SetConnectCallback(const std::string &name) { connect_cb_ = name; }
void MysqlConnection::SetResultCallback(const std::string &name) { result_cb_ = name; }
void MysqlConnection::SetState(::fakelua::State *state) { lua_state_ = state; }
void MysqlConnection::SetNativeObject(::fakelua::NativeObject *obj) { native_obj_ = obj; }
bool MysqlConnection::Connected() const { return ready_; }
bool MysqlConnection::Connecting() const { return state_ == ConnState::Connecting || state_ == ConnState::Handshaking; }
int MysqlConnection::TickDepth() const { return tick_depth_; }
bool MysqlConnection::ClosePending() const { return close_pending_; }
void MysqlConnection::RequestClose() { close_pending_ = true; }

void MysqlConnection::SetError(MysqlErrorType type, uint16_t code, const std::string &msg, const std::string &sql_state) {
    last_error_.type = type;
    last_error_.code = code;
    last_error_.message = msg;
    last_error_.sql_state = sql_state;
}

void MysqlConnection::DispatchConnect(const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    native::IoContext::DispatchScope dispatch_scope(io_);
    if (close_pending_) return;
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
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg && err_msg[0]) {
        args[1] = inter::NativeToFakeluaString(lua_state_, err_msg);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 0);
    } else {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 1);
    }
    inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
}

void MysqlConnection::DispatchResult(const char *err_msg) {
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

    const char *msg = err_msg && err_msg[0] ? err_msg : "query failed";
    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[2] = nil;
        inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
    } else if (dispatch_stmt_id_ != 0) {
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = inter::NativeToFakeluaInt(lua_state_, static_cast<int64_t>(dispatch_stmt_id_));
        inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
    } else if (pending_results_.empty()) {
        CVar nil{};
        nil.type_ = static_cast<int>(VarType::Nil);
        args[1] = nil;
        args[2] = table::TableHelper::CreateTable(lua_state_);
        inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
    } else {
        for (const auto &rs: pending_results_) {
            CVar nil{};
            nil.type_ = static_cast<int>(VarType::Nil);
            args[1] = nil;
            args[2] = ResultsetToLua(lua_state_, rs);
            inter::DispatchCall(lua_state_, addr, args, 3, jit_type);
            if (close_pending_) break;
        }
    }
}

CVar MysqlConnection::ResultsetToLua(::fakelua::State *s, const ResultsetData &result) {
    CVar tbl = table::TableHelper::CreateTable(s);
    if (result.is_resultset) {
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, true));
        CVar cols_tbl = table::TableHelper::CreateTable(s);
        int64_t col_idx = 1;
        for (const auto &col: result.columns) {
            CVar col_tbl = table::TableHelper::CreateTable(s);
            table::TableHelper::SetTableInt(s, col_tbl, 1, inter::NativeToFakeluaString(s, col.first));
            table::TableHelper::SetTableInt(s, col_tbl, 2, inter::NativeToFakeluaInt(s, col.second));
            table::TableHelper::SetTableInt(s, cols_tbl, col_idx++, col_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 2, cols_tbl);
        CVar rows_tbl = table::TableHelper::CreateTable(s);
        int64_t row_idx = 1;
        for (const auto &row: result.rows) {
            CVar row_tbl = table::TableHelper::CreateTable(s);
            int64_t col_pos = 1;
            for (const auto &fv: row) {
                if (fv.is_null) table::TableHelper::SetTableInt(s, row_tbl, col_pos, inter::NativeToFakeluaNil(s));
                else table::TableHelper::SetTableInt(s, row_tbl, col_pos, inter::NativeToFakeluaString(s, fv.value));
                ++col_pos;
            }
            table::TableHelper::SetTableInt(s, rows_tbl, row_idx++, row_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 3, rows_tbl);
    } else {
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, false));
        table::TableHelper::SetTableInt(s, tbl, 4, inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.affected_rows)));
        table::TableHelper::SetTableInt(s, tbl, 5, inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.last_insert_id)));
        table::TableHelper::SetTableInt(s, tbl, 6, inter::NativeToFakeluaString(s, result.info));
    }
    return tbl;
}

}// namespace fakelua::mysql
