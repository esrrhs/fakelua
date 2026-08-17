#include "native/net/net_common.h"

#include <cerrno>
#include <cstring>

#if defined(_WIN32)
#pragma comment(lib, "ws2_32.lib")
#elif defined(__linux__)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace fakelua::net {

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

int get_last_socket_error() {
#if defined(_WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool would_block(int err) {
#if defined(_WIN32)
    return err == WSAEWOULDBLOCK || err == WSAEINPROGRESS;
#else
    return err == EAGAIN || err == EWOULDBLOCK || err == EINPROGRESS;
#endif
}

void close_socket(socket_t fd) {
    if (fd == INVALID_SOCKET_VAL) return;
#if defined(_WIN32)
    closesocket(fd);
#else
    ::close(fd);
#endif
}

bool set_non_blocking(socket_t fd) {
#if defined(_WIN32)
    u_long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0;
#endif
}

void set_socket_options(socket_t fd, const NetConfig &cfg) {
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

bool fill_sockaddr(struct ::sockaddr_in &addr, const std::string &ip, uint16_t port) {
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip.empty() || ip == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
        return true;
    }
    return inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) == 1;
}

} // namespace fakelua::net
