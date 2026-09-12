#include "native/net/net_asio.h"

#include "native/tls_util.h"
#include "state/state.h"
#include "util/logging.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include <cstring>
#include <utility>

namespace fakelua::net {

namespace {

bool IsWebSocket(const NetConfig &cfg) {
    return cfg.framer == FramerType::WebSocket;
}

void SetSocketOptions(boost::asio::ip::tcp::socket &sock, const NetConfig &cfg) {
    boost::system::error_code ec;
    // Do not set non_blocking(true). Even with IOCP disabled, a reactor-style
    // write_some/read_some from the poll() thread can wait on the same
    // completion that poll() is already driving and deadlock. HTTP/MySQL/Redis
    // follow the same rule: async_* only, drive with IoContext::Poll().
    if (cfg.keep_alive) {
        sock.set_option(boost::asio::socket_base::keep_alive(true), ec);
    }
    if (cfg.no_delay) {
        sock.set_option(boost::asio::ip::tcp::no_delay(true), ec);
    }
    sock.set_option(boost::asio::socket_base::receive_buffer_size(cfg.recv_buf_size), ec);
    sock.set_option(boost::asio::socket_base::send_buffer_size(cfg.send_buf_size), ec);
}

}// namespace

// AsioConn
AsioConn::AsioConn(boost::asio::io_context &ioc, const NetConfig &cfg, int conn_id, bool from_client, EventSink sink, std::shared_ptr<boost::asio::ssl::context> ssl_ctx)
    : socket_(ioc), cfg_(cfg), conn_id_(conn_id), from_client_(from_client), sink_(std::move(sink)), recv_buf_(cfg.recv_buf_size), send_buf_(cfg.send_buf_size), ssl_ctx_(std::move(ssl_ctx)) {
}

AsioConn::~AsioConn() {
    Close(/*notify_sink=*/false);
}

void AsioConn::Start() {
    if (ssl_) {
        DoSslHandshake();
        return;
    }
    if (UsingWs()) {
        if (from_client_) {
            DoWsClientHandshake();
        } else {
            DoWsServerHandshake();
        }
        return;
    }
    DoRead();
}

void AsioConn::Close(bool notify_sink) {
    if (closed_) return;
    closed_ = true;
    ws_open_ = false;
    ws_write_queue_.clear();
    boost::system::error_code ec;
    if (socket_.is_open()) {
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
    if (notify_sink && sink_) {
        sink_({EventKind::Close, conn_id_, {}});
    }
}

void AsioConn::ResetSocket(boost::asio::ip::tcp::socket sock) {
    wss_.reset();
    ws_.reset();
    ssl_.reset();
    ws_open_ = false;
    ws_buffer_.consume(ws_buffer_.size());
    ws_write_queue_.clear();
    ws_req_ = {};
    boost::system::error_code ec;
    if (socket_.is_open()) socket_.close(ec);
    socket_ = std::move(sock);
    closed_ = false;
    if (cfg_.tls && ssl_ctx_ && IsWebSocket(cfg_)) {
        ssl_.emplace(socket_, *ssl_ctx_);
        wss_.emplace(*ssl_);
        wss_->text(true);
        wss_->read_message_max(static_cast<std::size_t>(cfg_.max_packet_len));
    } else if (IsWebSocket(cfg_)) {
        ws_.emplace(socket_);
        ws_->text(true);
        ws_->read_message_max(static_cast<std::size_t>(cfg_.max_packet_len));
    }
}

bool AsioConn::Send(const char *data, size_t len) {
    if (closed_ || !socket_.is_open()) return false;
    if (UsingWs()) {
        if (!ws_open_) return false;
        if (len > static_cast<size_t>(cfg_.max_packet_len)) return false;
        size_t queued = 0;
        for (const auto &msg: ws_write_queue_) queued += msg.size();
        if (queued + len > static_cast<size_t>(cfg_.send_buf_size)) return false;
        ws_write_queue_.emplace_back(data, len);
        DoWsWrite();
        return true;
    }

    bool ok = WritePacket(send_buf_, cfg_, data, len);
    if (!ok) return false;
    DoWrite();
    return true;
}

bool AsioConn::SendRaw(const char *data, size_t len) {
    if (closed_ || !socket_.is_open() || UsingWs()) return false;
    if (send_buf_.Write(data, len) != len) return false;
    DoWrite();
    return true;
}

void AsioConn::DoRead() {
    if (closed_ || !socket_.is_open() || UsingWs()) return;
    auto self = shared_from_this();
    auto region = recv_buf_.WritableRegion();
    if (region.second == 0) {
        // 缓冲满，关闭连接
        Close();
        return;
    }
    socket_.async_read_some(boost::asio::buffer(region.first, region.second), [self](boost::system::error_code ec, size_t bytes) { self->OnRead(ec, bytes); });
}

void AsioConn::OnRead(boost::system::error_code ec, size_t bytes) {
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            Close();
        }
        return;
    }
    recv_buf_.CommitWrite(bytes);

