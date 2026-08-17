#include "native/net/net_internal.h"

#include <algorithm>
#include <cerrno>
#include <cstring>

#if defined(_WIN32)
#pragma comment(lib, "ws2_32.lib")
#elif defined(__linux__)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace fakelua::net {

// ─────────────────────────────────────────────────────────────────────────────
// 平台工具
// ─────────────────────────────────────────────────────────────────────────────

static int get_last_socket_error() {
#if defined(_WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}

static bool would_block(int err) {
#if defined(_WIN32)
    return err == WSAEWOULDBLOCK || err == WSAEINPROGRESS;
#else
    return err == EAGAIN || err == EWOULDBLOCK || err == EINPROGRESS;
#endif
}

static void close_socket(socket_t fd) {
    if (fd == INVALID_SOCKET_VAL) return;
#if defined(_WIN32)
    closesocket(fd);
#else
    ::close(fd);
#endif
}

static bool set_non_blocking(socket_t fd) {
#if defined(_WIN32)
    u_long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0;
#endif
}

static void set_socket_options(socket_t fd, const NetConfig &cfg) {
    int opt = 1;
    if (cfg.keep_alive) {
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&opt), sizeof(opt));
    }
    if (cfg.no_delay) {
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&opt), sizeof(opt));
    }
    int sndbuf = cfg.send_buf_size;
    int rcvbuf = cfg.recv_buf_size;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&sndbuf), sizeof(sndbuf));
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&rcvbuf), sizeof(rcvbuf));
}

static bool fill_sockaddr(sockaddr_in &addr, const std::string &ip, uint16_t port) {
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip.empty() || ip == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
        return true;
    }
    return inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) == 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// CircularBuffer
// ─────────────────────────────────────────────────────────────────────────────

CircularBuffer::CircularBuffer(size_t capacity) : buf_(capacity) {}

CircularBuffer::~CircularBuffer() = default;

void CircularBuffer::clear() {
    head_ = tail_ = size_ = 0;
}

size_t CircularBuffer::write(const char *data, size_t len) {
    size_t cap = buf_.size();
    size_t avail = cap - size_;
    len = std::min(len, avail);
    if (len == 0) return 0;
    size_t first = std::min(len, cap - tail_);
    std::memcpy(buf_.data() + tail_, data, first);
    size_t second = len - first;
    if (second > 0) {
        std::memcpy(buf_.data(), data + first, second);
    }
    tail_ = (tail_ + len) % cap;
    size_ += len;
    return len;
}

size_t CircularBuffer::read(char *dst, size_t len) {
    len = std::min(len, size_);
    if (len == 0) return 0;
    size_t cap = buf_.size();
    size_t first = std::min(len, cap - head_);
    std::memcpy(dst, buf_.data() + head_, first);
    size_t second = len - first;
    if (second > 0) {
        std::memcpy(dst + first, buf_.data(), second);
    }
    head_ = (head_ + len) % cap;
    size_ -= len;
    return len;
}

size_t CircularBuffer::peek(char *dst, size_t len) const {
    len = std::min(len, size_);
    if (len == 0) return 0;
    size_t cap = buf_.size();
    size_t first = std::min(len, cap - head_);
    std::memcpy(dst, buf_.data() + head_, first);
    size_t second = len - first;
    if (second > 0) {
        std::memcpy(dst + first, buf_.data(), second);
    }
    return len;
}

size_t CircularBuffer::skip(size_t len) {
    len = std::min(len, size_);
    head_ = (head_ + len) % buf_.size();
    size_ -= len;
    return len;
}

std::pair<char *, size_t> CircularBuffer::writable_region() {
    size_t cap = buf_.size();
    if (size_ >= cap) return {nullptr, 0};
    size_t end = (head_ > tail_) ? head_ : cap;
    return {buf_.data() + tail_, end - tail_};
}

void CircularBuffer::commit_write(size_t bytes) {
    size_t cap = buf_.size();
    tail_ = (tail_ + bytes) % cap;
    size_ += bytes;
}

std::pair<const char *, size_t> CircularBuffer::readable_region() {
    size_t cap = buf_.size();
    if (size_ == 0) return {nullptr, 0};
    size_t end = (tail_ > head_) ? tail_ : cap;
    return {buf_.data() + head_, end - head_};
}

void CircularBuffer::commit_read(size_t bytes) {
    size_t cap = buf_.size();
    head_ = (head_ + bytes) % cap;
    size_ -= bytes;
}

