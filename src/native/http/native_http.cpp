#include "native/http/native_http.h"

#include "native/native_common.h"
#include "native/native_io_context.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "native/tls_util.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_multi.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url.hpp>

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fakelua::http {

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace urls = boost::urls;
using tcp = asio::ip::tcp;

static std::string CVarToString(CVar v) {
    return inter::FakeluaToNativeString(nullptr, v);
}

static bool CVarToBoolFlag(CVar v, bool default_val) {
    if (v.type_ == static_cast<int>(VarType::Nil)) return default_val;
    if (v.type_ == static_cast<int>(VarType::Bool)) return AsVar(v).GetBool();
    if (v.type_ == static_cast<int>(VarType::Int)) return v.data_.i != 0;
    std::string s = CVarToString(v);
    if (s == "false" || s == "0" || s == "disable" || s.empty()) return false;
    return true;
}

static CVar CallNamed(State *s, const std::string &name, CVar *args, int n) {
    if (!s || name.empty()) return inter::NativeToFakeluaNil(s);
    auto func = s->GetVM().GetFunction(name);
    if (func.Empty()) return inter::NativeToFakeluaNil(s);
    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }
    if (!addr) return inter::NativeToFakeluaNil(s);
    native::IoContext::DispatchScope scope(s->GetIoContext());
    return inter::DispatchCall(s, addr, args, n, jit_type);
}

static CVar HeadersToTable(State *s, const http::fields &fields) {
    CVar tbl = table::TableHelper::CreateTable(s);
    for (auto const &f: fields) {
        table::TableHelper::SetTableStrId(s, tbl, std::string(f.name_string()).c_str(), inter::NativeToFakeluaString(s, std::string(f.value())));
    }
    return tbl;
}

static void ApplyHeaders(State *s, CVar headers, http::fields &out) {
    if (headers.type_ != static_cast<int>(VarType::Table) || !headers.data_.t) return;
    auto kvs = table::TableHelper::CollectKVPairs(headers);
    for (auto &kv: kvs) {
        out.set(CVarToString(kv.key), CVarToString(kv.val));
    }
}

static CVar ResponseToTable(State *s, int status, const std::string &reason, const http::fields &fields, const std::string &body) {
    CVar tbl = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableStrId(s, tbl, "status", inter::NativeToFakeluaInt(s, status));
    table::TableHelper::SetTableStrId(s, tbl, "reason", inter::NativeToFakeluaString(s, reason));
    table::TableHelper::SetTableStrId(s, tbl, "headers", HeadersToTable(s, fields));
    table::TableHelper::SetTableStrId(s, tbl, "body", inter::NativeToFakeluaString(s, body));
    return tbl;
}

static CVar RequestToTable(State *s, const http::request<http::string_body> &req) {
    CVar tbl = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableStrId(s, tbl, "method", inter::NativeToFakeluaString(s, std::string(req.method_string())));
    table::TableHelper::SetTableStrId(s, tbl, "target", inter::NativeToFakeluaString(s, std::string(req.target())));
    table::TableHelper::SetTableStrId(s, tbl, "version", inter::NativeToFakeluaInt(s, req.version()));
    table::TableHelper::SetTableStrId(s, tbl, "headers", HeadersToTable(s, req));
    table::TableHelper::SetTableStrId(s, tbl, "body", inter::NativeToFakeluaString(s, req.body()));

    auto origin = urls::parse_origin_form(req.target());
    if (!origin.has_error()) {
        table::TableHelper::SetTableStrId(s, tbl, "path", inter::NativeToFakeluaString(s, std::string(origin->path())));
        if (origin->has_query()) {
            table::TableHelper::SetTableStrId(s, tbl, "query", inter::NativeToFakeluaString(s, std::string(origin->query())));
        }
    }
    return tbl;
}

struct HttpResponseData {
    int status = 0;
    std::string reason;
    http::fields headers;
    std::string body;
    std::string err;
};

class HttpClientOp : public std::enable_shared_from_this<HttpClientOp> {
public:
    HttpClientOp(State *state, NativeObject *nat) : io_(state->GetIoContext()), resolver_(io_.Get()), timer_(io_.Get()), lua_state_(state), native_obj_(nat) {
    }

