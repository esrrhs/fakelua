#include "native/redis/native_redis.h"

#include "native/native_common.h"
#include "native/native_io_context.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "native/tls_util.h"
#include "state/state.h"
#include "var/var.h"

#include <boost/redis.hpp>
#include <boost/redis/resp3/type.hpp>
#include <boost/redis/src.hpp>

#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "util/string_util.h"

namespace fakelua::redis {

namespace br = boost::redis;
namespace resp3 = boost::redis::resp3;

static std::string CVarToString(CVar v) {
    return inter::FakeluaToNativeString(nullptr, v);
}

static void CallNamed(State *s, const std::string &name, CVar *args, int n) {
    if (!s || name.empty()) return;
    auto func = s->GetVM().GetFunction(name);
    if (func.Empty()) return;
    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) return;
    native::IoContext::DispatchScope scope(s->GetIoContext());
    inter::DispatchCall(s, addr, args, n, jit_type);
}

static bool IsAggregate(resp3::type t) {
    return t == resp3::type::array || t == resp3::type::push || t == resp3::type::set || t == resp3::type::map || t == resp3::type::attribute;
}

static CVar NodeToLua(State *s, const std::vector<resp3::node> &nodes, size_t &i);

static CVar AggregateToLua(State *s, const std::vector<resp3::node> &nodes, size_t &i) {
    const auto &n = nodes[i++];
    CVar tbl = table::TableHelper::CreateTable(s);
    if (n.data_type == resp3::type::map || n.data_type == resp3::type::attribute) {
        for (std::size_t k = 0; k < n.aggregate_size; ++k) {
            CVar key = NodeToLua(s, nodes, i);
            CVar val = NodeToLua(s, nodes, i);
            table::TableHelper::SetTable(s, tbl, key, val);
        }
        return tbl;
    }
    for (std::size_t k = 0; k < n.aggregate_size; ++k) {
        table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(k + 1), NodeToLua(s, nodes, i));
    }
    return tbl;
}

static CVar NodeToLua(State *s, const std::vector<resp3::node> &nodes, size_t &i) {
    if (i >= nodes.size()) return inter::NativeToFakeluaNil(s);
    const auto &n = nodes[i];
    if (IsAggregate(n.data_type)) {
        return AggregateToLua(s, nodes, i);
    }
    ++i;
    switch (n.data_type) {
        case resp3::type::null:
            return inter::NativeToFakeluaNil(s);
        case resp3::type::boolean:
            return inter::NativeToFakeluaBool(s, n.value == "t" || n.value == "true" || n.value == "1");
        case resp3::type::number: {
            int64_t v = 0;
            if (TryParseInt64(n.value, v)) return inter::NativeToFakeluaLonglong(s, v);
            return inter::NativeToFakeluaString(s, n.value);
        }
        case resp3::type::doublean: {
            double v = 0;
            if (TryParseDouble(n.value, v)) return inter::NativeToFakeluaDouble(s, v);
            return inter::NativeToFakeluaString(s, n.value);
        }
        default:
            return inter::NativeToFakeluaString(s, n.value);
    }
}

struct TickDepthGuard {
    int &depth;

    explicit TickDepthGuard(int &d) : depth(d) {
        ++depth;
    }

    ~TickDepthGuard() {
        if (depth > 0) --depth;
    }
};

static CVar ResponseToLua(State *s, const br::generic_response &resp) {
    if (resp.has_error()) {
        return inter::NativeToFakeluaString(s, resp.error().diagnostic);
    }
    const auto &nodes = resp.value();
    if (nodes.empty()) return inter::NativeToFakeluaNil(s);
    size_t i = 0;
    return NodeToLua(s, nodes, i);
}

struct CmdOp {
    br::request req;
    br::generic_response resp;
    std::string cb;
    std::string err;
    bool done = false;
};

