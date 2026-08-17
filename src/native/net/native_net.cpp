#include "native/net/native_net.h"
#include "native/net/net_internal.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "var/var.h"

#include <cstring>
#include <memory>
#include <string>
#include <string_view>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// NetObject — 包装 liblu 风格的引擎 + fakelua 回调
// ─────────────────────────────────────────────────────────────────────────────

struct NetObject {
    State *state = nullptr;
    std::string dispatch_name;           // Lua 回调函数名（统一入口）
    bool is_server = false;
    CVar state_table{};                  // 用户传入的可变状态 table（dispatch 时绑定）

    std::unique_ptr<net::TcpServer> server;
    std::unique_ptr<net::TcpClient> client;
};

static constexpr int64_t kNetGroup = 999998;

// ─────────────────────────────────────────────────────────────────────────────
// C++ → Lua 回调派发（核心：按函数名查找，不存闭包）
// ─────────────────────────────────────────────────────────────────────────────

static void call_lua_event(State *state, const std::string &func_name,
                           const char *type, int connid,
                           const char *data, size_t len, int reason,
                           const CVar *state_table) {
    if (func_name.empty()) return;

    // 优先查找 JIT 编译的 Lua 函数
    auto func = state->GetVM().GetFunction(func_name);
    void *addr = nullptr;
    JITType jit_type = JIT_TCC;
    if (!func.Empty()) {
        addr = func.GetAddr(JIT_TCC);
        if (!addr) {
            addr = func.GetAddr(JIT_GCC);
            jit_type = JIT_GCC;
        }
    }

    if (addr) {
        // 构造参数: (type, connid, data, len, reason[, state_table])
        CVar args[6];
        args[0] = inter::NativeToFakeluaString(state, type);
        args[1] = inter::NativeToFakeluaInt(state, connid);
        if (data && len > 0) {
            args[2] = inter::NativeToFakeluaString(state, std::string(data, len));
            args[3] = inter::NativeToFakeluaInt(state, static_cast<int64_t>(len));
        } else if (data) {
            args[2] = inter::NativeToFakeluaString(state, "");
            args[3] = inter::NativeToFakeluaInt(state, 0);
        } else {
            args[2] = inter::NativeToFakeluaString(state, "");
            args[3] = inter::NativeToFakeluaInt(state, 0);
        }
        args[4] = inter::NativeToFakeluaInt(state, reason);
        // 如果用户绑定了 state table，作为第 6 个参数传入
        if (state_table && state_table->type_ == static_cast<int>(VarType::Table)) {
            args[5] = *state_table;
            inter::DispatchCall(addr, args, 6, jit_type);
        } else {
            inter::DispatchCall(addr, args, 5, jit_type);
        }
    } else {
        // 回退：尝试原生函数
        auto *entry = state->GetVM().FindNativeFunction(func_name);
        if (entry && entry->callback) {
            CVar args[6];
            args[0] = inter::NativeToFakeluaString(state, type);
            args[1] = inter::NativeToFakeluaInt(state, connid);
            if (data && len > 0) {
                args[2] = inter::NativeToFakeluaString(state, std::string(data, len));
                args[3] = inter::NativeToFakeluaInt(state, static_cast<int64_t>(len));
            } else {
                args[2] = inter::NativeToFakeluaString(state, "");
                args[3] = inter::NativeToFakeluaInt(state, 0);
            }
            args[4] = inter::NativeToFakeluaInt(state, reason);
            if (state_table && state_table->type_ == static_cast<int>(VarType::Table)) {
                args[5] = *state_table;
                entry->callback(state, args, 6);
            } else {
                entry->callback(state, args, 5);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 从 NativeObject 取出 NetObject*
// ─────────────────────────────────────────────────────────────────────────────

static NetObject *unwrap(NativeObject *self) {
    return reinterpret_cast<NetObject *>(self->GetInt("__net_obj__", 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeObject 方法实现
// ─────────────────────────────────────────────────────────────────────────────

// server:dispatch(func_name [, state_table]) — 注册统一回调函数名 + 可选状态
static CVar net_dispatch(NativeObject *self, State *s, CVar *args, int n) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);

    std::string fname;
    if (n >= 1) {
        CVar a0 = inter::GetNativeArg(s, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::String) && a0.data_.s) {
            fname = std::string(a0.data_.s->Str());
        } else if (a0.type_ == static_cast<int>(VarType::StringId)) {
            if (a0.data_.i) {
                const char *ptr = reinterpret_cast<const char *>(a0.data_.i);
                int sz = *reinterpret_cast<const int *>(ptr);
                fname.assign(ptr + 8, sz);
            }
        } else {
            ThrowBadArgument(1, "dispatch", "string expected");
        }
    }
    obj->dispatch_name = std::move(fname);

    // 可选的第 2 个参数：用户状态 table（在回调中原样传回）
    if (n >= 2) {
        CVar a1 = inter::GetNativeArg(s, args, n, 1);
        obj->state_table = a1;
    } else {
        obj->state_table = CVar{};
    }

    return inter::NativeToFakeluaNil(s);
}

// server:tick() — 驱动 IO 处理
static CVar net_tick(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);

    const CVar *st = (obj->state_table.type_ == static_cast<int>(VarType::Table)) ? &obj->state_table : nullptr;

    if (obj->is_server && obj->server && obj->server->running()) {
        obj->server->tick(
            // on_conn
            [obj, st](int connid) {
                call_lua_event(obj->state, obj->dispatch_name, "conn", connid, nullptr, 0, 0, st);
            },
            // on_recv
            [obj, st](int connid, const char *data, size_t len) {
                call_lua_event(obj->state, obj->dispatch_name, "recv", connid, data, len, 0, st);
            },
            // on_close
            [obj, st](int connid) {
                call_lua_event(obj->state, obj->dispatch_name, "close", connid, nullptr, 0, 0, st);
            });
    } else if (!obj->is_server && obj->client) {
        obj->client->tick(
            // on_recv
            [obj, st](const char *data, size_t len) {
                call_lua_event(obj->state, obj->dispatch_name, "recv", 0, data, len, 0, st);
            },
            // on_close
            [obj, st]() {
                call_lua_event(obj->state, obj->dispatch_name, "close", 0, nullptr, 0, 0, st);
            });
    }

    return inter::NativeToFakeluaNil(s);
}

// server:send(connid, data) / client:send(data)
static CVar net_send(NativeObject *self, State *s, CVar *args, int n) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaBool(s, false);

    if (obj->is_server && obj->server) {
        // server: send(connid, data)
        if (n < 2) ThrowBadArgument(1, "send", "connid and data expected");
        int connid = static_cast<int>(CheckIntegerArg(inter::GetNativeArg(s, args, n, 0), 1, "send"));
        std::string data = [&] {
            CVar a = inter::GetNativeArg(s, args, n, 1);
            if (a.type_ == static_cast<int>(VarType::String) && a.data_.s) {
                return std::string(a.data_.s->Str());
            } else if (a.type_ == static_cast<int>(VarType::StringId) && a.data_.i) {
                const char *ptr = reinterpret_cast<const char *>(a.data_.i);
                int sz = *reinterpret_cast<const int *>(ptr);
                return std::string(ptr + 8, sz);
            }
            return std::string();
        }();
        bool ok = obj->server->send(connid, data.data(), data.size());
        return inter::NativeToFakeluaBool(s, ok);
    } else if (!obj->is_server && obj->client) {
        // client: send(data)
        if (n < 1) ThrowBadArgument(1, "send", "data expected");
        std::string data = [&] {
            CVar a = inter::GetNativeArg(s, args, n, 0);
            if (a.type_ == static_cast<int>(VarType::String) && a.data_.s) {
                return std::string(a.data_.s->Str());
            } else if (a.type_ == static_cast<int>(VarType::StringId) && a.data_.i) {
                const char *ptr = reinterpret_cast<const char *>(a.data_.i);
                int sz = *reinterpret_cast<const int *>(ptr);
                return std::string(ptr + 8, sz);
            }
            return std::string();
        }();
        bool ok = obj->client->send(data.data(), data.size());
        return inter::NativeToFakeluaBool(s, ok);
    }

    return inter::NativeToFakeluaBool(s, false);
}

// server:close() / client:close()
static CVar net_close(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);

    if (obj->server) {
        obj->server->stop();
        obj->server.reset();
    }
    if (obj->client) {
        obj->client->disconnect();
        obj->client.reset();
    }

    delete obj;
    // 清空 NativeObject 中的指针，防止重复关闭
    self->SetInt("__net_obj__", 0);
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// 工厂函数
// ─────────────────────────────────────────────────────────────────────────────

// 从 Lua table 读取字符串键字段（仿 os.time 的模式）
static int64_t get_table_field(State *s, CVar tbl_cvar, const char *key_name, int64_t default_val) {
    if (tbl_cvar.type_ != static_cast<int>(VarType::Table) || !tbl_cvar.data_.t) return default_val;
    VarTable *t = tbl_cvar.data_.t;
    int64_t id = s->GetConstString().Alloc(key_name);
    CVar key{static_cast<int>(VarType::StringId)};
    key.data_.i = id;

    if (t->spec_get) {
        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
        auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
        bool finish = false;
        CVar r = get_fn(t, key, &finish);
        if (finish) return inter::CVarToInteger(r, default_val);
    }
    // 回退：遍历 quick_data
    for (const auto &qd : t->quick_data_) {
        if (qd.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(qd.key) == key_name) {
            return inter::CVarToInteger(qd.val, default_val);
        }
    }
    // 回退：遍历哈希节点
    if (t->nodes_ && t->bucket_count_ > 0 && t->active_list_) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (entry.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(entry.key) == key_name) {
                return inter::CVarToInteger(entry.val, default_val);
            }
        }
    }
    return default_val;
}