    void Start(std::string method, std::string url, http::fields headers, std::string body, int timeout_ms, unsigned version, bool tls_verify, std::string tls_ca) {
        timeout_ms_ = timeout_ms;
        tls_verify_ = tls_verify;
        tls_ca_ = std::move(tls_ca);
        auto parsed = urls::parse_uri(url);
        if (parsed.has_error()) {
            Finish("url: " + parsed.error().message());
            return;
        }
        urls::url_view u = *parsed;
        const bool https = u.has_scheme() && u.scheme_id() == urls::scheme::https;
        const bool http_plain = u.has_scheme() && u.scheme_id() == urls::scheme::http;
        if (!https && !http_plain) {
            Finish("only http:// and https:// URLs are supported");
            return;
        }
        use_tls_ = https;
        host_ = std::string(u.host());
        if (host_.empty()) {
            Finish("url missing host");
            return;
        }
        std::string port = u.has_port() ? std::string(u.port()) : (use_tls_ ? "443" : "80");
        std::string target = std::string(u.encoded_path());
        if (target.empty()) target = "/";
        if (u.has_query()) {
            target.push_back('?');
            target += std::string(u.encoded_query());
        }

        auto verb = http::string_to_verb(method);
        if (verb == http::verb::unknown) {
            req_.method_string(method);
        } else {
            req_.method(verb);
        }
        req_.target(target);
        req_.version(version);
        req_.set(http::field::host, u.has_port() ? (host_ + ":" + port) : host_);
        req_.set(http::field::user_agent, "fakelua");
        req_.set(http::field::connection, "close");
        for (auto const &f: headers) {
            req_.set(f.name_string(), f.value());
        }
        req_.body() = std::move(body);
        req_.prepare_payload();

        try {
            if (use_tls_) {
                ssl_ctx_ = std::make_unique<ssl::context>(tls::MakeClientContext(tls_verify_, tls_ca_));
                tls_.emplace(io_.Get(), *ssl_ctx_);
            } else {
                plain_.emplace(io_.Get());
            }
        } catch (const std::exception &e) {
            Finish(e.what());
            return;
        }

        ArmTimeout();
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        resolver_.async_resolve(host_, port, [self, watch](beast::error_code ec, tcp::resolver::results_type results) {
            if (!watch.Alive()) return;
            self->OnResolve(ec, std::move(results));
        });
    }

    void Tick() {
        io_.Poll();
        if (!pending_) return;
        pending_ = false;
        Dispatch();
    }

    void Close() {
        resolver_.cancel();
        ShutdownStream();
        if (!done_) {
            Finish("closed");
        }
    }

    bool Done() const {
        return done_ && !pending_;
    }

    void SetCallback(std::string name) {
        cb_ = std::move(name);
    }

private:
    void ArmTimeout() {
        if (timeout_ms_ <= 0) return;
        timer_.cancel();
        timer_.expires_after(std::chrono::milliseconds(timeout_ms_));
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        timer_.async_wait([self, watch](boost::system::error_code wait_ec) {
            if (!watch.Alive() || wait_ec) return;
            self->Finish("timeout");
        });
    }

    void ShutdownStream() {
        // Close the TCP socket only. ssl::stream::shutdown() is synchronous and
        // waits on the same reactor tick() poll()s — the Windows deadlock that
        // mysql avoids by skipping close_statement()/close().
        boost::system::error_code ec;
        timer_.cancel();
        if (tls_) {
            auto &sock = beast::get_lowest_layer(*tls_).socket();
            sock.cancel(ec);
            sock.shutdown(tcp::socket::shutdown_both, ec);
            sock.close(ec);
        } else if (plain_) {
            plain_->socket().cancel(ec);
            plain_->socket().shutdown(tcp::socket::shutdown_both, ec);
            plain_->socket().close(ec);
        }
    }