class RedisConnection {
public:
    explicit RedisConnection(State *state)
        : io_(state->GetIoContext()), conn_(std::make_unique<br::connection>(io_.Get(), br::logger{br::logger::level::disabled})), lua_state_(state) {
    }

    ~RedisConnection() {
        Teardown();
    }

    RedisConnection(const RedisConnection &) = delete;
    RedisConnection &operator=(const RedisConnection &) = delete;

    void Connect(const std::string &host, uint16_t port, const std::string &username, const std::string &password, int db, int timeout_ms) {
        cfg_.addr.host = host;
        cfg_.addr.port = std::to_string(port);
        cfg_.health_check_interval = std::chrono::seconds{0};
        cfg_.reconnect_wait_interval = std::chrono::seconds{0};
        if (timeout_ms > 0) {
            auto d = std::chrono::milliseconds{timeout_ms};
            cfg_.resolve_timeout = d;
            cfg_.connect_timeout = d;
        }

        // Boost.Redis 默认握手会发 HELLO 3。Redis 4/5 没有 HELLO，握手会失败。
        // use_setup=true 且 setup 里自己放 AUTH/SELECT，可以完全跳过 HELLO。
        br::request setup;
        if (!password.empty()) {
            if (!username.empty() && username != "default") {
                setup.push("AUTH", username, password);
            } else {
                setup.push("AUTH", password);
            }
        }
        if (db > 0) {
            setup.push("SELECT", std::to_string(db));
        }
        cfg_.use_setup = true;
        cfg_.setup = std::move(setup);

        auto watch = life_.GetWatch();
        conn_->async_run(cfg_, [this, watch](boost::system::error_code ec) {
            if (!watch.Alive()) return;
            run_done_ = true;
            if (!connect_notified_) {
                NotifyConnect(ec ? ec.message() : std::string("connection closed"));
            }
        });
        // Do not async_exec PING here. Boost.Redis queues it on writer_cv_
        // (expires_at max). Closing the TCP socket does not wake that wait;
        // destroying the connection then deadlocks the Windows select reactor.
        // Start PING only after the TCP socket is actually open.
    }

    void Command(std::vector<std::string> parts, std::string cb) {
        if (closed_ || close_pending_ || !conn_) {
            ThrowFakeluaException("redis: connection is closed");
        }
        if (parts.empty()) {
            ThrowFakeluaException("redis command: empty argv");
        }
        auto op = std::make_unique<CmdOp>();
        op->cb = std::move(cb);
        if (parts.size() == 1) {
            op->req.push(parts[0]);
        } else {
            op->req.push_range(parts[0], std::next(parts.begin()), parts.end());
        }
        auto *raw = op.get();
        cmds_.push_back(std::move(op));
        auto watch = life_.GetWatch();
        conn_->async_exec(raw->req, raw->resp, [this, raw, watch](boost::system::error_code ec, std::size_t) {
            if (!watch.Alive()) return;
            raw->done = true;
            if (ec) raw->err = ec.message();
        });
    }

    void Tick() {
        // Nested tick (Lua runtime.tick() from a callback) must not poll or
        // dispatch: same re-entrancy guard as MysqlConnection::Tick.
        if (tick_depth_ > 0) return;
        TickDepthGuard guard(tick_depth_);
        if (!conn_) return;
        // Lua :close() from a callback only sets flags. Do not Poll this
        // connection until a later TickAll reaps it (ReadyToDestroy) —
        // Boost.Redis writer_cv_ (expires_at max) deadlocks Windows select.
        if (close_pending_) return;

        io_.Poll();
        if (close_pending_) return;
        if (pending_connect_) {
            pending_connect_ = false;
            DispatchConnect();
            if (close_pending_) return;
        }
        MaybeStartPing();
        for (auto &op: cmds_) {
            if (op && op->done && !op->cb.empty()) {
                DispatchCmd(*op);
                op->cb.clear();
            }
        }
        while (!cmds_.empty() && cmds_.front() && cmds_.front()->done && cmds_.front()->cb.empty()) {
            cmds_.pop_front();
        }
    }