static net::NetConfig parse_config(State *s, CVar *args, int n) {
    net::NetConfig cfg;
    if (n < 1) return cfg;

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table)) return cfg;

    cfg.port = static_cast<uint16_t>(get_table_field(s, a0, "port", 8888));
    cfg.max_conn = static_cast<int>(get_table_field(s, a0, "maxconn", 1000));
    cfg.backlog = static_cast<int>(get_table_field(s, a0, "backlog", 128));
    cfg.non_blocking = get_table_field(s, a0, "nonblocking", 1) != 0;
    cfg.no_delay = get_table_field(s, a0, "nodelay", 1) != 0;
    cfg.keep_alive = get_table_field(s, a0, "keepalive", 1) != 0;

    return cfg;
}

// net.server(config) → server object
static CVar net_server(State *s, CVar *args, int n) {
    net::NetConfig cfg = parse_config(s, args, n);

    net::net_init();

    auto *obj = new NetObject();
    obj->state = s;
    obj->is_server = true;
    obj->server = std::make_unique<net::TcpServer>(cfg);
    obj->server->start();

    if (!obj->server->running()) {
        delete obj;
        ThrowFakeluaException(std::format("net.server: failed to listen on port {}", cfg.port));
    }

    // 包装为 NativeObject
    int64_t gid = NativeObjectManager::Instance().CreateGroup(kNetGroup);
    auto *nat = NativeObjectManager::Instance().Create(gid, "net_server");
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->RegisterMethod("dispatch", net_dispatch);
    nat->RegisterMethod("tick", net_tick);
    nat->RegisterMethod("send", net_send);
    nat->RegisterMethod("close", net_close);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// net.client(config) → client object
static CVar net_client(State *s, CVar *args, int n) {
    net::NetConfig cfg = parse_config(s, args, n);

    net::net_init();

    auto *obj = new NetObject();
    obj->state = s;
    obj->is_server = false;
    obj->client = std::make_unique<net::TcpClient>(cfg);
    obj->client->connect();

    // 包装为 NativeObject
    int64_t gid = NativeObjectManager::Instance().CreateGroup(kNetGroup);
    auto *nat = NativeObjectManager::Instance().Create(gid, "net_client");
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->RegisterMethod("dispatch", net_dispatch);
    nat->RegisterMethod("tick", net_tick);
    nat->RegisterMethod("send", net_send);
    nat->RegisterMethod("close", net_close);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ─────────────────────────────────────────────────────────────────────────────
// 注册
// ─────────────────────────────────────────────────────────────────────────────

void RegisterNetLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "net.server", 1, false, net_server);
    RegisterNativeFunction(s, "net.client", 1, false, net_client);
}

}// namespace fakelua