    void OnResolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) {
            Finish(ec.message());
            return;
        }
        ArmTimeout();
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        if (tls_) {
            beast::get_lowest_layer(*tls_).async_connect(results, [self, watch](beast::error_code conn_ec, tcp::resolver::results_type::endpoint_type) {
                if (!watch.Alive()) return;
                self->OnConnect(conn_ec);
            });
        } else {
            plain_->async_connect(results, [self, watch](beast::error_code conn_ec, tcp::resolver::results_type::endpoint_type) {
                if (!watch.Alive()) return;
                self->OnConnect(conn_ec);
            });
        }
    }

    void OnConnect(beast::error_code ec) {
        if (ec) {
            Finish(ec.message());
            return;
        }
        if (tls_) {
            boost::system::error_code sni_ec;
            if (!tls::SetSniHostname(*tls_, host_, sni_ec)) {
                Finish(sni_ec.message());
                return;
            }
            if (tls_verify_) {
                tls_->set_verify_callback(ssl::host_name_verification(host_));
            }
            ArmTimeout();
            auto self = shared_from_this();
            auto watch = life_.GetWatch();
            tls_->async_handshake(ssl::stream_base::client, [self, watch](beast::error_code hs_ec) {
                if (!watch.Alive()) return;
                self->OnSslHandshake(hs_ec);
            });
            return;
        }
        DoWrite();
    }

    void OnSslHandshake(beast::error_code ec) {
        if (ec) {
            Finish(ec.message());
            return;
        }
        DoWrite();
    }

    void DoWrite() {
        ArmTimeout();
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        auto on_write = [self, watch](beast::error_code write_ec, std::size_t) {
            if (!watch.Alive()) return;
            self->OnWrite(write_ec);
        };
        if (tls_) {
            http::async_write(*tls_, req_, on_write);
        } else {
            http::async_write(*plain_, req_, on_write);
        }
    }

    void OnWrite(beast::error_code ec) {
        if (ec) {
            Finish(ec.message());
            return;
        }
        ArmTimeout();
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        auto on_read = [self, watch](beast::error_code read_ec, std::size_t) {
            if (!watch.Alive()) return;
            self->OnRead(read_ec);
        };
        if (tls_) {
            http::async_read(*tls_, buffer_, res_, on_read);
        } else {
            http::async_read(*plain_, buffer_, res_, on_read);
        }
    }

    void OnRead(beast::error_code ec) {
        if (ec) {
            Finish(ec.message());
            return;
        }
        resp_.status = static_cast<int>(res_.result_int());
        resp_.reason = std::string(res_.reason());
        resp_.headers = res_.base();
        resp_.body = res_.body();
        Finish({});
    }

    void Finish(std::string err) {
        if (done_) return;
        done_ = true;
        pending_ = true;
        resp_.err = std::move(err);
        ShutdownStream();
    }

    void Dispatch() {
        if (!lua_state_ || cb_.empty()) return;
        CVar args[3];
        args[0] = native_obj_ ? inter::NativeToFakeluaNativeObject(lua_state_, native_obj_) : inter::NativeToFakeluaNil(lua_state_);
        if (!resp_.err.empty()) {
            args[1] = inter::NativeToFakeluaString(lua_state_, resp_.err);
            args[2] = inter::NativeToFakeluaNil(lua_state_);
        } else {
            args[1] = inter::NativeToFakeluaNil(lua_state_);
            args[2] = ResponseToTable(lua_state_, resp_.status, resp_.reason, resp_.headers, resp_.body);
        }
        CallNamed(lua_state_, cb_, args, 3);
    }

    native::IoContext &io_;
    std::unique_ptr<ssl::context> ssl_ctx_;
    std::optional<beast::tcp_stream> plain_;
    std::optional<beast::ssl_stream<beast::tcp_stream>> tls_;
    tcp::resolver resolver_;
    asio::steady_timer timer_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    HttpResponseData resp_;
    std::string host_;
    std::string cb_;
    std::string tls_ca_;
    State *lua_state_ = nullptr;
    NativeObject *native_obj_ = nullptr;
    int timeout_ms_ = 10000;
    bool use_tls_ = false;
    bool tls_verify_ = true;
    bool done_ = false;
    bool pending_ = false;
    native::LifeToken life_;
};

class HttpServerConn : public std::enable_shared_from_this<HttpServerConn> {
public:
    HttpServerConn(asio::io_context &ioc, tcp::socket sock, int conn_id, int timeout_ms, ssl::context *ctx)
        : timer_(ioc), conn_id_(conn_id), timeout_ms_(timeout_ms) {
        if (ctx) {
            tls_.emplace(std::move(sock), *ctx);
        } else {
            plain_.emplace(std::move(sock));
        }
    }

    void Start(std::function<void(int, http::request<http::string_body>)> on_request, std::function<void(int)> on_close) {
        on_request_ = std::move(on_request);
        on_close_ = std::move(on_close);
        if (tls_) {
            ArmTimeout();
            auto self = shared_from_this();
            auto watch = life_.GetWatch();
            tls_->async_handshake(ssl::stream_base::server, [self, watch](beast::error_code ec) {
                if (!watch.Alive()) return;
                if (ec) {
                    self->Close();
                    return;
                }
                self->DoRead();
            });
            return;
        }
        DoRead();
    }

