#include "native/net/native_net.h"
#include "native/net/net_internal.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "var/var.h"

#include <cstring>
#include <memory>
#include <string>
#include <string_view>

namespace fakelua::net {

// ─────────────────────────────────────────────────────────────────────────────
// NetObject — 包装引擎 + fakelua 回调
// 所有可变状态存 C++ 侧（fakelua 无状态设计）
// ─────────────────────────────────────────────────────────────────────────────

struct NetObject {
    State *state = nullptr;
    std::string dispatch_name;           // Lua 回调函数名（统一入口）
    bool is_server = false;

    // 可变状态全部存 C++ 侧
    std::vector<std::string> events;     // 事件记录
    std::string last_server_data;        // server 最后收到的数据
    std::string last_client_data;        // client 最后收到的数据
    int server_connid = -1;              // server 端连接 ID
    int conn_count = 0;                  // 连接计数
    int recv_count = 0;                  // 收包计数

    std::unique_ptr<net::TcpServer> server;
    std::unique_ptr<net::TcpClient> client;
};

// ─────────────────────────────────────────────────────────────────────────────
// 辅助：从 CVar 提取字符串
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
// C++ → Lua 回调派发（核心：按函数名查找，不存闭包）
// 回调是纯函数：接收事件参数，返回可选指令（echo 等）
// ─────────────────────────────────────────────────────────────────────────────

static CVar call_lua_event(State *state, const std::string &func_name,
                           const char *type, int connid,
                           const char *data, size_t len, int reason) {
    if (func_name.empty()) return CVar{static_cast<int>(VarType::Nil)};

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
        // 构造参数: (type, connid, data, len, reason)
        CVar args[5];
        args[0] = inter::NativeToFakeluaString(state, type);
        args[1] = inter::NativeToFakeluaInt(state, connid);
        if (data && len > 0) {
            args[2] = inter::NativeToFakeluaString(state, std::string(data, len));
            args[3] = inter::NativeToFakeluaInt(state, static_cast<int64_t>(len));
        } else {
            CVar nil{};
            nil.type_ = static_cast<int>(VarType::Nil);
            args[2] = nil;
            args[3] = nil;
        }
        args[4] = inter::NativeToFakeluaInt(state, reason);

        return inter::DispatchCall(addr, args, 5, jit_type);
    } else {
        // 回退：尝试原生函数
        auto *entry = state->GetVM().FindNativeFunction(func_name);
        if (entry && entry->callback) {
            CVar args[5];
            args[0] = inter::NativeToFakeluaString(state, type);
            args[1] = inter::NativeToFakeluaInt(state, connid);
            if (data && len > 0) {
                args[2] = inter::NativeToFakeluaString(state, std::string(data, len));
                args[3] = inter::NativeToFakeluaInt(state, static_cast<int64_t>(len));
            } else {
                CVar nil{};
                nil.type_ = static_cast<int>(VarType::Nil);
                args[2] = nil;
                args[3] = nil;
            }
            args[4] = inter::NativeToFakeluaInt(state, reason);
            return entry->callback(state, args, 5);
        }
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

// ─────────────────────────────────────────────────────────────────────────────
// 处理回调返回值：解析 Lua 返回的指令
// 返回值格式：nil（无操作）或 Multi {command, arg1, arg2, ...}
// 支持的指令：
//   "echo", data → 将 data 发回来源连接
// ─────────────────────────────────────────────────────────────────────────────

static void handle_callback_return(NetObject *obj, const CVar &ret, int connid) {
    // 检查是否为 Multi（多返回值）
    if (ret.type_ != static_cast<int>(VarType::Multi)) return;

    // 通过 GetMultiCVarElement 访问（避免依赖 VarMulti 完整定义）
    CVar first = inter::GetMultiCVarElement(ret, 0);
    std::string cmd = cvar_to_string(first);
    if (cmd == "echo") {
        CVar data_var = inter::GetMultiCVarElement(ret, 1);
        std::string echo_data = cvar_to_string(data_var);
        if (obj->is_server) {
            obj->server->send(connid, echo_data.data(), echo_data.size());
        } else {
            obj->client->send(echo_data.data(), echo_data.size());
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

// server:dispatch(func_name) — 注册统一回调函数名
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
    return inter::NativeToFakeluaNil(s);
}

// server:tick() — 驱动 IO 处理
static CVar net_tick(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);

    if (obj->is_server && obj->server && obj->server->running()) {
        obj->server->tick(
            // on_conn
            [obj](int connid) {
                obj->conn_count++;
                obj->server_connid = connid;
                obj->events.push_back("conn");
                CVar ret = call_lua_event(obj->state, obj->dispatch_name, "conn", connid, nullptr, 0, 0);
                handle_callback_return(obj, ret, connid);
            },
            // on_recv
            [obj](int connid, const char *data, size_t len) {
                obj->recv_count++;
                obj->last_server_data.assign(data, len);
                obj->events.push_back("recv");
                CVar ret = call_lua_event(obj->state, obj->dispatch_name, "recv", connid, data, len, 0);
                handle_callback_return(obj, ret, connid);
            },
            // on_close
            [obj](int connid) {
                obj->events.push_back("close");
                CVar ret = call_lua_event(obj->state, obj->dispatch_name, "close", connid, nullptr, 0, 0);
                handle_callback_return(obj, ret, connid);
            });
    } else if (!obj->is_server && obj->client) {
        obj->client->tick(
            // on_recv
            [obj](const char *data, size_t len) {
                obj->recv_count++;
                obj->last_client_data.assign(data, len);
                obj->events.push_back("recv");
                CVar ret = call_lua_event(obj->state, obj->dispatch_name, "recv", 0, data, len, 0);
                handle_callback_return(obj, ret, 0);
            },
            // on_close
            [obj]() {
                obj->events.push_back("close");
                CVar ret = call_lua_event(obj->state, obj->dispatch_name, "close", 0, nullptr, 0, 0);
                handle_callback_return(obj, ret, 0);
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

// 状态读取方法：get_events / get_last_data / get_conn_count / get_recv_count / get_connid
static CVar net_get_events(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    CVar multi = inter::AllocMultiCVar(s, static_cast<int>(obj->events.size()));
    for (size_t i = 0; i < obj->events.size(); ++i) {
        inter::SetMultiCVarElement(multi, static_cast<int>(i), inter::NativeToFakeluaString(s, obj->events[i]));
    }
    return multi;
}

static CVar net_get_last_data(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    if (obj->is_server) {
        return inter::NativeToFakeluaString(s, obj->last_server_data);
    } else {
        return inter::NativeToFakeluaString(s, obj->last_client_data);
    }
}

static CVar net_get_conn_count(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->conn_count);
}

static CVar net_get_recv_count(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->recv_count);
}

static CVar net_get_connid(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->server_connid);
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

static std::string get_table_field_string(State *s, CVar tbl_cvar, const char *key_name, const std::string &default_val = "") {
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
        if (finish) {
            std::string res = cvar_to_string(r);
            return (res.empty() && r.type_ == static_cast<int>(VarType::Nil)) ? default_val : res;
        }
    }
    for (const auto &qd : t->quick_data_) {
        if (qd.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(qd.key) == key_name) {
            return cvar_to_string(qd.val);
        }
    }
    if (t->nodes_ && t->bucket_count_ > 0 && t->active_list_) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (entry.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(entry.key) == key_name) {
                return cvar_to_string(entry.val);
            }
        }
    }
    return default_val;
}

static net::FramerType parse_framer_type(const std::string &framer_str) {
    if (framer_str.empty() || framer_str == "header4" || framer_str == "header4_be" || framer_str == "be4") {
        return net::FramerType::Header4BigEndian;
    }
    if (framer_str == "header4_le" || framer_str == "le4") {
        return net::FramerType::Header4LittleEndian;
    }
    if (framer_str == "header2" || framer_str == "header2_be" || framer_str == "be2") {
        return net::FramerType::Header2BigEndian;
    }
    if (framer_str == "header2_le" || framer_str == "le2") {
        return net::FramerType::Header2LittleEndian;
    }
    if (framer_str == "line" || framer_str == "lines" || framer_str == "delimiter") {
        return net::FramerType::LineDelimiter;
    }
    if (framer_str == "fixed" || framer_str == "fixed_length") {
        return net::FramerType::FixedLength;
    }
    if (framer_str == "raw" || framer_str == "raw_stream" || framer_str == "none") {
        return net::FramerType::RawStream;
    }
    if (framer_str == "custom") {
        return net::FramerType::Custom;
    }
    return net::FramerType::Header4BigEndian;
}

static net::NetConfig parse_config(State *s, CVar *args, int n) {
    net::NetConfig cfg;
    if (n < 1) return cfg;

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table)) return cfg;

