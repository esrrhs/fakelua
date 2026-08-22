#include "native/mysql/native_mysql.h"
#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_result.h"
#include "native/mysql/mysql_protocol.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "var/var.h"

#include <cstring>
#include <memory>
#include <string>

namespace fakelua::mysql {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

[[noreturn]] static void error(const std::string &msg) {
    ThrowFakeluaException("mysql: " + msg);
}

// Extract string from CVar (handles both String and StringId)
static std::string cvar_to_string(CVar v) {
    if (v.type_ == static_cast<int>(VarType::String) && v.data_.s) {
        return std::string(v.data_.s->Str());
    }
    if (v.type_ == static_cast<int>(VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string(ptr + 8, sz);
    }
    return {};
}

// Retrieve MysqlConnection* from NativeObject
MysqlConnection *unwrap_conn_native(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<MysqlConnection *>(self->GetInt("__mysql_conn__", 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_query(NativeObject *self, State *s, CVar *args, int n);
CVar conn_stmt_prepare(NativeObject *self, State *s, CVar *args, int n);
CVar conn_stmt_execute(NativeObject *self, State *s, CVar *args, int n);
CVar conn_stmt_close(NativeObject *self, State *s, CVar *args, int n);
CVar conn_tick(NativeObject *self, State *s, CVar *args, int n);
CVar conn_close(NativeObject *self, State *s, CVar *args, int n);

// ─────────────────────────────────────────────────────────────────────────────
// mysql.connect(config, on_connect) → connection object
// on_connect(err, success) called when connection completes
// ─────────────────────────────────────────────────────────────────────────────

static CVar mysql_connect(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "mysql.connect", "config table and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);

    // Read config fields
    std::string host = "127.0.0.1";
    uint16_t port = 3306;
    std::string user;
    std::string password;
    std::string database;

    if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
        CVar host_var = table::TableHelper::GetTableStrId(s, a0, "host");
        if (host_var.type_ != static_cast<int>(VarType::Nil)) host = cvar_to_string(host_var);

        CVar port_var = table::TableHelper::GetTableStrId(s, a0, "port");
        if (port_var.type_ != static_cast<int>(VarType::Nil)) {
            port = static_cast<uint16_t>(inter::CVarToInteger(port_var, 3306));
        }

        CVar user_var = table::TableHelper::GetTableStrId(s, a0, "user");
        if (user_var.type_ != static_cast<int>(VarType::Nil)) user = cvar_to_string(user_var);

        CVar pass_var = table::TableHelper::GetTableStrId(s, a0, "password");
        if (pass_var.type_ != static_cast<int>(VarType::Nil)) password = cvar_to_string(pass_var);

        CVar db_var = table::TableHelper::GetTableStrId(s, a0, "db");
        if (db_var.type_ != static_cast<int>(VarType::Nil)) database = cvar_to_string(db_var);
    } else {
        ThrowBadArgument(1, "mysql.connect", "config must be a table");
    }

    if (user.empty()) ThrowBadArgument(1, "mysql.connect", "user required");

    // Read callback function name
    std::string cb_name = cvar_to_string(a1);
    if (cb_name.empty()) ThrowBadArgument(1, "mysql.connect", "callback function expected");

    // Create NativeObject wrapper first (so callbacks can dispatch)
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "mysql_connection");
    nat->SetFinalizer([](NativeObject *self) {
        auto *c = unwrap_conn_native(self);
        if (c) {
            delete c;
            self->SetInt("__mysql_conn__", 0);
        }
    });
    nat->RegisterMethod("query", conn_query);
    nat->RegisterMethod("stmt_prepare", conn_stmt_prepare);
    nat->RegisterMethod("stmt_execute", conn_stmt_execute);
    nat->RegisterMethod("stmt_close", conn_stmt_close);
    nat->RegisterMethod("tick", conn_tick);
    nat->RegisterMethod("close", conn_close);

    // Create connection (async)
    auto *conn = new MysqlConnection();
    conn->set_state(s);
    conn->set_connect_callback(cb_name);
    conn->set_native_object(nat);  // Must be set before connect() for immediate-failure dispatch

    try {
        conn->connect(host, port, user, password, database);
    } catch (const std::exception &e) {
        delete conn;
        error(std::format("connect failed: {}", e.what()));
    }

    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:query(sql, on_result)
// on_result(err, result) called when query completes
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_query(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "conn:query", "sql and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string sql = cvar_to_string(a0);
    std::string cb_name = cvar_to_string(a1);

    auto *conn = unwrap_conn_native(self);
    if (!conn) error("conn:query: connection is closed");

    conn->set_state(s);
    conn->set_result_callback(cb_name);
    conn->query(sql);

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:stmt_prepare(sql, on_result)
// on_result(err, stmt_id) called when prepare completes
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_stmt_prepare(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "conn:stmt_prepare", "sql and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string sql = cvar_to_string(a0);
    std::string cb_name = cvar_to_string(a1);

    auto *conn = unwrap_conn_native(self);
    if (!conn) error("conn:stmt_prepare: connection is closed");

    conn->set_state(s);
    conn->set_result_callback(cb_name);
    conn->stmt_prepare(sql);

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:stmt_execute(stmt_id, params, on_result)
// on_result(err, result) called when execute completes
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_stmt_execute(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "conn:stmt_execute", "stmt_id, params, and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    CVar a2 = inter::GetNativeArg(s, args, n, 2);

    uint32_t stmt_id = static_cast<uint32_t>(inter::CVarToInteger(a0, 0));
    std::string cb_name = cvar_to_string(a2);

    // Parse params array
    std::vector<std::string> params;
    if (a1.type_ == static_cast<int>(VarType::Table) && a1.data_.t) {
        // Read array elements (1-based)
        CVar len_var = table::TableHelper::GetTableStrId(s, a1, "n");
        int64_t len = inter::CVarToInteger(len_var, 0);
        for (int64_t i = 1; i <= len; ++i) {
            CVar elem = table::TableHelper::GetTableInt(s, a1, i);
            params.push_back(cvar_to_string(elem));
        }
    }

    auto *conn = unwrap_conn_native(self);
    if (!conn) error("conn:stmt_execute: connection is closed");

    conn->set_state(s);
    conn->set_result_callback(cb_name);
    conn->stmt_execute(stmt_id, params);

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:stmt_close(stmt_id)
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_stmt_close(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "conn:stmt_close", "stmt_id expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    uint32_t stmt_id = static_cast<uint32_t>(inter::CVarToInteger(a0, 0));

    auto *conn = unwrap_conn_native(self);
    if (!conn) return inter::NativeToFakeluaNil(s);

    conn->stmt_close(stmt_id);
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:tick() — pump network events (call periodically from game loop)
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_tick(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *conn = unwrap_conn_native(self);
    if (!conn) return inter::NativeToFakeluaNil(s);

    conn->set_state(s);
    conn->tick();

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:close()
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_close(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = unwrap_conn_native(self);
    if (conn) {
        conn->close();
        delete conn;
        self->SetInt("__mysql_conn__", 0);
    }
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// Registration
// ─────────────────────────────────────────────────────────────────────────────

void RegisterMysqlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "mysql.connect", 2, false, mysql_connect);
}

}  // namespace fakelua::mysql
