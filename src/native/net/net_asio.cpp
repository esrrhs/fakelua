#include "native/net/net_asio.h"

#include "state/state.h"
#include "util/logging.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include <cstring>
#include <utility>

namespace fakelua::net {

namespace {

bool is_websocket(const NetConfig &cfg) { return cfg.framer == FramerType::WebSocket; }

void set_socket_options(boost::asio::ip::tcp::socket &sock, const NetConfig &cfg) {
    boost::system::error_code ec;
    // Do not set non_blocking(true): on Windows IOCP a reactor-style
    // write_some from the poll() thread waits on the same IOCP and deadlocks.
    if (cfg.keep_alive) {
        sock.set_option(boost::asio::socket_base::keep_alive(true), ec);
    }
    if (cfg.no_delay) {
        sock.set_option(boost::asio::ip::tcp::no_delay(true), ec);
    }
    sock.set_option(boost::asio::socket_base::receive_buffer_size(cfg.recv_buf_size), ec);
    sock.set_option(boost::asio::socket_base::send_buffer_size(cfg.send_buf_size), ec);
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// AsioConn
// ─────────────────────────────────────────────────────────────────────────────

AsioConn::AsioConn(boost::asio::io_context &ioc, const NetConfig &cfg, int conn_id, bool from_client, EventSink sink)
    : socket_(ioc), cfg_(cfg), conn_id_(conn_id), from_client_(from_client), sink_(std::move(sink)),
      recv_buf_(cfg.recv_buf_size), send_buf_(cfg.send_buf_size) {}

AsioConn::~AsioConn() {
    close(/*notify_sink=*/false);
}

void AsioConn::start() {
    if (ws_) {
        if (from_client_) {
            do_ws_client_handshake();
        } else {
            do_ws_server_handshake();
        }
        return;
    }
    do_read();
}

void AsioConn::close(bool notify_sink) {
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

void AsioConn::reset_socket(boost::asio::ip::tcp::socket sock) {
    ws_.reset();
    ws_open_ = false;
    ws_buffer_.consume(ws_buffer_.size());
    ws_write_queue_.clear();
    ws_req_ = {};
    boost::system::error_code ec;
    if (socket_.is_open()) socket_.close(ec);
    socket_ = std::move(sock);
    closed_ = false;
    if (is_websocket(cfg_)) {
        ws_.emplace(socket_);
        ws_->text(true);
        ws_->read_message_max(static_cast<std::size_t>(cfg_.max_packet_len));
    }
}

bool AsioConn::send(const char *data, size_t len) {
    if (closed_ || !socket_.is_open()) return false;
    if (ws_) {
        if (!ws_open_) return false;
        if (len > static_cast<size_t>(cfg_.max_packet_len)) return false;
        size_t queued = 0;
        for (const auto &msg : ws_write_queue_) queued += msg.size();
        if (queued + len > static_cast<size_t>(cfg_.send_buf_size)) return false;
        ws_write_queue_.emplace_back(data, len);
        do_ws_write();
        return true;
    }

    bool ok = write_packet(send_buf_, cfg_, data, len);
    if (!ok) return false;
    do_write();
    return true;
}

bool AsioConn::send_raw(const char *data, size_t len) {
    if (closed_ || !socket_.is_open() || ws_) return false;
    if (send_buf_.write(data, len) != len) return false;
    do_write();
    return true;
}

void AsioConn::do_read() {
    if (closed_ || !socket_.is_open() || ws_) return;
    auto self = shared_from_this();
    auto region = recv_buf_.writable_region();
    if (region.second == 0) {
        // 缓冲满，关闭连接
        close();
        return;
    }
    socket_.async_read_some(boost::asio::buffer(region.first, region.second),
                            [self](boost::system::error_code ec, size_t bytes) {
                                self->on_read(ec, bytes);
                            });
}

void AsioConn::on_read(boost::system::error_code ec, size_t bytes) {
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            close();
        }
        return;
    }
    recv_buf_.commit_write(bytes);

    while (!closed_) {
        const char *payload = nullptr;
        uint32_t payload_len = 0;
        bool parse_error = false;
        if (!try_parse_packet(recv_buf_, cfg_, payload, payload_len, parse_error)) {
            if (parse_error) {
                close();
                return;
            }
            break;
        }
        if (sink_) sink_({EventKind::Recv, conn_id_, std::string(payload, payload_len)});
    }

    if (!closed_) {
        do_read();
    }
}

void AsioConn::do_write() {
    if (writing_ || closed_ || !socket_.is_open() || ws_) return;
    if (send_buf_.empty()) return;

    auto region = send_buf_.readable_region();
    if (region.second == 0) return;

    writing_ = true;
    auto self = shared_from_this();
    socket_.async_write_some(boost::asio::buffer(region.first, region.second),
                             [self](boost::system::error_code ec, size_t bytes) {
                                 self->on_write(ec, bytes);
                             });
}

void AsioConn::on_write(boost::system::error_code ec, size_t bytes) {
    writing_ = false;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            close();
        }
        return;
    }
    send_buf_.commit_read(bytes);
    if (!send_buf_.empty() && !closed_) {
        do_write();
    }
}

