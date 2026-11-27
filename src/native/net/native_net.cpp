#include "native/net/native_net.h"
#include "native/native_common.h"
#include "native/net/net_event.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "state/state.h"
#include "util/logging.h"
#include "var/var.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <format>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua::net {

// NetObject — 包装引擎 + fakelua 回调
// 所有可变状态存 C++ 侧（fakelua 无状态设计）

struct NetObject {
    State *state = nullptr;
    std::string dispatch_name;// Lua 回调函数名（统一入口）
    JITType dispatch_jit = JIT_TCC;
    bool is_server = false;

    // 可变状态全部存 C++ 侧
    std::vector<std::string> events;// 事件记录（有上限，防止长跑泄漏）
    std::string last_server_data;   // server 最后收到的数据
    std::string last_client_data;   // client 最后收到的数据
    int server_connid = -1;         // server 端连接 ID
    int conn_count = 0;             // 连接计数
    int recv_count = 0;             // 收包计数

    std::unique_ptr<net::TcpServer> server;
    std::unique_ptr<net::TcpClient> client;
    std::unique_ptr<net::UdpSocket> udp;

    std::string last_peer_ip;
    uint16_t last_peer_port = 0;

    // tick 回调里 Lua 可能 :close()，不能立刻 delete this。延后到 tick 返回。
    int tick_depth = 0;
    bool close_pending = false;
    std::exception_ptr pending_exception;
    native::LifeToken life;

    // 回调上下文（native dispatch 期间）里的 send 先入队，由本轮 tick 派发完后统一
    // 泵出——消除"回调里 send 是否生效"的平台相关行为（事件驱动改轮询驱动）。
    struct PendingSend {
        enum class Kind { Server, Client, UdpPeer, UdpTo } kind;
        int connid = -1;// Server: 目标连接
        std::string data;
        std::string ip;   // UdpTo: 目标地址
        uint16_t port = 0;// UdpTo: 目标端口
    };

    std::vector<PendingSend> pending_sends;

    static constexpr size_t kMaxEvents = 1024;
    static constexpr size_t kMaxPendingSends = 4096;

    bool InDispatchContext() const;
    bool DoSend(const PendingSend &ps);
    bool SendOrQueue(PendingSend ps);
    void FlushPendingSends();
    void PushEvent(std::string_view ev);
    void HandleCallbackReturn(const CVar &ret, int connid);
    void DispatchEventFor(const ConnEvent &ev);
};

// 辅助：从 CVar 提取字符串

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

// C++ → Lua 回调派发（核心：按函数名查找，不存闭包）
// 回调是纯函数：接收事件参数，返回可选指令（echo 等）

static CVar CallLuaEvent(State *state, const std::string &func_name, JITType preferred_jit, const char *type, int connid, const char *data, size_t len, int reason) {
    if (!state || func_name.empty()) return CVar{static_cast<int>(VarType::Nil)};

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
    return DispatchNamedLuaCallback(state, func_name, args, 5, preferred_jit);
}

// 处理回调返回值：解析 Lua 返回的指令
// 返回值格式：nil（无操作）或 Multi {command, arg1, arg2, ...}
// 支持的指令：
//   "echo", data → 将 data 发回来源连接
//   "close"      → 延后关闭本对象（tick 回调内安全）

void NetObject::HandleCallbackReturn(const CVar &ret, int connid) {
    if (close_pending) return;
    // 检查是否为 Multi（多返回值）
    if (ret.type_ != static_cast<int>(VarType::Multi)) return;

    // 通过 GetMultiCVarElement 访问（避免依赖 VarMulti 完整定义）
    CVar first = inter::GetMultiCVarElement(ret, 0);
    std::string cmd = CVarToString(first);
    if (cmd == "close") {
        close_pending = true;
        return;
    }
    if (cmd == "echo") {
        CVar data_var = inter::GetMultiCVarElement(ret, 1);
        std::string echo_data = CVarToString(data_var);
        if (udp) {
            if (udp->Connected()) {
                SendOrQueue({PendingSend::Kind::UdpPeer, -1, std::move(echo_data)});
            } else {
                SendOrQueue({PendingSend::Kind::UdpTo, -1, std::move(echo_data), last_peer_ip, last_peer_port});
            }
        } else if (is_server) {
            SendOrQueue({PendingSend::Kind::Server, connid, std::move(echo_data)});
        } else {
            SendOrQueue({PendingSend::Kind::Client, -1, std::move(echo_data)});
        }
    }
}

