#include "native/net/net_asio.h"

#include "util/logging.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <cstring>
#include <thread>
#include <utility>

namespace fakelua::net {

namespace {

bool is_websocket(const NetConfig &cfg) { return cfg.framer == FramerType::WebSocket; }

void set_socket_options(boost::asio::ip::tcp::socket &sock, const NetConfig &cfg) {
    if (cfg.keep_alive) {
        boost::system::error_code ec;
        sock.set_option(boost::asio::socket_base::keep_alive(true), ec);
    }
    if (cfg.no_delay) {
        boost::system::error_code ec;
        sock.set_option(boost::asio::ip::tcp::no_delay(true), ec);
    }
    boost::system::error_code ec;
    sock.set_option(boost::asio::socket_base::receive_buffer_size(cfg.recv_buf_size), ec);
    sock.set_option(boost::asio::socket_base::send_buffer_size(cfg.send_buf_size), ec);
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// AsioConn
// ─────────────────────────────────────────────────────────────────────────────

AsioConn::AsioConn(boost::asio::io_context &ioc, const NetConfig &cfg, int conn_id, bool from_client, EventSink sink)
    : socket_(ioc), cfg_(cfg), conn_id_(conn_id), from_client_(from_client), sink_(std::move(sink)),
      recv_buf_(cfg.recv_buf_size), send_buf_(cfg.send_buf_size) {
    if (is_websocket(cfg_)) ws_state_ = WsState::Handshake;
}

AsioConn::~AsioConn() {
    boost::system::error_code ec;
    if (socket_.is_open()) socket_.close(ec);
}

void AsioConn::start() {
    do_read();
}

void AsioConn::close() {
    boost::system::error_code ec;
    if (socket_.is_open()) socket_.close(ec);
    if (sink_) sink_({EventKind::Close, conn_id_, {}});
}

void AsioConn::reset_socket(boost::asio::ip::tcp::socket sock) {
    boost::system::error_code ec;
    if (socket_.is_open()) socket_.close(ec);
    socket_ = std::move(sock);
}

bool AsioConn::send(const char *data, size_t len) {
    // WS 握手未完成时不能编码业务数据为 WS 帧（因为还没拿到握手响应，对方不会接受 WS 帧）
    // 返回 false 让调用方稍后重试
    if (is_websocket(cfg_) && ws_state_ != WsState::Open) return false;

    bool ok;
    if (is_websocket(cfg_)) {
        ok = write_ws_frame(send_buf_, cfg_, from_client_, WsOpcode::Text, data, len);
    } else {
        ok = write_packet(send_buf_, cfg_, data, len);
    }
    if (!ok) return false;

    if (!writing_) do_write();
    return true;
}

bool AsioConn::send_raw(const char *data, size_t len) {
    if (send_buf_.write(data, len) != len) return false;
    if (!writing_) do_write();
    return true;
}

void AsioConn::do_read() {
    auto self = shared_from_this();
    // 直接读入 recv_buf 的可写区域
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
        close();
        return;
    }
    LOG_DEBUG("net", "AsioConn::on_read: conn_id={} bytes={}", conn_id_, bytes);
    recv_buf_.commit_write(bytes);

    if (is_websocket(cfg_)) {
        // WS 路径：握手 + 帧解析
        LOG_DEBUG("net", "AsioConn::on_read: WS state={} from_client={} bytes={}", (int)ws_state_, from_client_, bytes);
        if (ws_state_ == WsState::Handshake) {
            if (from_client_) {
                // 客户端：解析握手响应
                bool done = false;
                bool need_more = false;
                bool hs_error = false;
                if (!try_ws_client_handshake(recv_buf_, done, need_more, hs_error)) {
                    LOG_DEBUG("net", "AsioConn::on_read: ws client handshake not done, need_more={} err={}", need_more, hs_error);
                    if (hs_error) {
                        close();
                        return;
                    }
                    do_read();
                    return;
                }
                LOG_DEBUG("net", "AsioConn::on_read: ws client handshake done, switching to Open");
                ws_state_ = WsState::Open;
                if (sink_) sink_({EventKind::Connect, conn_id_, {}});
            } else {
                // 服务端：解析握手请求并生成响应
                std::string response;
                bool need_more = false;
                bool hs_error = false;
                if (!try_ws_server_handshake(recv_buf_, cfg_, response, need_more, hs_error)) {
                    LOG_DEBUG("net", "AsioConn::on_read: ws server handshake not done, need_more={} err={}", need_more, hs_error);
                    if (hs_error) {
                        close();
                        return;
                    }
                    do_read();
                    return;
                }
                send_buf_.write(response.data(), response.size());
                LOG_DEBUG("net", "AsioConn::on_read: ws server handshake done, response={} bytes, switching to Open", response.size());
                ws_state_ = WsState::Open;
                if (!writing_) do_write();
                if (sink_) sink_({EventKind::Connect, conn_id_, {}});
            }
        }
        if (ws_state_ == WsState::Open) {
            while (ws_state_ == WsState::Open) {
                const char *payload = nullptr;
                uint32_t payload_len = 0;
                WsOpcode opcode = WsOpcode::Text;
                bool parse_error = false;
                if (!try_parse_ws_frame(recv_buf_, cfg_, !from_client_, payload, payload_len, opcode, parse_error)) {
                    LOG_DEBUG("net", "AsioConn::on_read: ws frame parse failed, error={} buf_size={}", parse_error, recv_buf_.size());
                    if (parse_error) {
                        close();
                        return;
                    }
                    break;
                }
                LOG_DEBUG("net", "AsioConn::on_read: ws frame parsed opcode={} len={}", (int)opcode, payload_len);
                if (opcode == WsOpcode::Close) {
                    close();
                    return;
                }
                if (opcode == WsOpcode::Ping) {
                    if (write_ws_pong(send_buf_, cfg_, from_client_, payload, payload_len)) {
                        if (!writing_) do_write();
                    }
                    continue;
                }
                if (opcode == WsOpcode::Text || opcode == WsOpcode::Binary) {
                    if (sink_) sink_({EventKind::Recv, conn_id_, std::string(payload, payload_len)});
                }
            }
        }
    } else {
        // 非 WS：按 framer 解包
        while (true) {
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
            LOG_DEBUG("net", "AsioConn: parsed packet conn_id={} len={}", conn_id_, payload_len);
            if (sink_) sink_({EventKind::Recv, conn_id_, std::string(payload, payload_len)});
        }
    }

    // 继续读
    do_read();
}

void AsioConn::do_write() {
    if (writing_) return;
    auto region = send_buf_.readable_region();
    if (region.second == 0) return;

    writing_ = true;
    write_inflight_.assign(region.first, region.first + region.second);
    auto self = shared_from_this();
    LOG_DEBUG("net", "AsioConn: do_write conn_id={} bytes={}", conn_id_, write_inflight_.size());
    boost::asio::async_write(socket_, boost::asio::buffer(write_inflight_.data(), write_inflight_.size()),
                             [self](boost::system::error_code ec, size_t bytes) {
                                 self->on_write(ec, bytes);
                             });
}

void AsioConn::on_write(boost::system::error_code ec, size_t bytes) {
    writing_ = false;
    if (ec) {
        close();
        return;
    }
    send_buf_.commit_read(bytes);
    write_inflight_.clear();
    // 若缓冲还有数据，继续写
    if (!send_buf_.empty()) do_write();
}

// ─────────────────────────────────────────────────────────────────────────────
// TcpServer
// ─────────────────────────────────────────────────────────────────────────────

TcpServer::TcpServer(const NetConfig &config)
    : config_(config) {}

TcpServer::~TcpServer() { stop(); }

void TcpServer::start() {
    boost::system::error_code ec;

    // 解析地址
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
    if (ec) return;
    acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) return;
    acceptor_open_ = true;

