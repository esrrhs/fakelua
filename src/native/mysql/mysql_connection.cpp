#include "native/mysql/mysql_connection.h"

#include "native/native_common.h"
#include "util/logging.h"

#include <mysql.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace fakelua::mysql {

namespace {

int64_t now_ms() {
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

MYSQL *mysql_handle(void *ptr) {
    return reinterpret_cast<MYSQL *>(ptr);
}

const char *safe_cstr(const std::string &s) {
    return s.empty() ? nullptr : s.c_str();
}

std::string copy_mysql_string(const char *s) {
    return s ? std::string(s) : std::string();
}

unsigned int timeout_seconds_from_ms(int timeout_ms) {
    if (timeout_ms <= 0) return 0;
    return static_cast<unsigned int>((timeout_ms + 999) / 1000);
}

MysqlErrorType classify_error_code(uint16_t code) {
    switch (code) {
    case 1045:
    case 1698:
        return MysqlErrorType::Authentication;
    case 1064:
    case 1149:
        return MysqlErrorType::Syntax;
    case 1205:
    case 1213:
        return MysqlErrorType::Timeout;
    case 2002:
    case 2003:
    case 2006:
    case 2013:
        return MysqlErrorType::Connection;
    default:
        if (code >= 1000 && code < 2000) return MysqlErrorType::Server;
        if (code >= 2000 && code < 3000) return MysqlErrorType::Connection;
        return MysqlErrorType::Unknown;
    }
}

MysqlError make_error_from_handle(MYSQL *mysql, const std::string &fallback) {
    const auto code = static_cast<uint16_t>(mysql ? mysql_errno(mysql) : 0);
    std::string msg = mysql ? copy_mysql_string(mysql_error(mysql)) : fallback;
    if (msg.empty()) msg = fallback.empty() ? "mysql operation failed" : fallback;
    return MysqlError{
        .type = classify_error_code(code),
        .code = static_cast<int>(code),
        .message = std::move(msg),
        .sql_state = mysql ? copy_mysql_string(mysql_sqlstate(mysql)) : ""
    };
}

ColumnDef field_to_column(const MYSQL_FIELD &field) {
    ColumnDef col;
    col.catalog = copy_mysql_string(field.catalog);
    col.schema = copy_mysql_string(field.db);
    col.table = copy_mysql_string(field.table);
    col.org_table = copy_mysql_string(field.org_table);
    col.name = copy_mysql_string(field.name);
    col.org_name = copy_mysql_string(field.org_name);
    col.charset = field.charsetnr;
    col.length = static_cast<uint32_t>(field.length);
    col.type = static_cast<ColType>(field.type);
    col.flags = static_cast<uint16_t>(field.flags);
    col.decimals = field.decimals;
    return col;
}

MysqlResult result_from_mysql_res(MYSQL_RES *res) {
    MysqlResult result;
    result.is_result_set = true;
    if (!res) return result;

    const auto field_count = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    result.columns.reserve(field_count);
    for (unsigned int i = 0; i < field_count; ++i) {
        result.columns.push_back(field_to_column(fields[i]));
    }

    MYSQL_ROW row = nullptr;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        unsigned long *lengths = mysql_fetch_lengths(res);
        std::vector<std::pair<bool, std::string>> out_row;
        out_row.reserve(field_count);
        for (unsigned int i = 0; i < field_count; ++i) {
            if (!row[i]) {
                out_row.emplace_back(true, "");
                continue;
            }
            const auto len = lengths ? static_cast<size_t>(lengths[i]) : std::strlen(row[i]);
            out_row.emplace_back(false, std::string(row[i], len));
        }
        result.rows.push_back(std::move(out_row));
    }
    return result;
}

MysqlResult status_result_from_mysql(MYSQL *mysql) {
    MysqlResult result;
    result.is_result_set = false;
    if (!mysql) return result;
    result.affected_rows = mysql_affected_rows(mysql);
    result.last_insert_id = mysql_insert_id(mysql);
    result.info = copy_mysql_string(mysql_info(mysql));
    return result;
}

bool is_placeholder_replacement_state(char quote) {
    return quote == '\0';
}

size_t count_placeholders(std::string_view sql) {
    size_t count = 0;
    char quote = '\0';
    bool line_comment = false;
    bool block_comment = false;

    for (size_t i = 0; i < sql.size(); ++i) {
        const char c = sql[i];
        const char next = (i + 1 < sql.size()) ? sql[i + 1] : '\0';

        if (line_comment) {
            if (c == '\n') line_comment = false;
            continue;
        }
        if (block_comment) {
            if (c == '*' && next == '/') {
                block_comment = false;
                ++i;
            }
            continue;
        }
        if (!is_placeholder_replacement_state(quote)) {
            if (c == '\\' && quote != '`' && next != '\0') {
                ++i;
                continue;
            }
            if (c == quote) quote = '\0';
            continue;
        }

        if (c == '\'' || c == '"' || c == '`') {
            quote = c;
            continue;
        }
        if (c == '#' || (c == '-' && next == '-')) {
            line_comment = true;
            if (c == '-') ++i;
            continue;
        }
        if (c == '/' && next == '*') {
            block_comment = true;
            ++i;
            continue;
        }
        if (c == '?') ++count;
    }
    return count;
}

std::string escape_param_value(MYSQL *mysql, const std::string &value) {
    std::string escaped(value.size() * 2 + 3, '\0');
    auto *dst = escaped.data() + 1;
    unsigned long len = mysql_real_escape_string_quote(mysql, dst, value.data(),
                                                       static_cast<unsigned long>(value.size()), '\'');
    escaped[0] = '\'';
    escaped[1 + len] = '\'';
    escaped.resize(static_cast<size_t>(len) + 2);
    return escaped;
}

std::string expand_stmt_sql(MYSQL *mysql, std::string_view sql, const std::vector<StmtParam> &params) {
    std::string out;
    out.reserve(sql.size() + params.size() * 8);

    char quote = '\0';
    bool line_comment = false;
    bool block_comment = false;
    size_t param_index = 0;

    for (size_t i = 0; i < sql.size(); ++i) {
        const char c = sql[i];
        const char next = (i + 1 < sql.size()) ? sql[i + 1] : '\0';

        if (line_comment) {
            out.push_back(c);
            if (c == '\n') line_comment = false;
            continue;
        }
        if (block_comment) {
            out.push_back(c);
            if (c == '*' && next == '/') {
                out.push_back(next);
                block_comment = false;
                ++i;
            }
            continue;
        }
        if (!is_placeholder_replacement_state(quote)) {
            out.push_back(c);
            if (c == '\\' && quote != '`' && next != '\0') {
                out.push_back(next);
                ++i;
                continue;
            }
            if (c == quote) quote = '\0';
            continue;
        }

        if (c == '\'' || c == '"' || c == '`') {
            quote = c;
            out.push_back(c);
            continue;
        }
        if (c == '#' || (c == '-' && next == '-')) {
            out.push_back(c);
            if (c == '-') {
                out.push_back(next);
                ++i;
            }
            line_comment = true;
            continue;
        }
        if (c == '/' && next == '*') {
            out.push_back(c);
            out.push_back(next);
            block_comment = true;
            ++i;
            continue;
        }
        if (c == '?') {
            if (param_index >= params.size()) {
                throw std::runtime_error("not enough statement parameters");
            }
            if (params[param_index].is_null) {
                out.append("NULL");
            } else {
                out.append(escape_param_value(mysql, params[param_index].value));
            }
            ++param_index;
            continue;
        }
        out.push_back(c);
    }

    if (param_index != params.size()) {
        throw std::runtime_error("too many statement parameters");
    }
    return out;
}

}  // namespace