// send 的统一实现：按 PendingSend 的 kind 写 socket。返回 false 表示引擎侧
// 拒绝（连接未就绪/已关闭），调用方记日志，不再静默。
bool NetObject::DoSend(const PendingSend &ps) {
    using Kind = PendingSend::Kind;
    switch (ps.kind) {
        case Kind::Server:
            if (server) return server->Send(ps.connid, ps.data.data(), ps.data.size());
            return false;
        case Kind::Client:
            if (client) return client->Send(ps.data.data(), ps.data.size());
            return false;
        case Kind::UdpPeer:
            if (udp && udp->Connected()) return udp->Send(ps.data.data(), ps.data.size());
            return false;
        case Kind::UdpTo:
            if (udp) return udp->SendTo(ps.data.data(), ps.data.size(), ps.ip, ps.port);
            return false;
    }
    return false;
}

// 判断是否处于 C++→Lua 回调派发上下文（on_event / mysql/http 回调等）。
bool NetObject::InDispatchContext() const {
    return state && state->GetIoContext().InDispatch();
}

// send 的统一入口：回调上下文里不直接写 socket，而是入队，由本轮 tick 派发完后
// 统一泵出——消除"回调里 send 偶尔静默丢失"的平台相关行为；非派发上下文立即发送。
bool NetObject::SendOrQueue(PendingSend ps) {
    if (InDispatchContext()) {
        if (pending_sends.size() >= kMaxPendingSends) {
            LOG_WARN(state, "net", "pending send queue full ({}), dropping {} bytes", pending_sends.size(), ps.data.size());
            return false;
        }
        pending_sends.push_back(std::move(ps));
        return true;
    }
    return DoSend(ps);
}

// 派发完成后统一泵出积压的 send。此时已脱离 DispatchScope，直接写 socket 是安全的。
void NetObject::FlushPendingSends() {
    if (pending_sends.empty()) return;
    auto sends = std::move(pending_sends);
    pending_sends.clear();
    for (const auto &ps: sends) {
        if (!DoSend(ps)) {
            LOG_WARN(state, "net", "deferred send failed (kind={} connid={} len={})", static_cast<int>(ps.kind), ps.connid, ps.data.size());
        }
    }
}

// 从 NativeObject 取出 NetObject*

static NetObject *Unwrap(NativeObject *self) {
    return reinterpret_cast<NetObject *>(self->GetInt("__net_obj__", 0));
}

// 本 State 上活着的 net NativeObject，DeleteState 时统一关掉 socket。存在 State 上而不是
// 这里的 static map：后者是全进程一份，多个线程各跑自己的 State 时会并发改同一个容器。
struct NetWrappers {
    std::vector<NativeObject *> list;
};

static void RegisterNetWrapper(State *s, NativeObject *nat) {
    if (!s || !nat) return;
    s->GetModuleState<NetWrappers>().list.push_back(nat);
}

static void UnregisterNetWrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt("__net_state__", 0));
    nat->SetInt("__net_state__", 0);
    if (!st) return;
    auto &v = st->GetModuleState<NetWrappers>().list;
    v.erase(std::remove(v.begin(), v.end(), nat), v.end());
}

void OnStateDeleted(State *s) {
    if (!s) return;
    auto *ws = s->TryGetModuleState<NetWrappers>();
    if (!ws) return;
    auto wrappers = std::move(ws->list);
    for (auto *nat: wrappers) {
        if (!nat) continue;
        nat->SetInt("__net_state__", 0);
        s->GetNativeObjectManager().DestroyGroup(nat->GetGroupId());
    }
}

// 事件记录加上限：超过 kMaxEvents 时丢弃最旧的事件，防止长跑无界增长。
void NetObject::PushEvent(std::string_view ev) {
    if (events.size() >= kMaxEvents) {
        // 批量丢弃前半部分，避免每个事件都 O(n) 搬移
        events.erase(events.begin(), events.begin() + kMaxEvents / 2);
    }
    events.emplace_back(ev);
}

static void ReleaseNetObject(NativeObject *self);

// NativeObject 方法实现

// server:dispatch(func_name) — 注册统一回调函数名
static CVar NetDispatch(NativeObject *self, State *s, CVar *args, int n) {
    auto *obj = Unwrap(self);
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
    obj->dispatch_jit = s->CurrentJit();
    return inter::NativeToFakeluaNil(s);
}

// 派发单条事件给 Lua（在 tick() 同步上下文里执行，保留 tick_depth/close_pending 语义）
void NetObject::DispatchEventFor(const ConnEvent &ev) {
    if (close_pending) return;
    switch (ev.kind) {
        case EventKind::Connect:
            conn_count++;
            server_connid = ev.conn_id;
            PushEvent("conn");
            {
                CVar ret = CallLuaEvent(state, dispatch_name, dispatch_jit, "conn", ev.conn_id, nullptr, 0, 0);
                HandleCallbackReturn(ret, ev.conn_id);
            }
            break;
        case EventKind::Recv:
            recv_count++;
            last_peer_ip = ev.peer_ip;
            last_peer_port = ev.peer_port;
            if (is_server) last_server_data = ev.data;
            else
                last_client_data = ev.data;
            PushEvent("recv");
            LOG_DEBUG(state, "net", "{} recv: connid={} len={}", is_server ? "server" : "client", ev.conn_id, ev.data.size());
            {
                CVar ret = CallLuaEvent(state, dispatch_name, dispatch_jit, "recv", ev.conn_id, ev.data.data(), ev.data.size(), 0);
                HandleCallbackReturn(ret, ev.conn_id);
            }
            break;
        case EventKind::Close:
            PushEvent("close");
            {
                CVar ret = CallLuaEvent(state, dispatch_name, dispatch_jit, "close", ev.conn_id, nullptr, 0, 0);
                HandleCallbackReturn(ret, ev.conn_id);
            }
            break;
    }
}

