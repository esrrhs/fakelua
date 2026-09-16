#include "native/http/native_http.h"

#include "native/native_common.h"
#include "native/native_io_context.h"
#include "native/net/net_common.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "native/tls_util.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_multi.h"

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <boost/url.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <format>
#include <limits>
#include <stdexcept>
#include <string_view>
#if defined(_WIN32)
#define strcasecmp _stricmp
#endif
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fakelua::http {

namespace urls = boost::urls;

using HeaderList = std::vector<std::pair<std::string, std::string>>;

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

static CVar HeadersToTable(State *s, const HeaderList &fields) {
    CVar tbl = table::TableHelper::CreateTable(s);
    for (auto const &f: fields) {
        table::TableHelper::SetTableStrId(s, tbl, f.first.c_str(), inter::NativeToFakeluaString(s, f.second));
    }
    return tbl;
}

static void ApplyHeaders(State *s, CVar headers, HeaderList &out) {
    if (headers.type_ != static_cast<int>(VarType::Table) || !headers.data_.t) return;
    auto kvs = table::TableHelper::CollectKVPairs(headers);
    for (auto &kv: kvs) {
        out.emplace_back(CVarToString(kv.key), CVarToString(kv.val));
    }
}

static std::string HeaderGet(const HeaderList &fields, const std::string &name) {
    for (auto const &f: fields) {
        if (strcasecmp(f.first.c_str(), name.c_str()) == 0) return f.second;
    }
    return {};
}

constexpr size_t kMaxHttpHeaderBytes = 256 * 1024;
constexpr size_t kMaxHttpBodyBytes = 32 * 1024 * 1024;

static bool ContainsCRLF(const std::string &s) {
    return s.find('\r') != std::string::npos || s.find('\n') != std::string::npos;
}

static void CheckHttpToken(const std::string &s, const char *what, bool header_name = false) {
    if (s.empty() || ContainsCRLF(s) || s.find(' ') != std::string::npos || s.find('\t') != std::string::npos) {
        ThrowFakeluaException(std::format("http: invalid {} (empty or contains CR/LF/space)", what));
    }
    if (header_name && s.find(':') != std::string::npos) {
        ThrowFakeluaException(std::format("http: invalid {} (contains ':')", what));
    }
}

static bool TransferEncodingChunked(const HeaderList &hdrs) {
    std::string te = HeaderGet(hdrs, "Transfer-Encoding");
    if (te.empty()) return false;
    for (char &c: te) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return te.find("chunked") != std::string::npos;
}

// 1 = complete, 0 = need more, -1 = malformed
static int DecodeChunkedBody(std::string_view data, std::string &out, size_t &consumed) {
    size_t i = 0;
    out.clear();
    while (i < data.size()) {
        auto nl = data.find("\r\n", i);
        if (nl == std::string_view::npos) return 0;
        std::string size_line(data.substr(i, nl - i));
        auto semi = size_line.find(';');
        if (semi != std::string::npos) size_line.resize(semi);
        while (!size_line.empty() && (size_line.back() == ' ' || size_line.back() == '\t')) size_line.pop_back();
        if (size_line.empty()) return -1;
        char *end = nullptr;
        unsigned long long sz = std::strtoull(size_line.c_str(), &end, 16);
        if (end == size_line.c_str() || *end != '\0') return -1;
        i = nl + 2;
        if (sz == 0) {
            if (i + 2 <= data.size() && data[i] == '\r' && data[i + 1] == '\n') {
                consumed = i + 2;
                return 1;
            }
            auto trail = data.find("\r\n\r\n", i);
            if (trail == std::string_view::npos) return 0;
            consumed = trail + 4;
            return 1;
        }
        if (sz > kMaxHttpBodyBytes || out.size() > kMaxHttpBodyBytes - static_cast<size_t>(sz)) return -1;
        if (i + static_cast<size_t>(sz) + 2 > data.size()) return 0;
        if (data[i + static_cast<size_t>(sz)] != '\r' || data[i + static_cast<size_t>(sz) + 1] != '\n') return -1;
        out.append(data.data() + i, static_cast<size_t>(sz));
        i += static_cast<size_t>(sz) + 2;
    }
    return 0;
}

static void HeaderSet(HeaderList &fields, const std::string &name, const std::string &value) {
    for (auto &f: fields) {
        if (strcasecmp(f.first.c_str(), name.c_str()) == 0) {
            f.second = value;
            return;
        }
    }
    fields.emplace_back(name, value);
}

static const char *ReasonPhrase(int status) {
    switch (status) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        default:
            return "OK";
    }
}

