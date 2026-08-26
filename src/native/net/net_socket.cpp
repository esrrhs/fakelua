#include "native/net/net_socket.h"

#include <vector>

namespace fakelua::net {

namespace {

bool is_websocket(const NetConfig &cfg) { return cfg.framer == FramerType::WebSocket; }

void reset_ws_state(TcpLink *link) {
    if (!link) return;
    link->ws_state = WsState::None;
    link->ws_handshake_sent = false;
}

} // namespace

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
    // start() 失败时 links_ 为空，必须用 size() 而不是 config_.max_conn，否则越界。
    const int n = static_cast<int>(links_.size());
    for (int i = 0; i < n; ++i) {
        if (links_[i] && links_[i]->connected) {
            selector_.remove(links_[i]->fd);
            close_socket(links_[i]->fd);
            links_[i]->connected = false;
            links_[i]->fd = INVALID_SOCKET_VAL;
            links_[i]->recv_buf.clear();
            links_[i]->send_buf.clear();
        }
    }
    selector_.clear();
    free_indices_.clear();
    for (int i = 0; i < n; ++i) free_indices_.push_back(n - 1 - i);
}

int TcpServer::alloc_link() {
    if (free_indices_.empty()) return -1;
    int slot = free_indices_.back();
    free_indices_.pop_back();
    return slot;
}

void TcpServer::free_link(int slot) {
    if (slot < 0 || slot >= static_cast<int>(links_.size())) return;
    links_[slot]->connected = false;
    links_[slot]->fd = INVALID_SOCKET_VAL;
    links_[slot]->recv_buf.clear();
    links_[slot]->send_buf.clear();
    reset_ws_state(links_[slot]);
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
            // 连接池满：关闭多余客户端后继续排空队列，不要 break 留下积压
            close_socket(client_fd);
            continue;
        }

        if (config_.non_blocking) set_non_blocking(client_fd);
        set_socket_options(client_fd, config_);

        auto *link = links_[slot];
        link->fd = client_fd;
        link->connected = true;
        link->recv_buf = CircularBuffer(config_.recv_buf_size);
        link->send_buf = CircularBuffer(config_.send_buf_size);

        selector_.add(client_fd, link);
        if (is_websocket(config_)) {
            link->ws_state = WsState::Handshake;
        } else {
            on_conn(slot);
        }
    }
}

void TcpServer::handle_link_read(TcpLink *link, const std::function<void(int)> &on_conn,
                                 const std::function<void(int, const char *, size_t)> &on_recv,
                                 const std::function<void(int)> &on_close) {
    if (!link || !link->connected || link->fd == INVALID_SOCKET_VAL) return;

    auto [ptr, len] = link->recv_buf.writable_region();
    if (len == 0) return;

    int n = static_cast<int>(::recv(link->fd, ptr, len, 0));
    int err = get_last_socket_error();
    if (n > 0) {
        link->recv_buf.commit_write(n);
        if (is_websocket(config_)) {
            if (link->ws_state == WsState::Handshake) {
                std::string response;
                bool need_more = false;
                bool hs_error = false;
                if (!try_ws_server_handshake(link->recv_buf, config_, response, need_more, hs_error)) {
                    if (hs_error) {
                        close_link(link, on_close);
                    }
                    return;
                }
                link->send_buf.write(response.data(), response.size());
                link->ws_state = WsState::Open;
                selector_.set_write_watch(link->fd, true);
                on_conn(link->conn_id);
            }
            if (link->ws_state == WsState::Open) {
                while (link->ws_state == WsState::Open) {
                    const char *payload = nullptr;
                    uint32_t payload_len = 0;
                    WsOpcode opcode = WsOpcode::Text;
                    bool parse_error = false;
                    if (!try_parse_ws_frame(link->recv_buf, config_, true, payload, payload_len, opcode, parse_error)) {
                        if (parse_error) close_link(link, on_close);
                        break;
                    }
                    if (opcode == WsOpcode::Close) {
                        close_link(link, on_close);
                        break;
                    }
                    if (opcode == WsOpcode::Ping) {
                        if (write_ws_pong(link->send_buf, config_, false, payload, payload_len)) {
                            selector_.set_write_watch(link->fd, true);
                        }
                        continue;
                    }
                    if (opcode == WsOpcode::Text || opcode == WsOpcode::Binary) {
                        on_recv(link->conn_id, payload, payload_len);
                    }
                }
            }
            return;
        }
        while (true) {
            const char *payload = nullptr;
            uint32_t payload_len = 0;
            bool parse_error = false;
            if (!try_parse_packet(link->recv_buf, config_, payload, payload_len, parse_error)) {
                if (parse_error) close_link(link, on_close);
                break;
            }
            on_recv(link->conn_id, payload, payload_len);
        }
    } else if (n == 0) {
        close_link(link, on_close);
    } else {
        if (!would_block(err)) close_link(link, on_close);
    }
}