    void Reply(int status, std::string reason, http::fields headers, std::string body) {
        if (closed_ || writing_) return;
        writing_ = true;
        res_.result(static_cast<http::status>(status));
        if (!reason.empty()) res_.reason(reason);
        res_.version(req_.version());
        res_.set(http::field::server, "fakelua");
        res_.set(http::field::connection, "close");
        for (auto const &f: headers) {
            res_.set(f.name_string(), f.value());
        }
        res_.body() = std::move(body);
        res_.prepare_payload();
        ArmTimeout();
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        auto on_write = [self, watch](beast::error_code ec, std::size_t) {
            if (!watch.Alive()) return;
            self->OnWrite(ec);
        };
        if (tls_) {
            http::async_write(*tls_, res_, on_write);
        } else {
            http::async_write(*plain_, res_, on_write);
        }
    }

    void Close() {
        if (closed_) return;
        closed_ = true;
        boost::system::error_code ec;
        timer_.cancel();
        if (tls_) {
            auto &sock = beast::get_lowest_layer(*tls_).socket();
            sock.cancel(ec);
            sock.shutdown(tcp::socket::shutdown_both, ec);
            sock.close(ec);
        } else if (plain_) {
            plain_->socket().cancel(ec);
            plain_->socket().shutdown(tcp::socket::shutdown_both, ec);
            plain_->socket().close(ec);
        }
        if (on_close_) on_close_(conn_id_);
    }

    int ConnId() const {
        return conn_id_;
    }

private:
    void ArmTimeout() {
        if (timeout_ms_ <= 0) return;
        timer_.cancel();
        timer_.expires_after(std::chrono::milliseconds(timeout_ms_));
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        timer_.async_wait([self, watch](boost::system::error_code wait_ec) {
            if (!watch.Alive() || wait_ec) return;
            self->Close();
        });
    }

    void DoRead() {
        if (closed_) return;
        req_ = {};
        buffer_.consume(buffer_.size());
        ArmTimeout();
        auto self = shared_from_this();
        auto watch = life_.GetWatch();
        auto on_read = [self, watch](beast::error_code ec, std::size_t) {
            if (!watch.Alive()) return;
            self->OnRead(ec);
        };
        if (tls_) {
            http::async_read(*tls_, buffer_, req_, on_read);
        } else {
            http::async_read(*plain_, buffer_, req_, on_read);
        }
    }

    void OnRead(beast::error_code ec) {
        if (ec) {
            Close();
            return;
        }
        if (on_request_) on_request_(conn_id_, req_);
    }

    void OnWrite(beast::error_code) {
        Close();
    }

    std::optional<beast::tcp_stream> plain_;
    std::optional<beast::ssl_stream<beast::tcp_stream>> tls_;
    asio::steady_timer timer_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    std::function<void(int, http::request<http::string_body>)> on_request_;
    std::function<void(int)> on_close_;
    int conn_id_ = 0;
    int timeout_ms_ = 10000;
    bool writing_ = false;
    bool closed_ = false;
    native::LifeToken life_;
};

class HttpServer {
public:
    explicit HttpServer(State *state) : io_(state->GetIoContext()), acceptor_(io_.Get()), lua_state_(state) {
    }

    void Listen(const std::string &ip, uint16_t port, int backlog, int timeout_ms, std::unique_ptr<ssl::context> tls_ctx) {
        timeout_ms_ = timeout_ms;
        ssl_ctx_ = std::move(tls_ctx);
        auto addr = asio::ip::make_address(ip);
        tcp::endpoint ep(addr, port);
        acceptor_.open(ep.protocol());
        acceptor_.set_option(asio::socket_base::reuse_address(true));
        acceptor_.bind(ep);
        acceptor_.listen(backlog > 0 ? backlog : asio::socket_base::max_listen_connections);
        bound_port_ = acceptor_.local_endpoint().port();
        DoAccept();
    }

    uint16_t BoundPort() const {
        return bound_port_;
    }

    void SetDispatch(std::string name) {
        dispatch_ = std::move(name);
    }

    void SetNativeObject(NativeObject *nat) {
        native_obj_ = nat;
    }

    void Tick() {
        io_.Poll();
        auto pending = std::move(pending_requests_);
        pending_requests_.clear();
        for (auto &item: pending) {
            DispatchRequest(item.first, item.second);
        }
    }

