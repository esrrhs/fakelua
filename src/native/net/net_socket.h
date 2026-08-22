#pragma once

#include "native/net/net_buffer.h"
#include "native/net/net_common.h"
#include "native/net/net_selector.h"

#include <functional>
#include <vector>

namespace fakelua::net {

class TcpServer {
public:
    explicit TcpServer(const NetConfig &config);
    ~TcpServer();

    void start();
    void stop();

    void tick(const std::function<void(int)> &on_conn,
              const std::function<void(int, const char *, size_t)> &on_recv,
              const std::function<void(int)> &on_close);

    bool send(int conn_id, const char *data, size_t len);
    void close_connection(int conn_id);

    [[nodiscard]] bool running() const { return listen_fd_ != INVALID_SOCKET_VAL; }

private:
    NetConfig config_;
    socket_t listen_fd_ = INVALID_SOCKET_VAL;
    Selector selector_;

    std::vector<TcpLink *> links_;
    std::vector<int> free_indices_;

    void accept_connections(const std::function<void(int)> &on_conn);
    void handle_link_read(TcpLink *link, const std::function<void(int, const char *, size_t)> &on_recv,
                          const std::function<void(int)> &on_close);
    void handle_link_write(TcpLink *link, const std::function<void(int)> &on_close);
    void close_link(TcpLink *link, const std::function<void(int)> &on_close);
    int alloc_link();
    void free_link(int slot);
};

class TcpClient {
public:
    explicit TcpClient(const NetConfig &config);
    ~TcpClient();

    void connect();
    void disconnect();

    void tick(const std::function<void(const char *, size_t)> &on_recv,
              const std::function<void()> &on_close);

    bool send(const char *data, size_t len);

    [[nodiscard]] bool connected() const { return link_ && link_->connected; }
    [[nodiscard]] bool connecting() const { return connecting_; }

private:
    NetConfig config_;
    TcpLink *link_ = nullptr;
    Selector selector_;
    bool connecting_ = false; // 非阻塞 connect 进行中，等待可写后 SO_ERROR 确认

    void handle_read(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close);
    void handle_write(const std::function<void()> &on_close);
};

} // namespace fakelua::net