// 驱动 IO 处理，由 runtime.tick() 调用
// tick_depth + close_pending 语义保持不变。
// 关闭后的对象 unwrap 返回空，于是自然变成 no-op。
static void TickNetObject(NativeObject *self) {
    auto *obj = Unwrap(self);
    if (!obj) return;
    if (obj->tick_depth > 0) return;
    if (obj->close_pending && !obj->pending_exception) {
        ReleaseNetObject(self);
        return;
    }

    auto finish_tick = [self, obj]() {
        if (obj->tick_depth < 0) obj->tick_depth = 0;
        if (obj->tick_depth == 0 && obj->close_pending) {
            ReleaseNetObject(self);
        }
    };

    obj->tick_depth++;
    try {
        if (obj->udp && obj->udp->Running()) {
            obj->udp->DrainEventsWith([obj](const ConnEvent &ev) { obj->DispatchEventFor(ev); });
        } else if (obj->is_server && obj->server && obj->server->Running()) {
            obj->server->DrainEventsWith([obj](const ConnEvent &ev) { obj->DispatchEventFor(ev); });
        } else if (!obj->is_server && obj->client) {
            obj->client->DrainEventsWith([obj](const ConnEvent &ev) { obj->DispatchEventFor(ev); });
        }
    } catch (...) {
        obj->pending_exception = nullptr;
        obj->tick_depth--;
        obj->FlushPendingSends();
        finish_tick();
        throw;
    }
    if (obj->pending_exception) {
        auto ex = obj->pending_exception;
        obj->pending_exception = nullptr;
        obj->tick_depth--;
        obj->FlushPendingSends();
        finish_tick();
        std::rethrow_exception(ex);
    }
    obj->tick_depth--;
    // 回调期间积压的 send 在这里统一泵出（socket 写在派发上下文之外执行）。
    obj->FlushPendingSends();
    finish_tick();
}

void TickAll(State *s) {
    if (!s) return;
    // 拷一份再遍历：派发进 Lua 的回调里可能 net.client() 或 del_native_group，两者都会
    // 改动这个 vector。快照里已经销毁的对象 unwrap 拿到空，tick 自己就是 no-op。
    auto *ws = s->TryGetModuleState<NetWrappers>();
    if (!ws) return;
    auto wrappers = ws->list;
    for (auto *nat: wrappers) {
        TickNetObject(nat);
    }
    auto post_wrappers = ws->list;
    for (auto *nat: post_wrappers) {
        auto *obj = Unwrap(nat);
        if (!obj || obj->tick_depth > 0) continue;
        if (obj->pending_exception) {
            TickNetObject(nat);
        } else if (obj->close_pending) {
            ReleaseNetObject(nat);
        }
    }
}

// server:send(connid, data) / client:send(data)
// 返回 true 表示已被引擎接受（直接写出，或回调上下文中入队待泵出）。
static CVar NetSend(NativeObject *self, State *s, CVar *args, int n) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaBool(s, false);

    if (obj->udp) {
        if (obj->udp->Connected()) {
            if (n < 1) ThrowBadArgument(1, "send", "data expected");
            std::string data = CVarToString(inter::GetNativeArg(s, args, n, 0));
            bool ok = obj->SendOrQueue({NetObject::PendingSend::Kind::UdpPeer, -1, std::move(data)});
            return inter::NativeToFakeluaBool(s, ok);
        }
        if (n < 3) ThrowBadArgument(1, "send", "data, ip and port expected");
        std::string data = CVarToString(inter::GetNativeArg(s, args, n, 0));
        std::string ip = CVarToString(inter::GetNativeArg(s, args, n, 1));
        int64_t port_val = CheckIntegerArg(inter::GetNativeArg(s, args, n, 2), 3, "send");
        if (port_val <= 0 || port_val > 65535) {
            ThrowFakeluaException(std::format("net: port {} out of range (1-65535)", port_val));
        }
        bool ok = obj->SendOrQueue({NetObject::PendingSend::Kind::UdpTo, -1, std::move(data), std::move(ip), static_cast<uint16_t>(port_val)});
        return inter::NativeToFakeluaBool(s, ok);
    }

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
        bool ok = obj->SendOrQueue({NetObject::PendingSend::Kind::Server, connid, std::move(data)});
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
        bool ok = obj->SendOrQueue({NetObject::PendingSend::Kind::Client, -1, std::move(data)});
        return inter::NativeToFakeluaBool(s, ok);
    }

    return inter::NativeToFakeluaBool(s, false);
}

