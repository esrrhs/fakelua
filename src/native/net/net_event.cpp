#include "native/net/net_event.h"

#include "native/crypto/crypto_digest.h"
#include "native/tls_util.h"
#include "state/state.h"

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/listener.h>

#include <cctype>
#include <cstring>
#include <random>
#include <utility>

namespace fakelua::net {

namespace {

bool IsWebSocket(const NetConfig &cfg) {
    return cfg.framer == FramerType::WebSocket;
}

void ApplyFdOptions(evutil_socket_t fd, const NetConfig &cfg) {
    SetSocketOptions(static_cast<socket_t>(fd), cfg);
}

std::string WsAcceptKey(const std::string &key) {
    static const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    auto digest = crypto::Sha1(key + magic);
    return crypto::Base64Encode(digest.data(), digest.size());
}

std::string RandomWsKey() {
    uint8_t raw[16];
    std::random_device rd;
    for (auto &b: raw) b = static_cast<uint8_t>(rd());
    return crypto::Base64Encode(raw, sizeof(raw));
}

std::string FindHeader(const std::string &raw, const std::string &name) {
    std::string key = name;
    for (char &c: key) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    key += ':';
    size_t line = 0;
    while (line < raw.size()) {
        const auto nl = raw.find("\r\n", line);
        const size_t line_end = nl == std::string::npos ? raw.size() : nl;
        if (line_end - line >= key.size()) {
            bool match = true;
            for (size_t i = 0; i < key.size(); ++i) {
                if (static_cast<char>(std::tolower(static_cast<unsigned char>(raw[line + i]))) != key[i]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                size_t start = line + key.size();
                while (start < line_end && (raw[start] == ' ' || raw[start] == '\t')) ++start;
                return raw.substr(start, line_end - start);
            }
        }
        if (nl == std::string::npos) break;
        line = nl + 2;
    }
    return {};
}

std::string RequestPath(const std::string &headers) {
    auto nl = headers.find("\r\n");
    std::string line = headers.substr(0, nl == std::string::npos ? headers.size() : nl);
    auto sp1 = line.find(' ');
    if (sp1 == std::string::npos) return {};
    auto sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return line.substr(sp1 + 1);
    return line.substr(sp1 + 1, sp2 - sp1 - 1);
}

bool EncodeWsText(const char *data, size_t len, bool mask, std::string &out) {
    uint8_t hdr[14];
    size_t hlen = 2;
    hdr[0] = 0x81;
    if (len < 126) {
        hdr[1] = static_cast<uint8_t>(len);
    } else if (len <= 0xFFFF) {
        hdr[1] = 126;
        hdr[2] = static_cast<uint8_t>((len >> 8) & 0xFF);
        hdr[3] = static_cast<uint8_t>(len & 0xFF);
        hlen = 4;
    } else {
        return false;
    }
    uint8_t mask_key[4]{};
    if (mask) {
        hdr[1] |= 0x80;
        std::random_device rd;
        for (int i = 0; i < 4; ++i) mask_key[i] = static_cast<uint8_t>(rd());
        std::memcpy(hdr + hlen, mask_key, 4);
        hlen += 4;
    }
    out.assign(reinterpret_cast<char *>(hdr), hlen);
    out.append(data, len);
    if (mask) {
        for (size_t i = 0; i < len; ++i) {
            out[hlen + i] = static_cast<char>(static_cast<uint8_t>(data[i]) ^ mask_key[i % 4]);
        }
    }
    return true;
}

bool TryParseWsFrame(CircularBuffer &buf, std::string &payload, bool &closed, bool &error) {
    closed = false;
    error = false;
    payload.clear();
    if (buf.Size() < 2) return false;
    uint8_t h[14];
    buf.Peek(reinterpret_cast<char *>(h), 2);
    size_t hlen = 2;
    uint64_t plen = h[1] & 0x7F;
    bool masked = (h[1] & 0x80) != 0;
    if (plen == 126) {
        if (buf.Size() < 4) return false;
        buf.Peek(reinterpret_cast<char *>(h), 4);
        plen = (static_cast<uint64_t>(h[2]) << 8) | h[3];
        hlen = 4;
    } else if (plen == 127) {
        error = true;
        return false;
    }
    if (masked) hlen += 4;
    if (buf.Size() < hlen + plen) return false;
    std::vector<char> raw(hlen + static_cast<size_t>(plen));
    buf.Peek(raw.data(), raw.size());
    uint8_t opcode = h[0] & 0x0F;
    const char *body = raw.data() + hlen;
    uint8_t mk[4]{};
    if (masked) std::memcpy(mk, raw.data() + hlen - 4, 4);
    payload.assign(body, static_cast<size_t>(plen));
    if (masked) {
        for (size_t i = 0; i < payload.size(); ++i) {
            payload[i] = static_cast<char>(static_cast<uint8_t>(payload[i]) ^ mk[i % 4]);
        }
    }
    buf.Skip(hlen + static_cast<size_t>(plen));
    if (opcode == 0x8) {
        closed = true;
        return true;
    }
    if (opcode != 0x1 && opcode != 0x2 && opcode != 0x0) {
        payload.clear();
        return true;
    }
    return true;
}

}// namespace

class EventConn : public std::enable_shared_from_this<EventConn> {
public:
    EventConn(native::IoContext &io, const NetConfig &cfg, int conn_id, bool from_client, bool tls, EventSink sink)
        : io_(io), cfg_(cfg), conn_id_(conn_id), from_client_(from_client), tls_(tls), sink_(std::move(sink)),
          recv_buf_(cfg.recv_buf_size), send_buf_(cfg.send_buf_size) {
    }

