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

TcpLink::TcpLink() : recv_buf(64 * 1024), send_buf(64 * 1024) {}

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
        pending_add_.push_back(fd);
        pending_add_ud_.push_back(userdata);
        fd_map_[fd] = userdata;
        return;
    }
    fd_map_[fd] = userdata;
#if defined(__linux__)
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLERR | EPOLLRDHUP;
    ev.data.fd = static_cast<int>(fd);
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
#endif
}

void Selector::remove(socket_t fd) {
    if (iterating_) {
        pending_remove_.push_back(fd);
        return;
    }
    fd_map_.erase(fd);
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
    pending_add_.clear();
    pending_add_ud_.clear();
    pending_remove_.clear();
}

#if !defined(__linux__)

void Selector::wait(int timeout_ms,
                    const std::function<void(void *)> &on_read,
                    const std::function<void(void *)> &on_write,
                    const std::function<void(void *)> &on_close) {
    if (fd_map_.empty()) return;

    fd_set read_set, write_set, except_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&except_set);

    socket_t max_fd = INVALID_SOCKET_VAL;
    bool has_write = false;
    for (auto &[fd, ud] : fd_map_) {
        FD_SET(fd, &read_set);
        auto *link = static_cast<TcpLink *>(ud);
        if (link && !link->send_buf.empty()) {
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
    for (auto &[fd, ud] : fd_map_) {
        if (FD_ISSET(fd, &except_set)) {
            on_close(ud);
            continue;
        }
        if (FD_ISSET(fd, &read_set)) on_read(ud);
        if (has_write && FD_ISSET(fd, &write_set)) on_write(ud);
    }
    iterating_ = false;

    for (auto fd : pending_remove_) fd_map_.erase(fd);
    pending_remove_.clear();
    pending_add_.clear();
    pending_add_ud_.clear();
}

#else // defined(__linux__)

void Selector::wait(int timeout_ms,
                    const std::function<void(void *)> &on_read,
                    const std::function<void(void *)> &on_write,
                    const std::function<void(void *)> &on_close) {
    if (epoll_fd_ < 0 || fd_map_.empty()) return;

    epoll_event events[1024];
    int nfds = epoll_wait(epoll_fd_, events, 1024, timeout_ms);
    if (nfds <= 0) return;

    iterating_ = true;
    for (int i = 0; i < nfds; ++i) {
        socket_t event_fd = events[i].data.fd;
        uint32_t ev = events[i].events;
        auto it = fd_map_.find(event_fd);
        if (it == fd_map_.end()) continue;
        void *ud = it->second;

        if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
            on_close(ud);
            continue;
        }
        if (ev & EPOLLIN) on_read(ud);
        if (ev & EPOLLOUT) on_write(ud);
    }
    iterating_ = false;

    for (auto fd : pending_remove_) {
        fd_map_.erase(fd);
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    }
    pending_remove_.clear();
    for (size_t i = 0; i < pending_add_.size(); ++i) {
        socket_t fd = pending_add_[i];
        void *ud = pending_add_ud_[i];
        epoll_event ev{};
        ev.events = (ud == nullptr) ? (EPOLLIN | EPOLLERR | EPOLLRDHUP)
                                   : (EPOLLIN | EPOLLERR | EPOLLRDHUP);
        ev.data.fd = static_cast<int>(fd);
        int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
        if (rc == 0) {
            fd_map_[fd] = ud;
        }
    }
    pending_add_.clear();
    pending_add_ud_.clear();
}

#endif

} // namespace fakelua::net
