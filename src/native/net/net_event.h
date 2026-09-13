#pragma once

#include "native/native_io_context.h"
#include "native/net/net_buffer.h"
#include "native/net/net_common.h"

#include <event2/event.h>
#include <event2/util.h>
#include <openssl/ssl.h>

struct evconnlistener;

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace fakelua {
class State;
}

namespace fakelua::net {

enum class EventKind {
    Connect,
    Recv,
    Close,
};

struct ConnEvent {
    EventKind kind;
    int conn_id;
    std::string data;
    std::string peer_ip;
    uint16_t peer_port = 0;
};

using EventSink = std::function<void(ConnEvent)>;

class EventConn;

class TcpServer {
public:
    TcpServer(const NetConfig &config, ::fakelua::State *state);
    ~TcpServer();

    void Start();
    void Stop();
    void DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher);
    bool Send(int conn_id, const char *data, size_t len);
    bool CloseConnection(int conn_id);

    [[nodiscard]] bool Running() const { return listener_ != nullptr; }

private:
    void EmitEvent(ConnEvent ev);
    void OnAccept(evutil_socket_t fd);

    NetConfig config_;
    native::IoContext &io_;
    struct evconnlistener *listener_ = nullptr;
    SSL_CTX *ssl_ctx_ = nullptr;
    std::vector<std::shared_ptr<EventConn>> conns_;
    std::vector<ConnEvent> events_;
    native::LifeToken life_;
};

class TcpClient {
public:
    TcpClient(const NetConfig &config, ::fakelua::State *state);
    ~TcpClient();

    void Connect();
    void Disconnect();
    void DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher);
    bool Send(const char *data, size_t len);

    [[nodiscard]] bool Connected() const;

private:
    void EmitEvent(ConnEvent ev);

    NetConfig config_;
    native::IoContext &io_;
    std::shared_ptr<EventConn> conn_;
    SSL_CTX *ssl_ctx_ = nullptr;
    bool connecting_ = false;
    std::vector<ConnEvent> events_;
    native::LifeToken life_;
};

class UdpSocket {
public:
    UdpSocket(const NetConfig &config, ::fakelua::State *state, bool connected);
    ~UdpSocket();

    bool Start();
    void Stop();
    void DrainEventsWith(const std::function<void(const ConnEvent &)> &dispatcher);
    bool Send(const char *data, size_t len);
    bool SendTo(const char *data, size_t len, const std::string &ip, uint16_t port);

    [[nodiscard]] bool Running() const { return open_; }
    [[nodiscard]] bool Connected() const { return connected_; }
    [[nodiscard]] uint16_t BoundPort() const { return bound_port_; }

private:
    void OnRecv();
    void EmitEvent(ConnEvent ev);

    NetConfig config_;
    bool connected_ = false;
    native::IoContext &io_;
    socket_t fd_ = INVALID_SOCKET_VAL;
    event *ev_ = nullptr;
    bool open_ = false;
    uint16_t bound_port_ = 0;
    std::vector<char> recv_buf_;
    std::vector<ConnEvent> events_;
    native::LifeToken life_;
};

}// namespace fakelua::net