    cfg.ip = get_table_field_string(s, a0, "ip", "127.0.0.1");
    cfg.port = static_cast<uint16_t>(get_table_field(s, a0, "port", 8888));
    cfg.max_conn = static_cast<int>(get_table_field(s, a0, "maxconn", 1000));
    cfg.backlog = static_cast<int>(get_table_field(s, a0, "backlog", 128));
    cfg.fixed_packet_len = static_cast<int>(get_table_field(s, a0, "fixed_len", 0));
    if (cfg.fixed_packet_len == 0) {
        cfg.fixed_packet_len = static_cast<int>(get_table_field(s, a0, "fixed_packet_len", 0));
    }
    cfg.non_blocking = get_table_field(s, a0, "nonblocking", 1) != 0;
    cfg.no_delay = get_table_field(s, a0, "nodelay", 1) != 0;
    cfg.keep_alive = get_table_field(s, a0, "keepalive", 1) != 0;

    std::string framer_str = get_table_field_string(s, a0, "framer", "");
    cfg.framer = parse_framer_type(framer_str);

    cfg.custom_parser_name = get_table_field_string(s, a0, "parser", "");
    if (cfg.custom_parser_name.empty()) {
        cfg.custom_parser_name = get_table_field_string(s, a0, "custom_parser", "");
    }
    if (!cfg.custom_parser_name.empty()) {
        cfg.framer = net::FramerType::Custom;
    }