    while (!closed_) {
        const char *payload = nullptr;
        uint32_t payload_len = 0;
        bool parse_error = false;
        if (!TryParsePacket(recv_buf_, cfg_, payload, payload_len, parse_error)) {
            if (parse_error) {
                Close();
                return;
            }
            break;
        }
        if (sink_) sink_({EventKind::Recv, conn_id_, std::string(payload, payload_len)});
    }

    if (!closed_) {
        DoRead();
    }
}

void AsioConn::DoWrite() {
    if (writing_ || closed_ || !socket_.is_open() || UsingWs()) return;
    if (send_buf_.Empty()) return;

    auto region = send_buf_.ReadableRegion();
    if (region.second == 0) return;

    writing_ = true;
    auto self = shared_from_this();
    socket_.async_write_some(boost::asio::buffer(region.first, region.second), [self](boost::system::error_code ec, size_t bytes) { self->OnWrite(ec, bytes); });
}

void AsioConn::OnWrite(boost::system::error_code ec, size_t bytes) {
    writing_ = false;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            Close();
        }
        return;
    }
    send_buf_.CommitRead(bytes);
    if (!send_buf_.Empty() && !closed_) {
        DoWrite();
    }
}

void AsioConn::DoSslHandshake() {
    if (closed_ || !ssl_) return;
    auto self = shared_from_this();
    if (from_client_) {
        std::string host = tls::HostnameWithoutPort(cfg_.ws_host.empty() ? cfg_.ip : cfg_.ws_host);
        boost::system::error_code sni_ec;
        if (!tls::SetSniHostname(*ssl_, host, sni_ec)) {
            Close();
            return;
        }
        if (cfg_.tls_verify) {
            ssl_->set_verify_callback(boost::asio::ssl::host_name_verification(host));
        }
        ssl_->async_handshake(boost::asio::ssl::stream_base::client, [self](boost::system::error_code ec) { self->OnSslHandshake(ec); });
    } else {
        ssl_->async_handshake(boost::asio::ssl::stream_base::server, [self](boost::system::error_code ec) { self->OnSslHandshake(ec); });
    }
}

void AsioConn::OnSslHandshake(boost::system::error_code ec) {
    if (closed_) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) Close();
        return;
    }
    if (UsingWs()) {
        if (from_client_) {
            DoWsClientHandshake();
        } else {
            DoWsServerHandshake();
        }
        return;
    }
    DoRead();
}

void AsioConn::DoWsServerHandshake() {
    if (closed_ || !UsingWs()) return;
    auto self = shared_from_this();
    auto on_req = [self](boost::system::error_code ec, size_t) { self->OnWsHttpRequest(ec); };
    if (ssl_) {
        boost::beast::http::async_read(*ssl_, ws_buffer_, ws_req_, on_req);
    } else {
        boost::beast::http::async_read(socket_, ws_buffer_, ws_req_, on_req);
    }
}

void AsioConn::OnWsHttpRequest(boost::system::error_code ec) {
    if (closed_ || !UsingWs()) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) Close();
        return;
    }
    const std::string expected = cfg_.ws_path.empty() ? "/" : cfg_.ws_path;
    if (ws_req_.method() != boost::beast::http::verb::get || ws_req_.target() != expected) {
        Close();
        return;
    }
    auto self = shared_from_this();
    auto on_accept = [self](boost::system::error_code accept_ec) { self->OnWsHandshake(accept_ec); };
    if (wss_) {
        wss_->async_accept(ws_req_, on_accept);
    } else {
        ws_->async_accept(ws_req_, on_accept);
    }
}

void AsioConn::DoWsClientHandshake() {
    if (closed_ || !UsingWs()) return;
    std::string host = cfg_.ws_host.empty() ? (cfg_.ip + ":" + std::to_string(cfg_.port)) : cfg_.ws_host;
    const std::string path = cfg_.ws_path.empty() ? "/" : cfg_.ws_path;
    const std::string origin = cfg_.ws_origin;
    auto decorator = [origin](boost::beast::websocket::request_type &req) {
        if (!origin.empty()) {
            req.set(boost::beast::http::field::origin, origin);
        }
    };
    auto self = shared_from_this();
    auto on_hs = [self](boost::system::error_code ec) { self->OnWsHandshake(ec); };
    if (wss_) {
        wss_->set_option(boost::beast::websocket::stream_base::decorator(decorator));
        wss_->async_handshake(host, path, on_hs);
    } else {
        ws_->set_option(boost::beast::websocket::stream_base::decorator(decorator));
        ws_->async_handshake(host, path, on_hs);
    }
}

