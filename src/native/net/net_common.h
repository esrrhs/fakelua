#pragma once

#include <cstdint>
#include <string>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_VAL = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
using socket_t = int;
constexpr socket_t INVALID_SOCKET_VAL = -1;
#endif

namespace fakelua::net {

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

void net_init();
void net_shutdown();

int get_last_socket_error();
bool would_block(int err);
void close_socket(socket_t fd);
bool set_non_blocking(socket_t fd);
void set_socket_options(socket_t fd, const NetConfig &cfg);
bool fill_sockaddr(struct ::sockaddr_in &addr, const std::string &ip, uint16_t port);

} // namespace fakelua::net
