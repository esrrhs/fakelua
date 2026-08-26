#pragma once

#include "native/net/net_buffer.h"
#include "native/net/net_common.h"
#include "native/net/net_websocket.h"

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
    WsState ws_state = WsState::None;
    bool ws_handshake_sent = false;

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

    // 设置/取消某 fd 的写就绪监视（EPOLLOUT）。
    // 当 send 缓冲区有数据（或客户端正在连接）时应开启，发完后关闭。
    void set_write_watch(socket_t fd, bool on);

    // 等待事件，对每个就绪 link 调用对应回调
    void wait(int timeout_ms,
              const std::function<void(void *userdata)> &on_read,
              const std::function<void(void *userdata)> &on_write,
              const std::function<void(void *userdata)> &on_close);

private:
    void flush_pending();

#if defined(__linux__)
    // 向 epoll 注册 fd（ADD），按需包含 EPOLLOUT
    void apply_epoll_add(socket_t fd, void *userdata, bool want_write);
    // 修改已注册 fd 的事件（MOD），调整 EPOLLOUT
    void apply_epoll_mod(socket_t fd, bool want_write);
#endif

    std::unordered_map<socket_t, void *> fd_map_;
    // 记录每个 fd 当前是否需要写就绪监视，避免重复 epoll_ctl 并供 Windows select 路径使用
    std::unordered_map<socket_t, bool> write_want_;
    bool iterating_ = false;
    std::vector<socket_t> pending_add_;
    std::vector<void *> pending_add_ud_;
    std::vector<socket_t> pending_remove_;
#if defined(__linux__)
    int epoll_fd_ = -1;
#endif
};

} // namespace fakelua::net