    // 预分配连接表
    conns_.assign(config_.max_conn, nullptr);

    // 先发起异步 accept（确保 ioc_ 上有 pending work），再启动工作线程
    // 否则 ioc_.run() 可能在线程启动后立即返回 0
    do_accept();

    worker_ = std::thread([this]() { worker_loop(); });

    LOG_DEBUG("net", "server listening on port {}", config_.port);
}

void TcpServer::stop() {
    acceptor_open_ = false;
    boost::system::error_code ec;
    acceptor_.close(ec);

    {
        std::lock_guard<std::mutex> lock(conns_mu_);
        for (auto &c : conns_) {
            if (c) c->close();
        }
        conns_.clear();
    }

    ioc_.stop();
    if (worker_.joinable()) worker_.join();

    // 重置 io_context 以便可能的复用
    ioc_.restart();
    acceptor_.open(boost::asio::ip::tcp::v4(), ec); // 重新打开 acceptor
}

void TcpServer::drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher) {
    // 让 tick 线程也驱动一下 io_context，并在没有 handler 跑时让出 CPU
    // 短暂 sleep 让 worker 线程有机会完成 IO（保留旧的「tick 同步推进 IO」语义）
    size_t n = ioc_.poll();
    if (n == 0 && events_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        n = ioc_.poll();
    }
    std::vector<ConnEvent> evs;
    {
        std::lock_guard<std::mutex> lock(events_mu_);
        evs.swap(events_);
    }
    for (auto &ev : evs) dispatcher(ev);
}

bool TcpServer::send(int conn_id, const char *data, size_t len) {
    std::shared_ptr<AsioConn> c;
    {
        std::lock_guard<std::mutex> lock(conns_mu_);
        if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size())) return false;
        c = conns_[conn_id];
    }
    if (!c || !c->is_open()) return false;
    return c->send(data, len);
}

