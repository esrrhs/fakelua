#pragma once

// net_asio.h — Boost.Asio 重写的 TCP 引擎
//
// 设计要点：
// - 每个 TcpServer / TcpClient 拥有独立的 io_context + 后台工作线程（与 mysql_connection.cpp 同模式）
// - 读循环通过 async_read_some + CircularBuffer + framer 解包，每解出一个完整包就在引擎内排队
// - 写循环使用 asio::async_write_some 配合每连接的发送缓冲
// - Lua 侧 :tick() 调用 drain_events() 把排队的回调派发出去（保持原 tick_depth / close_pending 语义）
// - 所有 framer 类型、custom Lua parser、WebSocket handshake 全部复用现有 net_buffer/net_websocket

#include "native/net/net_buffer.h"
#include "native/net/net_common.h"
#include "native/net/net_websocket.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace fakelua::net {

// ─────────────────────────────────────────────────────────────────────────────
// 事件队列：引擎线程产生，tick 线程（Lua）消费
// ─────────────────────────────────────────────────────────────────────────────

enum class EventKind {
    Connect, // server 端：新连接已建立
    Recv,    // server 端：完整包解出；client 端：完整包解出
    Close,   // 任意连接关闭
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

    // server 端 accept 后调用，开始读循环
    void start();
    // 主动关闭
    void close();

    // server 端 accept 后 / client 端 connect 后：用外部 socket 替换内部默认构造的 socket
    void reset_socket(boost::asio::ip::tcp::socket sock);

    // 写入数据（按 cfg.framer 自动封包）
    bool send(const char *data, size_t len);
    // 写入原始字节（用于 WS 客户端握手请求，不走 framer）
    bool send_raw(const char *data, size_t len);

    [[nodiscard]] bool is_open() const { return socket_.is_open(); }
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

    CircularBuffer recv_buf_;
    CircularBuffer send_buf_;

    // 当前正在 async_write 的剩余字节，避免与新的 send() 竞争
    std::vector<char> write_inflight_;
    bool writing_ = false;

    // WebSocket 状态机
    WsState ws_state_ = WsState::None;
    bool ws_handshake_sent_ = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// 服务端：异步 acceptor
// ─────────────────────────────────────────────────────────────────────────────

class TcpServer {
public:
    explicit TcpServer(const NetConfig &config);
    ~TcpServer();

    void start();
    void stop();

    // 排空事件并对每条事件调用 dispatcher（在 Lua 调用 tick() 时同步触发 Lua 回调）
    void drain_events_with(const std::function<void(const ConnEvent &)> &dispatcher);

    // 兼容旧 tick() 接口：把 ConnEvent 拆成 3 个 std::function 回调
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

    void worker_loop();
    void emit_event(ConnEvent ev);

    NetConfig config_;
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::acceptor acceptor_{ioc_};
    std::thread worker_;
    bool acceptor_open_ = false;

    // 连接表：conn_id → shared_ptr<AsioConn>
    std::vector<std::shared_ptr<AsioConn>> conns_;
    std::mutex conns_mu_;

    // 事件队列
    std::mutex events_mu_;
    std::vector<ConnEvent> events_;
};

// ─────────────────────────────────────────────────────────────────────────────
// 客户端
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
                case EventKind::Connect: /* client: ignore */ break;
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

    void worker_loop();
    void emit_event(ConnEvent ev);

    NetConfig config_;
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    std::thread worker_;
    std::shared_ptr<AsioConn> conn_;
    bool connecting_ = false;

    std::mutex events_mu_;
    std::vector<ConnEvent> events_;
};

} // namespace fakelua::net