// server:close_connection(connid) — 关闭单条连接（C++ 已有能力，现导出到 Lua）
static CVar NetCloseConnection(NativeObject *self, State *s, CVar *args, int n) {
    auto *obj = Unwrap(self);
    if (!obj || !obj->is_server || !obj->server || obj->close_pending) return inter::NativeToFakeluaBool(s, false);
    if (n < 1) ThrowBadArgument(1, "close_connection", "connid expected");
    int connid = static_cast<int>(CheckIntegerArg(inter::GetNativeArg(s, args, n, 0), 1, "close_connection"));
    if (!obj->server->CloseConnection(connid, false)) return inter::NativeToFakeluaBool(s, false);
    obj->PushEvent("close");
    struct CloseDispatchGuard {
        NativeObject *self;
        NetObject *obj;
        explicit CloseDispatchGuard(NativeObject *nat, NetObject *o) : self(nat), obj(o) {
            ++obj->tick_depth;
        }
        ~CloseDispatchGuard() {
            if (obj->tick_depth > 0) --obj->tick_depth;
            if (obj->tick_depth == 0) {
                obj->FlushPendingSends();
                if (obj->close_pending) {
                    ReleaseNetObject(self);
                }
            }
        }
    } guard(self, obj);
    native::IoContext::DispatchScope dispatch_scope(s->GetIoContext());
    CVar ret = CallLuaEvent(obj->state, obj->dispatch_name, obj->dispatch_jit, "close", connid, nullptr, 0, 0);
    obj->HandleCallbackReturn(ret, connid);
    return inter::NativeToFakeluaBool(s, true);
}

// 释放 NetObject 及其持有的 socket 引擎。可重入：__net_obj__ 已为 0 时是 no-op。
static void ReleaseNetObject(NativeObject *self) {
    auto *obj = reinterpret_cast<NetObject *>(self->GetInt("__net_obj__", 0));
    if (!obj) return;

    if (obj->server) {
        obj->server->Stop();
        obj->server.reset();
    }
    if (obj->client) {
        obj->client->Disconnect();
        obj->client.reset();
    }
    if (obj->udp) {
        obj->udp->Stop();
        obj->udp.reset();
    }
    delete obj;
    self->SetInt("__net_obj__", 0);
}

// server:close() / client:close()
static CVar NetClose(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);// 已关闭，no-op

    if (obj->tick_depth > 0) {
        obj->close_pending = true;
        return inter::NativeToFakeluaNil(s);
    }

    // 停止 socket、释放 NetObject、清空指针。保留 NativeObject 壳，后续方法 no-op。
    ReleaseNetObject(self);
    return inter::NativeToFakeluaNil(s);
}

// 状态读取方法：get_events / get_last_data / get_conn_count / get_recv_count / get_connid
static CVar NetGetEvents(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    CVar multi = inter::AllocMultiCVar(s, static_cast<int>(obj->events.size()));
    for (size_t i = 0; i < obj->events.size(); ++i) {
        inter::SetMultiCVarElement(multi, static_cast<int>(i), inter::NativeToFakeluaString(s, obj->events[i]));
    }
    return multi;
}

static CVar NetGetLastData(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    if (obj->is_server) {
        return inter::NativeToFakeluaString(s, obj->last_server_data);
    } else {
        return inter::NativeToFakeluaString(s, obj->last_client_data);
    }
}

static CVar NetGetConnCount(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->conn_count);
}

static CVar NetGetRecvCount(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->recv_count);
}

static CVar NetGetConnId(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->server_connid);
}

static CVar NetGetPort(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj || !obj->udp) return inter::NativeToFakeluaNil(s);
    return inter::NativeToFakeluaInt(s, obj->udp->BoundPort());
}

static CVar NetGetPeer(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = Unwrap(self);
    if (!obj || !obj->udp) return inter::NativeToFakeluaNil(s);
    CVar multi = inter::AllocMultiCVar(s, 2);
    inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaString(s, obj->last_peer_ip));
    inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(s, obj->last_peer_port));
    return multi;
}

// 工厂函数

// 从 Lua table 读取字符串键字段（仿 os.time 的模式）
static int64_t GetTableField(State *s, CVar tbl_cvar, const char *key_name, int64_t default_val) {
    if (tbl_cvar.type_ != static_cast<int>(VarType::Table) || !tbl_cvar.data_.t) return default_val;
    CVar v = table::TableHelper::GetTableStrId(s, tbl_cvar, key_name);
    return inter::CVarToInteger(v, default_val);
}

