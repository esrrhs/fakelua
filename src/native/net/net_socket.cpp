#include "native/net/net_socket.h"

#include <vector>

namespace fakelua::net {

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
            if (!try_parse_packet(link->recv_buf, config_, payload, payload_len)) break;
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
    write_packet(link->send_buf, config_, data, len);
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
            if (!try_parse_packet(link_->recv_buf, config_, payload, payload_len)) break;
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
    write_packet(link_->send_buf, config_, data, len);
    handle_write();
    return true;
}

} // namespace fakelua::net