    void Close() {
        closed_ = true;
        if (!connect_notified_) {
            NotifyConnect("closed");
        }
        // Lua :close() during a callback/tick must not abort/destroy: defer like
        // mysql RequestClose. Teardown closes the TCP socket (net style), not
        // boost::redis::connection::cancel().
        if (tick_depth_ > 0 || io_.InDispatch()) {
            close_pending_ = true;
            return;
        }
        Teardown();
    }

    int TickDepth() const {
        return tick_depth_;
    }

    bool ClosePending() const {
        return close_pending_;
    }

    void RequestClose() {
        closed_ = true;
        close_pending_ = true;
    }

    // First TickAll after a Lua :close() from a callback is flags-only.
    // AbortSocket on that same runtime.tick() deadlocks Windows select.
    // A later TickAll (ReadyToDestroy) tears down before anyone Polls.
    bool ReadyToDestroy() {
        if (!destroy_ready_) {
            destroy_ready_ = true;
            return false;
        }
        return true;
    }

    void SetConnectCallback(std::string name) {
        connect_cb_ = std::move(name);
    }

    void SetNativeObject(NativeObject *obj) {
        native_obj_ = obj;
    }

    bool Closed() const {
        return closed_;
    }

private:
    void Drain() {
        const int wait_ms = native::kWindowsAsio ? 1000 : 5000;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(wait_ms);
        while (std::chrono::steady_clock::now() < deadline) {
            if (io_.Poll() > 0) continue;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (io_.Poll() == 0) break;
        }
    }

    void AbortSocket() {
        if (!conn_) return;
        // next_layer() is deprecated but is the only way to reach the TCP socket.
        // connection::cancel() and socket.cancel() wait on this poll() thread.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        tls::CloseTcpSocket(conn_->next_layer().next_layer());
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    }

    void MaybeStartPing() {
        // Windows select: tcp::socket::is_open() is true as soon as the socket
        // exists, including during a connecting/refused handshake. async_exec
        // then waits on writer_cv_ (expires_at max); Teardown/reset segfaults
        // or deadlocks the reactor. Only PING after remote_endpoint succeeds.
        if (ping_started_ || closed_ || close_pending_ || !conn_) return;
        if (run_done_ || connect_notified_ || pending_connect_) return;
        boost::system::error_code ep_ec;
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        (void)conn_->next_layer().next_layer().remote_endpoint(ep_ec);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
        if (ep_ec) return;
        ping_started_ = true;
        ping_.req.push("PING");
        auto watch = life_.GetWatch();
        conn_->async_exec(ping_.req, ping_.resp, [this, watch](boost::system::error_code ec, std::size_t) {
            if (!watch.Alive()) return;
            if (ec) {
                NotifyConnect(ec.message());
                return;
            }
            NotifyConnect({});
        });
    }

    void Teardown() {
        if (!conn_) return;
        AbortSocket();
        Drain();
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(native::kWindowsAsio ? 1000 : 5000);
        while (!run_done_ && std::chrono::steady_clock::now() < deadline) {
            if (io_.Poll() == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        conn_.reset();
        Drain();
    }

    void NotifyConnect(std::string err) {
        if (connect_notified_) return;
        connect_notified_ = true;
        pending_connect_ = true;
        connect_err_ = std::move(err);
        if (connect_err_.empty()) ready_ = true;
    }

    void DispatchConnect() {
        TickDepthGuard guard(tick_depth_);
        native::IoContext::DispatchScope dispatch_scope(io_);
        if (close_pending_) return;
        if (!lua_state_ || connect_cb_.empty()) return;
        CVar args[3];
        args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);
        if (!connect_err_.empty()) {
            args[1] = inter::NativeToFakeluaString(lua_state_, connect_err_);
            args[2] = inter::NativeToFakeluaInt(lua_state_, 0);
        } else {
            args[1] = inter::NativeToFakeluaNil(lua_state_);
            args[2] = inter::NativeToFakeluaInt(lua_state_, 1);
        }
        CallNamed(lua_state_, connect_cb_, args, 3);
    }

    void DispatchCmd(CmdOp &op) {
        TickDepthGuard guard(tick_depth_);
        native::IoContext::DispatchScope dispatch_scope(io_);
        if (close_pending_) return;
        if (!lua_state_ || op.cb.empty()) return;
        CVar args[3];
        args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);
        if (!op.err.empty() || op.resp.has_error()) {
            std::string msg = op.err;
            if (msg.empty() && op.resp.has_error()) msg = op.resp.error().diagnostic;
            if (msg.empty()) msg = "redis command failed";
            args[1] = inter::NativeToFakeluaString(lua_state_, msg);
            args[2] = inter::NativeToFakeluaNil(lua_state_);
        } else {
            args[1] = inter::NativeToFakeluaNil(lua_state_);
            args[2] = ResponseToLua(lua_state_, op.resp);
        }
        CallNamed(lua_state_, op.cb, args, 3);
    }