    bool Reply(int conn_id, int status, std::string reason, http::fields headers, std::string body) {
        auto it = conns_.find(conn_id);
        if (it == conns_.end() || !it->second) return false;
        it->second->Reply(status, std::move(reason), std::move(headers), std::move(body));
        return true;
    }

    void Close() {
        closed_ = true;
        beast::error_code ec;
        acceptor_.close(ec);
        auto conns = conns_;
        for (auto &kv: conns) {
            if (kv.second) kv.second->Close();
        }
        conns_.clear();
    }

private:
    void DoAccept() {
        if (closed_ || !acceptor_.is_open()) return;
        auto watch = life_.GetWatch();
        acceptor_.async_accept([this, watch](beast::error_code ec, tcp::socket sock) {
            if (!watch.Alive()) return;
            OnAccept(ec, std::move(sock));
        });
    }

    void OnAccept(beast::error_code ec, tcp::socket sock) {
        if (closed_) return;
        if (!ec) {
            int id = ++next_id_;
            auto conn = std::make_shared<HttpServerConn>(io_.Get(), std::move(sock), id, timeout_ms_, ssl_ctx_.get());
            conns_[id] = conn;
            conn->Start(
                    [this](int conn_id, http::request<http::string_body> req) { pending_requests_.emplace_back(conn_id, std::move(req)); },
                    [this](int conn_id) { conns_.erase(conn_id); });
        }
        DoAccept();
    }

    void DispatchRequest(int conn_id, const http::request<http::string_body> &req) {
        if (!lua_state_ || dispatch_.empty()) return;
        CVar args[3];
        args[0] = inter::NativeToFakeluaString(lua_state_, "request");
        args[1] = inter::NativeToFakeluaInt(lua_state_, conn_id);
        args[2] = RequestToTable(lua_state_, req);
        CVar ret = CallNamed(lua_state_, dispatch_, args, 3);
        if (ret.type_ == static_cast<int>(VarType::Multi) && ret.data_.m && ret.data_.m->GetCount() > 0) {
            ret = ret.data_.m->GetVars()[0];
        }
        if (ret.type_ == static_cast<int>(VarType::Nil)) return;
        int status = 200;
        std::string reason;
        http::fields headers;
        std::string body;
        if (ret.type_ == static_cast<int>(VarType::Table) && ret.data_.t) {
            CVar st = table::TableHelper::GetTableStrId(lua_state_, ret, "status");
            if (st.type_ != static_cast<int>(VarType::Nil)) status = static_cast<int>(inter::CVarToInteger(st, 200));
            CVar rs = table::TableHelper::GetTableStrId(lua_state_, ret, "reason");
            if (rs.type_ != static_cast<int>(VarType::Nil)) reason = CVarToString(rs);
            ApplyHeaders(lua_state_, table::TableHelper::GetTableStrId(lua_state_, ret, "headers"), headers);
            CVar bd = table::TableHelper::GetTableStrId(lua_state_, ret, "body");
            if (bd.type_ != static_cast<int>(VarType::Nil)) body = CVarToString(bd);
        } else {
            body = CVarToString(ret);
        }
        Reply(conn_id, status, std::move(reason), std::move(headers), std::move(body));
    }

    native::IoContext &io_;
    tcp::acceptor acceptor_;
    std::unique_ptr<ssl::context> ssl_ctx_;
    std::unordered_map<int, std::shared_ptr<HttpServerConn>> conns_;
    std::vector<std::pair<int, http::request<http::string_body>>> pending_requests_;
    std::string dispatch_;
    State *lua_state_ = nullptr;
    NativeObject *native_obj_ = nullptr;
    uint16_t bound_port_ = 0;
    int next_id_ = 0;
    int timeout_ms_ = 10000;
    bool closed_ = false;
    native::LifeToken life_;
};

struct HttpWrappers {
    std::vector<NativeObject *> clients;
    std::vector<NativeObject *> servers;
};

static std::vector<NativeObject *> &WrapperList(State *s, bool is_server) {
    auto &ws = s->GetModuleState<HttpWrappers>();
    return is_server ? ws.servers : ws.clients;
}

static void EraseWrapper(State *st, bool is_server, NativeObject *nat) {
    if (!st) return;
    auto &v = WrapperList(st, is_server);
    v.erase(std::remove(v.begin(), v.end(), nat), v.end());
}

