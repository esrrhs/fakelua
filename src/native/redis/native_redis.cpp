#include "native/redis/native_redis.h"

#include "native/native_common.h"
#include "native/native_io_context.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "state/state.h"
#include "util/string_util.h"
#include "var/var.h"

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace fakelua::redis {

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

static constexpr int kMaxRedisReplyDepth = 64;

static CVar ReplyToLua(State *s, const redisReply *r, int depth);

static CVar ArrayToLua(State *s, const redisReply *r, int depth) {
    if (depth > kMaxRedisReplyDepth) {
        ThrowFakeluaException("redis: nested reply too deep");
    }
    CVar tbl = table::TableHelper::CreateTable(s);
    for (size_t i = 0; i < r->elements; ++i) {
        table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), ReplyToLua(s, r->element[i], depth + 1));
    }
    return tbl;
}

static CVar MapToLua(State *s, const redisReply *r, int depth) {
    if (depth > kMaxRedisReplyDepth) {
        ThrowFakeluaException("redis: nested reply too deep");
    }
    CVar tbl = table::TableHelper::CreateTable(s);
    for (size_t i = 0; i + 1 < r->elements; i += 2) {
        CVar key = ReplyToLua(s, r->element[i], depth + 1);
        CVar val = ReplyToLua(s, r->element[i + 1], depth + 1);
        table::TableHelper::SetTable(s, tbl, key, val);
    }
    return tbl;
}

static CVar ReplyToLua(State *s, const redisReply *r, int depth = 0) {
    if (!r) return inter::NativeToFakeluaNil(s);
    if (depth > kMaxRedisReplyDepth) {
        ThrowFakeluaException("redis: nested reply too deep");
    }
    switch (r->type) {
        case REDIS_REPLY_NIL:
            return inter::NativeToFakeluaNil(s);
        case REDIS_REPLY_INTEGER:
            return inter::NativeToFakeluaLonglong(s, r->integer);
        case REDIS_REPLY_BOOL:
            return inter::NativeToFakeluaBool(s, r->integer != 0);
        case REDIS_REPLY_DOUBLE: {
            double v = 0;
            if (r->str && TryParseDouble(r->str, v)) return inter::NativeToFakeluaDouble(s, v);
            return inter::NativeToFakeluaDouble(s, r->dval);
        }
        case REDIS_REPLY_ARRAY:
        case REDIS_REPLY_SET:
        case REDIS_REPLY_PUSH:
            return ArrayToLua(s, r, depth);
        case REDIS_REPLY_MAP:
        case REDIS_REPLY_ATTR:
            return MapToLua(s, r, depth);
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
        case REDIS_REPLY_VERB:
        case REDIS_REPLY_BIGNUM:
        default:
            if (!r->str) return inter::NativeToFakeluaNil(s);
            if (r->type == REDIS_REPLY_INTEGER) return inter::NativeToFakeluaLonglong(s, r->integer);
            return inter::NativeToFakeluaString(s, std::string(r->str, r->len));
    }
}

struct TickDepthGuard {
    int &depth;
    explicit TickDepthGuard(int &d) : depth(d) { ++depth; }
    ~TickDepthGuard() {
        if (depth > 0) --depth;
    }
};

struct CmdOp {
    std::string cb;
    std::string err;
    redisReply *reply = nullptr;
    bool done = false;
};

class RedisConnection {
public:
    explicit RedisConnection(State *state) : io_(state->GetIoContext()), lua_state_(state) {}

    ~RedisConnection() { Teardown(); }

    RedisConnection(const RedisConnection &) = delete;
    RedisConnection &operator=(const RedisConnection &) = delete;

    void Connect(const std::string &host, uint16_t port, const std::string &username, const std::string &password, int db, int timeout_ms) {
        host_ = host;
        port_ = port;
        username_ = username;
        password_ = password;
        db_ = db;
        connecting_ = true;
        const int ms = timeout_ms > 0 ? timeout_ms : 10000;
        deadline_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);

