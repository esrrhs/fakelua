#include "native/mysql/native_mysql.h"
#include "native/mysql/mysql_connection_pool.h"
#include "native/mysql/mysql_connection.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "var/var.h"

#include <cstring>
#include <memory>
#include <string>
#include <unordered_set>

namespace fakelua {
class NativeObject;
}

namespace fakelua::mysql {

// ── Forward declarations ──
static CVar pool_acquire(NativeObject *self, State *s, CVar *args, int n);
static CVar pool_release(NativeObject *self, State *s, CVar *args, int n);
static CVar pool_tick(NativeObject *self, State *s, CVar *args, int n);
static CVar pool_close(NativeObject *self, State *s, CVar *args, int n);
static CVar pool_stats(NativeObject *self, State *s, CVar *args, int n);
static CVar conn_pool_release(NativeObject *self, State *s, CVar *args, int n);
static CVar conn_error_info(NativeObject *self, State *s, CVar *args, int n);
static MysqlConnection *unwrap_conn(CVar v);

// ─────────────────────────────────────────────────────────────────────────────
// Pool object wrapper
// ─────────────────────────────────────────────────────────────────────────────

struct PoolObject {
    std::unique_ptr<MysqlConnectionPool> pool;
    PoolConfig config;
    std::unordered_set<NativeObject *> wrappers;
};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

[[noreturn]] static void pool_error(const std::string &msg) {
    ThrowFakeluaException("mysql pool: " + msg);
}

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

static PoolObject *unwrap_pool(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<PoolObject *>(self->GetInt("__mysql_pool__", 0));
}

static void zero_acquired_conn_ptrs(PoolObject *po) {
    if (!po) return;
    for (auto *nat : po->wrappers) {
        if (!nat) continue;
        auto *c = unwrap_conn_native(nat);
        if (c) c->set_native_object(nullptr);
        nat->SetInt("__mysql_conn__", 0);
    }
}

static void invalidate_acquired_wrappers(PoolObject *po) {
    if (!po) return;
    auto wrappers = std::move(po->wrappers);
    po->wrappers.clear();
    for (auto *nat : wrappers) {
        if (!nat) continue;
        auto *c = unwrap_conn_native(nat);
        if (c) c->set_native_object(nullptr);
        nat->SetInt("__mysql_conn__", 0);
        nat->SetInt("__mysql_pool_ptr__", 0);
        nat->SetInt("__mysql_pool_obj__", 0);
    }
}

static void detach_acquired_wrapper(NativeObject *nat) {
    if (!nat) return;
    auto *po = reinterpret_cast<PoolObject *>(nat->GetInt("__mysql_pool_obj__", 0));
    auto *c = unwrap_conn_native(nat);
    if (po) {
        po->wrappers.erase(nat);
        if (po->pool && c) {
            c->set_native_object(nullptr);
            po->pool->release(c);
        }
    }
    nat->SetInt("__mysql_conn__", 0);
    nat->SetInt("__mysql_pool_ptr__", 0);
    nat->SetInt("__mysql_pool_obj__", 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// mysql_pool.create(config) → pool object
// ─────────────────────────────────────────────────────────────────────────────

static CVar pool_create(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "mysql_pool.create", "config table expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);

    PoolConfig config;
    config.pool_size = 4;
    config.connect_timeout_ms = 5000;
    config.read_timeout_ms = 5000;
    config.heartbeat_interval_ms = 30000;
    config.max_retries = 3;
    config.retry_base_ms = 1000;

    if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
        CVar host_var = table::TableHelper::GetTableStrId(s, a0, "host");
        if (host_var.type_ != static_cast<int>(VarType::Nil)) config.host = cvar_to_string(host_var);

        CVar port_var = table::TableHelper::GetTableStrId(s, a0, "port");
        if (port_var.type_ != static_cast<int>(VarType::Nil)) {
            config.port = static_cast<uint16_t>(inter::CVarToInteger(port_var, 3306));
        }

        CVar user_var = table::TableHelper::GetTableStrId(s, a0, "user");
        if (user_var.type_ != static_cast<int>(VarType::Nil)) config.user = cvar_to_string(user_var);

        CVar pass_var = table::TableHelper::GetTableStrId(s, a0, "password");
        if (pass_var.type_ != static_cast<int>(VarType::Nil)) config.password = cvar_to_string(pass_var);

        CVar db_var = table::TableHelper::GetTableStrId(s, a0, "db");
        if (db_var.type_ != static_cast<int>(VarType::Nil)) config.database = cvar_to_string(db_var);

        CVar size_var = table::TableHelper::GetTableStrId(s, a0, "pool_size");
        if (size_var.type_ != static_cast<int>(VarType::Nil)) {
            config.pool_size = static_cast<int>(inter::CVarToInteger(size_var, 4));
        }

        CVar timeout_var = table::TableHelper::GetTableStrId(s, a0, "timeout_ms");
        if (timeout_var.type_ != static_cast<int>(VarType::Nil)) {
            config.connect_timeout_ms = static_cast<int>(inter::CVarToInteger(timeout_var, 5000));
            config.read_timeout_ms = config.connect_timeout_ms;
        }

        CVar heartbeat_var = table::TableHelper::GetTableStrId(s, a0, "heartbeat_ms");
        if (heartbeat_var.type_ != static_cast<int>(VarType::Nil)) {
            config.heartbeat_interval_ms = static_cast<int>(inter::CVarToInteger(heartbeat_var, 30000));
        }

        CVar retries_var = table::TableHelper::GetTableStrId(s, a0, "max_retries");
        if (retries_var.type_ != static_cast<int>(VarType::Nil)) {
            config.max_retries = static_cast<int>(inter::CVarToInteger(retries_var, 3));
        }
    } else {
        ThrowBadArgument(1, "mysql_pool.create", "config must be a table");
    }

    if (config.user.empty()) ThrowBadArgument(1, "mysql_pool.create", "user required");
    if (config.pool_size < 1) config.pool_size = 1;
    if (config.pool_size > 256) config.pool_size = 256;
    if (config.max_retries < 0) config.max_retries = 0;

    auto *pool_obj = new PoolObject();
    pool_obj->config = config;
    pool_obj->pool = std::make_unique<MysqlConnectionPool>(config);

    try {
        pool_obj->pool->initialize();
    } catch (const std::exception &e) {
        delete pool_obj;
        pool_error(std::format("initialize failed: {}", e.what()));
    }

    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "mysql_pool");
    nat->SetInt("__mysql_pool__", reinterpret_cast<int64_t>(pool_obj));
    RegisterMysqlNativeWrapper(s, nat, true);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterMysqlNativeWrapper(self);
        auto *p = unwrap_pool(self);
        if (p) {
            invalidate_acquired_wrappers(p);
            delete p;
            self->SetInt("__mysql_pool__", 0);
        }
    });
    nat->RegisterMethod("acquire", pool_acquire);
    nat->RegisterMethod("release", pool_release);
    nat->RegisterMethod("tick", pool_tick);
    nat->RegisterMethod("close", pool_close);
    nat->RegisterMethod("stats", pool_stats);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ─────────────────────────────────────────────────────────────────────────────
