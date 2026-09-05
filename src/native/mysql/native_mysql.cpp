#include "native/mysql/native_mysql.h"
#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_connection_pool.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "var/var.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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

static std::unordered_map<State *, std::vector<NativeObject *>> g_mysql_conns;
static std::unordered_map<State *, std::vector<NativeObject *>> g_mysql_pools;

static void erase_wrapper(std::unordered_map<State *, std::vector<NativeObject *>> &map,
                          State *st, NativeObject *nat) {
    if (!st) return;
    auto it = map.find(st);
    if (it == map.end()) return;
    auto &v = it->second;
    v.erase(std::remove(v.begin(), v.end(), nat), v.end());
    if (v.empty()) map.erase(it);
}

void RegisterMysqlNativeWrapper(State *s, NativeObject *nat, bool is_pool) {
    if (!s || !nat) return;
    nat->SetInt("__mysql_state__", reinterpret_cast<int64_t>(s));
    nat->SetInt("__mysql_is_pool__", is_pool ? 1 : 0);
    if (is_pool) {
        g_mysql_pools[s].push_back(nat);
    } else {
        g_mysql_conns[s].push_back(nat);
    }
}

void UnregisterMysqlNativeWrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt("__mysql_state__", 0));
    bool is_pool = nat->GetInt("__mysql_is_pool__", 0) != 0;
    nat->SetInt("__mysql_state__", 0);
    erase_wrapper(is_pool ? g_mysql_pools : g_mysql_conns, st, nat);
}

static void destroy_mysql_wrappers(std::unordered_map<State *, std::vector<NativeObject *>> &map, State *s) {
    auto it = map.find(s);
    if (it == map.end()) return;
    auto wrappers = std::move(it->second);
    map.erase(it);
    for (auto *nat : wrappers) {
        if (!nat) continue;
        nat->SetInt("__mysql_state__", 0);
        NativeObjectManager::Instance().DestroyGroup(nat->GetGroupId());
    }
}

void OnStateDeleted(State *s) {
    if (!s) return;
    // Connection wrappers first so pool acquire finalizers can still release().
    destroy_mysql_wrappers(g_mysql_conns, s);
    destroy_mysql_wrappers(g_mysql_pools, s);
}

static void maybe_reap_pool(NativeObject *self) {
    if (!self) return;
    auto *pool = reinterpret_cast<MysqlConnectionPool *>(self->GetInt("__mysql_pool_ptr__", 0));
    if (pool) pool->reap();
}