MysqlConnection::MysqlConnection() = default;

MysqlConnection::~MysqlConnection() {
    close();
}

void MysqlConnection::connect(const std::string &host, uint16_t port,
                              const std::string &user, const std::string &password,
                              const std::string &database, int timeout_ms) {
    close();

    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    connect_timeout_ms_ = timeout_ms;
    connect_start_ms_ = now_ms();
    pending_connect_err_.clear();
    close_pending_ = false;
    pending_sql_.clear();
    pending_stmt_id_ = 0;
    prepared_statements_.clear();
    next_stmt_id_ = 1;
    last_error_ = {};

    MYSQL *mysql = mysql_init(nullptr);
    if (!mysql) {
        pending_connect_err_ = "mysql_init failed";
        state_ = State::Error;
        ready_ = false;
        return;
    }

    unsigned int timeout_s = timeout_seconds_from_ms(timeout_ms);
    if (timeout_s > 0) {
        mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_s);
        mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &timeout_s);
        mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, &timeout_s);
    }
    bool reconnect = false;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);

    mysql_ = mysql;
    state_ = State::Connecting;
    operation_ = OperationType::None;
    phase_ = AsyncPhase::None;
    ready_ = false;
}

void MysqlConnection::query(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !mysql_handle(mysql_)) {
        dispatch_result({}, "connection not ready");
        return;
    }

    pending_sql_ = sql;
    pending_stmt_id_ = 0;
    pending_result_ = {};
    last_error_ = {};
    state_ = State::Busy;
    operation_ = OperationType::Query;
    phase_ = AsyncPhase::Start;
}