// pool:acquire() → connection
// ─────────────────────────────────────────────────────────────────────────────

static CVar pool_acquire(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = unwrap_pool(self);
    if (!pool_obj || !pool_obj->pool) return inter::NativeToFakeluaNil(s);

    auto *conn = pool_obj->pool->acquire();
    if (!conn) return inter::NativeToFakeluaNil(s);

    // Wrap connection in NativeObject for Lua (use a new group for each connection)
    int64_t conn_gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(conn_gid, "mysql_connection");
    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));
    nat->SetInt("__mysql_pool_ptr__", reinterpret_cast<int64_t>(pool_obj->pool.get()));
    nat->SetInt("__mysql_pool_obj__", reinterpret_cast<int64_t>(pool_obj));
    nat->SetInt("__mysql_owned__", 0);
    pool_obj->wrappers.insert(nat);
    RegisterMysqlNativeWrapper(s, nat, false);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterMysqlNativeWrapper(self);
        detach_acquired_wrapper(self);
    });
    nat->RegisterMethod("query", conn_query);
    nat->RegisterMethod("stmt_prepare", conn_stmt_prepare);
    nat->RegisterMethod("stmt_execute", conn_stmt_execute);
    nat->RegisterMethod("stmt_close", conn_stmt_close);
    nat->RegisterMethod("tick", conn_tick);
    nat->RegisterMethod("close", conn_pool_release);
    nat->RegisterMethod("error", conn_error_info);

    conn->set_state(s);
    conn->set_native_object(nat);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ─────────────────────────────────────────────────────────────────────────────