    ~EventConn() { Close(false); }

    void Attach(bufferevent *bev) {
        bev_ = bev;
        if (tls_ && bev_) bufferevent_openssl_set_allow_dirty_shutdown(bev_, 1);
        bufferevent_setcb(bev_, &EventConn::ReadCb, nullptr, &EventConn::EventCb, this);
        bufferevent_enable(bev_, EV_READ | EV_WRITE);
        if (IsWebSocket(cfg_) && from_client_) SendWsClientHandshake();
        if (!from_client_ && !tls_ && !IsWebSocket(cfg_)) MarkReady(false);
    }

    void Close(bool notify_sink = true) {
        if (closed_) return;
        closed_ = true;
        ready_ = false;
        ws_open_ = false;
        if (bev_) {
            bufferevent_setcb(bev_, nullptr, nullptr, nullptr, nullptr);
            bufferevent_disable(bev_, EV_READ | EV_WRITE);
            bufferevent_free(bev_);
            bev_ = nullptr;
        }
        if (notify_sink && sink_) sink_({EventKind::Close, conn_id_, {}});
    }

    bool Send(const char *data, size_t len) {
        if (closed_ || !bev_ || !ready_) return false;
        if (IsWebSocket(cfg_)) {
            if (!ws_open_) return false;
            std::string frame;
            if (!EncodeWsText(data, len, from_client_, frame)) return false;
            bufferevent_write(bev_, frame.data(), frame.size());
            return true;
        }
        if (!WritePacket(send_buf_, cfg_, data, len)) return false;
        FlushSend();
        return true;
    }

    [[nodiscard]] bool IsOpen() const { return !closed_ && bev_; }
    [[nodiscard]] bool Ready() const { return ready_ && !closed_ && bev_; }
    [[nodiscard]] int ConnId() const { return conn_id_; }

    void OnConnected() {
        if (IsWebSocket(cfg_)) return;
        MarkReady(from_client_ || tls_);
    }

private:
    void MarkReady(bool emit) {
        if (closed_ || ready_) return;
        ready_ = true;
        if (emit && sink_) sink_({EventKind::Connect, conn_id_, {}});
    }

    void FlushSend() {
        if (!bev_) return;
        auto region = send_buf_.ReadableRegion();
        if (region.second == 0) return;
        bufferevent_write(bev_, region.first, region.second);
        send_buf_.CommitRead(region.second);
        if (!send_buf_.Empty()) FlushSend();
    }

    void SendWsClientHandshake() {
        ws_key_ = RandomWsKey();
        std::string host = cfg_.ws_host.empty() ? (cfg_.ip + ":" + std::to_string(cfg_.port)) : cfg_.ws_host;
        std::string req = "GET " + cfg_.ws_path + " HTTP/1.1\r\nHost: " + host +
                          "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " + ws_key_ +
                          "\r\nSec-WebSocket-Version: 13\r\n";
        if (!cfg_.ws_origin.empty()) req += "Origin: " + cfg_.ws_origin + "\r\n";
        req += "\r\n";
        bufferevent_write(bev_, req.data(), req.size());
    }

