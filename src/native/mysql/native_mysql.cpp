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

// Helpers

[[noreturn]] static void error(const std::string &msg) {
    ThrowFakeluaException("mysql: " + msg);
}

// Extract string from CVar (handles both String and StringId)
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

// Retrieve MysqlConnection* from NativeObject
MysqlConnection *UnwrapConnNative(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<MysqlConnection *>(self->GetInt("__mysql_conn__", 0));
}

// 存在 State 上而不是这里的 static map：后者是全进程一份，多个线程各跑自己的 State 时
// 会并发改同一个容器。
struct MysqlWrappers {
    std::vector<NativeObject *> conns;
    std::vector<NativeObject *> pools;
};

static std::vector<NativeObject *> &WrapperList(State *s, bool is_pool) {
    auto &ws = s->GetModuleState<MysqlWrappers>();
    return is_pool ? ws.pools : ws.conns;
}

static void EraseWrapper(State *st, bool is_pool, NativeObject *nat) {
    if (!st) return;
    auto &v = WrapperList(st, is_pool);
    v.erase(std::remove(v.begin(), v.end(), nat), v.end());
}

void RegisterMysqlNativeWrapper(State *s, NativeObject *nat, bool is_pool) {
    if (!s || !nat) return;
    nat->SetInt("__mysql_state__", reinterpret_cast<int64_t>(s));
    nat->SetInt("__mysql_is_pool__", is_pool ? 1 : 0);
    WrapperList(s, is_pool).push_back(nat);
}

void UnregisterMysqlNativeWrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt("__mysql_state__", 0));
    bool is_pool = nat->GetInt("__mysql_is_pool__", 0) != 0;
    nat->SetInt("__mysql_state__", 0);
    EraseWrapper(st, is_pool, nat);
}

static void DestroyMysqlWrappers(State *s, bool is_pool) {
    auto *ws = s->TryGetModuleState<MysqlWrappers>();
    if (!ws) return;
    auto wrappers = std::move(is_pool ? ws->pools : ws->conns);
    for (auto *nat: wrappers) {
        if (!nat) continue;
        nat->SetInt("__mysql_state__", 0);
        s->GetNativeObjectManager().DestroyGroup(nat->GetGroupId());
    }
}

void TickAll(State *s) {
    if (!s) return;
    // 两处都是先拷一份再遍历：回调里可能 pool:acquire() 或者关连接，都会改动这些 vector。
    // 快照里已经销毁的对象 unwrap 拿到空，tick 自己就是 no-op。
    // 先池后连接：池这一步推进心跳和重连，让本轮拿到的连接尽量是可用的。
    auto *ws = s->TryGetModuleState<MysqlWrappers>();
    if (!ws) return;
    auto pools = ws->pools;
    for (auto *nat: pools) TickMysqlPool(nat);
    auto conns = ws->conns;
    for (auto *nat: conns) TickMysqlConnection(nat, s);
}

void OnStateDeleted(State *s) {
    if (!s) return;
    // Connection wrappers first so pool acquire finalizers can still release().
    DestroyMysqlWrappers(s, false);
    DestroyMysqlWrappers(s, true);
}

static void MaybeReapPool(NativeObject *self) {
    if (!self) return;
    auto *pool = reinterpret_cast<MysqlConnectionPool *>(self->GetInt("__mysql_pool_ptr__", 0));
    if (pool) pool->Reap();
}

static void MaybeReleaseOwnedConn(NativeObject *self) {
    if (!self) return;
    if (self->GetInt("__mysql_owned__", 0) == 0) return;
    auto *conn = UnwrapConnNative(self);
    if (!conn || conn->TickDepth() > 0 || !conn->ClosePending()) return;
    conn->Close();
}

// Forward declarations

CVar ConnQuery(NativeObject *self, State *s, CVar *args, int n);
CVar ConnStmtPrepare(NativeObject *self, State *s, CVar *args, int n);
CVar ConnStmtExecute(NativeObject *self, State *s, CVar *args, int n);
CVar ConnStmtClose(NativeObject *self, State *s, CVar *args, int n);
CVar ConnClose(NativeObject *self, State *s, CVar *args, int n);
CVar ConnPing(NativeObject *self, State *s, CVar *args, int n);

// mysql.connect(config, on_connect) → connection object
// on_connect(err, success) called when connection completes

