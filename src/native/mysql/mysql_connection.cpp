#include "native/mysql/mysql_connection.h"

#include <chrono>
#include <cstring>
#include <format>
#include <stdexcept>

#if !defined(_WIN32)
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#endif

namespace fakelua::mysql {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

[[noreturn]] void MysqlConnection::net_error(const std::string &msg) {
    throw std::runtime_error("mysql net: " + msg);
}

#if defined(_WIN32)
static int get_socket_error() { return WSAGetLastError(); }
static bool would_block(int err) { return err == WSAEWOULDBLOCK || err == WSAEINPROGRESS; }
#else
static int get_socket_error() { return errno; }
static bool would_block(int err) { return err == EAGAIN || err == EWOULDBLOCK || err == EINPROGRESS; }
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Lifetime
// ─────────────────────────────────────────────────────────────────────────────

MysqlConnection::MysqlConnection() = default;

MysqlConnection::~MysqlConnection() {
    close();
}

// ─────────────────────────────────────────────────────────────────────────────
// Socket I/O (blocking with timeout)
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::socket_connect(const std::string &host, uint16_t port) {
    // Resolve address
    addrinfo hints{};
    hints.ai_family = AF_INET;       // IPv4 only (matches net module)
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *res = nullptr;
    std::string port_str = std::to_string(port);
    int rc = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (rc != 0 || !res) {
        net_error(std::format("resolve {} failed: {}", host, gai_strerror(rc)));
    }

    sock_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_ == INVALID_SOCKET_VAL) {
        freeaddrinfo(res);
        net_error("socket() failed");
    }

    // Set socket options: TCP_NODELAY, timeouts
#if !defined(_WIN32)
    int opt = 1;
    setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    struct timeval tv;
    tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#else
    int opt = 1;
    setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, (const char *)&opt, sizeof(opt));
    DWORD tv = 5000;
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));
#endif

    rc = ::connect(sock_, res->ai_addr, static_cast<int>(res->ai_addrlen));
    freeaddrinfo(res);
    if (rc != 0) {
        int err = get_socket_error();
        if (!would_block(err)) {
            close();
            net_error(std::format("connect failed: {}", err));
        }
        // Blocking connect: use poll/select to wait for completion with timeout
#if !defined(_WIN32)
        struct pollfd pfd{};
        pfd.fd = sock_;
        pfd.events = POLLOUT;
        int pr = poll(&pfd, 1, 5000);
        if (pr <= 0) {
            close();
            net_error("connect timeout");
        }
        // Check SO_ERROR
        int so_error = 0;
        socklen_t len = sizeof(so_error);
        getsockopt(sock_, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error != 0) {
            close();
            net_error(std::format("connect failed: {}", so_error));
        }
#else
        fd_set wset, eset;
        FD_ZERO(&wset); FD_ZERO(&eset);
        FD_SET(sock_, &wset); FD_SET(sock_, &eset);
        struct timeval tv0;
        tv0.tv_sec = 5; tv0.tv_usec = 0;
        int sel = select(0, nullptr, &wset, &eset, &tv0);
        if (sel <= 0) {
            close();
            net_error("connect timeout");
        }
        int so_error = 0;
        int len = sizeof(so_error);
        getsockopt(sock_, SOL_SOCKET, SO_ERROR, (char *)&so_error, &len);
        if (so_error != 0) {
            close();
            net_error(std::format("connect failed: {}", so_error));
        }
#endif
    }
}

void MysqlConnection::socket_send(const char *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
#if defined(_WIN32)
        int n = ::send(sock_, data + sent, static_cast<int>(len - sent), 0);
#else
        ssize_t n = ::send(sock_, data + sent, len - sent, 0);
#endif
        if (n <= 0) {
            net_error(std::format("send failed: {}", get_socket_error()));
        }
        sent += static_cast<size_t>(n);
    }
}