// pool:release(conn)
// ─────────────────────────────────────────────────────────────────────────────

static CVar pool_release(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "pool:release", "connection expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    NativeObject *nat = NativeObject::Unwrap(a0);
    if (nat) {
        detach_acquired_wrapper(nat);
        return inter::NativeToFakeluaNil(s);
    }
    auto *conn = unwrap_conn(a0);
    if (!conn) return inter::NativeToFakeluaNil(s);

    auto *pool_obj = unwrap_pool(self);
    if (pool_obj && pool_obj->pool) {
        conn->set_native_object(nullptr);
        pool_obj->pool->release(conn);
    }
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// pool:tick() — drive heartbeat and reconnect
// ─────────────────────────────────────────────────────────────────────────────

static CVar pool_tick(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = unwrap_pool(self);
    if (!pool_obj || !pool_obj->pool) return inter::NativeToFakeluaNil(s);

    pool_obj->pool->tick();
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// pool:close()
// ─────────────────────────────────────────────────────────────────────────────

static CVar pool_close(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = unwrap_pool(self);
    if (pool_obj && pool_obj->pool) {
        zero_acquired_conn_ptrs(pool_obj);
        pool_obj->pool->close();
        pool_obj->pool->reap();
    }
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// pool:stats() → {total, healthy}
// ─────────────────────────────────────────────────────────────────────────────

static CVar pool_stats(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = unwrap_pool(self);
    CVar tbl = table::TableHelper::CreateTable(s);
    if (pool_obj && pool_obj->pool) {
        table::TableHelper::SetTableInt(s, tbl, 1,
            inter::NativeToFakeluaInt(s, static_cast<int64_t>(pool_obj->pool->total_count())));
        table::TableHelper::SetTableInt(s, tbl, 2,
            inter::NativeToFakeluaInt(s, static_cast<int64_t>(pool_obj->pool->healthy_count())));
    }
    return tbl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Connection methods (shared with direct connect)
// ─────────────────────────────────────────────────────────────────────────────

// unwrap_conn_native is defined in native_mysql.cpp (shared)

static CVar conn_pool_release(NativeObject *self, State *s, CVar *args, int n) {
    detach_acquired_wrapper(self);
    return inter::NativeToFakeluaNil(s);
}

static CVar conn_error_info(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = unwrap_conn_native(self);
    if (!conn) return inter::NativeToFakeluaNil(s);

    auto err = conn->last_error();
    CVar tbl = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableInt(s, tbl, 1,
        inter::NativeToFakeluaInt(s, static_cast<int64_t>(err.type)));
    table::TableHelper::SetTableInt(s, tbl, 2,
        inter::NativeToFakeluaInt(s, static_cast<int64_t>(err.code)));
    table::TableHelper::SetTableInt(s, tbl, 3,
        inter::NativeToFakeluaString(s, err.message));
    table::TableHelper::SetTableInt(s, tbl, 4,
        inter::NativeToFakeluaString(s, err.sql_state));
    return tbl;
}

// ── Unwrap connection from CVar ──

static MysqlConnection *unwrap_conn(CVar v) {
    if (v.type_ != static_cast<int>(VarType::Table) || !v.data_.t) return nullptr;
    const VarTable *tbl = v.data_.t;
    if (!tbl || !tbl->spec) return nullptr;
    // Check if this is a NativeObject wrapper
    void *spec_get = tbl->spec_get;
    if (spec_get != reinterpret_cast<void *>(NativeSpecGet)) return nullptr;
    auto *spec = static_cast<NativeObjectSpec *>(tbl->spec);
    return reinterpret_cast<MysqlConnection *>(spec->obj->GetInt("__mysql_conn__", 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Registration
// ─────────────────────────────────────────────────────────────────────────────

void RegisterMysqlPoolApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "mysql_pool.create", 1, false, pool_create);
}

}  // namespace fakelua::mysql