static CVar ResponseToTable(State *s, int status, const std::string &reason, const HeaderList &fields, const std::string &body) {
    CVar tbl = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableStrId(s, tbl, "status", inter::NativeToFakeluaInt(s, status));
    table::TableHelper::SetTableStrId(s, tbl, "reason", inter::NativeToFakeluaString(s, reason));
    table::TableHelper::SetTableStrId(s, tbl, "headers", HeadersToTable(s, fields));
    table::TableHelper::SetTableStrId(s, tbl, "body", inter::NativeToFakeluaString(s, body));
    return tbl;
}

struct HttpRequestData {
    std::string method;
    std::string target;
    int version = 11;
    HeaderList headers;
    std::string body;
};

static CVar RequestToTable(State *s, const HttpRequestData &req) {
    CVar tbl = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableStrId(s, tbl, "method", inter::NativeToFakeluaString(s, req.method));
    table::TableHelper::SetTableStrId(s, tbl, "target", inter::NativeToFakeluaString(s, req.target));
    table::TableHelper::SetTableStrId(s, tbl, "version", inter::NativeToFakeluaInt(s, req.version));
    table::TableHelper::SetTableStrId(s, tbl, "headers", HeadersToTable(s, req.headers));
    table::TableHelper::SetTableStrId(s, tbl, "body", inter::NativeToFakeluaString(s, req.body));

    auto origin = urls::parse_origin_form(req.target);
    if (!origin.has_error()) {
        table::TableHelper::SetTableStrId(s, tbl, "path", inter::NativeToFakeluaString(s, std::string(origin->path())));
        if (origin->has_query()) {
            table::TableHelper::SetTableStrId(s, tbl, "query", inter::NativeToFakeluaString(s, std::string(origin->query())));
        }
    }
    return tbl;
}

static bool ParseHeaders(const std::string &raw, HeaderList &out) {
    size_t i = 0;
    while (i < raw.size()) {
        auto nl = raw.find("\r\n", i);
        std::string line = raw.substr(i, nl == std::string::npos ? std::string::npos : nl - i);
        if (line.empty()) break;
        auto colon = line.find(':');
        if (colon == std::string::npos) return false;
        std::string name = line.substr(0, colon);
        std::string val = line.substr(colon + 1);
        while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());
        out.emplace_back(std::move(name), std::move(val));
        if (nl == std::string::npos) break;
        i = nl + 2;
    }
    return true;
}

