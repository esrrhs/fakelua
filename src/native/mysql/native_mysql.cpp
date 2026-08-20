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
static MysqlConnection *unwrap_conn(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<MysqlConnection *>(self->GetInt("__mysql_conn__", 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_query(NativeObject *self, State *s, CVar *args, int n);
static CVar conn_tick(NativeObject *self, State *s, CVar *args, int n);
static CVar conn_close(NativeObject *self, State *s, CVar *args, int n);

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

    // Create connection (async)
    auto *conn = new MysqlConnection();
    conn->set_state(s);
    conn->set_connect_callback(cb_name);

    try {
        conn->connect(host, port, user, password, database);
    } catch (const std::exception &e) {
        delete conn;
        error(std::format("connect failed: {}", e.what()));
    }

    // Wrap in NativeObject (need a group_id)
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "mysql_connection");
    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));
    nat->SetFinalizer([](NativeObject *self) {
        auto *c = unwrap_conn(self);
        if (c) {
            delete c;
            self->SetInt("__mysql_conn__", 0);
        }
    });
    nat->RegisterMethod("query", conn_query);
    nat->RegisterMethod("tick", conn_tick);
    nat->RegisterMethod("close", conn_close);

    // Set NativeObject on connection (for callback dispatch)
    conn->set_native_object(nat);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:query(sql, on_result)
// on_result(err, result) called when query completes
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_query(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "conn:query", "sql and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string sql = cvar_to_string(a0);
    std::string cb_name = cvar_to_string(a1);

    auto *conn = unwrap_conn(self);
    if (!conn) error("conn:query: connection is closed");

    conn->set_state(s);
    conn->set_result_callback(cb_name);
    conn->query(sql);

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:tick() — pump network events (call periodically from game loop)
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_tick(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *conn = unwrap_conn(self);
    if (!conn) return inter::NativeToFakeluaNil(s);

    conn->set_state(s);
    conn->tick();

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:close()
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_close(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = unwrap_conn(self);
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