    void HandleRead() {
        if (!bev_ || closed_) return;
        evbuffer *in = bufferevent_get_input(bev_);
        while (evbuffer_get_length(in) > 0) {
            auto region = recv_buf_.WritableRegion();
            if (region.second == 0) {
                Close();
                return;
            }
            size_t n = evbuffer_remove(in, region.first, region.second);
            recv_buf_.CommitWrite(n);
        }
        if (IsWebSocket(cfg_) && !ws_open_) {
            PumpWsHandshake();
            return;
        }
        if (IsWebSocket(cfg_)) {
            for (;;) {
                std::string payload;
                bool closed = false, err = false;
                if (!TryParseWsFrame(recv_buf_, payload, closed, err)) break;
                if (err || closed) {
                    Close();
                    return;
                }
                if (!payload.empty() && sink_) sink_({EventKind::Recv, conn_id_, std::move(payload)});
            }
            return;
        }
        for (;;) {
            const char *payload = nullptr;
            uint32_t plen = 0;
            bool err = false;
            if (!TryParsePacket(recv_buf_, cfg_, payload, plen, err)) {
                if (err) Close();
                break;
            }
            if (sink_) sink_({EventKind::Recv, conn_id_, std::string(payload, plen)});
        }
    }

    void PumpWsHandshake() {
        std::string chunk(recv_buf_.Size(), '\0');
        recv_buf_.Peek(chunk.data(), chunk.size());
        hs_buf_ += chunk;
        recv_buf_.Skip(recv_buf_.Size());
        constexpr size_t kMaxWsHandshakeBytes = 256 * 1024;
        if (hs_buf_.size() > kMaxWsHandshakeBytes) {
            Close();
            return;
        }
        auto pos = hs_buf_.find("\r\n\r\n");
        if (pos == std::string::npos) return;
        std::string headers = hs_buf_.substr(0, pos + 4);
        hs_buf_.erase(0, pos + 4);
        if (!hs_buf_.empty()) recv_buf_.Write(hs_buf_.data(), hs_buf_.size());
        hs_buf_.clear();
        if (from_client_) {
            auto nl = headers.find("\r\n");
            std::string status_line = headers.substr(0, nl == std::string::npos ? headers.size() : nl);
            auto sp1 = status_line.find(' ');
            auto sp2 = (sp1 == std::string::npos) ? std::string::npos : status_line.find(' ', sp1 + 1);
            std::string code = (sp1 == std::string::npos) ? std::string{} : status_line.substr(sp1 + 1, (sp2 == std::string::npos ? status_line.size() : sp2) - sp1 - 1);
            if (code != "101") {
                Close();
                return;
            }
            std::string accept = FindHeader(headers, "Sec-WebSocket-Accept");
            if (accept.empty() || accept != WsAcceptKey(ws_key_)) {
                Close();
                return;
            }
            ws_open_ = true;
            MarkReady(true);
            HandleRead();
            return;
        }
        std::string path = RequestPath(headers);
        if (!path.empty() && path != cfg_.ws_path) {
            Close();
            return;
        }
        std::string key = FindHeader(headers, "Sec-WebSocket-Key");
        if (key.empty()) {
            Close();
            return;
        }
        std::string accept = WsAcceptKey(key);
        std::string resp = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + accept + "\r\n\r\n";
        bufferevent_write(bev_, resp.data(), resp.size());
        ws_open_ = true;
        MarkReady(true);
        HandleRead();
    }

    static void ReadCb(bufferevent *, void *ctx) {
        auto *self = static_cast<EventConn *>(ctx);
        if (self) self->HandleRead();
    }

    static void EventCb(bufferevent *, short what, void *ctx) {
        auto *self = static_cast<EventConn *>(ctx);
        if (!self) return;
        if (what & BEV_EVENT_CONNECTED) {
            self->OnConnected();
            return;
        }
        if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) {
            self->Close();
        }
    }

