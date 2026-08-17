#pragma once

#include "native/net/net_buffer.h"
#include "native/net/net_common.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace fakelua::net {

struct TcpLink {
    socket_t fd = INVALID_SOCKET_VAL;
    CircularBuffer recv_buf;
    CircularBuffer send_buf;
    bool connected = false;
    int conn_id = -1;

    TcpLink();
    ~TcpLink();
};

class Selector {
public:
    Selector();
    ~Selector();

    void add(socket_t fd, void *userdata);
    void remove(socket_t fd);
    void clear();

    // 等待事件，对每个就绪 link 调用对应回调
    void wait(int timeout_ms,
              const std::function<void(void *userdata)> &on_read,
              const std::function<void(void *userdata)> &on_write,
              const std::function<void(void *userdata)> &on_close);

private:
    std::unordered_map<socket_t, void *> fd_map_;
    bool iterating_ = false;
    std::vector<socket_t> pending_add_;
    std::vector<void *> pending_add_ud_;
    std::vector<socket_t> pending_remove_;
#if defined(__linux__)
    int epoll_fd_ = -1;
#endif
};

} // namespace fakelua::net