void TcpServer::handle_link_write(TcpLink *link, const std::function<void(int)> &on_close) {
    if (!link || !link->connected || link->fd == INVALID_SOCKET_VAL) return;

    auto [ptr, len] = link->send_buf.readable_region();
    if (len == 0) {
        selector_.set_write_watch(link->fd, false);
        return;
    }

    int n;
#if defined(_WIN32)
    n = ::send(link->fd, ptr, static_cast<int>(len), 0);
#else
    n = static_cast<int>(::send(link->fd, ptr, len, 0));
#endif
    if (n > 0) {
        link->send_buf.commit_read(n);
        if (link->send_buf.empty()) selector_.set_write_watch(link->fd, false);
    } else if (n < 0) {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            // send 失败：与 close_link 走同一条路径（回收槽位 + 回调 close）
            close_link(link, on_close);
        }
    }
}

void TcpServer::close_link(TcpLink *link, const std::function<void(int)> &on_close) {
    if (!link || !link->connected) return;
    int fd = link->fd;
    int id = link->conn_id;
    link->connected = false;
    link->fd = INVALID_SOCKET_VAL;
    selector_.remove(fd);
    close_socket(fd);
    free_link(id);
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
                          handle_link_read(static_cast<TcpLink *>(ud), on_conn, on_recv, on_close);
                      }
                  },
                  [this, &on_close](void *ud) {
                      if (ud) handle_link_write(static_cast<TcpLink *>(ud), on_close);
                  },
                  [this, &on_close](void *ud) {
                      if (ud) close_link(static_cast<TcpLink *>(ud), on_close);
                  });

    // 尝试发送所有连接的缓冲数据（兜底，确保数据在 EPOLLOUT 未触发时也能发出）
    const int n = static_cast<int>(links_.size());
    for (int i = 0; i < n; ++i) {
        if (links_[i] && links_[i]->connected && !links_[i]->send_buf.empty()) {
            handle_link_write(links_[i], on_close);
        }
    }
}

bool TcpServer::send(int conn_id, const char *data, size_t len) {
    if (conn_id < 0 || conn_id >= static_cast<int>(links_.size())) return false;
    auto *link = links_[conn_id];
    if (!link || !link->connected) return false;
    if (is_websocket(config_)) {
        if (link->ws_state != WsState::Open) return false;
        if (!write_ws_frame(link->send_buf, config_, false, WsOpcode::Text, data, len)) return false;
    } else if (!write_packet(link->send_buf, config_, data, len)) {
        return false;
    }
    selector_.set_write_watch(link->fd, true);
    return true;
}

bool TcpServer::close_connection(int conn_id) {
    if (conn_id < 0 || conn_id >= static_cast<int>(links_.size())) return false;
    auto *link = links_[conn_id];
    if (!link || !link->connected) return false;
    selector_.remove(link->fd);
    close_socket(link->fd);
    link->fd = INVALID_SOCKET_VAL;
    link->connected = false;
    free_link(conn_id);
    return true;
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
    bool in_progress = false;
    if (ret != 0) {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            // 立即失败
            close_socket(fd);
            return;
        }
        // 非阻塞 connect 正在进行，需等待可写后通过 SO_ERROR 确认结果
        in_progress = true;
    }

    set_socket_options(fd, config_);

    link_ = new TcpLink();
    link_->fd = fd;
    link_->connected = !in_progress; // 仅当立即成功时置为已连接
    link_->recv_buf = CircularBuffer(config_.recv_buf_size);
    link_->send_buf = CircularBuffer(config_.send_buf_size);
    connecting_ = in_progress;
    if (is_websocket(config_)) {
        link_->ws_state = WsState::Handshake;
    }

    selector_.add(fd, link_);
    if (connecting_) selector_.set_write_watch(fd, true);
    if (!connecting_ && is_websocket(config_) && !link_->ws_handshake_sent) {
        std::string request;
        std::string key;
        build_ws_client_handshake_request(config_, request, key);
        link_->send_buf.write(request.data(), request.size());
        link_->ws_handshake_sent = true;
        selector_.set_write_watch(fd, true);
    }
}

void TcpClient::disconnect() {
    teardown_fd(nullptr);
    delete link_;
    link_ = nullptr;
    connecting_ = false;
}

void TcpClient::teardown_fd(const std::function<void()> *on_close) {
    if (!link_) return;
    const bool was_live = link_->fd != INVALID_SOCKET_VAL || connecting_ || link_->connected;
    if (link_->fd != INVALID_SOCKET_VAL) {
        selector_.remove(link_->fd);
        close_socket(link_->fd);
        link_->fd = INVALID_SOCKET_VAL;
    }
    connecting_ = false;
    link_->connected = false;
    reset_ws_state(link_);
    if (on_close && was_live) (*on_close)();
}