    native::IoContext &io_;
    NetConfig cfg_;
    int conn_id_ = 0;
    bool from_client_ = false;
    bool tls_ = false;
    EventSink sink_;
    bufferevent *bev_ = nullptr;
    CircularBuffer recv_buf_;
    CircularBuffer send_buf_;
    std::string hs_buf_;
    std::string ws_key_;
    bool closed_ = false;
    bool ready_ = false;
    bool ws_open_ = false;
};

static bufferevent *MakeBev(native::IoContext &io, evutil_socket_t fd, SSL_CTX *ssl_ctx, bool client) {
    if (ssl_ctx) {
        SSL *ssl = SSL_new(ssl_ctx);
        if (!ssl) return nullptr;
        auto *bev = bufferevent_openssl_socket_new(io.Get(), fd, ssl, client ? BUFFEREVENT_SSL_CONNECTING : BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
        if (!bev) {
            SSL_free(ssl);
            return nullptr;
        }
        bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
        return bev;
    }
    return bufferevent_socket_new(io.Get(), fd, BEV_OPT_CLOSE_ON_FREE);
}

TcpServer::TcpServer(const NetConfig &config, ::fakelua::State *state) : config_(config), io_(state->GetIoContext()) {
    if (config_.tls) ssl_ctx_ = tls::MakeServerContext(config_.tls_cert, config_.tls_key);
}

TcpServer::~TcpServer() {
    Stop();
    if (ssl_ctx_) SSL_CTX_free(ssl_ctx_);
}

void TcpServer::Start() {
    if (config_.tls && !ssl_ctx_) return;
    sockaddr_in addr{};
    if (!FillSockaddr(addr, config_.ip, config_.port)) return;
    listener_ = evconnlistener_new_bind(io_.Get(), [](evconnlistener *, evutil_socket_t fd, sockaddr *, int, void *p) {
        static_cast<TcpServer *>(p)->OnAccept(fd);
    }, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, config_.backlog > 0 ? config_.backlog : -1, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

void TcpServer::Stop() {
    if (listener_) {
        evconnlistener_free(listener_);
        listener_ = nullptr;
    }
    auto conns = conns_;
    for (auto &c: conns) {
        if (c) c->Close(false);
    }
    conns_.clear();
}

void TcpServer::OnAccept(evutil_socket_t fd) {
    ApplyFdOptions(fd, config_);
    int slot = -1;
    for (int i = 0; i < static_cast<int>(conns_.size()); ++i) {
        if (!conns_[i] || !conns_[i]->IsOpen()) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (static_cast<int>(conns_.size()) >= config_.max_conn) {
            evutil_closesocket(fd);
            return;
        }
        slot = static_cast<int>(conns_.size());
        conns_.push_back(nullptr);
    }
    auto conn = std::make_shared<EventConn>(io_, config_, slot, false, config_.tls, [this](ConnEvent ev) { EmitEvent(std::move(ev)); });
    auto *bev = MakeBev(io_, fd, ssl_ctx_, false);
    if (!bev) {
        evutil_closesocket(fd);
        return;
    }
    conn->Attach(bev);
    conns_[static_cast<size_t>(slot)] = conn;
    if (!IsWebSocket(config_) && !config_.tls) EmitEvent({EventKind::Connect, slot, {}});
}

void TcpServer::EmitEvent(ConnEvent ev) {
    events_.push_back(std::move(ev));
}

void TcpServer::DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope scope(io_);
    for (const auto &ev: evs) dispatcher(ev);
    for (auto &c: conns_) {
        if (c && !c->IsOpen()) c.reset();
    }
}

bool TcpServer::Send(int conn_id, const char *data, size_t len) {
    if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size()) || !conns_[static_cast<size_t>(conn_id)]) return false;
    return conns_[static_cast<size_t>(conn_id)]->Send(data, len);
}

bool TcpServer::CloseConnection(int conn_id) {
    if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size()) || !conns_[static_cast<size_t>(conn_id)]) return false;
    conns_[static_cast<size_t>(conn_id)]->Close(true);
    return true;
}

TcpClient::TcpClient(const NetConfig &config, ::fakelua::State *state) : config_(config), io_(state->GetIoContext()) {
    if (config_.tls) ssl_ctx_ = tls::MakeClientContext(config_.tls_verify, config_.tls_ca);
}

TcpClient::~TcpClient() {
    Disconnect();
    if (ssl_ctx_) SSL_CTX_free(ssl_ctx_);
}

void TcpClient::Connect() {
    if (connecting_ || conn_) return;
    if (config_.tls && !ssl_ctx_) {
        EmitEvent({EventKind::Close, 0, {}});
        return;
    }
    connecting_ = true;
    conn_ = std::make_shared<EventConn>(io_, config_, 0, true, config_.tls, [this](ConnEvent ev) { EmitEvent(std::move(ev)); });
    auto *bev = MakeBev(io_, -1, ssl_ctx_, true);
    if (!bev) {
        connecting_ = false;
        conn_.reset();
        EmitEvent({EventKind::Close, 0, {}});
        return;
    }
    if (ssl_ctx_) {
        SSL *ssl = bufferevent_openssl_get_ssl(bev);
        tls::SetSniHostname(ssl, tls::HostnameWithoutPort(config_.ip));
    }
    conn_->Attach(bev);
    sockaddr_in addr{};
    if (FillSockaddr(addr, config_.ip, config_.port)) {
        bufferevent_socket_connect(bev, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
        return;
    }
    bufferevent_socket_connect_hostname(bev, io_.Dns(), AF_UNSPEC, config_.ip.c_str(), config_.port);
}

void TcpClient::Disconnect() {
    connecting_ = false;
    if (conn_) {
        conn_->Close(false);
        conn_.reset();
    }
}

void TcpClient::EmitEvent(ConnEvent ev) {
    if (ev.kind == EventKind::Close) connecting_ = false;
    if (ev.kind == EventKind::Connect) connecting_ = false;
    events_.push_back(std::move(ev));
}

void TcpClient::DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope scope(io_);
    for (const auto &ev: evs) dispatcher(ev);
    if (conn_ && !conn_->IsOpen()) conn_.reset();
}

