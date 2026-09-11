#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_connection_pool.h"
#include "native/mysql/native_mysql.h"
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

// Forward declarations
static CVar PoolAcquire(NativeObject *self, State *s, CVar *args, int n);
static CVar PoolRelease(NativeObject *self, State *s, CVar *args, int n);
static CVar PoolClose(NativeObject *self, State *s, CVar *args, int n);
static CVar PoolStats(NativeObject *self, State *s, CVar *args, int n);
static CVar ConnPoolRelease(NativeObject *self, State *s, CVar *args, int n);
static CVar ConnErrorInfo(NativeObject *self, State *s, CVar *args, int n);
static MysqlConnection *UnwrapConn(CVar v);

// Pool object wrapper

struct PoolObject {
    std::unique_ptr<MysqlConnectionPool> pool;
    PoolConfig config;
    std::unordered_set<NativeObject *> wrappers;
};

// Helpers

[[noreturn]] static void PoolError(const std::string &msg) {
    ThrowFakeluaException("mysql pool: " + msg);
}

static std::string CVarToString(CVar v) {
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

static PoolObject *UnwrapPool(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<PoolObject *>(self->GetInt("__mysql_pool__", 0));
}

static void ZeroAcquiredConnPtrs(PoolObject *po) {
    if (!po) return;
    for (auto *nat: po->wrappers) {
        if (!nat) continue;
        auto *c = UnwrapConnNative(nat);
        if (c) c->SetNativeObject(nullptr);
        nat->SetInt("__mysql_conn__", 0);
    }
}

static void InvalidateAcquiredWrappers(PoolObject *po) {
    if (!po) return;
    auto wrappers = std::move(po->wrappers);
    po->wrappers.clear();
    for (auto *nat: wrappers) {
        if (!nat) continue;
        auto *c = UnwrapConnNative(nat);
        if (c) c->SetNativeObject(nullptr);
        nat->SetInt("__mysql_conn__", 0);
        nat->SetInt("__mysql_pool_ptr__", 0);
        nat->SetInt("__mysql_pool_obj__", 0);
    }
}

static void DetachAcquiredWrapper(NativeObject *nat) {
    if (!nat) return;
    auto *po = reinterpret_cast<PoolObject *>(nat->GetInt("__mysql_pool_obj__", 0));
    auto *c = UnwrapConnNative(nat);
    if (po) {
        po->wrappers.erase(nat);
        if (po->pool && c) {
            c->SetNativeObject(nullptr);
            po->pool->Release(c);
        }
    }
    nat->SetInt("__mysql_conn__", 0);
    nat->SetInt("__mysql_pool_ptr__", 0);
    nat->SetInt("__mysql_pool_obj__", 0);
}

// mysql_pool.create(config) → pool object

static CVar PoolCreate(State *s, CVar *args, int n) {
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
        if (host_var.type_ != static_cast<int>(VarType::Nil)) config.host = CVarToString(host_var);

        CVar port_var = table::TableHelper::GetTableStrId(s, a0, "port");
        if (port_var.type_ != static_cast<int>(VarType::Nil)) {
            config.port = static_cast<uint16_t>(inter::CVarToInteger(port_var, 3306));
        }

        CVar user_var = table::TableHelper::GetTableStrId(s, a0, "user");
        if (user_var.type_ != static_cast<int>(VarType::Nil)) config.user = CVarToString(user_var);

        CVar pass_var = table::TableHelper::GetTableStrId(s, a0, "password");
        if (pass_var.type_ != static_cast<int>(VarType::Nil)) config.password = CVarToString(pass_var);

        CVar db_var = table::TableHelper::GetTableStrId(s, a0, "db");
        if (db_var.type_ != static_cast<int>(VarType::Nil)) config.database = CVarToString(db_var);

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

        CVar ssl_var = table::TableHelper::GetTableStrId(s, a0, "ssl");
        if (ssl_var.type_ == static_cast<int>(VarType::Bool)) {
            config.ssl = AsVar(ssl_var).GetBool() ? boost::mysql::ssl_mode::require : boost::mysql::ssl_mode::disable;
        } else if (ssl_var.type_ == static_cast<int>(VarType::Int)) {
            config.ssl = ssl_var.data_.i ? boost::mysql::ssl_mode::require : boost::mysql::ssl_mode::disable;
        } else if (ssl_var.type_ != static_cast<int>(VarType::Nil)) {
            std::string mode = CVarToString(ssl_var);
            if (mode == "require" || mode == "true") config.ssl = boost::mysql::ssl_mode::require;
            else if (mode == "enable") config.ssl = boost::mysql::ssl_mode::enable;
            else config.ssl = boost::mysql::ssl_mode::disable;
        }
        CVar ca_var = table::TableHelper::GetTableStrId(s, a0, "ssl_ca");
        if (ca_var.type_ != static_cast<int>(VarType::Nil)) config.ssl_ca = CVarToString(ca_var);
    } else {
        ThrowBadArgument(1, "mysql_pool.create", "config must be a table");
    }

    if (config.user.empty()) ThrowBadArgument(1, "mysql_pool.create", "user required");
    if (config.pool_size < 1) config.pool_size = 1;
    if (config.pool_size > 256) config.pool_size = 256;
    if (config.max_retries < 0) config.max_retries = 0;

    auto *pool_obj = new PoolObject();
    pool_obj->config = config;
    pool_obj->pool = std::make_unique<MysqlConnectionPool>(config, s);

    try {
        pool_obj->pool->Initialize();
    } catch (const std::exception &e) {
        delete pool_obj;
        PoolError(std::format("initialize failed: {}", e.what()));
    }

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, "mysql_pool");
    nat->SetInt("__mysql_pool__", reinterpret_cast<int64_t>(pool_obj));
    RegisterMysqlNativeWrapper(s, nat, true);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterMysqlNativeWrapper(self);
        auto *p = UnwrapPool(self);
        if (p) {
            InvalidateAcquiredWrappers(p);
            delete p;
            self->SetInt("__mysql_pool__", 0);
        }
    });
    nat->RegisterMethod("acquire", PoolAcquire);
    nat->RegisterMethod("release", PoolRelease);
    nat->RegisterMethod("close", PoolClose);
    nat->RegisterMethod("stats", PoolStats);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// pool:acquire() → connection

