#pragma once

// net_asio.h — Boost.Asio 基于单线程非阻塞模型的 TCP 引擎
// 设计要点：
// - 纯单线程非阻塞架构：移除后台工作线程，所有 IO 操作（accept/read/write/resolve/connect）
//   在 tick() 调用的同一线程上驱动，与 fakelua 单线程极简模型完全契合。
// - 事件循环按 State 共享：ioc_ 取自 State，不是自己持有的 io_context，原因见
//   native_io_context.h。
// - 零数据竞争、零锁开销：无多线程竞争，无 std::mutex 开销。
// - 纯非阻塞 tick()：移除无事件时的强制 sleep_for(1ms)，消除帧延迟与 CPU 浪费。
// - 连接槽自动回收：连接关闭时自动重置 slot，彻底解决连接池泄漏和 DoS 风险。
// - 幂等 close 控制：杜绝 duplicate Close 事件风暴。

#include "native/native_io_context.h"
#include "native/net/net_buffer.h"
#include "native/net/net_common.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fakelua {
class State;
}

namespace fakelua::net {

// 事件队列：单线程驱动产生，tick() 时派发给 Lua
enum class EventKind {
    // server 端：新连接已建立；client 端：连接已建立
    Connect,
    // 完整包解出
    Recv,
    // 连接关闭
    Close,
};

struct ConnEvent {
    EventKind kind;
    // server 端连接 ID；client 端恒为 0；UDP 恒为 0
    int conn_id;
    // Recv 时携带载荷
    std::string data;
    // UDP Recv 时对端地址
    std::string peer_ip;
    uint16_t peer_port = 0;
};

using EventSink = std::function<void(ConnEvent)>;

// 单条连接（asio::ip::tcp::socket + 收发缓冲）
class AsioConn : public std::enable_shared_from_this<AsioConn> {
public:
    AsioConn(boost::asio::io_context &ioc, const NetConfig &cfg, int conn_id, bool from_client, EventSink sink, std::shared_ptr<boost::asio::ssl::context> ssl_ctx = nullptr);
    ~AsioConn();

    // server 端 accept 后 / client 端 connect 后调用，开始读循环
    void Start();
    // 主动关闭（notify_sink 控制是否通知外部 sink，避免主动 close 时重复发 Close 事件）
    void Close(bool notify_sink = true);

    // 用外部已连接/已接收的 socket 替换内部 socket
    void ResetSocket(boost::asio::ip::tcp::socket sock);

    // 写入数据（按 cfg.framer 自动封包）
    bool Send(const char *data, size_t len);
    // 写入原始字节（非 WebSocket 连接）
    bool SendRaw(const char *data, size_t len);

    [[nodiscard]] bool IsOpen() const {
        return !closed_ && socket_.is_open();
    }

    [[nodiscard]] int ConnId() const {
        return conn_id_;
    }

private:
    void DoRead();
    void OnRead(boost::system::error_code ec, size_t bytes);
    void DoWrite();
    void OnWrite(boost::system::error_code ec, size_t bytes);

    void DoSslHandshake();
    void OnSslHandshake(boost::system::error_code ec);
    void DoWsServerHandshake();
    void OnWsHttpRequest(boost::system::error_code ec);
    void DoWsClientHandshake();
    void OnWsHandshake(boost::system::error_code ec);
    void DoWsRead();
    void OnWsRead(boost::system::error_code ec, size_t bytes);
    void DoWsWrite();
    void OnWsWrite(boost::system::error_code ec, size_t bytes);
    [[nodiscard]] bool UsingWs() const {
        return static_cast<bool>(ws_) || static_cast<bool>(wss_);
    }

    boost::asio::ip::tcp::socket socket_;
    NetConfig cfg_;
    int conn_id_;
    bool from_client_ = false;
    EventSink sink_;
    bool closed_ = false;

    CircularBuffer recv_buf_;
    CircularBuffer send_buf_;

    bool writing_ = false;

    std::shared_ptr<boost::asio::ssl::context> ssl_ctx_;
    using SslStream = boost::asio::ssl::stream<boost::asio::ip::tcp::socket &>;
    using WsStream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket &>;
    using WssStream = boost::beast::websocket::stream<SslStream &>;
    std::optional<SslStream> ssl_;
    std::optional<WsStream> ws_;
    std::optional<WssStream> wss_;
    boost::beast::flat_buffer ws_buffer_;
    boost::beast::http::request<boost::beast::http::string_body> ws_req_;
    std::deque<std::string> ws_write_queue_;
    bool ws_open_ = false;
};

// 服务端：单线程非阻塞 acceptor
class TcpServer {
public:
    TcpServer(const NetConfig &config, ::fakelua::State *state);
    ~TcpServer();