static std::string GetTableFieldString(State *s, CVar tbl_cvar, const char *key_name, const std::string &default_val = "") {
    if (tbl_cvar.type_ != static_cast<int>(VarType::Table) || !tbl_cvar.data_.t) return default_val;
    CVar v = table::TableHelper::GetTableStrId(s, tbl_cvar, key_name);
    if (v.type_ == static_cast<int>(VarType::Nil)) return default_val;
    std::string res = CVarToString(v);
    return res.empty() && v.type_ == static_cast<int>(VarType::Nil) ? default_val : res;
}

static net::FramerType ParseFramerType(const std::string &framer_str) {
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
    if (framer_str == "websocket" || framer_str == "ws") {
        return net::FramerType::WebSocket;
    }
    return net::FramerType::Header4BigEndian;
}

static net::NetConfig ParseConfig(State *s, CVar *args, int n) {
    net::NetConfig cfg;
    if (n < 1) return cfg;

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table)) return cfg;

    cfg.ip = GetTableFieldString(s, a0, "ip", "127.0.0.1");
    {
        // 端口超出 uint16 范围时拒绝，避免静默截断（如 70000 → 4464）
        int64_t port_val = GetTableField(s, a0, "port", 8888);
        if (port_val <= 0 || port_val > 65535) {
            ThrowFakeluaException(std::format("net: port {} out of range (1-65535)", port_val));
        }
        cfg.port = static_cast<uint16_t>(port_val);
    }
    {
        int64_t maxc = GetTableField(s, a0, "maxconn", 1000);
        if (maxc < 1 || maxc > 100000) {
            ThrowFakeluaException(std::format("net: maxconn {} out of range (1-100000)", maxc));
        }
        cfg.max_conn = static_cast<int>(maxc);
    }
    cfg.backlog = CheckInt32Range(GetTableField(s, a0, "backlog", 128), "net", "backlog", 1);
    {
        int64_t sbs = GetTableField(s, a0, "send_buf_size", 0);
        if (sbs == 0) sbs = GetTableField(s, a0, "sendbuf", 0);
        if (sbs > 0) {
            if (sbs > static_cast<int64_t>(std::numeric_limits<int>::max())) {
                ThrowFakeluaException(std::format("net: send_buf_size {} out of range", sbs));
            }
            cfg.send_buf_size = static_cast<int>(sbs);
        }

        int64_t rbs = GetTableField(s, a0, "recv_buf_size", 0);
        if (rbs == 0) rbs = GetTableField(s, a0, "recvbuf", 0);
        if (rbs > 0) {
            if (rbs > static_cast<int64_t>(std::numeric_limits<int>::max())) {
                ThrowFakeluaException(std::format("net: recv_buf_size {} out of range", rbs));
            }
            cfg.recv_buf_size = static_cast<int>(rbs);
        }

        int64_t mpl = GetTableField(s, a0, "max_packet_len", 0);
        if (mpl > 0) {
            if (mpl > static_cast<int64_t>(std::numeric_limits<int>::max())) {
                ThrowFakeluaException(std::format("net: max_packet_len {} out of range", mpl));
            }
            cfg.max_packet_len = static_cast<int>(mpl);
        }
    }
    {
        int64_t flen = GetTableField(s, a0, "fixed_len", 0);
        if (flen == 0) flen = GetTableField(s, a0, "fixed_packet_len", 0);
        if (flen != 0) {
            cfg.fixed_packet_len = CheckInt32Range(flen, "net", "fixed_packet_len", 1);
        }
    }
    cfg.no_delay = GetTableField(s, a0, "nodelay", 1) != 0;
    cfg.keep_alive = GetTableField(s, a0, "keepalive", 1) != 0;


    std::string framer_str = GetTableFieldString(s, a0, "framer", "");
    cfg.framer = ParseFramerType(framer_str);

    cfg.custom_parser_name = GetTableFieldString(s, a0, "parser", "");
    if (cfg.custom_parser_name.empty()) {
        cfg.custom_parser_name = GetTableFieldString(s, a0, "custom_parser", "");
    }
    if (!cfg.custom_parser_name.empty()) {
        cfg.framer = net::FramerType::Custom;
    }

    cfg.ws_path = GetTableFieldString(s, a0, "ws_path", "/");
    if (cfg.ws_path.empty()) cfg.ws_path = "/";
    cfg.ws_host = GetTableFieldString(s, a0, "ws_host", "");
    cfg.ws_origin = GetTableFieldString(s, a0, "ws_origin", "");
    auto check_ws_field = [](const std::string &val, const char *what) {
        for (unsigned char c: val) {
            if (c < 0x20 || c == 0x7f || c == ' ') {
                ThrowFakeluaException(std::format("net: invalid {} (contains control or space)", what));
            }
        }
    };
    check_ws_field(cfg.ws_path, "ws_path");
    check_ws_field(cfg.ws_host, "ws_host");
    check_ws_field(cfg.ws_origin, "ws_origin");
    check_ws_field(cfg.ip, "ip");

    CVar tls_var = table::TableHelper::GetTableStrId(s, a0, "tls");
    if (tls_var.type_ == static_cast<int>(VarType::Bool)) {
        cfg.tls = AsVar(tls_var).GetBool();
    } else if (tls_var.type_ == static_cast<int>(VarType::Int)) {
        cfg.tls = tls_var.data_.i != 0;
    } else if (tls_var.type_ != static_cast<int>(VarType::Nil)) {
        std::string ts = CVarToString(tls_var);
        cfg.tls = !(ts.empty() || ts == "0" || ts == "false" || ts == "disable");
    }
    CVar tv = table::TableHelper::GetTableStrId(s, a0, "tls_verify");
    if (tv.type_ == static_cast<int>(VarType::Bool)) {
        cfg.tls_verify = AsVar(tv).GetBool();
    } else if (tv.type_ == static_cast<int>(VarType::Int)) {
        cfg.tls_verify = tv.data_.i != 0;
    } else if (tv.type_ != static_cast<int>(VarType::Nil)) {
        std::string ts = CVarToString(tv);
        cfg.tls_verify = !(ts.empty() || ts == "0" || ts == "false");
    }
    cfg.tls_cert = GetTableFieldString(s, a0, "cert", "");
    if (cfg.tls_cert.empty()) cfg.tls_cert = GetTableFieldString(s, a0, "tls_cert", "");
    cfg.tls_key = GetTableFieldString(s, a0, "key", "");
    if (cfg.tls_key.empty()) cfg.tls_key = GetTableFieldString(s, a0, "tls_key", "");
    cfg.tls_ca = GetTableFieldString(s, a0, "tls_ca", "");

    return cfg;
}