static void RegisterWrapper(State *s, NativeObject *nat, bool is_server) {
    if (!s || !nat) return;
    nat->SetInt("__http_state__", reinterpret_cast<int64_t>(s));
    nat->SetInt("__http_is_server__", is_server ? 1 : 0);
    WrapperList(s, is_server).push_back(nat);
}

static void UnregisterWrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt("__http_state__", 0));
    bool is_server = nat->GetInt("__http_is_server__", 0) != 0;
    nat->SetInt("__http_state__", 0);
    EraseWrapper(st, is_server, nat);
}

static HttpClientOp *UnwrapClient(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<HttpClientOp *>(self->GetInt("__http_client__", 0));
}

static HttpServer *UnwrapServer(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<HttpServer *>(self->GetInt("__http_server__", 0));
}

static void DestroyWrappers(State *s, bool is_server) {
    auto *ws = s->TryGetModuleState<HttpWrappers>();
    if (!ws) return;
    auto wrappers = std::move(is_server ? ws->servers : ws->clients);
    for (auto *nat: wrappers) {
        if (!nat) continue;
        nat->SetInt("__http_state__", 0);
        s->GetNativeObjectManager().DestroyGroup(nat->GetGroupId());
    }
}

void TickAll(State *s) {
    if (!s) return;
    auto *ws = s->TryGetModuleState<HttpWrappers>();
    if (!ws) return;
    auto servers = ws->servers;
    for (auto *nat: servers) {
        auto *srv = UnwrapServer(nat);
        if (srv) srv->Tick();
    }
    auto clients = ws->clients;
    for (auto *nat: clients) {
        auto *op = UnwrapClient(nat);
        if (op) op->Tick();
    }
}

void OnStateDeleted(State *s) {
    if (!s) return;
    DestroyWrappers(s, false);
    DestroyWrappers(s, true);
}

static CVar ClientClose(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *op = UnwrapClient(self);
    if (op) op->Close();
    (void) s;
    return inter::NativeToFakeluaNil(s);
}

static CVar ServerDispatch(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "dispatch", "function name expected");
    auto *srv = UnwrapServer(self);
    if (!srv) return inter::NativeToFakeluaNil(s);
    srv->SetDispatch(CVarToString(inter::GetNativeArg(s, args, n, 0)));
    return inter::NativeToFakeluaNil(s);
}

static CVar ServerReply(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "reply", "connid and response table expected");
    auto *srv = UnwrapServer(self);
    if (!srv) return inter::NativeToFakeluaBool(s, false);
    int connid = static_cast<int>(CheckIntegerArg(inter::GetNativeArg(s, args, n, 0), 1, "reply"));
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    int status = 200;
    std::string reason;
    http::fields headers;
    std::string body;
    if (a1.type_ == static_cast<int>(VarType::Table) && a1.data_.t) {
        CVar st = table::TableHelper::GetTableStrId(s, a1, "status");
        if (st.type_ != static_cast<int>(VarType::Nil)) status = static_cast<int>(inter::CVarToInteger(st, 200));
        CVar rs = table::TableHelper::GetTableStrId(s, a1, "reason");
        if (rs.type_ != static_cast<int>(VarType::Nil)) reason = CVarToString(rs);
        ApplyHeaders(s, table::TableHelper::GetTableStrId(s, a1, "headers"), headers);
        CVar bd = table::TableHelper::GetTableStrId(s, a1, "body");
        if (bd.type_ != static_cast<int>(VarType::Nil)) body = CVarToString(bd);
    } else {
        body = CVarToString(a1);
    }
    bool ok = srv->Reply(connid, status, std::move(reason), std::move(headers), std::move(body));
    return inter::NativeToFakeluaBool(s, ok);
}

static CVar ServerClose(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *srv = UnwrapServer(self);
    if (srv) srv->Close();
    return inter::NativeToFakeluaNil(s);
}

static CVar ServerPort(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *srv = UnwrapServer(self);
    if (!srv) return inter::NativeToFakeluaInt(s, 0);
    return inter::NativeToFakeluaInt(s, srv->BoundPort());
}