static CVar PoolAcquire(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = UnwrapPool(self);
    if (!pool_obj || !pool_obj->pool) return inter::NativeToFakeluaNil(s);

    auto *conn = pool_obj->pool->Acquire();
    if (!conn) return inter::NativeToFakeluaNil(s);

    // Wrap connection in NativeObject for Lua (use a new group for each connection)
    int64_t conn_gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(conn_gid, "mysql_connection");
    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));
    nat->SetInt("__mysql_pool_ptr__", reinterpret_cast<int64_t>(pool_obj->pool.get()));
    nat->SetInt("__mysql_pool_obj__", reinterpret_cast<int64_t>(pool_obj));
    nat->SetInt("__mysql_owned__", 0);
    pool_obj->wrappers.insert(nat);
    RegisterMysqlNativeWrapper(s, nat, false);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterMysqlNativeWrapper(self);
        DetachAcquiredWrapper(self);
    });
    nat->RegisterMethod("query", ConnQuery);
    nat->RegisterMethod("stmt_prepare", ConnStmtPrepare);
    nat->RegisterMethod("stmt_execute", ConnStmtExecute);
    nat->RegisterMethod("stmt_close", ConnStmtClose);
    nat->RegisterMethod("close", ConnPoolRelease);
    nat->RegisterMethod("error", ConnErrorInfo);

    conn->SetState(s);
    conn->SetNativeObject(nat);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// pool:release(conn)

static CVar PoolRelease(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "pool:release", "connection expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    NativeObject *nat = NativeObject::Unwrap(a0);
    if (nat) {
        DetachAcquiredWrapper(nat);
        return inter::NativeToFakeluaNil(s);
    }
    auto *conn = UnwrapConn(a0);
    if (!conn) return inter::NativeToFakeluaNil(s);

    auto *pool_obj = UnwrapPool(self);
    if (pool_obj && pool_obj->pool) {
        conn->SetNativeObject(nullptr);
        pool_obj->pool->Release(conn);
    }
    return inter::NativeToFakeluaNil(s);
}

// Drive the pool's heartbeat and reconnect, via runtime.tick()

// 连接池关闭后 unwrap 返回空，于是自然变成 no-op。
void TickMysqlPool(NativeObject *self) {
    auto *pool_obj = UnwrapPool(self);
    if (!pool_obj || !pool_obj->pool) return;

    pool_obj->pool->Tick();
}

// pool:close()

static CVar PoolClose(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = UnwrapPool(self);
    if (pool_obj && pool_obj->pool) {
        ZeroAcquiredConnPtrs(pool_obj);
        pool_obj->pool->Close();
        pool_obj->pool->Reap();
    }
    return inter::NativeToFakeluaNil(s);
}

// pool:stats() → {total, healthy}

static CVar PoolStats(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *pool_obj = UnwrapPool(self);
    CVar tbl = table::TableHelper::CreateTable(s);
    if (pool_obj && pool_obj->pool) {
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaInt(s, static_cast<int64_t>(pool_obj->pool->TotalCount())));
        table::TableHelper::SetTableInt(s, tbl, 2, inter::NativeToFakeluaInt(s, static_cast<int64_t>(pool_obj->pool->HealthyCount())));
    }
    return tbl;
}

// Connection methods (shared with direct connect)

// UnwrapConnNative is defined in native_mysql.cpp (shared)

static CVar ConnPoolRelease(NativeObject *self, State *s, CVar *args, int n) {
    DetachAcquiredWrapper(self);
    return inter::NativeToFakeluaNil(s);
}

static CVar ConnErrorInfo(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = UnwrapConnNative(self);
    if (!conn) return inter::NativeToFakeluaNil(s);

    auto err = conn->LastError();
    CVar tbl = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaInt(s, static_cast<int64_t>(err.type)));
    table::TableHelper::SetTableInt(s, tbl, 2, inter::NativeToFakeluaInt(s, static_cast<int64_t>(err.code)));
    table::TableHelper::SetTableInt(s, tbl, 3, inter::NativeToFakeluaString(s, err.message));
    table::TableHelper::SetTableInt(s, tbl, 4, inter::NativeToFakeluaString(s, err.sql_state));
    return tbl;
}

// Unwrap connection from CVar
static MysqlConnection *UnwrapConn(CVar v) {
    if (v.type_ != static_cast<int>(VarType::Table) || !v.data_.t) return nullptr;
    const VarTable *tbl = v.data_.t;
    if (!tbl || !tbl->spec) return nullptr;
    // Check if this is a NativeObject wrapper
    void *spec_get = tbl->spec_get;
    if (spec_get != reinterpret_cast<void *>(NativeSpecGet)) return nullptr;
    auto *spec = static_cast<NativeObjectSpec *>(tbl->spec);
    return reinterpret_cast<MysqlConnection *>(spec->obj->GetInt("__mysql_conn__", 0));
}

// Registration

void RegisterMysqlPoolApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "mysql_pool.create", 1, false, PoolCreate);
}

}// namespace fakelua::mysql