    native::IoContext &io_;
    br::config cfg_;
    std::unique_ptr<br::connection> conn_;
    CmdOp ping_;
    std::deque<std::unique_ptr<CmdOp>> cmds_;
    std::string connect_cb_;
    std::string connect_err_;
    State *lua_state_ = nullptr;
    NativeObject *native_obj_ = nullptr;
    bool connect_notified_ = false;
    bool pending_connect_ = false;
    bool ready_ = false;
    bool closed_ = false;
    bool close_pending_ = false;
    bool ping_started_ = false;
    bool run_done_ = false;
    bool destroy_ready_ = false;
    int tick_depth_ = 0;
    native::LifeToken life_;
};

struct RedisWrappers {
    std::vector<NativeObject *> conns;
};

static RedisConnection *Unwrap(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<RedisConnection *>(self->GetInt("__redis_conn__", 0));
}

static void RegisterWrapper(State *s, NativeObject *nat) {
    if (!s || !nat) return;
    nat->SetInt("__redis_state__", reinterpret_cast<int64_t>(s));
    s->GetModuleState<RedisWrappers>().conns.push_back(nat);
}

static void UnregisterWrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt("__redis_state__", 0));
    nat->SetInt("__redis_state__", 0);
    if (!st) return;
    auto *ws = st->TryGetModuleState<RedisWrappers>();
    if (!ws) return;
    ws->conns.erase(std::remove(ws->conns.begin(), ws->conns.end(), nat), ws->conns.end());
}

static void MaybeReleaseOwnedConn(NativeObject *self) {
    if (!self) return;
    auto *conn = Unwrap(self);
    if (!conn || conn->TickDepth() > 0 || !conn->ClosePending()) return;
    if (!conn->ReadyToDestroy()) return;
    conn->Close();
    self->SetInt("__redis_conn__", 0);
    delete conn;
}

void TickAll(State *s) {
    if (!s) return;
    auto *ws = s->TryGetModuleState<RedisWrappers>();
    if (!ws) return;
    auto conns = ws->conns;
    // Reap a previous tick's Lua :close() BEFORE any Poll. Leaving Boost.Redis
    // writer_cv_ (expires_at max) in the shared io_context makes Windows select
    // wait forever on the next connection's Tick().
    for (auto *nat: conns) {
        MaybeReleaseOwnedConn(nat);
    }
    conns = ws->conns;
    for (auto *nat: conns) {
        auto *c = Unwrap(nat);
        if (!c) continue;
        if (c->TickDepth() > 0) continue;
        c->Tick();
        MaybeReleaseOwnedConn(nat);
    }
}

void OnStateDeleted(State *s) {
    if (!s) return;
    auto *ws = s->TryGetModuleState<RedisWrappers>();
    if (!ws) return;
    auto wrappers = std::move(ws->conns);
    for (auto *nat: wrappers) {
        if (!nat) continue;
        nat->SetInt("__redis_state__", 0);
        s->GetNativeObjectManager().DestroyGroup(nat->GetGroupId());
    }
}

