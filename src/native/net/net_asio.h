#pragma once

// net_asio.h — Boost.Asio 基于单线程非阻塞模型的 TCP 引擎
//
// 设计要点：
// - 纯单线程非阻塞架构：移除后台工作线程，所有 IO 操作（accept/read/write/resolve/connect）
//   在 tick() 调用的同一线程上通过 ioc_.poll() 驱动，与 fakelua 单线程极简模型完全契合。
// - 零数据竞争、零锁开销：无多线程竞争，无 std::mutex 开销。
// - 纯非阻塞 tick()：移除无事件时的强制 sleep_for(1ms)，消除帧延迟与 CPU 浪费。
// - 连接槽自动回收：连接关闭时自动重置 slot，彻底解决连接池泄漏和 DoS 风险。
// - 幂等 close 控制：杜绝 duplicate Close 事件风暴。

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "native/net/net_buffer.h"
#include "native/net/net_common.h"
#include "native/net/net_websocket.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace fakelua::net {

// ─────────────────────────────────────────────────────────────────────────────
// 事件队列：单线程驱动产生，tick() 时派发给 Lua
// ─────────────────────────────────────────────────────────────────────────────

enum class EventKind {
    Connect, // server 端：新连接已建立；client 端：连接已建立
    Recv,    // 完整包解出
    Close,   // 连接关闭
};

struct ConnEvent {
    EventKind kind;
    int conn_id;        // server 端连接 ID；client 端恒为 0
    std::string data;   // Recv 时携带载荷
};

using EventSink = std::function<void(ConnEvent)>;

// ─────────────────────────────────────────────────────────────────────────────
// 单条连接（asio::ip::tcp::socket + 收发缓冲）
// ─────────────────────────────────────────────────────────────────────────────

class AsioConn : public std::enable_shared_from_this<AsioConn> {
public:
    AsioConn(boost::asio::io_context &ioc, const NetConfig &cfg, int conn_id, bool from_client, EventSink sink);
    ~AsioConn();

    // server 端 accept 后 / client 端 connect 后调用，开始读循环
    void start();
    // 主动关闭（notify_sink 控制是否通知外部 sink，避免主动 close 时重复发 Close 事件）
    void close(bool notify_sink = true);

    // 用外部已连接/已接收的 socket 替换内部 socket
    void reset_socket(boost::asio::ip::tcp::socket sock);

    // 写入数据（按 cfg.framer 自动封包）
    bool send(const char *data, size_t len);
    // 写入原始字节（用于 WS 客户端握手请求等）
    bool send_raw(const char *data, size_t len);

    [[nodiscard]] bool is_open() const { return !closed_ && socket_.is_open(); }
    [[nodiscard]] int conn_id() const { return conn_id_; }

private:
    void do_read();
    void on_read(boost::system::error_code ec, size_t bytes);
    void do_write();
    void on_write(boost::system::error_code ec, size_t bytes);

    boost::asio::ip::tcp::socket socket_;
    NetConfig cfg_;
    int conn_id_;
    bool from_client_ = false;
    EventSink sink_;
    bool closed_ = false;

    CircularBuffer recv_buf_;
    CircularBuffer send_buf_;

    bool writing_ = false;

    // WebSocket 状态机
    WsState ws_state_ = WsState::None;
    bool ws_handshake_sent_ = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// 服务端：单线程非阻塞 acceptor
// ─────────────────────────────────────────────────────────────────────────────

class TcpServer {
public:
    explicit TcpServer(const NetConfig &config);
    ~TcpServer();

    void start();
    void stop();

    // 排空并派发事件（单线程 ioc_.poll() 驱动就绪 IO）
    void drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher);

    // 兼容旧 tick() 接口
    void tick(const std::function<void(int)> &on_conn,
              const std::function<void(int, const char *, size_t)> &on_recv,
              const std::function<void(int)> &on_close) {
        drain_events_with([&](const ConnEvent &ev) {
            switch (ev.kind) {
                case EventKind::Connect: on_conn(ev.conn_id); break;
                case EventKind::Recv:    on_recv(ev.conn_id, ev.data.data(), ev.data.size()); break;
                case EventKind::Close:   on_close(ev.conn_id); break;
            }
        });
    }

    bool send(int conn_id, const char *data, size_t len);
    bool close_connection(int conn_id);

    [[nodiscard]] bool running() const { return acceptor_open_; }

private:
    void do_accept();
    void on_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock);
    void emit_event(ConnEvent ev);

    NetConfig config_;
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::acceptor acceptor_{ioc_};
    bool acceptor_open_ = false;

    // 连接表：conn_id → shared_ptr<AsioConn>
    std::vector<std::shared_ptr<AsioConn>> conns_;
    std::vector<ConnEvent> events_;
};

// ─────────────────────────────────────────────────────────────────────────────
// 客户端：单线程非阻塞连接
// ─────────────────────────────────────────────────────────────────────────────

class TcpClient {
public:
    explicit TcpClient(const NetConfig &config);
    ~TcpClient();

    void connect();
    void disconnect();

    void drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher);

    bool send(const char *data, size_t len);

    // 兼容旧 tick() 接口
    void tick(const std::function<void(const char *, size_t)> &on_recv,
              const std::function<void()> &on_close) {
        drain_events_with([&](const ConnEvent &ev) {
            switch (ev.kind) {
                case EventKind::Connect: break;
                case EventKind::Recv:    on_recv(ev.data.data(), ev.data.size()); break;
                case EventKind::Close:   on_close(); break;
            }
        });
    }

    [[nodiscard]] bool connected() const { return conn_ && conn_->is_open(); }

private:
    void do_resolve();
    void on_resolve(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
    void on_connect(boost::system::error_code ec, boost::asio::ip::tcp::socket sock);
    void emit_event(ConnEvent ev);

    NetConfig config_;
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::resolver resolver_{ioc_};
    std::shared_ptr<AsioConn> conn_;
    bool connecting_ = false;

    std::vector<ConnEvent> events_;
};

} // namespace fakelua::net