void AsioConn::OnWsHandshake(boost::system::error_code ec) {
    if (closed_) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) Close();
        return;
    }
    ws_open_ = true;
    if (sink_) sink_({EventKind::Connect, conn_id_, {}});
    DoWsRead();
}

void AsioConn::DoWsRead() {
    if (closed_ || !UsingWs() || !ws_open_) return;
    auto self = shared_from_this();
    auto on_read = [self](boost::system::error_code ec, size_t bytes) { self->OnWsRead(ec, bytes); };
    if (wss_) {
        wss_->async_read(ws_buffer_, on_read);
    } else {
        ws_->async_read(ws_buffer_, on_read);
    }
}

void AsioConn::OnWsRead(boost::system::error_code ec, size_t) {
    if (closed_) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) Close();
        return;
    }
    std::string payload = boost::beast::buffers_to_string(ws_buffer_.data());
    ws_buffer_.consume(ws_buffer_.size());
    if (sink_) sink_({EventKind::Recv, conn_id_, std::move(payload)});
    DoWsRead();
}

void AsioConn::DoWsWrite() {
    if (writing_ || closed_ || !UsingWs() || !ws_open_) return;
    if (ws_write_queue_.empty()) return;
    writing_ = true;
    auto self = shared_from_this();
    auto on_write = [self](boost::system::error_code ec, size_t bytes) { self->OnWsWrite(ec, bytes); };
    if (wss_) {
        wss_->text(true);
        wss_->async_write(boost::asio::buffer(ws_write_queue_.front()), on_write);
    } else {
        ws_->text(true);
        ws_->async_write(boost::asio::buffer(ws_write_queue_.front()), on_write);
    }
}

void AsioConn::OnWsWrite(boost::system::error_code ec, size_t) {
    writing_ = false;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) Close();
        return;
    }
    if (!ws_write_queue_.empty()) ws_write_queue_.pop_front();
    if (!closed_) DoWsWrite();
}

// TcpServer
TcpServer::TcpServer(const NetConfig &config, ::fakelua::State *state) : config_(config), io_(state->GetIoContext()), ioc_(io_.Get()) {
}

TcpServer::~TcpServer() {
    Stop();
}

void TcpServer::Start() {
    if (acceptor_open_) return;
    boost::system::error_code ec;

    if (config_.tls && IsWebSocket(config_)) {
        if (config_.tls_cert.empty() || config_.tls_key.empty()) return;
        try {
            ssl_ctx_ = std::make_shared<boost::asio::ssl::context>(tls::MakeServerContext(config_.tls_cert, config_.tls_key));
        } catch (...) {
            return;
        }
    }

    boost::asio::ip::tcp::endpoint endpoint;
    if (config_.ip.empty() || config_.ip == "0.0.0.0") {
        endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), config_.port);
    } else {
        boost::asio::ip::address addr = boost::asio::ip::make_address(config_.ip, ec);
        if (ec) return;
        endpoint = boost::asio::ip::tcp::endpoint(addr, config_.port);
    }

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) return;
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    acceptor_.bind(endpoint, ec);
    if (ec) {
        acceptor_.close(ec);
        return;
    }
    acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        acceptor_.close(ec);
        return;
    }
    acceptor_open_ = true;

    conns_.assign(config_.max_conn, nullptr);
    DoAccept();
}

void TcpServer::Stop() {
    if (!acceptor_open_) return;
    acceptor_open_ = false;
    boost::system::error_code ec;
    acceptor_.close(ec);

    for (auto &c: conns_) {
        if (c) c->Close(/*notify_sink=*/false);
    }
    conns_.clear();
    events_.clear();

    io_.Poll();
}

void TcpServer::DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope dispatch_scope(io_);
    for (const auto &ev: evs) {
        dispatcher(ev);
    }
}

bool TcpServer::Send(int conn_id, const char *data, size_t len) {
    if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size())) return false;
    auto c = conns_[conn_id];
    if (!c || !c->IsOpen()) return false;
    return c->Send(data, len);
}

bool TcpServer::CloseConnection(int conn_id) {
    if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size())) return false;
    auto c = conns_[conn_id];
    if (!c) return false;
    c->Close(/*notify_sink=*/false);
    conns_[conn_id].reset();
    return true;
}