// 辅助：建立 Lua 自定义解包函数的 C++ parser 桥接
static void SetupLuaCustomParser(State *s, NetObject *obj, net::NetConfig &cfg, const std::string &parser_name) {
    if (parser_name.empty() || !obj) return;

    cfg.framer = net::FramerType::Custom;
    cfg.custom_parser_name = parser_name;

    // 创建捕获 parser_name 的 C++ 回调，从 CircularBuffer 窥视数据，调用 Lua parser 解包
    // 按值捕获 max_packet_len 与登记时的脚本引擎，避免悬空或走错后端
    const int max_pkt = cfg.max_packet_len;
    const JITType parser_jit = s->CurrentJit();
    auto watch = obj->life.GetWatch();
    cfg.custom_parser_fn = [s, obj, watch, parser_name, parser_jit, max_pkt](net::CircularBuffer &buf, const char *&out_payload, uint32_t &out_len) -> bool {
        if (!watch.Alive() || obj->close_pending || obj->pending_exception || buf.Empty()) return false;

        // 用 buf 自己的暂存区：写在 lambda 体内的 static 是所有连接、所有 State 共享同一
        // 份，按缓冲区各存一份既没有竞争，也不会解析另一条连接就把这里的数据冲掉。
        auto &peek_buf = buf.HeaderScratch();
        auto &payload_buf = buf.PayloadScratch();
        // 限制窥视上限为 max_packet_len，避免半包时每 tick O(缓冲) 全量分配/拷贝
        size_t total = std::min(buf.Size(), static_cast<size_t>(max_pkt));
        if (total == 0) return false;
        if (peek_buf.size() < total) peek_buf.resize(total);
        buf.Peek(peek_buf.data(), total);

        struct ParserTickGuard {
            NetObject *obj;
            explicit ParserTickGuard(NetObject *o) : obj(o) {
                ++obj->tick_depth;
            }
            ~ParserTickGuard() {
                if (obj->tick_depth > 0) --obj->tick_depth;
            }
        } tick_guard(obj);
        native::IoContext::DispatchScope dispatch_scope(s->GetIoContext());

        // 调用 Lua 解包函数: parser(buffer_str)
        // 必须在此处拦截 C++ 异常，绝不让异常穿过 C 编译的 libevent 栈帧：
        // 异常暂存在 obj->pending_exception，并在不消费 buf 的前提下返回 true，
        // 使 TryParsePacket 标记 out_error=true 并干净关闭连接，随后由 TickNetObject 在外层重抛。
        std::string parsed_str;
        size_t consumed = 0;
        try {
            CVar in_arg = inter::NativeToFakeluaString(s, std::string(peek_buf.data(), total));
            CVar args[1] = {in_arg};
            CVar res = DispatchNamedLuaCallback(s, parser_name, args, 1, parser_jit);

            if (obj->close_pending || res.type_ == static_cast<int>(VarType::Nil)) {
                return false;
            }

            if (res.type_ == static_cast<int>(VarType::Multi)) {
                CVar p_var = inter::GetMultiCVarElement(res, 0);
                parsed_str = CVarToString(p_var);
                CVar c_var = inter::GetMultiCVarElement(res, 1);
                if (c_var.type_ != static_cast<int>(VarType::Nil)) {
                    int64_t raw_consumed = inter::CVarToInteger(c_var, static_cast<int64_t>(parsed_str.size()));
                    consumed = raw_consumed > 0 ? static_cast<size_t>(raw_consumed) : 0;
                } else {
                    consumed = parsed_str.size();
                }
            } else {
                parsed_str = CVarToString(res);
                consumed = parsed_str.size();
            }
        } catch (...) {
            if (!obj->pending_exception) {
                obj->pending_exception = std::current_exception();
            }
            out_payload = nullptr;
            out_len = 0;
            return true;
        }

        if (consumed == 0 && parsed_str.empty()) {
            return false;
        }

        if (consumed > total) {
            consumed = total;
        }
        buf.Skip(consumed);

        if (payload_buf.size() < parsed_str.size()) payload_buf.resize(parsed_str.size());
        if (!parsed_str.empty()) {
            std::memcpy(payload_buf.data(), parsed_str.data(), parsed_str.size());
        }
        out_payload = payload_buf.data();
        out_len = static_cast<uint32_t>(parsed_str.size());
        return true;
    };
}