bool TcpClient::Send(const char *data, size_t len) {
    return conn_ && conn_->Send(data, len);
}

bool TcpClient::Connected() const {
    return conn_ && conn_->Ready();
}

UdpSocket::UdpSocket(const NetConfig &config, ::fakelua::State *state, bool connected)
    : config_(config), connected_(connected), io_(state->GetIoContext()),
      recv_buf_(config.recv_buf_size > 0 ? static_cast<size_t>(config.recv_buf_size) : 65536) {
}

UdpSocket::~UdpSocket() { Stop(); }

bool UdpSocket::Start() {
    fd_ = static_cast<socket_t>(socket(AF_INET, SOCK_DGRAM, 0));
    if (fd_ == INVALID_SOCKET_VAL) return false;
    evutil_make_socket_nonblocking(fd_);
    int opt = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));
    sockaddr_in addr{};
    if (!FillSockaddr(addr, config_.ip, config_.port)) {
        Stop();
        return false;
    }
    if (connected_) {
        if (connect(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 && !WouldBlock(GetLastSocketError())) {
            Stop();
            return false;
        }
    } else {
        if (bind(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
            Stop();
            return false;
        }
        sockaddr_in local{};
        socklen_t slen = sizeof(local);
        if (getsockname(fd_, reinterpret_cast<sockaddr *>(&local), &slen) == 0) bound_port_ = ntohs(local.sin_port);
    }
    ev_ = event_new(io_.Get(), fd_, EV_READ | EV_PERSIST, [](evutil_socket_t, short, void *p) { static_cast<UdpSocket *>(p)->OnRecv(); }, this);
    event_add(ev_, nullptr);
    open_ = true;
    return true;
}

void UdpSocket::Stop() {
    open_ = false;
    if (ev_) {
        event_free(ev_);
        ev_ = nullptr;
    }
    if (fd_ != INVALID_SOCKET_VAL) {
        CloseSocket(fd_);
        fd_ = INVALID_SOCKET_VAL;
    }
}

void UdpSocket::OnRecv() {
    sockaddr_in peer{};
    socklen_t slen = sizeof(peer);
    if (recv_buf_.empty()) recv_buf_.resize(65536);
#if defined(_WIN32)
    auto n = recvfrom(fd_, recv_buf_.data(), static_cast<int>(recv_buf_.size()), 0, reinterpret_cast<sockaddr *>(&peer), &slen);
#else
    auto n = recvfrom(fd_, recv_buf_.data(), recv_buf_.size(), 0, reinterpret_cast<sockaddr *>(&peer), &slen);
#endif
    if (n <= 0) return;
    ConnEvent ev{EventKind::Recv, 0, std::string(recv_buf_.data(), static_cast<size_t>(n))};
    char ip[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
    ev.peer_ip = ip;
    ev.peer_port = ntohs(peer.sin_port);
    EmitEvent(std::move(ev));
}

void UdpSocket::EmitEvent(ConnEvent ev) { events_.push_back(std::move(ev)); }

void UdpSocket::DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope scope(io_);
    for (const auto &ev: evs) dispatcher(ev);
}

bool UdpSocket::Send(const char *data, size_t len) {
    if (!open_ || !connected_) return false;
#if defined(_WIN32)
    return send(fd_, data, static_cast<int>(len), 0) >= 0;
#else
    return send(fd_, data, len, 0) >= 0;
#endif
}

bool UdpSocket::SendTo(const char *data, size_t len, const std::string &ip, uint16_t port) {
    if (!open_) return false;
    sockaddr_in addr{};
    if (!FillSockaddr(addr, ip, port)) return false;
#if defined(_WIN32)
    return sendto(fd_, data, static_cast<int>(len), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) >= 0;
#else
    return sendto(fd_, data, len, 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) >= 0;
#endif
}

}// namespace fakelua::net