void MysqlConnection::socket_recv_exact(char *buf, size_t len) {
    size_t got = 0;
    while (got < len) {
#if defined(_WIN32)
        int n = ::recv(sock_, buf + got, static_cast<int>(len - got), 0);
#else
        ssize_t n = ::recv(sock_, buf + got, len - got, 0);
#endif
        if (n == 0) {
            net_error("connection closed by server");
        }
        if (n < 0) {
            int err = get_socket_error();
            if (would_block(err)) {
                net_error("recv timeout");
            }
            net_error(std::format("recv failed: {}", err));
        }
        got += static_cast<size_t>(n);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Packet I/O
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::send_packet(uint8_t seq, const char *payload, size_t len) {
    socket_send(payload, len);  // payload already has 4-byte header prepended by caller
}

std::vector<char> MysqlConnection::recv_packet() {
    // Read 4-byte header
    char header[4];
    socket_recv_exact(header, 4);
    uint32_t payload_len = static_cast<uint8_t>(header[0]) |
                           (static_cast<uint8_t>(header[1]) << 8) |
                           (static_cast<uint8_t>(header[2]) << 16);
    uint8_t seq = static_cast<uint8_t>(header[3]);
    (void)seq;  // sequence checked/updated by caller if needed

    // Read payload
    std::vector<char> payload(payload_len);
    if (payload_len > 0) {
        socket_recv_exact(payload.data(), payload_len);
    }

    // Multi-packet: if payload is exactly MAX_PACKET_SIZE, more packets follow
    std::vector<char> full_payload;
    full_payload.reserve(payload_len);
    full_payload.insert(full_payload.end(), payload.begin(), payload.end());

    while (payload_len == MAX_PACKET_SIZE) {
        socket_recv_exact(header, 4);
        payload_len = static_cast<uint8_t>(header[0]) |
                      (static_cast<uint8_t>(header[1]) << 8) |
                      (static_cast<uint8_t>(header[2]) << 16);
        payload.resize(payload_len);
        if (payload_len > 0) {
            socket_recv_exact(payload.data(), payload_len);
        }
        full_payload.insert(full_payload.end(), payload.begin(), payload.end());
    }

    return full_payload;
}

// ─────────────────────────────────────────────────────────────────────────────
// Handshake
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::do_handshake(const std::string &user, const std::string &password,
                                   const std::string &database) {
    // 1) Read server handshake (seq 0)
    auto server_packet = recv_packet();
    if (server_packet.empty()) {
        net_error("empty handshake packet");
    }

    auto info = parse_handshake(server_packet);

    // We require mysql_native_password or at least the classic scramble
    if (!info.auth_plugin_name.empty() &&
        info.auth_plugin_name != "mysql_native_password") {
        net_error(std::format("unsupported auth plugin '{}'", info.auth_plugin_name));
    }

    // 2) Build and send client response (seq 1)
    auto response_payload = build_handshake_response(info, user, password, database);
    auto response_pkt = make_packet(1, response_payload.data(), response_payload.size());
    socket_send(response_pkt.data(), response_pkt.size());

    // 3) Read server response (seq 2): OK, ERR, or auth switch
    auto auth_reply = recv_packet();
    if (auth_reply.empty()) {
        net_error("empty auth reply");
    }

    uint8_t type = static_cast<uint8_t>(auth_reply[0]);
    if (type == PACKET_ERR) {
        auto err = parse_err(auth_reply);
        net_error(std::format("auth failed: {} ({})", err.message, err.sql_state));
    }
    if (type == PACKET_OK) {
        // Success — save capabilities/charset for later
        capabilities_ = info.capabilities;
        charset_ = info.charset;
        seq_ = 0;  // reset sequence for commands
        return;
    }
    // Auth switch request (0xFE) or legacy — not supported in v1
    if (type == PACKET_EOF) {
        net_error("legacy auth (pre-4.1) not supported");
    }
    // 0xFE could be auth switch — try to handle simple mysql_native_password switch
    net_error(std::format("unexpected auth reply type 0x{:02x}", type));
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void MysqlConnection::connect(const std::string &host, uint16_t port,
                              const std::string &user, const std::string &password,
                              const std::string &database) {
    if (sock_ != INVALID_SOCKET_VAL) {
        close();
    }
    seq_ = 0;
    capabilities_ = 0;
    charset_ = 0;
    socket_connect(host, port);
    do_handshake(user, password, database);
}

MysqlResult MysqlConnection::query(const std::string &sql) {
    // Build COM_QUERY packet (seq 0)
    std::string payload;
    payload.push_back(static_cast<char>(COM_QUERY));
    payload.append(sql);
    auto pkt = make_packet(0, payload.data(), payload.size());
    socket_send(pkt.data(), pkt.size());

    // Read first response packet
    auto first = recv_packet();
    if (first.empty()) {
        net_error("empty query response");
    }

    uint8_t type = static_cast<uint8_t>(first[0]);
    if (type == PACKET_ERR) {
        auto err = parse_err(first);
        net_error(std::format("query failed: {} ({})", err.message, err.sql_state));
    }
    if (type == PACKET_OK) {
        // INSERT/UPDATE/DELETE or empty result
        MysqlResult result;
        result.is_result_set = false;
        result.affected_rows = 0;
        result.last_insert_id = 0;
        result.status_flags = 0;
        result.info.clear();
        if (first.size() > 1) {
            // Re-parse OK from payload (skip the 0x00 header byte already consumed by parse_ok)
            result = parse_ok_to_result(first);
        }
        return result;
    }
    if (type == PACKET_EOF) {
        // Under CLIENT_DEPRECATE_EOF, 0xFE can be OK-as-EOF or genuine EOF.
        // For a query response, 0xFE as first packet is unusual; treat as error.
        net_error("unexpected EOF as first query response");
    }
    // Otherwise: column count (length-encoded integer) → result set
    MysqlResult result;
    result.is_result_set = true;
    size_t pos = 0;
    uint64_t col_count = read_lenenc_int(first, pos);

    // Read column definitions
    result.columns.resize(static_cast<size_t>(col_count));
    for (uint64_t i = 0; i < col_count; ++i) {
        auto col_pkt = recv_packet();
        result.columns[i] = parse_column_def(col_pkt);
    }

    // Read EOF (or OK-as-EOF under DEPRECATE_EOF) between columns and rows
    {
        auto eof_pkt = recv_packet();
        if (!eof_pkt.empty()) {
            uint8_t h = static_cast<uint8_t>(eof_pkt[0]);
            if (h != PACKET_EOF && h != PACKET_OK) {
                net_error(std::format("expected EOF after columns, got 0x{:02x}", h));
            }
        }
    }

    // Read rows until EOF/OK
    while (true) {
        auto row_pkt = recv_packet();
        if (row_pkt.empty()) break;
        uint8_t h = static_cast<uint8_t>(row_pkt[0]);
        if (h == PACKET_EOF || h == PACKET_OK) {
            // End of result set — if OK, capture status
            if (h == PACKET_OK && row_pkt.size() > 1) {
                // Could parse trailing status here if needed
            }
            break;
        }
        result.rows.push_back(parse_row(row_pkt, result.columns.size()));
    }

    return result;
}

void MysqlConnection::close() {
    if (sock_ != INVALID_SOCKET_VAL) {
        // Send COM_QUIT only if the handshake completed (capabilities_ set).
        // On a never-established connection the socket may be in an error state
        // and ::send could block.
        if (capabilities_ != 0) {
            auto quit = make_packet(0, "\x01", 1);  // COM_QUIT = 0x01
            ::send(sock_, quit.data(), static_cast<int>(quit.size()), 0);
        }
#if defined(_WIN32)
        closesocket(sock_);
#else
        ::close(sock_);
#endif
        sock_ = INVALID_SOCKET_VAL;
    }
    seq_ = 0;
    capabilities_ = 0;
    charset_ = 0;
}

}  // namespace fakelua::mysql