static void maybe_release_owned_conn(NativeObject *self) {
    if (!self) return;
    if (self->GetInt("__mysql_owned__", 0) == 0) return;
    auto *conn = unwrap_conn_native(self);
    if (!conn || conn->tick_depth() > 0 || !conn->close_pending()) return;
    conn->close();
    delete conn;
    self->SetInt("__mysql_conn__", 0);
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
CVar conn_ping(NativeObject *self, State *s, CVar *args, int n);

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
    int timeout_ms = 0;

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

        CVar timeout_var = table::TableHelper::GetTableStrId(s, a0, "timeout_ms");
        if (timeout_var.type_ != static_cast<int>(VarType::Nil)) {
            timeout_ms = static_cast<int>(inter::CVarToInteger(timeout_var, 0));
        }
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
        UnregisterMysqlNativeWrapper(self);
        auto *c = unwrap_conn_native(self);
        if (c) {
            if (c->tick_depth() > 0) {
                c->request_close();
            } else {
                delete c;
            }
            self->SetInt("__mysql_conn__", 0);
        }
    });
    nat->RegisterMethod("query", conn_query);
    nat->RegisterMethod("stmt_prepare", conn_stmt_prepare);
    nat->RegisterMethod("stmt_execute", conn_stmt_execute);
    nat->RegisterMethod("stmt_close", conn_stmt_close);
    nat->RegisterMethod("tick", conn_tick);
    nat->RegisterMethod("close", conn_close);
    nat->RegisterMethod("ping", conn_ping);
    nat->SetInt("__mysql_owned__", 1);
    RegisterMysqlNativeWrapper(s, nat, false);

    // Create connection (async)
    auto *conn = new MysqlConnection();
    conn->set_state(s);
    conn->set_connect_callback(cb_name);
    conn->set_native_object(nat);
    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));

    try {
        conn->connect(host, port, user, password, database, timeout_ms);
    } catch (const std::exception &e) {
        nat->SetInt("__mysql_conn__", 0);
        delete conn;
        NativeObjectManager::Instance().DestroyGroup(gid);
        error(std::format("connect failed: {}", e.what()));
    }

    maybe_release_owned_conn(nat);
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
    maybe_release_owned_conn(self);

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
    maybe_release_owned_conn(self);

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

    std::vector<StmtParam> params;
    if (a1.type_ == static_cast<int>(VarType::Table) && a1.data_.t) {
        CVar len_var = table::TableHelper::GetTableStrId(s, a1, "n");
        int64_t len = 0;
        if (len_var.type_ == static_cast<int>(VarType::Int)) {
            len = len_var.data_.i;
        } else {
            len = table::TableHelper::GetTableLen(a1);
            table::TableHelper::ForEachKV(a1, [&](CVar k, CVar /*v*/) {
                if (k.type_ == static_cast<int>(VarType::Int) && k.data_.i > len) {
                    len = k.data_.i;
                }
            });
        }
        for (int64_t i = 1; i <= len; ++i) {
            CVar elem = table::TableHelper::GetTableInt(s, a1, i);
            StmtParam p;
            if (elem.type_ == static_cast<int>(VarType::Nil)) {
                p.is_null = true;
            } else if (elem.type_ == static_cast<int>(VarType::Int)) {
                p.value = std::to_string(elem.data_.i);
            } else if (elem.type_ == static_cast<int>(VarType::Float)) {
                p.value = std::to_string(elem.data_.f);
            } else if (elem.type_ == static_cast<int>(VarType::Bool)) {
                p.value = elem.data_.b ? "1" : "0";
            } else {
                p.value = cvar_to_string(elem);
            }
            params.push_back(std::move(p));
        }
    }

    auto *conn = unwrap_conn_native(self);
    if (!conn) error("conn:stmt_execute: connection is closed");

    conn->set_state(s);
    conn->set_result_callback(cb_name);
    conn->stmt_execute(stmt_id, params);
    maybe_release_owned_conn(self);

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
    maybe_release_owned_conn(self);
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:tick() — pump network events (call periodically from game loop)
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_tick(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *conn = unwrap_conn_native(self);
    if (!conn) return inter::NativeToFakeluaNil(s);
    if (conn->tick_depth() > 0) return inter::NativeToFakeluaNil(s);

    conn->set_state(s);
    conn->tick();
    maybe_release_owned_conn(self);
    maybe_reap_pool(self);

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:close()
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_close(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = unwrap_conn_native(self);
    if (conn) {
        if (conn->tick_depth() > 0) {
            conn->request_close();
            return inter::NativeToFakeluaNil(s);
        }
        conn->close();
        delete conn;
        self->SetInt("__mysql_conn__", 0);
    }
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:ping() — send COM_PING heartbeat (for connection pool keepalive)
// ─────────────────────────────────────────────────────────────────────────────

CVar conn_ping(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *conn = unwrap_conn_native(self);
    if (!conn) return inter::NativeToFakeluaBool(s, false);
    bool sent = conn->ping();
    return inter::NativeToFakeluaBool(s, sent);
}

// ─────────────────────────────────────────────────────────────────────────────
// Registration
// ─────────────────────────────────────────────────────────────────────────────

void RegisterMysqlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "mysql.connect", 2, false, mysql_connect);
}

}  // namespace fakelua::mysql