    void Start();
    void Stop();

    // 排空并派发事件（单线程 ioc_.poll() 驱动就绪 IO）
    void DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher);

    // 兼容旧 tick() 接口
    void Tick(const std::function<void(int)> &on_conn, const std::function<void(int, const char *, size_t)> &on_recv, const std::function<void(int)> &on_close) {
        DrainEventsWith([&](const ConnEvent &ev) {
            switch (ev.kind) {
                case EventKind::Connect:
                    on_conn(ev.conn_id);
                    break;
                case EventKind::Recv:
                    on_recv(ev.conn_id, ev.data.data(), ev.data.size());
                    break;
                case EventKind::Close:
                    on_close(ev.conn_id);
                    break;
            }
        });
    }

    bool Send(int conn_id, const char *data, size_t len);
    bool CloseConnection(int conn_id);

    [[nodiscard]] bool Running() const {
        return acceptor_open_;
    }

private:
    void DoAccept();
    void OnAccept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock);
    void EmitEvent(ConnEvent ev);

    NetConfig config_;
    native::IoContext &io_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::tcp::acceptor acceptor_{ioc_};
    bool acceptor_open_ = false;
    std::shared_ptr<boost::asio::ssl::context> ssl_ctx_;

    // 连接表：conn_id → shared_ptr<AsioConn>
    std::vector<std::shared_ptr<AsioConn>> conns_;
    std::vector<ConnEvent> events_;

    // 放在最后：io_ 比本对象活得久，未完成的 accept 会真的被投递（以前 io_context
    // 跟着对象一起销毁，这些操作是被直接丢弃的），所以捕获裸 this 的回调必须先确认
    // 这个标记还活着。
    native::LifeToken life_;
};

// 客户端：单线程非阻塞连接
class TcpClient {
public:
    TcpClient(const NetConfig &config, ::fakelua::State *state);
    ~TcpClient();

    void Connect();
    void Disconnect();

    void DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher);

    bool Send(const char *data, size_t len);

    // 兼容旧 tick() 接口
    void Tick(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close) {
        DrainEventsWith([&](const ConnEvent &ev) {
            switch (ev.kind) {
                case EventKind::Connect:
                    break;
                case EventKind::Recv:
                    on_recv(ev.data.data(), ev.data.size());
                    break;
                case EventKind::Close:
                    on_close();
                    break;
            }
        });
    }

    [[nodiscard]] bool Connected() const {
        return conn_ && conn_->IsOpen();
    }

private:
    void DoResolve();
    void OnResolve(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
    void OnConnect(boost::system::error_code ec, boost::asio::ip::tcp::socket sock);
    void EmitEvent(ConnEvent ev);

    NetConfig config_;
    native::IoContext &io_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::tcp::resolver resolver_{ioc_};
    std::shared_ptr<AsioConn> conn_;
    std::shared_ptr<boost::asio::ssl::context> ssl_ctx_;
    bool connecting_ = false;

    std::vector<ConnEvent> events_;

    // 同 TcpServer::life_：resolve/connect 回调据此判断本对象是否还在。
    native::LifeToken life_;
};

// 数据报套接字：单线程非阻塞，由 runtime.tick() 的 poll() 驱动。
// connected=false：bind 本地 ip:port，send 需带对端；connected=true：connect 到 ip:port。
class UdpSocket {
public:
    UdpSocket(const NetConfig &config, ::fakelua::State *state, bool connected);
    ~UdpSocket();

    bool Start();
    void Stop();

    void DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher);

    bool Send(const char *data, size_t len);
    bool SendTo(const char *data, size_t len, const std::string &ip, uint16_t port);

    [[nodiscard]] bool Running() const {
        return open_;
    }

    [[nodiscard]] bool Connected() const {
        return connected_;
    }

    [[nodiscard]] uint16_t BoundPort() const {
        return bound_port_;
    }

private:
    void DoReceive();
    void OnReceive(boost::system::error_code ec, std::size_t n);
    void EmitEvent(ConnEvent ev);

    NetConfig config_;
    bool connected_ = false;
    native::IoContext &io_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::udp::socket socket_{ioc_};
    boost::asio::ip::udp::endpoint sender_;
    std::vector<char> recv_buf_;
    bool open_ = false;
    uint16_t bound_port_ = 0;
    std::vector<ConnEvent> events_;

    native::LifeToken life_;
};

}// namespace fakelua::net