static CVar MysqlConnect(State *s, CVar *args, int n) {
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
        if (host_var.type_ != static_cast<int>(VarType::Nil)) host = CVarToString(host_var);

        CVar port_var = table::TableHelper::GetTableStrId(s, a0, "port");
        if (port_var.type_ != static_cast<int>(VarType::Nil)) {
            port = static_cast<uint16_t>(inter::CVarToInteger(port_var, 3306));
        }

        CVar user_var = table::TableHelper::GetTableStrId(s, a0, "user");
        if (user_var.type_ != static_cast<int>(VarType::Nil)) user = CVarToString(user_var);

        CVar pass_var = table::TableHelper::GetTableStrId(s, a0, "password");
        if (pass_var.type_ != static_cast<int>(VarType::Nil)) password = CVarToString(pass_var);

        CVar db_var = table::TableHelper::GetTableStrId(s, a0, "db");
        if (db_var.type_ != static_cast<int>(VarType::Nil)) database = CVarToString(db_var);

        CVar timeout_var = table::TableHelper::GetTableStrId(s, a0, "timeout_ms");
        if (timeout_var.type_ != static_cast<int>(VarType::Nil)) {
            timeout_ms = static_cast<int>(inter::CVarToInteger(timeout_var, 0));
        }
    } else {
        ThrowBadArgument(1, "mysql.connect", "config must be a table");
    }

    if (user.empty()) ThrowBadArgument(1, "mysql.connect", "user required");

    // Read callback function name
    std::string cb_name = CVarToString(a1);
    if (cb_name.empty()) ThrowBadArgument(1, "mysql.connect", "callback function expected");

    // Create NativeObject wrapper first (so callbacks can dispatch)
    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, "mysql_connection");
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterMysqlNativeWrapper(self);
        auto *c = UnwrapConnNative(self);
        if (c) {
            self->SetInt("__mysql_conn__", 0);
            if (c->TickDepth() > 0) {
                c->RequestClose();
            } else {
                delete c;
            }
        }
    });
    nat->RegisterMethod("query", ConnQuery);
    nat->RegisterMethod("stmt_prepare", ConnStmtPrepare);
    nat->RegisterMethod("stmt_execute", ConnStmtExecute);
    nat->RegisterMethod("stmt_close", ConnStmtClose);
    nat->RegisterMethod("close", ConnClose);
    nat->RegisterMethod("ping", ConnPing);
    nat->SetInt("__mysql_owned__", 1);
    RegisterMysqlNativeWrapper(s, nat, false);

    // Create connection (async)
    auto *conn = new MysqlConnection(s);
    conn->SetConnectCallback(cb_name);
    conn->SetNativeObject(nat);
    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));

    try {
        conn->Connect(host, port, user, password, database, timeout_ms);
    } catch (const std::exception &e) {
        nat->SetInt("__mysql_conn__", 0);
        delete conn;
        s->GetNativeObjectManager().DestroyGroup(gid);
        error(std::format("connect failed: {}", e.what()));
    }

    MaybeReleaseOwnedConn(nat);
    return inter::NativeToFakeluaNativeObject(s, nat);
}

// conn:query(sql, on_result)
// on_result(err, result) called when query completes

CVar ConnQuery(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "conn:query", "sql and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string sql = CVarToString(a0);
    std::string cb_name = CVarToString(a1);

    auto *conn = UnwrapConnNative(self);
    if (!conn || !conn->Connected()) error("conn:query: connection is closed");

    conn->SetState(s);
    conn->SetResultCallback(cb_name);
    conn->Query(sql);
    MaybeReleaseOwnedConn(self);

    return inter::NativeToFakeluaNil(s);
}

// conn:stmt_prepare(sql, on_result)
// on_result(err, stmt_id) called when prepare completes

CVar ConnStmtPrepare(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "conn:stmt_prepare", "sql and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string sql = CVarToString(a0);
    std::string cb_name = CVarToString(a1);

    auto *conn = UnwrapConnNative(self);
    if (!conn || !conn->Connected()) error("conn:stmt_prepare: connection is closed");

    conn->SetState(s);
    conn->SetResultCallback(cb_name);
    conn->StmtPrepare(sql);
    MaybeReleaseOwnedConn(self);

    return inter::NativeToFakeluaNil(s);
}

// conn:stmt_execute(stmt_id, params, on_result)
// on_result(err, result) called when execute completes

CVar ConnStmtExecute(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "conn:stmt_execute", "stmt_id, params, and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    CVar a2 = inter::GetNativeArg(s, args, n, 2);

    uint32_t stmt_id = static_cast<uint32_t>(inter::CVarToInteger(a0, 0));
    std::string cb_name = CVarToString(a2);

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
                p.value = CVarToString(elem);
            }
            params.push_back(std::move(p));
        }
    }

    auto *conn = UnwrapConnNative(self);
    if (!conn || !conn->Connected()) error("conn:stmt_execute: connection is closed");

    conn->SetState(s);
    conn->SetResultCallback(cb_name);
    conn->StmtExecute(stmt_id, params);
    MaybeReleaseOwnedConn(self);

    return inter::NativeToFakeluaNil(s);
}

// conn:stmt_close(stmt_id)

CVar ConnStmtClose(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "conn:stmt_close", "stmt_id expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    uint32_t stmt_id = static_cast<uint32_t>(inter::CVarToInteger(a0, 0));

    auto *conn = UnwrapConnNative(self);
    if (!conn || !conn->Connected()) return inter::NativeToFakeluaNil(s);

    conn->StmtClose(stmt_id);
    MaybeReleaseOwnedConn(self);
    return inter::NativeToFakeluaNil(s);
}

// Pump the connection's network events, via runtime.tick()

// 连接关闭后 unwrap 返回空，于是自然变成 no-op。
void TickMysqlConnection(NativeObject *self, State *s) {
    auto *conn = UnwrapConnNative(self);
    if (!conn) return;
    if (conn->TickDepth() > 0) return;

    conn->SetState(s);
    conn->Tick();
    MaybeReleaseOwnedConn(self);
    MaybeReapPool(self);
}

// conn:close()

CVar ConnClose(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = UnwrapConnNative(self);
    if (conn) {
        if (conn->TickDepth() > 0) {
            conn->RequestClose();
            return inter::NativeToFakeluaNil(s);
        }
        conn->Close();
    }
    return inter::NativeToFakeluaNil(s);
}

// conn:ping() — send COM_PING heartbeat (for connection pool keepalive)

CVar ConnPing(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *conn = UnwrapConnNative(self);
    if (!conn || !conn->Connected()) return inter::NativeToFakeluaBool(s, false);
    bool sent = conn->Ping();
    return inter::NativeToFakeluaBool(s, sent);
}

// Registration

void RegisterMysqlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "mysql.connect", 2, false, MysqlConnect);
}

}// namespace fakelua::mysql