// ─────────────────────────────────────────────────────────────────────────────
// TcpLink
// ─────────────────────────────────────────────────────────────────────────────

TcpLink::TcpLink() : recv_buf(64 * 1024), send_buf(64 * 1024) {}

TcpLink::~TcpLink() {
    if (fd != INVALID_SOCKET_VAL) {
        close_socket(fd);
        fd = INVALID_SOCKET_VAL;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Selector
// ─────────────────────────────────────────────────────────────────────────────

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
    for (auto &[fd, _] : fd_map_) {
        FD_SET(fd, &read_set);
        FD_SET(fd, &write_set);
        FD_SET(fd, &except_set);
        if (max_fd == INVALID_SOCKET_VAL || fd > max_fd) max_fd = fd;
    }

    timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(static_cast<int>(max_fd) + 1, &read_set, &write_set, &except_set, &tv);
    if (ret <= 0) return;

    iterating_ = true;
    for (auto &[fd, ud] : fd_map_) {
        if (FD_ISSET(fd, &except_set)) {
            on_close(ud);
            continue;
        }
        if (FD_ISSET(fd, &read_set)) on_read(ud);
        if (FD_ISSET(fd, &write_set)) on_write(ud);
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

// ─────────────────────────────────────────────────────────────────────────────
// TcpServer
// ─────────────────────────────────────────────────────────────────────────────

TcpServer::TcpServer(const NetConfig &config) : config_(config) {}

TcpServer::~TcpServer() {
    stop();
    for (auto *link : links_) delete link;
}

void TcpServer::start() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd_ == INVALID_SOCKET_VAL) return;

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));

    sockaddr_in addr{};
    if (!fill_sockaddr(addr, config_.ip, config_.port)) {
        close_socket(listen_fd_);
        listen_fd_ = INVALID_SOCKET_VAL;
        return;
    }

    if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
        close_socket(listen_fd_);
        listen_fd_ = INVALID_SOCKET_VAL;
        return;
    }

    if (listen(listen_fd_, config_.backlog) != 0) {
        close_socket(listen_fd_);
        listen_fd_ = INVALID_SOCKET_VAL;
        return;
    }

    if (config_.non_blocking) set_non_blocking(listen_fd_);

    // 预分配连接池
    links_.resize(config_.max_conn, nullptr);
    for (int i = 0; i < config_.max_conn; ++i) {
        links_[i] = new TcpLink();
        links_[i]->conn_id = i;
        free_indices_.push_back(config_.max_conn - 1 - i);
    }

    selector_.add(listen_fd_, nullptr); // listen_fd 的 userdata 为 nullptr
}

void TcpServer::stop() {
    if (listen_fd_ != INVALID_SOCKET_VAL) {
        selector_.remove(listen_fd_);
        close_socket(listen_fd_);
        listen_fd_ = INVALID_SOCKET_VAL;
    }
    for (int i = 0; i < config_.max_conn; ++i) {
        if (links_[i] && links_[i]->connected) {
            close_socket(links_[i]->fd);
            links_[i]->connected = false;
            links_[i]->fd = INVALID_SOCKET_VAL;
            links_[i]->recv_buf.clear();
            links_[i]->send_buf.clear();
        }
    }
    selector_.clear();
    free_indices_.clear();
    for (int i = 0; i < config_.max_conn; ++i) free_indices_.push_back(config_.max_conn - 1 - i);
}

int TcpServer::alloc_link() {
    if (free_indices_.empty()) return -1;
    int slot = free_indices_.back();
    free_indices_.pop_back();
    return slot;
}

void TcpServer::free_link(int slot) {
    if (slot < 0 || slot >= config_.max_conn) return;
    links_[slot]->connected = false;
    links_[slot]->fd = INVALID_SOCKET_VAL;
    links_[slot]->recv_buf.clear();
    links_[slot]->send_buf.clear();
    free_indices_.push_back(slot);
}

void TcpServer::accept_connections(const std::function<void(int)> &on_conn) {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        socket_t client_fd = accept(listen_fd_, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);
        if (client_fd == INVALID_SOCKET_VAL) break;

        int slot = alloc_link();
        if (slot < 0) {
            close_socket(client_fd);
            break;
        }

        if (config_.non_blocking) set_non_blocking(client_fd);
        set_socket_options(client_fd, config_);

        auto *link = links_[slot];
        link->fd = client_fd;
        link->connected = true;
        link->recv_buf = CircularBuffer(config_.recv_buf_size);
        link->send_buf = CircularBuffer(config_.send_buf_size);

        selector_.add(client_fd, link);
        on_conn(slot);
    }
}