    return cfg;
}

// 辅助：建立 Lua 自定义解包函数的 C++ parser 桥接
static void setup_lua_custom_parser(State *s, net::NetConfig &cfg, const std::string &parser_name) {
    if (parser_name.empty()) return;

    cfg.framer = net::FramerType::Custom;
    cfg.custom_parser_name = parser_name;

    // 创建捕获 parser_name 的 C++ 回调，从 CircularBuffer 窥视数据，调用 Lua parser 解包
    cfg.custom_parser_fn = [s, parser_name](net::CircularBuffer &buf, const char *&out_payload, uint32_t &out_len) -> bool {
        if (buf.empty()) return false;

        static thread_local std::vector<char> peek_buf;
        static thread_local std::vector<char> payload_buf;
        size_t total = buf.size();
        if (peek_buf.size() < total) peek_buf.resize(total);
        buf.peek(peek_buf.data(), total);

        // 调用 Lua 解包函数: parser(buffer_str)
        // 期望返回值:
        // 1. (packet_str, consumed_bytes)
        // 2. 或仅 packet_str（默认 consumed_bytes = packet_str.length()）
        // 3. 或 nil（表示数据尚未就绪，需等待更多数据）
        auto func = s->GetVM().GetFunction(parser_name);
        void *addr = nullptr;
        JITType jit_type = JIT_TCC;
        if (!func.Empty()) {
            addr = func.GetAddr(JIT_TCC);
            if (!addr) {
                addr = func.GetAddr(JIT_GCC);
                jit_type = JIT_GCC;
            }
        }

        CVar res{static_cast<int>(VarType::Nil)};
        CVar in_arg = inter::NativeToFakeluaString(s, std::string(peek_buf.data(), total));

        if (addr) {
            CVar args[1] = {in_arg};
            res = inter::DispatchCall(addr, args, 1, jit_type);
        } else {
            auto *entry = s->GetVM().FindNativeFunction(parser_name);
            if (entry && entry->callback) {
                CVar args[1] = {in_arg};
                res = entry->callback(s, args, 1);
            }
        }

        if (res.type_ == static_cast<int>(VarType::Nil)) {
            return false;
        }

        std::string parsed_str;
        size_t consumed = 0;

        if (res.type_ == static_cast<int>(VarType::Multi)) {
            CVar p_var = inter::GetMultiCVarElement(res, 0);
            parsed_str = cvar_to_string(p_var);
            CVar c_var = inter::GetMultiCVarElement(res, 1);
            if (c_var.type_ != static_cast<int>(VarType::Nil)) {
                consumed = static_cast<size_t>(inter::CVarToInteger(c_var, static_cast<int64_t>(parsed_str.size())));
            } else {
                consumed = parsed_str.size();
            }
        } else {
            parsed_str = cvar_to_string(res);
            consumed = parsed_str.size();
        }

        if (consumed == 0 && parsed_str.empty()) {
            return false;
        }

        if (consumed > total) {
            consumed = total;
        }
        buf.skip(consumed);

        if (payload_buf.size() < parsed_str.size()) payload_buf.resize(parsed_str.size());
        if (!parsed_str.empty()) {
            std::memcpy(payload_buf.data(), parsed_str.data(), parsed_str.size());
        }
        out_payload = payload_buf.data();
        out_len = static_cast<uint32_t>(parsed_str.size());
        return true;
    };
}