static CVar CreateNetServer(State *s, net::NetConfig cfg, const char *type_name) {
    if (cfg.tls && (cfg.tls_cert.empty() || cfg.tls_key.empty())) {
        ThrowFakeluaException(std::format("{}: tls=true requires cert and key", type_name));
    }

    net::NetInit();

    auto *obj = new NetObject();
    obj->state = s;
    obj->is_server = true;
    if (!cfg.custom_parser_name.empty()) {
        SetupLuaCustomParser(s, obj, cfg, cfg.custom_parser_name);
    }
    obj->server = std::make_unique<net::TcpServer>(cfg, s);
    obj->server->Start();

    if (!obj->server->Running()) {
        delete obj;
        net::NetShutdown();
        LOG_ERROR(s, "net", "{}: failed to listen on port {}", type_name, cfg.port);
        ThrowFakeluaException(std::format("{}: failed to listen on port {}", type_name, cfg.port));
    }
    LOG_DEBUG(s, "net", "{}: listening on port {}", type_name, cfg.port);

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, type_name);
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->SetInt("__net_state__", reinterpret_cast<int64_t>(s));
    RegisterNetWrapper(s, nat);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterNetWrapper(self);
        net::NetShutdown();
        ReleaseNetObject(self);
    });
    nat->RegisterMethod("dispatch", NetDispatch);
    nat->RegisterMethod("send", NetSend);
    nat->RegisterMethod("close", NetClose);
    nat->RegisterMethod("close_connection", NetCloseConnection);
    nat->RegisterMethod("get_events", NetGetEvents);
    nat->RegisterMethod("get_last_data", NetGetLastData);
    nat->RegisterMethod("get_conn_count", NetGetConnCount);
    nat->RegisterMethod("get_recv_count", NetGetRecvCount);
    nat->RegisterMethod("get_connid", NetGetConnId);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

static CVar CreateNetClient(State *s, net::NetConfig cfg, const char *type_name) {
    net::NetInit();

    auto *obj = new NetObject();
    obj->state = s;
    obj->is_server = false;
    if (!cfg.custom_parser_name.empty()) {
        SetupLuaCustomParser(s, obj, cfg, cfg.custom_parser_name);
    }
    obj->client = std::make_unique<net::TcpClient>(cfg, s);
    obj->client->Connect();

    LOG_DEBUG(s, "net", "{}: connecting to {}:{}", type_name, cfg.ip, cfg.port);

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, type_name);
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->SetInt("__net_state__", reinterpret_cast<int64_t>(s));
    RegisterNetWrapper(s, nat);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterNetWrapper(self);
        net::NetShutdown();
        ReleaseNetObject(self);
    });
    nat->RegisterMethod("dispatch", NetDispatch);
    nat->RegisterMethod("send", NetSend);
    nat->RegisterMethod("close", NetClose);
    nat->RegisterMethod("get_events", NetGetEvents);
    nat->RegisterMethod("get_last_data", NetGetLastData);
    nat->RegisterMethod("get_conn_count", NetGetConnCount);
    nat->RegisterMethod("get_recv_count", NetGetRecvCount);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// net.server(config) → server object
static CVar NetServer(State *s, CVar *args, int n) {
    return CreateNetServer(s, ParseConfig(s, args, n), "net_server");
}

