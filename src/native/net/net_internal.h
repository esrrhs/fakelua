#pragma once

// net_internal.h — TCP 网络引擎内部类型与实现声明
//
// 参考 liblu 的架构（epoll/select IO 复用、预分配连接池、环形缓冲区、length-prefixed 分包），
// 按 fakelua 的 C++23 风格重写。去掉了加密/压缩/校验，只做原始字节收发。

#include "fakelua.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_VAL = INVALID_SOCKET;
#else
#include <sys/types.h>
using socket_t = int;
constexpr socket_t INVALID_SOCKET_VAL = -1;
#endif

namespace fakelua::net {

// ─────────────────────────────────────────────────────────────────────────────
// 配置
// ─────────────────────────────────────────────────────────────────────────────

struct NetConfig {
    std::string ip = "127.0.0.1";
    uint16_t port = 8888;
    int max_conn = 1000;
    int send_buf_size = 1024 * 1024;
    int recv_buf_size = 1024 * 1024;
    int max_packet_len = 100 * 1024;
    int wait_timeout_ms = 1;
    int backlog = 128;
    bool non_blocking = true;
    bool no_delay = true;
    bool keep_alive = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// 分包格式: [length: 4 bytes uint32 big-endian][payload: length bytes]
// ─────────────────────────────────────────────────────────────────────────────

constexpr size_t kPacketHeaderSize = 4;

// ─────────────────────────────────────────────────────────────────────────────
// 环形缓冲区
// ─────────────────────────────────────────────────────────────────────────────

class CircularBuffer {
public:
    explicit CircularBuffer(size_t capacity);
    ~CircularBuffer();

    [[nodiscard]] size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }
    [[nodiscard]] bool full() const { return size_ >= buf_.size(); }
    [[nodiscard]] size_t capacity() const { return buf_.size(); }

    // 写入数据，返回实际写入字节数
    size_t write(const char *data, size_t len);
    // 读取数据（消费），返回实际读取字节数
    size_t read(char *dst, size_t len);
    // 查看数据但不消费
    size_t peek(char *dst, size_t len) const;
    // 丢弃指定字节数
    size_t skip(size_t len);

    // 获取可写入的连续缓冲区（用于直接 recv 到缓冲区）
    [[nodiscard]] std::pair<char *, size_t> writable_region();
    void commit_write(size_t bytes);
    // 获取可读的连续缓冲区（用于直接 send）
    [[nodiscard]] std::pair<const char *, size_t> readable_region();
    void commit_read(size_t bytes);

    void clear();

private:
    std::vector<char> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// TcpLink — 单个 TCP 连接
// ─────────────────────────────────────────────────────────────────────────────

struct TcpLink {
    socket_t fd = INVALID_SOCKET_VAL;
    CircularBuffer recv_buf;
    CircularBuffer send_buf;
    bool connected = false;
    int conn_id = -1;

    TcpLink();
    ~TcpLink();
};

// ─────────────────────────────────────────────────────────────────────────────
// Selector — IO 复用（epoll on Linux, select on Windows）
// 回调直接接收 userdata（TcpLink*），listen socket 用 nullptr
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
// TcpServer — TCP 服务端
// ─────────────────────────────────────────────────────────────────────────────

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
    void handle_link_write(TcpLink *link);
    void close_link(TcpLink *link, const std::function<void(int)> &on_close);
    int alloc_link();
    void free_link(int slot);
};

// ─────────────────────────────────────────────────────────────────────────────
// TcpClient — TCP 客户端
// ─────────────────────────────────────────────────────────────────────────────

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

private:
    NetConfig config_;
    TcpLink *link_ = nullptr;
    Selector selector_;

    void handle_read(const std::function<void(const char *, size_t)> &on_recv, const std::function<void()> &on_close);
    void handle_write();
};

// ─────────────────────────────────────────────────────────────────────────────
// 平台初始化
// ─────────────────────────────────────────────────────────────────────────────

void net_init();
void net_shutdown();

// ─────────────────────────────────────────────────────────────────────────────
// 分包工具
// ─────────────────────────────────────────────────────────────────────────────

void write_packet_header(CircularBuffer &buf, uint32_t payload_len);
// 尝试从缓冲区读取一个完整包，返回 payload 指针和长度（payload 指向内部临时缓冲区）
bool try_parse_packet(CircularBuffer &buf, const char *&out_payload, uint32_t &out_len);

}// namespace fakelua::net