void MysqlConnection::stmt_prepare(const std::string &sql) {
    if (close_pending_) return;
    if (state_ != State::Ready || !mysql_handle(mysql_)) {
        dispatch_result({}, "connection not ready for prepare");
        return;
    }

    pending_stmt_id_ = next_stmt_id_++;
    prepared_statements_[pending_stmt_id_] = PreparedStatement{sql, count_placeholders(sql)};
    pending_result_ = {};
    pending_result_.stmt_id = pending_stmt_id_;
    last_error_ = {};
    state_ = State::Busy;
    operation_ = OperationType::Prepare;
    phase_ = AsyncPhase::DispatchPrepare;
}

void MysqlConnection::stmt_execute(uint32_t stmt_id, const std::vector<StmtParam> &params) {
    if (close_pending_) return;
    if (state_ != State::Ready || !mysql_handle(mysql_)) {
        dispatch_result({}, "connection not ready for execute");
        return;
    }

    const auto it = prepared_statements_.find(stmt_id);
    if (it == prepared_statements_.end()) {
        dispatch_result({}, "unknown statement id");
        return;
    }
    if (it->second.param_count != params.size()) {
        dispatch_result({}, "statement parameter count mismatch");
        return;
    }

    try {
        pending_sql_ = expand_stmt_sql(mysql_handle(mysql_), it->second.sql, params);
    } catch (const std::exception &e) {
        dispatch_result({}, e.what());
        return;
    }

    pending_stmt_id_ = 0;
    pending_result_ = {};
    last_error_ = {};
    state_ = State::Busy;
    operation_ = OperationType::Query;
    phase_ = AsyncPhase::Start;
}

void MysqlConnection::stmt_close(uint32_t stmt_id) {
    prepared_statements_.erase(stmt_id);
}

bool MysqlConnection::ping() {
    if (close_pending_) return false;
    if (state_ != State::Ready || !mysql_handle(mysql_)) return false;

    pending_sql_ = "DO 1";
    pending_stmt_id_ = 0;
    pending_result_ = {};
    last_error_ = {};
    state_ = State::Busy;
    operation_ = OperationType::Heartbeat;
    phase_ = AsyncPhase::Start;
    return true;
}

void MysqlConnection::close() {
    auto *mysql = mysql_handle(mysql_);
    if (mysql) {
        mysql_close(mysql);
    }
    mysql_ = nullptr;
    state_ = State::Idle;
    operation_ = OperationType::None;
    phase_ = AsyncPhase::None;
    ready_ = false;
    pending_sql_.clear();
    pending_stmt_id_ = 0;
    prepared_statements_.clear();
}