static void ParseRequestOpts(State *s, CVar opts, std::string &method, std::string &url, http::fields &headers, std::string &body, int &timeout_ms, unsigned &version, bool &tls_verify, std::string &tls_ca) {
    method = "GET";
    timeout_ms = 10000;
    version = 11;
    tls_verify = true;
    tls_ca.clear();
    if (opts.type_ != static_cast<int>(VarType::Table) || !opts.data_.t) {
        ThrowBadArgument(1, "http.request", "config table expected");
    }
    CVar m = table::TableHelper::GetTableStrId(s, opts, "method");
    if (m.type_ != static_cast<int>(VarType::Nil)) method = CVarToString(m);
    CVar u = table::TableHelper::GetTableStrId(s, opts, "url");
    if (u.type_ == static_cast<int>(VarType::Nil)) ThrowBadArgument(1, "http.request", "url required");
    url = CVarToString(u);
    ApplyHeaders(s, table::TableHelper::GetTableStrId(s, opts, "headers"), headers);
    CVar b = table::TableHelper::GetTableStrId(s, opts, "body");
    if (b.type_ != static_cast<int>(VarType::Nil)) body = CVarToString(b);
    CVar t = table::TableHelper::GetTableStrId(s, opts, "timeout_ms");
    if (t.type_ != static_cast<int>(VarType::Nil)) timeout_ms = static_cast<int>(inter::CVarToInteger(t, 10000));
    CVar v = table::TableHelper::GetTableStrId(s, opts, "version");
    if (v.type_ != static_cast<int>(VarType::Nil)) {
        int64_t ver = inter::CVarToInteger(v, 11);
        version = (ver == 10) ? 10u : 11u;
    }
    CVar tv = table::TableHelper::GetTableStrId(s, opts, "tls_verify");
    if (tv.type_ != static_cast<int>(VarType::Nil)) tls_verify = CVarToBoolFlag(tv, true);
    CVar ca = table::TableHelper::GetTableStrId(s, opts, "tls_ca");
    if (ca.type_ != static_cast<int>(VarType::Nil)) tls_ca = CVarToString(ca);
}

static CVar StartClient(State *s, std::string method, std::string url, http::fields headers, std::string body, int timeout_ms, unsigned version, std::string cb, bool tls_verify, std::string tls_ca) {
    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, "http_request");
    auto op = std::make_shared<HttpClientOp>(s, nat);
    op->SetCallback(std::move(cb));
    nat->SetInt("__http_client__", reinterpret_cast<int64_t>(op.get()));
    nat->SetInt("__http_owned__", 1);
    nat->RegisterMethod("close", ClientClose);
    RegisterWrapper(s, nat, false);
    struct ClientHold {
        std::shared_ptr<HttpClientOp> ptr;
    };
    auto *hold = new ClientHold{op};
    nat->SetInt("__http_hold__", reinterpret_cast<int64_t>(hold));
    nat->SetFinalizer([hold](NativeObject *self) {
        UnregisterWrapper(self);
        auto *c = UnwrapClient(self);
        if (c) {
            self->SetInt("__http_client__", 0);
            c->Close();
        }
        self->SetInt("__http_hold__", 0);
        delete hold;
    });
    op->Start(std::move(method), std::move(url), std::move(headers), std::move(body), timeout_ms, version, tls_verify, std::move(tls_ca));
    return inter::NativeToFakeluaNativeObject(s, nat);
}

static CVar HttpRequest(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "http.request", "config table and callback expected");
    std::string method, url, body, cb, tls_ca;
    http::fields headers;
    int timeout_ms = 10000;
    unsigned version = 11;
    bool tls_verify = true;
    ParseRequestOpts(s, inter::GetNativeArg(s, args, n, 0), method, url, headers, body, timeout_ms, version, tls_verify, tls_ca);
    cb = CVarToString(inter::GetNativeArg(s, args, n, 1));
    if (cb.empty()) ThrowBadArgument(2, "http.request", "callback function expected");
    return StartClient(s, std::move(method), std::move(url), std::move(headers), std::move(body), timeout_ms, version, std::move(cb), tls_verify, std::move(tls_ca));
}

static CVar HttpGet(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "http.get", "url and callback expected");
    CheckStringArg(inter::GetNativeArg(s, args, n, 0), 1, "http.get");
    std::string url = CVarToString(inter::GetNativeArg(s, args, n, 0));
    std::string cb = CVarToString(inter::GetNativeArg(s, args, n, 1));
    if (cb.empty()) ThrowBadArgument(2, "http.get", "callback function expected");
    return StartClient(s, "GET", std::move(url), {}, {}, 10000, 11, std::move(cb), true, {});
}