void AsioConn::do_ws_server_handshake() {
    if (closed_ || !ws_) return;
    auto self = shared_from_this();
    boost::beast::http::async_read(socket_, ws_buffer_, ws_req_,
                                   [self](boost::system::error_code ec, size_t) {
                                       self->on_ws_http_request(ec);
                                   });
}

void AsioConn::on_ws_http_request(boost::system::error_code ec) {
    if (closed_ || !ws_) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) close();
        return;
    }
    const std::string expected = cfg_.ws_path.empty() ? "/" : cfg_.ws_path;
    if (ws_req_.method() != boost::beast::http::verb::get ||
        ws_req_.target() != expected) {
        close();
        return;
    }
    auto self = shared_from_this();
    ws_->async_accept(ws_req_, [self](boost::system::error_code accept_ec) {
        self->on_ws_handshake(accept_ec);
    });
}

void AsioConn::do_ws_client_handshake() {
    if (closed_ || !ws_) return;
    std::string host = cfg_.ws_host.empty() ? (cfg_.ip + ":" + std::to_string(cfg_.port)) : cfg_.ws_host;
    const std::string path = cfg_.ws_path.empty() ? "/" : cfg_.ws_path;
    const std::string origin = cfg_.ws_origin;
    ws_->set_option(boost::beast::websocket::stream_base::decorator(
        [origin](boost::beast::websocket::request_type &req) {
            if (!origin.empty()) {
                req.set(boost::beast::http::field::origin, origin);
            }
        }));
    auto self = shared_from_this();
    ws_->async_handshake(host, path, [self](boost::system::error_code ec) {
        self->on_ws_handshake(ec);
    });
}

void AsioConn::on_ws_handshake(boost::system::error_code ec) {
    if (closed_) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) close();
        return;
    }
    ws_open_ = true;
    if (sink_) sink_({EventKind::Connect, conn_id_, {}});
    do_ws_read();
}

void AsioConn::do_ws_read() {
    if (closed_ || !ws_ || !ws_open_) return;
    auto self = shared_from_this();
    ws_->async_read(ws_buffer_, [self](boost::system::error_code ec, size_t bytes) {
        self->on_ws_read(ec, bytes);
    });
}

void AsioConn::on_ws_read(boost::system::error_code ec, size_t) {
    if (closed_) return;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) close();
        return;
    }
    std::string payload = boost::beast::buffers_to_string(ws_buffer_.data());
    ws_buffer_.consume(ws_buffer_.size());
    if (sink_) sink_({EventKind::Recv, conn_id_, std::move(payload)});
    do_ws_read();
}

void AsioConn::do_ws_write() {
    if (writing_ || closed_ || !ws_ || !ws_open_) return;
    if (ws_write_queue_.empty()) return;
    writing_ = true;
    ws_->text(true);
    auto self = shared_from_this();
    ws_->async_write(boost::asio::buffer(ws_write_queue_.front()),
                     [self](boost::system::error_code ec, size_t bytes) {
                         self->on_ws_write(ec, bytes);
                     });
}

void AsioConn::on_ws_write(boost::system::error_code ec, size_t) {
    writing_ = false;
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) close();
        return;
    }
    if (!ws_write_queue_.empty()) ws_write_queue_.pop_front();
    if (!closed_) do_ws_write();
}

// ─────────────────────────────────────────────────────────────────────────────
// TcpServer
// ─────────────────────────────────────────────────────────────────────────────

TcpServer::TcpServer(const NetConfig &config, ::fakelua::State *state)
    : config_(config), io_(state->GetIoContext()), ioc_(io_.Get()) {}

TcpServer::~TcpServer() { stop(); }

void TcpServer::start() {
    if (acceptor_open_) return;
    boost::system::error_code ec;

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
    do_accept();
}

void TcpServer::stop() {
    if (!acceptor_open_) return;
    acceptor_open_ = false;
    boost::system::error_code ec;
    acceptor_.close(ec);

    for (auto &c : conns_) {
        if (c) c->close(/*notify_sink=*/false);
    }
    conns_.clear();
    events_.clear();

    io_.Poll();
}

void TcpServer::drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope dispatch_scope(io_);
    for (const auto &ev : evs) {
        dispatcher(ev);
    }
}

bool TcpServer::send(int conn_id, const char *data, size_t len) {
    if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size())) return false;
    auto c = conns_[conn_id];
    if (!c || !c->is_open()) return false;
    return c->send(data, len);
}

bool TcpServer::close_connection(int conn_id) {
    if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size())) return false;
    auto c = conns_[conn_id];
    if (!c) return false;
    c->close(/*notify_sink=*/false);
    conns_[conn_id].reset();
    return true;
}