void TcpServer::DoAccept() {
    if (!acceptor_open_) return;
    acceptor_.async_accept([this, alive = life_.GetWatch()](boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
        if (!alive.Alive()) return;
        OnAccept(ec, std::move(sock));
    });
}

void TcpServer::OnAccept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
    if (ec) return;

    SetSocketOptions(sock, config_);

    int slot = -1;
    for (int i = 0; i < static_cast<int>(conns_.size()); ++i) {
        if (!conns_[i]) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        // 连接池满，拒绝连接
        boost::system::error_code ignore;
        sock.close(ignore);
    } else {
        auto conn = std::make_shared<AsioConn>(ioc_, config_, slot, /*from_client=*/false, [this, alive = life_.GetWatch()](ConnEvent ev) {
            if (!alive.Alive()) return;
            EmitEvent(std::move(ev));
        }, ssl_ctx_);
        conns_[slot] = conn;
        conn->ResetSocket(std::move(sock));
        conn->Start();
        if (!IsWebSocket(config_)) {
            EmitEvent({EventKind::Connect, slot, {}});
        }
    }

    DoAccept();
}

void TcpServer::EmitEvent(ConnEvent ev) {
    if (ev.kind == EventKind::Close) {
        if (ev.conn_id >= 0 && ev.conn_id < static_cast<int>(conns_.size())) {
            conns_[ev.conn_id].reset();
        }
    }
    events_.push_back(std::move(ev));
}

// TcpClient
TcpClient::TcpClient(const NetConfig &config, ::fakelua::State *state) : config_(config), io_(state->GetIoContext()), ioc_(io_.Get()), resolver_(ioc_) {
}

TcpClient::~TcpClient() {
    Disconnect();
}

void TcpClient::Connect() {
    Disconnect();
    connecting_ = true;
    if (config_.tls && IsWebSocket(config_)) {
        try {
            ssl_ctx_ = std::make_shared<boost::asio::ssl::context>(tls::MakeClientContext(config_.tls_verify, config_.tls_ca));
        } catch (...) {
            connecting_ = false;
            EmitEvent({EventKind::Close, 0, {}});
            return;
        }
    }
    DoResolve();
}

void TcpClient::Disconnect() {
    connecting_ = false;
    resolver_.cancel();
    if (conn_) {
        conn_->Close(/*notify_sink=*/false);
        conn_.reset();
    }
    events_.clear();
    io_.Poll();
}

void TcpClient::DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope dispatch_scope(io_);
    for (const auto &ev: evs) {
        dispatcher(ev);
    }
}

bool TcpClient::Send(const char *data, size_t len) {
    if (!conn_ || !conn_->IsOpen()) return false;
    return conn_->Send(data, len);
}

void TcpClient::EmitEvent(ConnEvent ev) {
    if (ev.kind == EventKind::Close) {
        if (conn_) {
            conn_.reset();
        }
    }
    events_.push_back(std::move(ev));
}

void TcpClient::DoResolve() {
    boost::system::error_code ec;
    auto addr = boost::asio::ip::make_address(config_.ip, ec);
    if (!ec) {
        // 直接按 IP 地址连接，跳过 DNS 解析
        auto ep = boost::asio::ip::tcp::endpoint(addr, config_.port);
        auto sock = std::make_shared<boost::asio::ip::tcp::socket>(ioc_);
        sock->open(ep.protocol(), ec);
        if (ec) {
            connecting_ = false;
            EmitEvent({EventKind::Close, 0, {}});
            return;
        }
        SetSocketOptions(*sock, config_);
        sock->async_connect(ep, [this, sock, alive = life_.GetWatch()](boost::system::error_code ec) {
            if (!alive.Alive()) return;
            OnConnect(ec, std::move(*sock));
        });
        return;
    }

    resolver_.async_resolve(config_.ip, std::to_string(config_.port), [this, alive = life_.GetWatch()](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
        if (!alive.Alive()) return;
        OnResolve(ec, std::move(results));
    });
}

void TcpClient::OnResolve(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) {
        connecting_ = false;
        EmitEvent({EventKind::Close, 0, {}});
        return;
    }

    auto sock = std::make_shared<boost::asio::ip::tcp::socket>(ioc_);
    boost::asio::async_connect(*sock, results, [this, sock, alive = life_.GetWatch()](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
        if (!alive.Alive()) return;
        OnConnect(ec, std::move(*sock));
    });
}