static bool TryParseHttpMessage(std::string &buf, bool request, HttpRequestData *req, int *status, std::string *reason, HeaderList *headers, std::string *body, bool eof) {
    auto hdr_end = buf.find("\r\n\r\n");
    if (hdr_end == std::string::npos) return false;
    std::string head = buf.substr(0, hdr_end);
    auto nl = head.find("\r\n");
    std::string line = head.substr(0, nl == std::string::npos ? head.size() : nl);
    HeaderList hdrs;
    if (nl != std::string::npos) {
        if (!ParseHeaders(head.substr(nl + 2) + "\r\n", hdrs)) return false;
    }
    if (request && req) {
        auto sp1 = line.find(' ');
        auto sp2 = line.rfind(' ');
        if (sp1 == std::string::npos || sp2 == sp1) return false;
        req->method = line.substr(0, sp1);
        req->target = line.substr(sp1 + 1, sp2 - sp1 - 1);
        req->version = (line.find("HTTP/1.0") != std::string::npos) ? 10 : 11;
        req->headers = hdrs;
    } else if (status && reason) {
        // HTTP/1.1 200 OK
        auto sp1 = line.find(' ');
        if (sp1 == std::string::npos) return false;
        auto sp2 = line.find(' ', sp1 + 1);
        *status = std::atoi(line.c_str() + sp1 + 1);
        *reason = (sp2 == std::string::npos) ? ReasonPhrase(*status) : line.substr(sp2 + 1);
        if (headers) *headers = hdrs;
    }
    const size_t body_off = hdr_end + 4;
    std::string te_body;
    size_t te_consumed = 0;
    if (TransferEncodingChunked(hdrs)) {
        if (body_off > buf.size()) return false;
        int rc = DecodeChunkedBody(std::string_view(buf).substr(body_off), te_body, te_consumed);
        if (rc == 0) return false;
        if (rc < 0) return false;
        if (body) *body = te_body;
        if (req) req->body = te_body;
        buf.erase(0, body_off + te_consumed);
        return true;
    }
    std::string cl = HeaderGet(hdrs, "Content-Length");
    if (!cl.empty()) {
        // Reject non-decimal / overflow so body_off + n cannot wrap.
        char *end = nullptr;
        unsigned long long parsed = std::strtoull(cl.c_str(), &end, 10);
        if (end == cl.c_str() || *end != '\0') return false;
        if (parsed > kMaxHttpBodyBytes) return false;
        if (parsed > std::numeric_limits<size_t>::max() - body_off) return false;
        size_t n = static_cast<size_t>(parsed);
        if (buf.size() < body_off || buf.size() - body_off < n) return false;
        if (body) *body = buf.substr(body_off, n);
        if (req) req->body = buf.substr(body_off, n);
        buf.erase(0, body_off + n);
        return true;
    }
    if (request) {
        if (req) req->body.clear();
        if (body) body->clear();
        buf.erase(0, body_off);
        return true;
    }
    if (eof) {
        if (buf.size() < body_off) return false;
        if (buf.size() - body_off > kMaxHttpBodyBytes) return false;
        if (body) *body = buf.substr(body_off);
        buf.clear();
        return true;
    }
    return false;
}

static std::string FormatRequest(const std::string &method, const std::string &target, unsigned version, HeaderList headers, const std::string &body) {
    CheckHttpToken(method, "method");
    CheckHttpToken(target, "target");
    for (auto const &h: headers) {
        CheckHttpToken(h.first, "header name", true);
        if (ContainsCRLF(h.second)) {
            ThrowFakeluaException("http: invalid header value (contains CR/LF)");
        }
    }
    HeaderSet(headers, "Content-Length", std::to_string(body.size()));
    std::string out = method + " " + target + (version == 10 ? " HTTP/1.0\r\n" : " HTTP/1.1\r\n");
    for (auto const &h: headers) {
        out += h.first + ": " + h.second + "\r\n";
    }
    out += "\r\n";
    out += body;
    return out;
}

static std::string FormatResponse(int status, std::string reason, HeaderList headers, const std::string &body, int version) {
    if (reason.empty()) reason = ReasonPhrase(status);
    if (ContainsCRLF(reason)) {
        ThrowFakeluaException("http: invalid reason phrase (contains CR/LF)");
    }
    for (auto const &h: headers) {
        CheckHttpToken(h.first, "header name", true);
        if (ContainsCRLF(h.second)) {
            ThrowFakeluaException("http: invalid header value (contains CR/LF)");
        }
    }
    HeaderSet(headers, "Content-Length", std::to_string(body.size()));
    std::string out = std::string(version == 10 ? "HTTP/1.0 " : "HTTP/1.1 ") + std::to_string(status) + " " + reason + "\r\n";
    for (auto const &h: headers) {
        out += h.first + ": " + h.second + "\r\n";
    }
    out += "\r\n";
    out += body;
    return out;
}

struct HttpResponseData {
    int status = 0;
    std::string reason;
    HeaderList headers;
    std::string body;
    std::string err;
};

class HttpClientOp : public std::enable_shared_from_this<HttpClientOp> {
public:
    HttpClientOp(State *state, NativeObject *nat) : io_(state->GetIoContext()), lua_state_(state), native_obj_(nat) {}