void TcpClient::handle_read(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close) {
    if (!link_ || !link_->connected || link_->fd == INVALID_SOCKET_VAL) return;

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
        if (is_websocket(config_)) {
            if (link_->ws_state == WsState::Handshake) {
                bool done = false;
                bool need_more = false;
                bool hs_error = false;
                if (!try_ws_client_handshake(link_->recv_buf, done, need_more, hs_error)) {
                    if (hs_error) {
                        teardown_fd(&on_close);
                    }
                    return;
                }
                link_->ws_state = WsState::Open;
            }
            if (link_->ws_state == WsState::Open) {
                while (link_->ws_state == WsState::Open) {
                    const char *payload = nullptr;
                    uint32_t payload_len = 0;
                    WsOpcode opcode = WsOpcode::Text;
                    bool parse_error = false;
                    if (!try_parse_ws_frame(link_->recv_buf, config_, false, payload, payload_len, opcode, parse_error)) {
                        if (parse_error) teardown_fd(&on_close);
                        break;
                    }
                    if (opcode == WsOpcode::Close) {
                        teardown_fd(&on_close);
                        break;
                    }
                    if (opcode == WsOpcode::Ping) {
                        if (write_ws_pong(link_->send_buf, config_, true, payload, payload_len)) {
                            selector_.set_write_watch(link_->fd, true);
                        }
                        continue;
                    }
                    if (opcode == WsOpcode::Text || opcode == WsOpcode::Binary) {
                        on_recv(payload, payload_len);
                    }
                }
            }
            return;
        }
        while (true) {
            const char *payload = nullptr;
            uint32_t payload_len = 0;
            bool parse_error = false;
            if (!try_parse_packet(link_->recv_buf, config_, payload, payload_len, parse_error)) {
                if (parse_error) {
                    teardown_fd(&on_close);
                }
                break;
            }
            on_recv(payload, payload_len);
        }
    } else if (n == 0) {
        teardown_fd(&on_close);
    } else {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            teardown_fd(&on_close);
        }
    }
}

void TcpClient::handle_write(const std::function<void()> &on_close) {
    if (!link_ || (!link_->connected && !connecting_) || link_->fd == INVALID_SOCKET_VAL) return;

    // 正在连接中：等待可写后通过 getsockopt(SO_ERROR) 确认连接结果
    if (connecting_) {
        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(link_->fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&err), &len) < 0 || err != 0) {
            teardown_fd(&on_close);
            return;
        }
        connecting_ = false;
        link_->connected = true;
        // 连接成功后若已有待发数据则保持写监视，否则关闭
        selector_.set_write_watch(link_->fd, !link_->send_buf.empty());
    }

    if (link_->connected && is_websocket(config_) && link_->ws_state == WsState::Handshake && !link_->ws_handshake_sent) {
        std::string request;
        std::string key;
        build_ws_client_handshake_request(config_, request, key);
        link_->send_buf.write(request.data(), request.size());
        link_->ws_handshake_sent = true;
        selector_.set_write_watch(link_->fd, true);
    }

    if (!link_->connected) return;

    auto [ptr, len] = link_->send_buf.readable_region();
    if (len == 0) {
        selector_.set_write_watch(link_->fd, false);
        return;
    }

    int n;
#if defined(_WIN32)
    n = ::send(link_->fd, ptr, static_cast<int>(len), 0);
#else
    n = static_cast<int>(::send(link_->fd, ptr, len, 0));
#endif
    if (n > 0) {
        link_->send_buf.commit_read(n);
        if (link_->send_buf.empty()) selector_.set_write_watch(link_->fd, false);
    } else if (n < 0) {
        int err = get_last_socket_error();
        if (!would_block(err)) {
            teardown_fd(&on_close);
        }
    }
}

void TcpClient::tick(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close) {
    if (!link_ || (!link_->connected && !connecting_)) return;

    selector_.wait(config_.wait_timeout_ms,
                  [this, &on_recv, &on_close](void * /*ud*/) {
                      // 仅已连接时读取（连接建立过程中不读）
                      if (link_ && link_->connected) handle_read(on_recv, on_close);
                  },
                  [this, &on_close](void * /*ud*/) { handle_write(on_close); },
                  [this, &on_close](void * /*ud*/) {
                      teardown_fd(&on_close);
                  });

    if (link_ && link_->connected && !link_->send_buf.empty()) handle_write(on_close);
}

bool TcpClient::send(const char *data, size_t len) {
    if (!link_ || !link_->connected || connecting_) return false;
    if (is_websocket(config_)) {
        if (link_->ws_state != WsState::Open) return false;
        if (!write_ws_frame(link_->send_buf, config_, true, WsOpcode::Text, data, len)) return false;
    } else if (!write_packet(link_->send_buf, config_, data, len)) {
        return false;
    }
    selector_.set_write_watch(link_->fd, true);
    return true;
}

} // namespace fakelua::net