// net.client(config) → client object
static CVar NetClient(State *s, CVar *args, int n) {
    return CreateNetClient(s, ParseConfig(s, args, n), "net_client");
}

// net.ws_server(config) → WebSocket 服务端（等价于 framer="websocket"）
static CVar NetWsServer(State *s, CVar *args, int n) {
    net::NetConfig cfg = ParseConfig(s, args, n);
    cfg.framer = net::FramerType::WebSocket;
    cfg.custom_parser_name.clear();
    cfg.custom_parser_fn = nullptr;
    return CreateNetServer(s, cfg, "net_ws_server");
}

// net.ws_client(config) → WebSocket 客户端
static CVar NetWsClient(State *s, CVar *args, int n) {
    net::NetConfig cfg = ParseConfig(s, args, n);
    cfg.framer = net::FramerType::WebSocket;
    cfg.custom_parser_name.clear();
    cfg.custom_parser_fn = nullptr;
    return CreateNetClient(s, cfg, "net_ws_client");
}

static net::NetConfig ParseUdpConfig(State *s, CVar *args, int n, bool connected) {
    net::NetConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 0;
    if (n < 1) {
        if (connected) ThrowFakeluaException("net.udp_client: config table expected");
        return cfg;
    }
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table)) {
        ThrowFakeluaException("net.udp: config table expected");
    }
    cfg.ip = GetTableFieldString(s, a0, "ip", "127.0.0.1");
    int64_t port_val = GetTableField(s, a0, "port", connected ? 0 : 0);
    if (connected) {
        if (port_val <= 0 || port_val > 65535) {
            ThrowFakeluaException(std::format("net.udp_client: port {} out of range (1-65535)", port_val));
        }
    } else if (port_val < 0 || port_val > 65535) {
        ThrowFakeluaException(std::format("net.udp_server: port {} out of range (0-65535)", port_val));
    }
    cfg.port = static_cast<uint16_t>(port_val);
    int64_t mpl = GetTableField(s, a0, "max_packet_len", 0);
    if (mpl > 0) cfg.max_packet_len = static_cast<int>(mpl);
    return cfg;
}

static CVar CreateUdp(State *s, net::NetConfig cfg, bool connected, const char *type_name) {
    net::NetInit();

    auto *obj = new NetObject();
    obj->state = s;
    obj->is_server = !connected;
    obj->udp = std::make_unique<net::UdpSocket>(cfg, s, connected);
    if (!obj->udp->Start()) {
        delete obj;
        net::NetShutdown();
        LOG_ERROR(s, "net", "{}: failed to {} {}:{}", type_name, connected ? "connect to" : "bind", cfg.ip, cfg.port);
        ThrowFakeluaException(std::format("{}: failed to {} {}:{}", type_name, connected ? "connect to" : "bind", cfg.ip, cfg.port));
    }
    LOG_DEBUG(s, "net", "{}: {} {}:{} (bound {})", type_name, connected ? "connected to" : "bound", cfg.ip, cfg.port, obj->udp->BoundPort());

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, type_name);
    nat->SetInt("__net_obj__", reinterpret_cast<int64_t>(obj));
    nat->SetInt("__net_state__", reinterpret_cast<int64_t>(s));
    RegisterNetWrapper(s, nat);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterNetWrapper(self);
        net::NetShutdown();
        ReleaseNetObject(self);
    });
    nat->RegisterMethod("dispatch", NetDispatch);
    nat->RegisterMethod("send", NetSend);
    nat->RegisterMethod("close", NetClose);
    nat->RegisterMethod("get_events", NetGetEvents);
    nat->RegisterMethod("get_last_data", NetGetLastData);
    nat->RegisterMethod("get_recv_count", NetGetRecvCount);
    nat->RegisterMethod("get_port", NetGetPort);
    nat->RegisterMethod("get_peer", NetGetPeer);
    return inter::NativeToFakeluaNativeObject(s, nat);
}

static CVar NetUdpServer(State *s, CVar *args, int n) {
    return CreateUdp(s, ParseUdpConfig(s, args, n, false), false, "net_udp_server");
}

static CVar NetUdpClient(State *s, CVar *args, int n) {
    return CreateUdp(s, ParseUdpConfig(s, args, n, true), true, "net_udp_client");
}

// 注册

void RegisterNetLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "net.server", 1, false, NetServer);
    RegisterNativeFunction(s, "net.client", 1, false, NetClient);
    RegisterNativeFunction(s, "net.ws_server", 1, false, NetWsServer);
    RegisterNativeFunction(s, "net.ws_client", 1, false, NetWsClient);
    RegisterNativeFunction(s, "net.udp_server", 1, false, NetUdpServer);
    RegisterNativeFunction(s, "net.udp_client", 1, false, NetUdpClient);
}

}// namespace fakelua::net