void TcpServer::do_accept() {
    if (!acceptor_open_) return;
    acceptor_.async_accept([this, alive = life_.GetWatch()](boost::system::error_code ec,
                                                            boost::asio::ip::tcp::socket sock) {
        if (!alive.Alive()) return;
        on_accept(ec, std::move(sock));
    });
}

void TcpServer::on_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
    if (ec) return;

    set_socket_options(sock, config_);

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
        auto conn = std::make_shared<AsioConn>(ioc_, config_, slot, /*from_client=*/false,
                                               [this, alive = life_.GetWatch()](ConnEvent ev) {
                                                   if (!alive.Alive()) return;
                                                   emit_event(std::move(ev));
                                               });
        conns_[slot] = conn;
        conn->reset_socket(std::move(sock));
        conn->start();
        if (!is_websocket(config_)) {
            emit_event({EventKind::Connect, slot, {}});
        }
    }

    do_accept();
}

void TcpServer::emit_event(ConnEvent ev) {
    if (ev.kind == EventKind::Close) {
        if (ev.conn_id >= 0 && ev.conn_id < static_cast<int>(conns_.size())) {
            conns_[ev.conn_id].reset();
        }
    }
    events_.push_back(std::move(ev));
}

// ─────────────────────────────────────────────────────────────────────────────
// TcpClient
// ─────────────────────────────────────────────────────────────────────────────

TcpClient::TcpClient(const NetConfig &config, ::fakelua::State *state)
    : config_(config), io_(state->GetIoContext()), ioc_(io_.Get()), resolver_(ioc_) {}

TcpClient::~TcpClient() { disconnect(); }

void TcpClient::connect() {
    disconnect();
    connecting_ = true;
    do_resolve();
}

void TcpClient::disconnect() {
    connecting_ = false;
    resolver_.cancel();
    if (conn_) {
        conn_->close(/*notify_sink=*/false);
        conn_.reset();
    }
    events_.clear();
    io_.Poll();
}

void TcpClient::drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher) {
    io_.Poll();
    std::vector<ConnEvent> evs;
    evs.swap(events_);
    native::IoContext::DispatchScope dispatch_scope(io_);
    for (const auto &ev : evs) {
        dispatcher(ev);
    }
}

bool TcpClient::send(const char *data, size_t len) {
    if (!conn_ || !conn_->is_open()) return false;
    return conn_->send(data, len);
}

void TcpClient::emit_event(ConnEvent ev) {
    if (ev.kind == EventKind::Close) {
        if (conn_) {
            conn_.reset();
        }
    }
    events_.push_back(std::move(ev));
}

void TcpClient::do_resolve() {
    boost::system::error_code ec;
    auto addr = boost::asio::ip::make_address(config_.ip, ec);
    if (!ec) {
        // 直接按 IP 地址连接，跳过 DNS 解析
        auto ep = boost::asio::ip::tcp::endpoint(addr, config_.port);
        auto sock = std::make_shared<boost::asio::ip::tcp::socket>(ioc_);
        sock->open(ep.protocol(), ec);
        if (ec) {
            connecting_ = false;
            emit_event({EventKind::Close, 0, {}});
            return;
        }
        set_socket_options(*sock, config_);
        sock->async_connect(ep, [this, sock, alive = life_.GetWatch()](boost::system::error_code ec) {
            if (!alive.Alive()) return;
            on_connect(ec, std::move(*sock));
        });
        return;
    }

    resolver_.async_resolve(config_.ip, std::to_string(config_.port),
                            [this, alive = life_.GetWatch()](boost::system::error_code ec,
                                                             boost::asio::ip::tcp::resolver::results_type results) {
                                if (!alive.Alive()) return;
                                on_resolve(ec, std::move(results));
                            });
}

void TcpClient::on_resolve(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) {
        connecting_ = false;
        emit_event({EventKind::Close, 0, {}});
        return;
    }

    auto sock = std::make_shared<boost::asio::ip::tcp::socket>(ioc_);
    boost::asio::async_connect(*sock, results,
                               [this, sock, alive = life_.GetWatch()](boost::system::error_code ec,
                                                                      boost::asio::ip::tcp::endpoint) {
                                   if (!alive.Alive()) return;
                                   on_connect(ec, std::move(*sock));
                               });
}

void TcpClient::on_connect(boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
    connecting_ = false;
    if (ec) {
        emit_event({EventKind::Close, 0, {}});
        return;
    }

    set_socket_options(sock, config_);

    conn_ = std::make_shared<AsioConn>(ioc_, config_, /*conn_id=*/0, /*from_client=*/true,
                                       [this, alive = life_.GetWatch()](ConnEvent ev) {
                                           if (!alive.Alive()) return;
                                           emit_event(std::move(ev));
                                       });
    conn_->reset_socket(std::move(sock));
    conn_->start();
    if (!is_websocket(config_)) {
        emit_event({EventKind::Connect, 0, {}});
    }
}

} // namespace fakelua::net