void TcpServer::handle_link_read(TcpLink *link, const std::function<void(int, const char *, size_t)> &on_recv,
                                 const std::function<void(int)> &on_close) {
    auto [ptr, len] = link->recv_buf.writable_region();
    if (len == 0) return;

    int n = static_cast<int>(::recv(link->fd, ptr, len, 0));
    int err = get_last_socket_error();
    if (n > 0) {
        link->recv_buf.commit_write(n);
        while (true) {
            const char *payload = nullptr;
            uint32_t payload_len = 0;
            if (!try_parse_packet(link->recv_buf, payload, payload_len)) break;
            on_recv(link->conn_id, payload, payload_len);
        }
    } else if (n == 0) {
        close_link(link, on_close);
    } else {
        if (!would_block(err)) close_link(link, on_close);
    }
}

void TcpServer::handle_link_write(TcpLink *link) {
    auto [ptr, len] = link->send_buf.readable_region();
    if (len == 0) return;

    int n;
#if defined(_WIN32)
    n = ::send(link->fd, ptr, static_cast<int>(len), 0);
#else
    n = static_cast<int>(::send(link->fd, ptr, len, 0));
#endif
    if (n > 0) {
        link->send_buf.commit_read(n);
    } else if (n < 0) {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            selector_.remove(link->fd);
            close_socket(link->fd);
            link->fd = INVALID_SOCKET_VAL;
            link->connected = false;
        }
    }
}

void TcpServer::close_link(TcpLink *link, const std::function<void(int)> &on_close) {
    if (!link || !link->connected) return;
    selector_.remove(link->fd);
    close_socket(link->fd);
    int id = link->conn_id;
    free_link(link->conn_id);
    on_close(id);
}

void TcpServer::tick(const std::function<void(int)> &on_conn,
                     const std::function<void(int, const char *, size_t)> &on_recv,
                     const std::function<void(int)> &on_close) {
    selector_.wait(config_.wait_timeout_ms,
                  [this, &on_conn, &on_recv, &on_close](void *ud) {
                      if (ud == nullptr || static_cast<TcpLink *>(ud)->fd == listen_fd_) {
                          accept_connections(on_conn);
                      } else {
                          handle_link_read(static_cast<TcpLink *>(ud), on_recv, on_close);
                      }
                  },
                  [this](void *ud) {
                      if (ud) handle_link_write(static_cast<TcpLink *>(ud));
                  },
                  [this, &on_close](void *ud) {
                      if (ud) close_link(static_cast<TcpLink *>(ud), on_close);
                  });

    // 尝试发送所有连接的缓冲数据
    for (int i = 0; i < config_.max_conn; ++i) {
        if (links_[i] && links_[i]->connected && !links_[i]->send_buf.empty()) {
            handle_link_write(links_[i]);
        }
    }
}

bool TcpServer::send(int conn_id, const char *data, size_t len) {
    if (conn_id < 0 || conn_id >= config_.max_conn) return false;
    auto *link = links_[conn_id];
    if (!link || !link->connected) return false;
    write_packet_header(link->send_buf, static_cast<uint32_t>(len));
    link->send_buf.write(data, len);
    handle_link_write(link);
    return true;
}

void TcpServer::close_connection(int conn_id) {
    if (conn_id < 0 || conn_id >= config_.max_conn) return;
    auto *link = links_[conn_id];
    if (!link || !link->connected) return;
    selector_.remove(link->fd);
    close_socket(link->fd);
    free_link(conn_id);
}

// ─────────────────────────────────────────────────────────────────────────────
// TcpClient
// ─────────────────────────────────────────────────────────────────────────────

TcpClient::TcpClient(const NetConfig &config) : config_(config) {}

TcpClient::~TcpClient() {
    disconnect();
}

void TcpClient::connect() {
    if (link_) disconnect();

    socket_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET_VAL) return;

    if (config_.non_blocking) set_non_blocking(fd);

    sockaddr_in addr{};
    if (!fill_sockaddr(addr, config_.ip, config_.port)) {
        close_socket(fd);
        return;
    }

    int ret = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (ret != 0) {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            close_socket(fd);
            return;
        }
    }

    set_socket_options(fd, config_);

    link_ = new TcpLink();
    link_->fd = fd;
    link_->connected = true;
    link_->recv_buf = CircularBuffer(config_.recv_buf_size);
    link_->send_buf = CircularBuffer(config_.send_buf_size);

    selector_.add(fd, link_);
}