bool MysqlConnection::is_retryable(MysqlErrorType type) {
    switch (type) {
    case MysqlErrorType::Connection:
    case MysqlErrorType::Timeout:
        return true;
    case MysqlErrorType::Authentication:
    case MysqlErrorType::Syntax:
    case MysqlErrorType::Protocol:
    case MysqlErrorType::Server:
    case MysqlErrorType::Unknown:
    case MysqlErrorType::None:
    default:
        return false;
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

    if (state_ == State::Connecting) {
        tick_connect();
        return;
    }
    if (state_ == State::Busy) {
        tick_operation();
    }
}

void MysqlConnection::tick_connect() {
    auto *mysql = mysql_handle(mysql_);
    if (!mysql) {
        pending_connect_err_ = "mysql handle is closed";
        state_ = State::Error;
        return;
    }

    if (connect_timeout_ms_ > 0 && now_ms() - connect_start_ms_ >= connect_timeout_ms_) {
        set_error(MysqlErrorType::Timeout, 0, "connect timeout", "");
        dispatch_connect(last_error_.message.c_str());
        close();
        state_ = State::Error;
        return;
    }

    const auto status = mysql_real_connect_nonblocking(
        mysql, host_.c_str(), user_.c_str(), safe_cstr(password_), safe_cstr(database_),
        port_, nullptr, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS);

    if (status == NET_ASYNC_NOT_READY) return;
    if (status == NET_ASYNC_COMPLETE) {
        ready_ = true;
        state_ = State::Ready;
        dispatch_connect(nullptr);
        return;
    }

    auto err = make_error_from_handle(mysql, "connect failed");
    set_error(err.type, static_cast<uint16_t>(err.code), err.message, err.sql_state);
    dispatch_connect(last_error_.message.c_str());
    close();
    state_ = State::Error;
}

void MysqlConnection::tick_operation() {
    auto *mysql = mysql_handle(mysql_);
    if (!mysql) {
        if (operation_ != OperationType::Heartbeat) {
            dispatch_result({}, "connection is closed");
        }
        state_ = State::Error;
        operation_ = OperationType::None;
        phase_ = AsyncPhase::None;
        return;
    }

    if (phase_ == AsyncPhase::DispatchPrepare) {
        state_ = State::Ready;
        operation_ = OperationType::None;
        phase_ = AsyncPhase::None;
        dispatch_result(pending_result_, nullptr);
        return;
    }

    if (phase_ == AsyncPhase::Start) {
        const auto status = mysql_real_query_nonblocking(
            mysql, pending_sql_.c_str(), static_cast<unsigned long>(pending_sql_.size()));
        if (status == NET_ASYNC_NOT_READY) return;
        if (status == NET_ASYNC_COMPLETE) {
            phase_ = AsyncPhase::StoreResult;
            return;
        }

        auto err = make_error_from_handle(mysql, "query failed");
        set_error(err.type, static_cast<uint16_t>(err.code), err.message, err.sql_state);
        const bool connection_error = last_error_.type == MysqlErrorType::Connection ||
                                      last_error_.type == MysqlErrorType::Timeout ||
                                      last_error_.type == MysqlErrorType::Protocol;
        if (operation_ != OperationType::Heartbeat) {
            dispatch_result({}, last_error_.message.c_str());
        }
        ready_ = !connection_error;
        state_ = connection_error ? State::Error : State::Ready;
        operation_ = OperationType::None;
        phase_ = AsyncPhase::None;
        if (connection_error) {
            close();
            state_ = State::Error;
        }
        return;
    }

    if (phase_ == AsyncPhase::StoreResult) {
        MYSQL_RES *res = nullptr;
        const auto status = mysql_store_result_nonblocking(mysql, &res);
        if (status == NET_ASYNC_NOT_READY) return;
        if (status == NET_ASYNC_ERROR) {
            auto err = make_error_from_handle(mysql, "store result failed");
            set_error(err.type, static_cast<uint16_t>(err.code), err.message, err.sql_state);
            const bool connection_error = last_error_.type == MysqlErrorType::Connection ||
                                          last_error_.type == MysqlErrorType::Timeout ||
                                          last_error_.type == MysqlErrorType::Protocol;
            if (operation_ != OperationType::Heartbeat) {
                dispatch_result({}, last_error_.message.c_str());
            }
            ready_ = !connection_error;
            state_ = connection_error ? State::Error : State::Ready;
            operation_ = OperationType::None;
            phase_ = AsyncPhase::None;
            if (res) mysql_free_result(res);
            if (connection_error) {
                close();
                state_ = State::Error;
            }
            return;
        }

        pending_result_ = res ? result_from_mysql_res(res) : status_result_from_mysql(mysql);
        if (res) mysql_free_result(res);

        if (operation_ != OperationType::Heartbeat) {
            dispatch_result(pending_result_, nullptr);
        }

        if (mysql_more_results(mysql)) {
            phase_ = AsyncPhase::NextResult;
            return;
        }

        ready_ = true;
        state_ = State::Ready;
        operation_ = OperationType::None;
        phase_ = AsyncPhase::None;
        return;
    }

    if (phase_ == AsyncPhase::NextResult) {
        const auto status = mysql_next_result_nonblocking(mysql);
        if (status == NET_ASYNC_NOT_READY) return;
        if (status == NET_ASYNC_COMPLETE) {
            phase_ = AsyncPhase::StoreResult;
            return;
        }
        if (status == NET_ASYNC_COMPLETE_NO_MORE_RESULTS) {
            ready_ = true;
            state_ = State::Ready;
            operation_ = OperationType::None;
            phase_ = AsyncPhase::None;
            return;
        }

        auto err = make_error_from_handle(mysql, "next result failed");
        set_error(err.type, static_cast<uint16_t>(err.code), err.message, err.sql_state);
        const bool connection_error = last_error_.type == MysqlErrorType::Connection ||
                                      last_error_.type == MysqlErrorType::Timeout ||
                                      last_error_.type == MysqlErrorType::Protocol;
        if (operation_ != OperationType::Heartbeat) {
            dispatch_result({}, last_error_.message.c_str());
        }
        ready_ = !connection_error;
        state_ = connection_error ? State::Error : State::Ready;
        operation_ = OperationType::None;
        phase_ = AsyncPhase::None;
        if (connection_error) {
            close();
            state_ = State::Error;
        }
    }
}

void MysqlConnection::dispatch_connect(const char *err_msg) {
    TickDepthGuard guard(tick_depth_);
    if (close_pending_) return;
    LOG_DEBUG("mysql", "dispatch_connect: err_msg={} cb={}", err_msg ? err_msg : "(null)", connect_cb_.c_str());

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
    if (err_msg && err_msg[0]) {
        args[1] = inter::NativeToFakeluaString(lua_state_, err_msg);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 0);
    } else {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
        args[2] = inter::NativeToFakeluaInt(lua_state_, 1);
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

    const char *msg = err_msg && err_msg[0] ? err_msg : "query failed";

    LOG_DEBUG("mysql", "dispatch_result: err_msg={} cb={} result.is_result_set={} rows={} cols={}",
              err_msg ? err_msg : "(null)", result_cb_.c_str(),
              result.is_result_set, result.rows.size(), result.columns.size());

    CVar args[3];
    args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_)
                          : inter::NativeToFakeluaNil(lua_state_);
    if (err_msg) {
        args[1] = inter::NativeToFakeluaString(lua_state_, msg);
        args[2] = inter::NativeToFakeluaNil(lua_state_);
    } else if (result.stmt_id != 0) {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
        args[2] = inter::NativeToFakeluaInt(lua_state_, static_cast<int64_t>(result.stmt_id));
    } else {
        args[1] = inter::NativeToFakeluaNil(lua_state_);
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