        redisOptions opt;
        std::memset(&opt, 0, sizeof(opt));
        REDIS_OPTIONS_SET_TCP(&opt, host_.c_str(), port_);
        timeval tv{};
        if (timeout_ms > 0) {
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            opt.connect_timeout = &tv;
        }
        ac_ = redisAsyncConnectWithOptions(&opt);
        if (!ac_ || ac_->err) {
            std::string err = ac_ && ac_->errstr[0] ? ac_->errstr : "connect failed";
            if (ac_) {
                redisAsyncFree(ac_);
                ac_ = nullptr;
            }
            connecting_ = false;
            NotifyConnect(ConnectErr(err));
            return;
        }
        ac_->data = this;
        if (redisLibeventAttach(ac_, io_.Get()) != REDIS_OK) {
            redisAsyncFree(ac_);
            ac_ = nullptr;
            connecting_ = false;
            NotifyConnect(ConnectErr("libevent attach failed"));
            return;
        }
        redisAsyncSetConnectCallback(ac_, &RedisConnection::OnConnect);
        redisAsyncSetDisconnectCallback(ac_, &RedisConnection::OnDisconnect);
    }

    void Command(std::vector<std::string> parts, std::string cb) {
        if (closed_ || close_pending_ || !ac_ || !ready_) {
            ThrowFakeluaException("redis: connection is closed");
        }
        if (parts.empty()) ThrowFakeluaException("redis command: empty argv");
        auto op = std::make_unique<CmdOp>();
        op->cb = std::move(cb);
        auto *raw = op.get();
        cmds_.push_back(std::move(op));
        IssueArgv(std::move(parts), &RedisConnection::OnCommand, raw);
    }

    void Tick() {
        if (tick_depth_ > 0) return;
        TickDepthGuard guard(tick_depth_);
        io_.Poll();
        if (connecting_ && !connect_notified_ && std::chrono::steady_clock::now() >= deadline_) {
            connecting_ = false;
            NotifyConnect(ConnectErr("timeout"));
            Teardown();
        }
        if (close_pending_) return;
        if (pending_connect_) {
            pending_connect_ = false;
            DispatchConnect();
            if (close_pending_) return;
        }
        for (auto &op: cmds_) {
            if (op && op->done && !op->cb.empty()) {
                DispatchCmd(*op);
                op->cb.clear();
            }
        }
        while (!cmds_.empty() && cmds_.front() && cmds_.front()->done && cmds_.front()->cb.empty()) {
            if (cmds_.front()->reply) {
                freeReplyObject(cmds_.front()->reply);
                cmds_.front()->reply = nullptr;
            }
            cmds_.pop_front();
        }
    }

    void Close() {
        closed_ = true;
        if (!connect_notified_) NotifyConnect("closed");
        if (tick_depth_ > 0 || io_.InDispatch()) {
            close_pending_ = true;
            return;
        }
        Teardown();
    }

    int TickDepth() const { return tick_depth_; }
    bool ClosePending() const { return close_pending_; }
    void RequestClose() {
        closed_ = true;
        close_pending_ = true;
    }
    void SetConnectCallback(std::string name) { connect_cb_ = std::move(name); }
    void SetNativeObject(NativeObject *obj) { native_obj_ = obj; }
    bool Closed() const { return closed_; }

