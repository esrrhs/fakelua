#include "native/net/net_selector.h"

#include <vector>

#if defined(_WIN32)
// Windows Winsock headers already included via net_common.h
#elif defined(__linux__)
#include <sys/epoll.h>
#include <unistd.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

namespace fakelua::net {

TcpLink::TcpLink() : recv_buf(0), send_buf(0) {
    // 不预分配：accept / connect 时会按 config 分配真实大小的缓冲，避免 64KB→1MB 的浪费
}

TcpLink::~TcpLink() {
    if (fd != INVALID_SOCKET_VAL) {
        close_socket(fd);
        fd = INVALID_SOCKET_VAL;
    }
}

Selector::Selector() {
#if defined(__linux__)
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
#endif
}

Selector::~Selector() {
#if defined(__linux__)
    if (epoll_fd_ >= 0) ::close(epoll_fd_);
#endif
}

void Selector::add(socket_t fd, void *userdata) {
    if (iterating_) {
        // 迭代中禁止改写 fd_map_（Windows select 正在遍历该 map，属于 UB）。
        // 只进 pending 队列，本轮 wait 结束后再统一应用。
        pending_add_.push_back(fd);
        pending_add_ud_.push_back(userdata);
        return;
    }
    fd_map_[fd] = userdata;
    write_want_[fd] = false;
#if defined(__linux__)
    apply_epoll_add(fd, userdata, false);
#endif
}

void Selector::remove(socket_t fd) {
    if (iterating_) {
        pending_remove_.push_back(fd);
        return;
    }
    fd_map_.erase(fd);
    write_want_.erase(fd);
#if defined(__linux__)
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
#endif
}

void Selector::clear() {
    for (auto &[fd, _] : fd_map_) {
#if defined(__linux__)
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
#endif
    }
    fd_map_.clear();
    write_want_.clear();
    pending_add_.clear();
    pending_add_ud_.clear();
    pending_remove_.clear();
}

void Selector::set_write_watch(socket_t fd, bool on) {
    auto it = write_want_.find(fd);
    if (it != write_want_.end() && it->second == on) return;
    write_want_[fd] = on;
#if defined(__linux__)
    // 若 fd 已在 epoll 中则 MOD；若尚在 pending_add 里则只记录，apply_epoll_add 时会带上
    if (fd_map_.count(fd)) {
        apply_epoll_mod(fd, on);
    }
#endif
}

#if defined(__linux__)

void Selector::apply_epoll_add(socket_t fd, void *userdata, bool want_write) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLERR | EPOLLRDHUP;
    if (want_write) ev.events |= EPOLLOUT;
    ev.data.fd = static_cast<int>(fd);
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    fd_map_[fd] = userdata;
    write_want_[fd] = want_write;
}

void Selector::apply_epoll_mod(socket_t fd, bool want_write) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLERR | EPOLLRDHUP;
    if (want_write) ev.events |= EPOLLOUT;
    ev.data.fd = static_cast<int>(fd);
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    write_want_[fd] = want_write;
}

#endif

void Selector::flush_pending() {
    for (auto fd : pending_remove_) {
        fd_map_.erase(fd);
        write_want_.erase(fd);
#if defined(__linux__)
        if (epoll_fd_ >= 0) epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
#endif
    }
    pending_remove_.clear();
    for (size_t i = 0; i < pending_add_.size(); ++i) {
        socket_t fd = pending_add_[i];
        void *ud = pending_add_ud_[i];
#if defined(__linux__)
        bool want_write = write_want_.count(fd) && write_want_[fd];
        apply_epoll_add(fd, ud, want_write);
#else
        fd_map_[fd] = ud;
        if (!write_want_.count(fd)) write_want_[fd] = false;
#endif
    }
    pending_add_.clear();
    pending_add_ud_.clear();
}

#if !defined(__linux__)

void Selector::wait(int timeout_ms,
                    const std::function<void(void *)> &on_read,
                    const std::function<void(void *)> &on_write,
                    const std::function<void(void *)> &on_close) {
    flush_pending();
    if (fd_map_.empty()) return;

    fd_set read_set, write_set, except_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&except_set);

    socket_t max_fd = INVALID_SOCKET_VAL;
    bool has_write = false;
    for (auto &[fd, ud] : fd_map_) {
        FD_SET(fd, &read_set);
        if (write_want_.count(fd) && write_want_[fd]) {
            FD_SET(fd, &write_set);
            has_write = true;
        }
        FD_SET(fd, &except_set);
        if (max_fd == INVALID_SOCKET_VAL || fd > max_fd) max_fd = fd;
    }

    timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(static_cast<int>(max_fd) + 1, &read_set, has_write ? &write_set : nullptr, &except_set, &tv);
    if (ret <= 0) return;

    iterating_ = true;
    try {
        for (auto &[fd, ud] : fd_map_) {
            if (FD_ISSET(fd, &except_set)) {
                on_close(ud);
                continue;
            }
            if (FD_ISSET(fd, &read_set)) on_read(ud);
            if (has_write && FD_ISSET(fd, &write_set)) on_write(ud);
        }
    } catch (...) {
        iterating_ = false;
        flush_pending();
        throw;
    }
    iterating_ = false;
    flush_pending();
}

#else // defined(__linux__)

void Selector::wait(int timeout_ms,
                    const std::function<void(void *)> &on_read,
                    const std::function<void(void *)> &on_write,
                    const std::function<void(void *)> &on_close) {
    flush_pending();
    if (epoll_fd_ < 0 || fd_map_.empty()) return;

    epoll_event events[1024];
    int nfds = epoll_wait(epoll_fd_, events, 1024, timeout_ms);
    if (nfds <= 0) return;

    iterating_ = true;
    try {
        for (int i = 0; i < nfds; ++i) {
            socket_t event_fd = events[i].data.fd;
            uint32_t ev = events[i].events;
            auto it = fd_map_.find(event_fd);
            if (it == fd_map_.end()) continue;
            void *ud = it->second;

            // 同一轮中若该连接已被关闭（fd 失效），跳过后续事件，避免对无效 fd 操作。
            if (ud && static_cast<TcpLink *>(ud)->fd == INVALID_SOCKET_VAL) continue;

            if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                on_close(ud);
                continue;
            }
            if (ev & EPOLLIN) on_read(ud);
            if (ev & EPOLLOUT) on_write(ud);
        }
    } catch (...) {
        iterating_ = false;
        flush_pending();
        throw;
    }
    iterating_ = false;
    flush_pending();
}

#endif

} // namespace fakelua::net