void TcpClient::OnConnect(boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
    connecting_ = false;
    if (ec) {
        EmitEvent({EventKind::Close, 0, {}});
        return;
    }

    SetSocketOptions(sock, config_);

    conn_ = std::make_shared<AsioConn>(ioc_, config_, /*conn_id=*/0, /*from_client=*/true, [this, alive = life_.GetWatch()](ConnEvent ev) {
        if (!alive.Alive()) return;
        EmitEvent(std::move(ev));
    }, ssl_ctx_);
    conn_->ResetSocket(std::move(sock));
    conn_->Start();
    if (!IsWebSocket(config_)) {
        EmitEvent({EventKind::Connect, 0, {}});
    }
}

// UdpSocket
UdpSocket::UdpSocket(const NetConfig &config, ::fakelua::State *state, bool connected)
    : config_(config), connected_(connected), io_(state->GetIoContext()), ioc_(io_.Get()) {
}

UdpSocket::~UdpSocket() {
    Stop();
}

bool UdpSocket::Start() {
    if (open_) return true;
    boost::system::error_code ec;
    boost::asio::ip::udp::endpoint ep;
    if (config_.ip.empty() || config_.ip == "0.0.0.0") {
        ep = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), config_.port);
    } else {
        auto addr = boost::asio::ip::make_address(config_.ip, ec);
        if (ec) return false;
        ep = boost::asio::ip::udp::endpoint(addr, config_.port);
    }

    socket_.open(ep.protocol(), ec);
    if (ec) return false;
    socket_.set_option(boost::asio::socket_base::reuse_address(true), ec);

    if (connected_) {
        socket_.connect(ep, ec);
        if (ec) {
            socket_.close();
            return false;
        }
    } else {
        socket_.bind(ep, ec);
        if (ec) {
            socket_.close();
            return false;
        }
    }

    boost::system::error_code lec;
    auto local = socket_.local_endpoint(lec);
    if (!lec) bound_port_ = local.port();

    size_t cap = static_cast<size_t>(config_.max_packet_len);
    if (cap < 1) cap = 64 * 1024;
    recv_buf_.assign(cap, 0);
    open_ = true;
    DoReceive();
    return true;
}

void UdpSocket::Stop() {
    if (!open_ && !socket_.is_open()) return;
    open_ = false;
    boost::system::error_code ec;
    if (socket_.is_open()) {
        socket_.cancel(ec);
        socket_.close(ec);
    }
    events_.clear();
    io_.Poll();
}

void UdpSocket::DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope dispatch_scope(io_);
    for (const auto &ev: evs) {
        dispatcher(ev);
    }
}

bool UdpSocket::Send(const char *data, size_t len) {
    if (!open_ || !socket_.is_open() || !connected_) return false;
    if (len > recv_buf_.size() && !recv_buf_.empty()) return false;
    boost::system::error_code ec;
    socket_.send(boost::asio::buffer(data, len), 0, ec);
    return !ec;
}

bool UdpSocket::SendTo(const char *data, size_t len, const std::string &ip, uint16_t port) {
    if (!open_ || !socket_.is_open()) return false;
    if (!recv_buf_.empty() && len > recv_buf_.size()) return false;
    boost::system::error_code ec;
    auto addr = boost::asio::ip::make_address(ip, ec);
    if (ec) return false;
    socket_.send_to(boost::asio::buffer(data, len), boost::asio::ip::udp::endpoint(addr, port), 0, ec);
    return !ec;
}

void UdpSocket::EmitEvent(ConnEvent ev) {
    events_.push_back(std::move(ev));
}

void UdpSocket::DoReceive() {
    if (!open_ || !socket_.is_open()) return;
    socket_.async_receive_from(boost::asio::buffer(recv_buf_), sender_, [this, alive = life_.GetWatch()](boost::system::error_code ec, std::size_t n) {
        if (!alive.Alive()) return;
        OnReceive(ec, n);
    });
}

void UdpSocket::OnReceive(boost::system::error_code ec, std::size_t n) {
    if (!open_) return;
    if (ec == boost::asio::error::operation_aborted) return;
    if (ec && ec != boost::asio::error::message_size) {
        if (ec == boost::asio::error::connection_refused || ec == boost::asio::error::network_unreachable ||
            ec == boost::asio::error::host_unreachable) {
            DoReceive();
            return;
        }
        EmitEvent({EventKind::Close, 0, {}});
        open_ = false;
        return;
    }
    ConnEvent ev;
    ev.kind = EventKind::Recv;
    ev.conn_id = 0;
    if (n > recv_buf_.size()) n = recv_buf_.size();
    ev.data.assign(recv_buf_.data(), n);
    ev.peer_ip = sender_.address().to_string();
    ev.peer_port = sender_.port();
    EmitEvent(std::move(ev));
    DoReceive();
}

}// namespace fakelua::net