void TcpClient::disconnect() {
    if (link_) {
        selector_.remove(link_->fd);
        close_socket(link_->fd);
        delete link_;
        link_ = nullptr;
    }
}

void TcpClient::handle_read(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close) {
    if (!link_ || !link_->connected) return;

    auto [ptr, len] = link_->recv_buf.writable_region();
    if (len == 0) return;

    int n;
#if defined(_WIN32)
    n = recv(link_->fd, ptr, static_cast<int>(len), 0);
#else
    n = static_cast<int>(::recv(link_->fd, ptr, len, 0));
#endif
    if (n > 0) {
        link_->recv_buf.commit_write(n);
        while (true) {
            const char *payload = nullptr;
            uint32_t payload_len = 0;
            if (!try_parse_packet(link_->recv_buf, payload, payload_len)) break;
            on_recv(payload, payload_len);
        }
    } else if (n == 0) {
        link_->connected = false;
        on_close();
    } else {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            link_->connected = false;
            on_close();
        }
    }
}

void TcpClient::handle_write() {
    if (!link_ || !link_->connected) return;
    auto [ptr, len] = link_->send_buf.readable_region();
    if (len == 0) return;

    int n;
#if defined(_WIN32)
    n = ::send(link_->fd, ptr, static_cast<int>(len), 0);
#else
    n = static_cast<int>(::send(link_->fd, ptr, len, 0));
#endif
    if (n > 0) {
        link_->send_buf.commit_read(n);
    } else if (n < 0) {
        int err = get_last_socket_error();
        if (!would_block(err)) link_->connected = false;
    }
}

void TcpClient::tick(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close) {
    if (!link_ || !link_->connected) return;

    selector_.wait(config_.wait_timeout_ms,
                  [this, &on_recv, &on_close](void * /*ud*/) { handle_read(on_recv, on_close); },
                  [this](void * /*ud*/) { handle_write(); },
                  [this, &on_close](void * /*ud*/) {
                      link_->connected = false;
                      on_close();
                  });

    if (link_ && link_->connected && !link_->send_buf.empty()) handle_write();
}

bool TcpClient::send(const char *data, size_t len) {
    if (!link_ || !link_->connected) return false;
    write_packet_header(link_->send_buf, static_cast<uint32_t>(len));
    link_->send_buf.write(data, len);
    handle_write();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// 分包工具
// ─────────────────────────────────────────────────────────────────────────────

void write_packet_header(CircularBuffer &buf, uint32_t payload_len) {
    char header[kPacketHeaderSize];
    header[0] = static_cast<char>((payload_len >> 24) & 0xFF);
    header[1] = static_cast<char>((payload_len >> 16) & 0xFF);
    header[2] = static_cast<char>((payload_len >> 8) & 0xFF);
    header[3] = static_cast<char>(payload_len & 0xFF);
    buf.write(header, kPacketHeaderSize);
}

bool try_parse_packet(CircularBuffer &buf, const char *&out_payload, uint32_t &out_len) {
    if (buf.size() < kPacketHeaderSize) return false;

    char header[kPacketHeaderSize];
    buf.peek(header, kPacketHeaderSize);

    uint32_t payload_len = (static_cast<uint8_t>(header[0]) << 24) |
                           (static_cast<uint8_t>(header[1]) << 16) |
                           (static_cast<uint8_t>(header[2]) << 8) |
                           static_cast<uint8_t>(header[3]);

    if (buf.size() < kPacketHeaderSize + payload_len) return false;

    buf.skip(kPacketHeaderSize);

    // 拷贝 payload（环形缓冲区可能回绕，需要线性缓冲区）
    static thread_local std::vector<char> parse_tmp;
    if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
    buf.read(parse_tmp.data(), payload_len);
    out_payload = parse_tmp.data();
    out_len = payload_len;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// 平台初始化
// ─────────────────────────────────────────────────────────────────────────────

static int g_net_initialized = 0;

void net_init() {
    if (g_net_initialized++ > 0) return;
#if defined(_WIN32)
    WSADATA wsa_data{};
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

void net_shutdown() {
    if (--g_net_initialized > 0) return;
    if (g_net_initialized < 0) g_net_initialized = 0;
#if defined(_WIN32)
    WSACleanup();
#endif
}

}// namespace fakelua::net