bool TcpServer::close_connection(int conn_id) {
    std::shared_ptr<AsioConn> c;
    {
        std::lock_guard<std::mutex> lock(conns_mu_);
        if (conn_id < 0 || conn_id >= static_cast<int>(conns_.size())) return false;
        c = conns_[conn_id];
        if (c) conns_[conn_id].reset();
    }
    if (!c) return false;
    c->close();
    return true;
}

void TcpServer::do_accept() {
    if (!acceptor_open_) return;
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
        on_accept(ec, std::move(sock));
    });
}

void TcpServer::on_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
    LOG_DEBUG("net", "TcpServer::on_accept: ec={} has_socket={}", ec.message(), sock.is_open());
    if (ec) {
        // acceptor 已关闭，停止
        LOG_DEBUG("net", "TcpServer::on_accept error: {}", ec.message());
        return;
    }

    set_socket_options(sock, config_);

    // 找一个空闲槽
    int slot = -1;
    {
        std::lock_guard<std::mutex> lock(conns_mu_);
        for (int i = 0; i < static_cast<int>(conns_.size()); ++i) {
            if (!conns_[i]) {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0) {
        // 池满，关闭客户端
        boost::system::error_code ignore;
        sock.close(ignore);
    } else {
        auto conn = std::make_shared<AsioConn>(ioc_, config_, slot, /*from_client=*/false,
                                               [this](ConnEvent ev) { emit_event(std::move(ev)); });
        {
            std::lock_guard<std::mutex> lock(conns_mu_);
            conns_[slot] = conn;
        }
        conn->reset_socket(std::move(sock));
        conn->start();
        // 非 WS 模式：accept 成功后立即发 Connect 事件（WS 模式在握手完成时发）
        if (!is_websocket(config_)) {
            emit_event({EventKind::Connect, slot, {}});
        }
    }

    // 继续 accept
    do_accept();
}

void TcpServer::worker_loop() {
    ioc_.run();
}

void TcpServer::emit_event(ConnEvent ev) {
    std::lock_guard<std::mutex> lock(events_mu_);
    events_.push_back(std::move(ev));
}

// ─────────────────────────────────────────────────────────────────────────────
// TcpClient
// ─────────────────────────────────────────────────────────────────────────────

TcpClient::TcpClient(const NetConfig &config)
    : config_(config), resolver_(ioc_) {}

TcpClient::~TcpClient() { disconnect(); }

void TcpClient::connect() {
    disconnect();

    connecting_ = true;
    // 先发起异步操作（确保 ioc_ 上有 pending work），再启动工作线程
    // 否则 ioc_.run() 可能在线程启动后立即返回 0（无 work）
    do_resolve();

    worker_ = std::thread([this]() { worker_loop(); });
}

void TcpClient::disconnect() {
    connecting_ = false;
    {
        std::lock_guard<std::mutex> lock(events_mu_);
        events_.clear();
    }
    if (conn_) {
        conn_->close();
        conn_.reset();
    }
    ioc_.stop();
    if (worker_.joinable()) worker_.join();
    ioc_.restart();
}

void TcpClient::drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher) {
    // 与 server 同理
    size_t n = ioc_.poll();
    if (n == 0 && events_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        n = ioc_.poll();
    }
    std::vector<ConnEvent> evs;
    {
        std::lock_guard<std::mutex> lock(events_mu_);
        evs.swap(events_);
    }
    for (auto &ev : evs) dispatcher(ev);
}

bool TcpClient::send(const char *data, size_t len) {
    if (!conn_ || !conn_->is_open()) return false;
    return conn_->send(data, len);
}

void TcpClient::do_resolve() {
    LOG_INFO("net", "TcpClient::do_resolve {}:{}", config_.ip, config_.port);
    resolver_.async_resolve(config_.ip, std::to_string(config_.port),
                            [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
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
                               [this, sock](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
                                   on_connect(ec, std::move(*sock));
                               });
}

void TcpClient::on_connect(boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
    connecting_ = false;
    if (ec) {
        LOG_INFO("net", "TcpClient::on_connect error: {}", ec.message());
        emit_event({EventKind::Close, 0, {}});
        return;
    }
    LOG_INFO("net", "TcpClient::on_connect: success");

    set_socket_options(sock, config_);

    conn_ = std::make_shared<AsioConn>(ioc_, config_, /*conn_id=*/0, /*from_client=*/true,
                                       [this](ConnEvent ev) { emit_event(std::move(ev)); });
    conn_->reset_socket(std::move(sock));

    if (is_websocket(config_)) {
        std::string request, key;
        build_ws_client_handshake_request(config_, request, key);
        conn_->send_raw(request.data(), request.size());
    }

    conn_->start();
    emit_event({EventKind::Connect, 0, {}});
}

void TcpClient::worker_loop() {
    ioc_.run();
}

void TcpClient::emit_event(ConnEvent ev) {
    std::lock_guard<std::mutex> lock(events_mu_);
    events_.push_back(std::move(ev));
}

} // namespace fakelua::net