static CVar HttpPost(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "http.post", "url, body, and callback expected");
    CheckStringArg(inter::GetNativeArg(s, args, n, 0), 1, "http.post");
    std::string url = CVarToString(inter::GetNativeArg(s, args, n, 0));
    std::string body = CVarToString(inter::GetNativeArg(s, args, n, 1));
    std::string cb = CVarToString(inter::GetNativeArg(s, args, n, 2));
    if (cb.empty()) ThrowBadArgument(3, "http.post", "callback function expected");
    http::fields headers;
    headers.set(http::field::content_type, "text/plain");
    return StartClient(s, "POST", std::move(url), std::move(headers), std::move(body), 10000, 11, std::move(cb), true, {});
}

static CVar HttpServerFn(State *s, CVar *args, int n) {
    std::string ip = "127.0.0.1";
    uint16_t port = 0;
    int backlog = 128;
    int timeout_ms = 10000;
    bool tls = false;
    std::string cert;
    std::string key;
    if (n >= 1) {
        CVar a0 = inter::GetNativeArg(s, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
            CVar ip_var = table::TableHelper::GetTableStrId(s, a0, "ip");
            if (ip_var.type_ != static_cast<int>(VarType::Nil)) ip = CVarToString(ip_var);
            CVar port_var = table::TableHelper::GetTableStrId(s, a0, "port");
            if (port_var.type_ != static_cast<int>(VarType::Nil)) port = static_cast<uint16_t>(inter::CVarToInteger(port_var, 0));
            CVar bl = table::TableHelper::GetTableStrId(s, a0, "backlog");
            if (bl.type_ != static_cast<int>(VarType::Nil)) backlog = static_cast<int>(inter::CVarToInteger(bl, 128));
            CVar t = table::TableHelper::GetTableStrId(s, a0, "timeout_ms");
            if (t.type_ != static_cast<int>(VarType::Nil)) timeout_ms = static_cast<int>(inter::CVarToInteger(t, 10000));
            CVar tls_var = table::TableHelper::GetTableStrId(s, a0, "tls");
            if (tls_var.type_ != static_cast<int>(VarType::Nil)) tls = CVarToBoolFlag(tls_var, false);
            CVar cert_var = table::TableHelper::GetTableStrId(s, a0, "cert");
            if (cert_var.type_ != static_cast<int>(VarType::Nil)) cert = CVarToString(cert_var);
            CVar key_var = table::TableHelper::GetTableStrId(s, a0, "key");
            if (key_var.type_ != static_cast<int>(VarType::Nil)) key = CVarToString(key_var);
        } else {
            ThrowBadArgument(1, "http.server", "config table expected");
        }
    }

    std::unique_ptr<ssl::context> tls_ctx;
    if (tls) {
        if (cert.empty() || key.empty()) {
            ThrowFakeluaException("http.server: tls=true requires cert and key");
        }
        try {
            tls_ctx = std::make_unique<ssl::context>(tls::MakeServerContext(cert, key));
        } catch (const std::exception &e) {
            ThrowFakeluaException(std::string("http.server: ") + e.what());
        }
    }

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, "http_server");
    auto *srv = new HttpServer(s);
    srv->SetNativeObject(nat);
    try {
        srv->Listen(ip, port, backlog, timeout_ms, std::move(tls_ctx));
    } catch (const std::exception &e) {
        delete srv;
        s->GetNativeObjectManager().DestroyGroup(gid);
        ThrowFakeluaException(std::string("http.server: ") + e.what());
    }
    nat->SetInt("__http_server__", reinterpret_cast<int64_t>(srv));
    nat->SetInt("port", srv->BoundPort());
    nat->RegisterMethod("dispatch", ServerDispatch);
    nat->RegisterMethod("reply", ServerReply);
    nat->RegisterMethod("close", ServerClose);
    nat->RegisterMethod("get_port", ServerPort);
    nat->SetFinalizer([](NativeObject *self) {
        UnregisterWrapper(self);
        auto *p = UnwrapServer(self);
        if (p) {
            self->SetInt("__http_server__", 0);
            p->Close();
            delete p;
        }
    });
    RegisterWrapper(s, nat, true);
    return inter::NativeToFakeluaNativeObject(s, nat);
}

void RegisterHttpLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "http.request", 2, false, HttpRequest);
    RegisterNativeFunction(s, "http.get", 2, false, HttpGet);
    RegisterNativeFunction(s, "http.post", 3, false, HttpPost);
    RegisterNativeFunction(s, "http.server", 1, false, HttpServerFn);
}

}// namespace fakelua::http