static std::vector<std::string> TableToArgv(State *s, CVar tbl) {
    std::vector<std::string> parts;
    int64_t len = table::TableHelper::GetTableLen(tbl);
    for (int64_t i = 1; i <= len; ++i) {
        parts.push_back(CVarToString(table::TableHelper::GetTableInt(s, tbl, i)));
    }
    return parts;
}

static CVar ConnCommand(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "command", "argv table and callback expected");
    auto *conn = Unwrap(self);
    if (!conn || conn->Closed()) ThrowFakeluaException("redis: connection is closed");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string cb = CVarToString(a1);
    if (cb.empty()) ThrowBadArgument(2, "command", "callback function expected");
    std::vector<std::string> parts;
    if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
        parts = TableToArgv(s, a0);
    } else {
        ThrowBadArgument(1, "command", "argv table expected");
    }
    conn->Command(std::move(parts), std::move(cb));
    return inter::NativeToFakeluaNil(s);
}

static CVar ConnClose(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *conn = Unwrap(self);
    if (conn) {
        if (conn->TickDepth() > 0) {
            conn->RequestClose();
            return inter::NativeToFakeluaNil(s);
        }
        conn->Close();
    }
    return inter::NativeToFakeluaNil(s);
}

static CVar RedisConnect(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "redis.connect", "config table and callback expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string host = "127.0.0.1";
    uint16_t port = 6379;
    std::string user;
    std::string password;
    int db = 0;
    int timeout_ms = 1000;
    if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
        CVar v = table::TableHelper::GetTableStrId(s, a0, "host");
        if (v.type_ != static_cast<int>(VarType::Nil)) host = CVarToString(v);
        v = table::TableHelper::GetTableStrId(s, a0, "port");
        if (v.type_ != static_cast<int>(VarType::Nil)) port = static_cast<uint16_t>(inter::CVarToInteger(v, 6379));
        v = table::TableHelper::GetTableStrId(s, a0, "user");
        if (v.type_ != static_cast<int>(VarType::Nil)) user = CVarToString(v);
        v = table::TableHelper::GetTableStrId(s, a0, "password");
        if (v.type_ != static_cast<int>(VarType::Nil)) password = CVarToString(v);
        v = table::TableHelper::GetTableStrId(s, a0, "db");
        if (v.type_ != static_cast<int>(VarType::Nil)) db = static_cast<int>(inter::CVarToInteger(v, 0));
        v = table::TableHelper::GetTableStrId(s, a0, "timeout_ms");
        if (v.type_ != static_cast<int>(VarType::Nil)) timeout_ms = static_cast<int>(inter::CVarToInteger(v, 1000));
    } else {
        ThrowBadArgument(1, "redis.connect", "config must be a table");
    }
    std::string cb = CVarToString(a1);
    if (cb.empty()) ThrowBadArgument(2, "redis.connect", "callback function expected");

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, "redis_connection");
    auto *conn = new RedisConnection(s);
    conn->SetConnectCallback(std::move(cb));
    conn->SetNativeObject(nat);
    nat->SetInt("__redis_conn__", reinterpret_cast<int64_t>(conn));
    nat->RegisterMethod("command", ConnCommand);
    nat->RegisterMethod("close", ConnClose);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterWrapper(self);
        auto *c = Unwrap(self);
        if (c) {
            if (c->TickDepth() > 0) {
                c->RequestClose();
            } else {
                self->SetInt("__redis_conn__", 0);
                c->Close();
                delete c;
            }
        }
    });
    RegisterWrapper(s, nat);
    conn->Connect(host, port, user, password, db, timeout_ms);
    return inter::NativeToFakeluaNativeObject(s, nat);
}

void RegisterRedisLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "redis.connect", 2, false, RedisConnect);
}

}// namespace fakelua::redis
