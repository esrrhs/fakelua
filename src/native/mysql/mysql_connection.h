#pragma once

// mysql_connection.h — synchronous MySQL client connection (blocking I/O).
// Not based on the net module: MySQL is request-response, blocking is natural.

#include "native/mysql/mysql_protocol.h"
#include "native/mysql/mysql_result.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_VAL = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
using socket_t = int;
constexpr socket_t INVALID_SOCKET_VAL = -1;
#endif

#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::mysql {

class MysqlConnection {
public:
    MysqlConnection();
    ~MysqlConnection();

    MysqlConnection(const MysqlConnection &) = delete;
    MysqlConnection &operator=(const MysqlConnection &) = delete;

    // Connect: TCP connect → read handshake → authenticate → OK
    void connect(const std::string &host, uint16_t port,
                 const std::string &user, const std::string &password,
                 const std::string &database);

    // Execute a text query.
    // SELECT → MysqlResult with rows; INSERT/UPDATE/DELETE → MysqlResult with status.
    MysqlResult query(const std::string &sql);

    // Close connection (send COM_QUIT + close socket)
    void close();

    bool connected() const { return sock_ >= 0; }

private:
    socket_t sock_ = INVALID_SOCKET_VAL;
    uint8_t seq_ = 0;
    uint32_t capabilities_ = 0;
    uint8_t charset_ = 0;

    // ── socket I/O (blocking with timeout) ──
    void socket_connect(const std::string &host, uint16_t port);
    void socket_send(const char *data, size_t len);
    void socket_recv_exact(char *buf, size_t len);  // block until len bytes read

    // ── packet I/O ──
    void send_packet(uint8_t seq, const char *payload, size_t len);
    // Read one packet (handles multi-packet concatenation for payloads > 16MB).
    std::vector<char> recv_packet();

    // ── handshake ──
    void do_handshake(const std::string &user, const std::string &password,
                      const std::string &database);

    // ── helpers ──
    [[noreturn]] static void net_error(const std::string &msg);
};

}  // namespace fakelua::mysql