private:
    static std::string ConnectErr(const std::string &detail) {
        return "connect failed: " + detail;
    }

    void IssueArgv(std::vector<std::string> parts, redisCallbackFn *cb, void *priv) {
        if (!ac_) return;
        std::vector<const char *> argv;
        std::vector<size_t> argvlen;
        argv.reserve(parts.size());
        argvlen.reserve(parts.size());
        for (auto &p: parts) {
            argv.push_back(p.c_str());
            argvlen.push_back(p.size());
        }
        redisAsyncCommandArgv(ac_, cb, priv, static_cast<int>(argv.size()), argv.data(), argvlen.data());
    }

    void StartHandshake() {
        hs_step_ = 0;
        NextHandshake();
    }

    void NextHandshake() {
        if (!ac_ || closed_ || close_pending_) return;
        if (hs_step_ == 0 && !password_.empty()) {
            hs_step_ = 1;
            if (!username_.empty() && username_ != "default") {
                IssueArgv({"AUTH", username_, password_}, &RedisConnection::OnHandshake, this);
            } else {
                IssueArgv({"AUTH", password_}, &RedisConnection::OnHandshake, this);
            }
            return;
        }
        if ((hs_step_ <= 1) && db_ > 0) {
            hs_step_ = 2;
            IssueArgv({"SELECT", std::to_string(db_)}, &RedisConnection::OnHandshake, this);
            return;
        }
        hs_step_ = 3;
        IssueArgv({"PING"}, &RedisConnection::OnHandshake, this);
    }

    static void OnConnect(const redisAsyncContext *c, int status) {
        auto *self = static_cast<RedisConnection *>(c->data);
        if (!self) return;
        if (status != REDIS_OK) {
            self->connecting_ = false;
            self->NotifyConnect(ConnectErr(c->errstr[0] ? c->errstr : "connect failed"));
            self->ac_ = nullptr;
            return;
        }
        self->StartHandshake();
    }

    static void OnDisconnect(const redisAsyncContext *c, int) {
        auto *self = static_cast<RedisConnection *>(c->data);
        if (!self) return;
        self->ac_ = nullptr;
        if (!self->connect_notified_) self->NotifyConnect(ConnectErr("connection closed"));
    }

    static void OnHandshake(redisAsyncContext *c, void *r, void *priv) {
        auto *self = static_cast<RedisConnection *>(priv);
        auto *reply = static_cast<redisReply *>(r);
        if (!self) return;
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            std::string msg = (reply && reply->str) ? std::string(reply->str, reply->len) : (c->errstr[0] ? c->errstr : "handshake failed");
            self->connecting_ = false;
            self->NotifyConnect(ConnectErr(msg));
            return;
        }
        if (self->hs_step_ == 3) {
            self->connecting_ = false;
            self->NotifyConnect({});
            return;
        }
        self->NextHandshake();
    }

    static void OnCommand(redisAsyncContext *c, void *r, void *priv) {
        auto *op = static_cast<CmdOp *>(priv);
        if (!op) return;
        op->done = true;
        auto *reply = static_cast<redisReply *>(r);
        if (!reply) {
            op->err = c && c->errstr[0] ? c->errstr : "redis command failed";
            return;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            op->err = std::string(reply->str, reply->len);
            return;
        }
        op->reply = reply;
        // hiredis frees reply after callback unless we take it. Copy by keeping?
        // Default: hiredis frees reply after callback returns. We must copy now.
        // We'll convert later in Tick — so duplicate via redisReply isn't easy.
        // Convert immediately into Lua? Can't, might be inside poll.
        // Duplicate the reply tree:
        op->reply = CloneReply(reply);
    }

    static redisReply *CloneReply(const redisReply *r, int depth = 0) {
        if (!r) return nullptr;
        if (depth > kMaxRedisReplyDepth) {
            ThrowFakeluaException("redis: nested reply too deep");
        }
        auto *out = static_cast<redisReply *>(calloc(1, sizeof(redisReply)));
        if (!out) {
            ThrowFakeluaException("redis: out of memory");
        }
        out->type = r->type;
        out->integer = r->integer;
        out->dval = r->dval;
        out->len = r->len;
        if (r->str) {
            out->str = static_cast<char *>(malloc(static_cast<size_t>(r->len) + 1));
            if (!out->str) {
                freeReplyObject(out);
                ThrowFakeluaException("redis: out of memory");
            }
            std::memcpy(out->str, r->str, static_cast<size_t>(r->len));
            out->str[r->len] = 0;
        }
        if (r->elements && r->element) {
            out->elements = r->elements;
            out->element = static_cast<redisReply **>(calloc(r->elements, sizeof(redisReply *)));
            if (!out->element) {
                out->elements = 0;
                freeReplyObject(out);
                ThrowFakeluaException("redis: out of memory");
            }
            for (size_t i = 0; i < r->elements; ++i) {
                try {
                    out->element[i] = CloneReply(r->element[i], depth + 1);
                } catch (...) {
                    freeReplyObject(out);
                    throw;
                }
            }
        }
        return out;
    }

    void Teardown() {
        closed_ = true;
        connecting_ = false;
        if (ac_) {
            ac_->data = nullptr;
            redisAsyncFree(ac_);
            ac_ = nullptr;
        }
        for (auto &op: cmds_) {
            if (op && op->reply) {
                freeReplyObject(op->reply);
                op->reply = nullptr;
            }
        }
        cmds_.clear();
    }

    void NotifyConnect(std::string err) {
        if (connect_notified_) return;
        connecting_ = false;
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
        if (!op.err.empty()) {
            args[1] = inter::NativeToFakeluaString(lua_state_, op.err);
            args[2] = inter::NativeToFakeluaNil(lua_state_);
        } else {
            args[1] = inter::NativeToFakeluaNil(lua_state_);
            args[2] = ReplyToLua(lua_state_, op.reply);
        }
        CallNamed(lua_state_, op.cb, args, 3);
    }

    native::IoContext &io_;
    redisAsyncContext *ac_ = nullptr;
    std::string host_;
    uint16_t port_ = 6379;
    std::string username_;
    std::string password_;
    int db_ = 0;
    int hs_step_ = 0;
    std::chrono::steady_clock::time_point deadline_{};
    std::deque<std::unique_ptr<CmdOp>> cmds_;
    std::string connect_cb_;
    std::string connect_err_;
    State *lua_state_ = nullptr;
    NativeObject *native_obj_ = nullptr;
    bool connecting_ = false;
    bool connect_notified_ = false;
    bool pending_connect_ = false;
    bool ready_ = false;
    bool closed_ = false;
    bool close_pending_ = false;
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
    conn->Close();
}

void TickAll(State *s) {
    if (!s) return;
    auto *ws = s->TryGetModuleState<RedisWrappers>();
    if (!ws) return;
    auto conns = ws->conns;
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
        if (v.type_ != static_cast<int>(VarType::Nil)) port = CheckPortRange(inter::CVarToInteger(v, 6379), "redis.connect", 1, 65535);
        v = table::TableHelper::GetTableStrId(s, a0, "user");
        if (v.type_ != static_cast<int>(VarType::Nil)) user = CVarToString(v);
        v = table::TableHelper::GetTableStrId(s, a0, "password");
        if (v.type_ != static_cast<int>(VarType::Nil)) password = CVarToString(v);
        v = table::TableHelper::GetTableStrId(s, a0, "db");
        if (v.type_ != static_cast<int>(VarType::Nil)) {
            db = CheckInt32Range(inter::CVarToInteger(v, 0), "redis.connect", "db");
        }
        v = table::TableHelper::GetTableStrId(s, a0, "timeout_ms");
        if (v.type_ != static_cast<int>(VarType::Nil)) {
            timeout_ms = CheckInt32Range(inter::CVarToInteger(v, 1000), "redis.connect", "timeout_ms");
        }
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