// net.server(config) → server object
static CVar net_server(State *s, CVar *args, int n) {
    net::NetConfig cfg = parse_config(s, args, n);
    if (!cfg.custom_parser_name.empty()) {
        setup_lua_custom_parser(s, cfg, cfg.custom_parser_name);
    }

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
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "net_server");
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->RegisterMethod("dispatch", net_dispatch);
    nat->RegisterMethod("tick", net_tick);
    nat->RegisterMethod("send", net_send);
    nat->RegisterMethod("close", net_close);
    nat->RegisterMethod("get_events", net_get_events);
    nat->RegisterMethod("get_last_data", net_get_last_data);
    nat->RegisterMethod("get_conn_count", net_get_conn_count);
    nat->RegisterMethod("get_recv_count", net_get_recv_count);
    nat->RegisterMethod("get_connid", net_get_connid);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// net.client(config) → client object
static CVar net_client(State *s, CVar *args, int n) {
    net::NetConfig cfg = parse_config(s, args, n);
    if (!cfg.custom_parser_name.empty()) {
        setup_lua_custom_parser(s, cfg, cfg.custom_parser_name);
    }

    net::net_init();

    auto *obj = new NetObject();
    obj->state = s;
    obj->is_server = false;
    obj->client = std::make_unique<net::TcpClient>(cfg);
    obj->client->connect();

    // 包装为 NativeObject
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "net_client");
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->RegisterMethod("dispatch", net_dispatch);
    nat->RegisterMethod("tick", net_tick);
    nat->RegisterMethod("send", net_send);
    nat->RegisterMethod("close", net_close);
    nat->RegisterMethod("get_events", net_get_events);
    nat->RegisterMethod("get_last_data", net_get_last_data);
    nat->RegisterMethod("get_conn_count", net_get_conn_count);
    nat->RegisterMethod("get_recv_count", net_get_recv_count);

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

} // namespace fakelua::net