    ~HttpClientOp() { Close(); }

    void Start(std::string method, std::string url, HeaderList headers, std::string body, int timeout_ms, unsigned version, bool tls_verify, std::string tls_ca) {
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
        int port = u.has_port() ? std::atoi(std::string(u.port()).c_str()) : (use_tls_ ? 443 : 80);
        std::string target = std::string(u.encoded_path());
        if (target.empty()) target = "/";
        if (u.has_query()) {
            target.push_back('?');
            target += std::string(u.encoded_query());
        }
        HeaderSet(headers, "Host", u.has_port() ? (host_ + ":" + std::to_string(port)) : host_);
        HeaderSet(headers, "User-Agent", "fakelua");
        HeaderSet(headers, "Connection", "close");
        req_wire_ = FormatRequest(method, target, version, std::move(headers), body);

        if (use_tls_) {
            ssl_ctx_ = tls::MakeClientContext(tls_verify_, tls_ca_);
            if (!ssl_ctx_) {
                Finish("tls context failed");
                return;
            }
        }
        bev_ = MakeBev();
        if (!bev_) {
            Finish("socket failed");
            return;
        }
        ArmTimeout();
        sockaddr_in addr{};
        if (net::FillSockaddr(addr, host_, static_cast<uint16_t>(port))) {
            bufferevent_socket_connect(bev_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
            return;
        }
        bufferevent_socket_connect_hostname(bev_, io_.Dns(), AF_UNSPEC, host_.c_str(), port);
    }

    void Tick() {
        io_.Poll();
        if (!pending_) return;
        pending_ = false;
        Dispatch();
    }

    void Close() {
        ClearTimeout();
        FreeBev();
        if (!done_) Finish("closed");
    }

    bool Done() const { return done_ && !pending_; }

    void SetCallback(std::string name) { cb_ = std::move(name); }

private:
    bufferevent *MakeBev() {
        if (ssl_ctx_) {
            SSL *ssl = SSL_new(ssl_ctx_);
            if (!ssl) return nullptr;
            tls::SetSniHostname(ssl, host_);
            auto *bev = bufferevent_openssl_socket_new(io_.Get(), -1, ssl, BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
            if (!bev) {
                SSL_free(ssl);
                return nullptr;
            }
            bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
            bufferevent_setcb(bev, &HttpClientOp::ReadCb, nullptr, &HttpClientOp::EventCb, this);
            bufferevent_enable(bev, EV_READ | EV_WRITE);
            return bev;
        }
        auto *bev = bufferevent_socket_new(io_.Get(), -1, BEV_OPT_CLOSE_ON_FREE);
        if (bev) {
            bufferevent_setcb(bev, &HttpClientOp::ReadCb, nullptr, &HttpClientOp::EventCb, this);
            bufferevent_enable(bev, EV_READ | EV_WRITE);
        }
        return bev;
    }

    void ArmTimeout() {
        ClearTimeout();
        if (timeout_ms_ <= 0) return;
        timeout_ev_ = evtimer_new(io_.Get(), [](evutil_socket_t, short, void *p) {
            static_cast<HttpClientOp *>(p)->Finish("timeout");
        }, this);
        timeval tv{};
        tv.tv_sec = timeout_ms_ / 1000;
        tv.tv_usec = (timeout_ms_ % 1000) * 1000;
        event_add(timeout_ev_, &tv);
    }

    void ClearTimeout() {
        if (timeout_ev_) {
            event_free(timeout_ev_);
            timeout_ev_ = nullptr;
        }
    }

    void FreeBev() {
        if (bev_) {
            bufferevent_setcb(bev_, nullptr, nullptr, nullptr, nullptr);
            bufferevent_free(bev_);
            bev_ = nullptr;
        }
        if (ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
            ssl_ctx_ = nullptr;
        }
    }

    void OnConnected() {
        if (bev_ && !req_wire_.empty()) {
            bufferevent_write(bev_, req_wire_.data(), req_wire_.size());
            req_wire_.clear();
        }
    }

    void Finish(std::string err) {
        if (done_) return;
        done_ = true;
        pending_ = true;
        resp_.err = std::move(err);
        ClearTimeout();
        FreeBev();
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

    static void ReadCb(bufferevent *, void *ctx) {
        auto *self = static_cast<HttpClientOp *>(ctx);
        if (self) self->HandleRead(false);
    }

    static void EventCb(bufferevent *, short what, void *ctx) {
        auto *self = static_cast<HttpClientOp *>(ctx);
        if (!self) return;
        if (what & BEV_EVENT_CONNECTED) {
            self->OnConnected();
            return;
        }
        if (what & BEV_EVENT_EOF) {
            self->HandleRead(true);
            if (!self->done_) self->Finish("connection closed");
            return;
        }
        if (what & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) {
            self->Finish("connect failed: " + std::string(evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR())));
        }
    }

    void HandleRead(bool eof) {
        if (done_ || !bev_) return;
        evbuffer *in = bufferevent_get_input(bev_);
        size_t n = evbuffer_get_length(in);
        if (n > 0) {
            size_t off = recv_.size();
            const bool have_hdr = recv_.find("\r\n\r\n") != std::string::npos;
            if (!have_hdr && off + n > kMaxHttpHeaderBytes) {
                Finish("headers too large");
                return;
            }
            if (off + n > kMaxHttpHeaderBytes + kMaxHttpBodyBytes) {
                Finish("body too large");
                return;
            }
            recv_.resize(off + n);
            evbuffer_remove(in, recv_.data() + off, n);
        }
        HeaderList hdrs;
        std::string reason, body;
        int status = 0;
        std::string tmp = recv_;
        if (!TryParseHttpMessage(tmp, false, nullptr, &status, &reason, &hdrs, &body, eof)) {
            if (eof) Finish("connection closed");
            return;
        }
        recv_.swap(tmp);
        resp_.status = status;
        resp_.reason = std::move(reason);
        resp_.headers = std::move(hdrs);
        resp_.body = std::move(body);
        Finish({});
    }

    native::IoContext &io_;
    SSL_CTX *ssl_ctx_ = nullptr;
    bufferevent *bev_ = nullptr;
    event *timeout_ev_ = nullptr;
    std::string req_wire_;
    std::string recv_;
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
    HttpServerConn(native::IoContext &io, evutil_socket_t fd, int conn_id, int timeout_ms, SSL_CTX *ctx)
        : io_(io), conn_id_(conn_id), timeout_ms_(timeout_ms) {
        if (ctx) {
            SSL *ssl = SSL_new(ctx);
            if (ssl) {
                bev_ = bufferevent_openssl_socket_new(io_.Get(), fd, ssl, BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
                if (!bev_) SSL_free(ssl);
            }
            if (bev_) bufferevent_openssl_set_allow_dirty_shutdown(bev_, 1);
            else evutil_closesocket(fd);
        } else {
            bev_ = bufferevent_socket_new(io_.Get(), fd, BEV_OPT_CLOSE_ON_FREE);
            if (!bev_) evutil_closesocket(fd);
        }
        if (bev_) {
            bufferevent_setcb(bev_, &HttpServerConn::ReadCb, nullptr, &HttpServerConn::EventCb, this);
            bufferevent_enable(bev_, EV_READ | EV_WRITE);
        }
        ArmTimeout();
    }

    ~HttpServerConn() { Close(); }

    void Start(std::function<void(int, HttpRequestData)> on_request, std::function<void(int)> on_close) {
        on_request_ = std::move(on_request);
        on_close_ = std::move(on_close);
    }

    void Reply(int status, std::string reason, HeaderList headers, std::string body) {
        if (closed_ || writing_ || !bev_) return;
        writing_ = true;
        HeaderSet(headers, "Server", "fakelua");
        HeaderSet(headers, "Connection", "close");
        std::string wire = FormatResponse(status, std::move(reason), std::move(headers), body, req_.version);
        bufferevent_write(bev_, wire.data(), wire.size());
        ArmTimeout();
    }

    void Close() {
        if (closed_) return;
        closed_ = true;
        ClearTimeout();
        if (bev_) {
            bufferevent_setcb(bev_, nullptr, nullptr, nullptr, nullptr);
            bufferevent_free(bev_);
            bev_ = nullptr;
        }
        if (on_close_) on_close_(conn_id_);
    }

    int ConnId() const { return conn_id_; }

private:
    void ArmTimeout() {
        ClearTimeout();
        if (timeout_ms_ <= 0) return;
        timeout_ev_ = evtimer_new(io_.Get(), [](evutil_socket_t, short, void *p) {
            static_cast<HttpServerConn *>(p)->Close();
        }, this);
        timeval tv{};
        tv.tv_sec = timeout_ms_ / 1000;
        tv.tv_usec = (timeout_ms_ % 1000) * 1000;
        event_add(timeout_ev_, &tv);
    }

    void ClearTimeout() {
        if (timeout_ev_) {
            event_free(timeout_ev_);
            timeout_ev_ = nullptr;
        }
    }

    void HandleRead(bool eof) {
        if (closed_ || !bev_ || writing_) return;
        evbuffer *in = bufferevent_get_input(bev_);
        size_t n = evbuffer_get_length(in);
        if (n > 0) {
            size_t off = recv_.size();
            const bool have_hdr = recv_.find("\r\n\r\n") != std::string::npos;
            if (!have_hdr && off + n > kMaxHttpHeaderBytes) {
                Close();
                return;
            }
            if (off + n > kMaxHttpHeaderBytes + kMaxHttpBodyBytes) {
                Close();
                return;
            }
            recv_.resize(off + n);
            evbuffer_remove(in, recv_.data() + off, n);
        }
        HttpRequestData req;
        std::string tmp = recv_;
        if (!TryParseHttpMessage(tmp, true, &req, nullptr, nullptr, nullptr, nullptr, eof)) {
            if (eof) Close();
            return;
        }
        recv_.swap(tmp);
        req_ = req;
        if (on_request_) on_request_(conn_id_, std::move(req));
    }

    static void ReadCb(bufferevent *, void *ctx) {
        auto *self = static_cast<HttpServerConn *>(ctx);
        if (self) self->HandleRead(false);
    }

    static void EventCb(bufferevent *, short what, void *ctx) {
        auto *self = static_cast<HttpServerConn *>(ctx);
        if (!self) return;
        if (what & BEV_EVENT_CONNECTED) return;
        if (what & BEV_EVENT_EOF) {
            self->HandleRead(true);
            if (self->writing_ || self->closed_) {
                self->Close();
            }
            return;
        }
        if (what & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) self->Close();
    }

    native::IoContext &io_;
    bufferevent *bev_ = nullptr;
    event *timeout_ev_ = nullptr;
    std::string recv_;
    HttpRequestData req_;
    std::function<void(int, HttpRequestData)> on_request_;
    std::function<void(int)> on_close_;
    int conn_id_ = 0;
    int timeout_ms_ = 10000;
    bool writing_ = false;
    bool closed_ = false;
    native::LifeToken life_;
};

class HttpServer {
public:
    explicit HttpServer(State *state) : io_(state->GetIoContext()), lua_state_(state) {}

    ~HttpServer() { Close(); }

    void Listen(const std::string &ip, uint16_t port, int backlog, int timeout_ms, SSL_CTX *tls_ctx) {
        timeout_ms_ = timeout_ms;
        ssl_ctx_ = tls_ctx;
        sockaddr_in addr{};
        if (!net::FillSockaddr(addr, ip, port)) {
            throw std::runtime_error("invalid listen address");
        }
        listener_ = evconnlistener_new_bind(io_.Get(), [](evconnlistener *, evutil_socket_t fd, sockaddr *, int, void *p) {
            static_cast<HttpServer *>(p)->OnAccept(fd);
        }, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, backlog > 0 ? backlog : -1, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
        if (!listener_) throw std::runtime_error("listen failed");
        auto fd = evconnlistener_get_fd(listener_);
        sockaddr_in local{};
        socklen_t slen = sizeof(local);
        if (getsockname(fd, reinterpret_cast<sockaddr *>(&local), &slen) == 0) bound_port_ = ntohs(local.sin_port);
        else bound_port_ = port;
    }

    uint16_t BoundPort() const { return bound_port_; }

    void SetDispatch(std::string name) { dispatch_ = std::move(name); }

    void SetNativeObject(NativeObject *nat) { native_obj_ = nat; }

    void Tick() {
        io_.Poll();
        auto pending = std::move(pending_requests_);
        pending_requests_.clear();
        for (auto &item: pending) {
            DispatchRequest(item.first, item.second);
        }
        for (auto it = conns_.begin(); it != conns_.end();) {
            if (!it->second) it = conns_.erase(it);
            else ++it;
        }
    }

    bool Reply(int conn_id, int status, std::string reason, HeaderList headers, std::string body) {
        auto it = conns_.find(conn_id);
        if (it == conns_.end() || !it->second) return false;
        it->second->Reply(status, std::move(reason), std::move(headers), std::move(body));
        return true;
    }

    void Close() {
        closed_ = true;
        if (listener_) {
            evconnlistener_free(listener_);
            listener_ = nullptr;
        }
        auto conns = conns_;
        for (auto &kv: conns) {
            if (kv.second) kv.second->Close();
        }
        conns_.clear();
        if (ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
            ssl_ctx_ = nullptr;
        }
    }

private:
    void OnAccept(evutil_socket_t fd) {
        if (closed_) {
            evutil_closesocket(fd);
            return;
        }
        int id = ++next_id_;
        auto conn = std::make_shared<HttpServerConn>(io_, fd, id, timeout_ms_, ssl_ctx_);
        conns_[id] = conn;
        conn->Start(
                [this](int conn_id, HttpRequestData req) { pending_requests_.emplace_back(conn_id, std::move(req)); },
                [this](int conn_id) { conns_.erase(conn_id); });
    }

    void DispatchRequest(int conn_id, const HttpRequestData &req) {
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
        HeaderList headers;
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
    evconnlistener *listener_ = nullptr;
    SSL_CTX *ssl_ctx_ = nullptr;
    std::unordered_map<int, std::shared_ptr<HttpServerConn>> conns_;
    std::vector<std::pair<int, HttpRequestData>> pending_requests_;
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
    HeaderList headers;
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

static void ParseRequestOpts(State *s, CVar opts, std::string &method, std::string &url, HeaderList &headers, std::string &body, int &timeout_ms, unsigned &version, bool &tls_verify, std::string &tls_ca) {
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

static CVar StartClient(State *s, std::string method, std::string url, HeaderList headers, std::string body, int timeout_ms, unsigned version, std::string cb, bool tls_verify, std::string tls_ca) {
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
    HeaderList headers;
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
    HeaderList headers;
    headers.emplace_back("Content-Type", "text/plain");
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
            if (port_var.type_ != static_cast<int>(VarType::Nil)) {
                int64_t port_val = inter::CVarToInteger(port_var, 0);
                if (port_val < 0 || port_val > 65535) {
                    ThrowFakeluaException(std::format("http.server: port {} out of range (0-65535)", port_val));
                }
                port = static_cast<uint16_t>(port_val);
            }
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

    SSL_CTX *tls_ctx = nullptr;
    if (tls) {
        if (cert.empty() || key.empty()) {
            ThrowFakeluaException("http.server: tls=true requires cert and key");
        }
        tls_ctx = tls::MakeServerContext(cert, key);
        if (!tls_ctx) ThrowFakeluaException("http.server: failed to load tls cert/key");
    }

    int64_t gid = s->GetNativeObjectManager().CreateGroup();
    auto *nat = s->GetNativeObjectManager().Create(gid, "http_server");
    auto *srv = new HttpServer(s);
    srv->SetNativeObject(nat);
    try {
        // Listen takes ownership of tls_ctx immediately (stores in ssl_ctx_).
        // On failure, ~HttpServer/Close frees it — do not SSL_CTX_free here.
        srv->Listen(ip, port, backlog, timeout_ms, tls_ctx);
        tls_ctx = nullptr;
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
