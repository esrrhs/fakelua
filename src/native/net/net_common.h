#pragma once

#include <cstdint>
#include <functional>
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

enum class FramerType {
    // 4 字节大端整数长度头（默认）
    Header4BigEndian = 0,
    // 4 字节小端整数长度头
    Header4LittleEndian,
    // 2 字节大端整数长度头
    Header2BigEndian,
    // 2 字节小端整数长度头
    Header2LittleEndian,
    // \n 换行符（或 \r\n）分隔（自动剥离末尾换行符）
    LineDelimiter,
    // 固定长度封包（由 fixed_packet_len 指定）
    FixedLength,
    // 原始流透传（直接转发收到的字节）
    RawStream,
    // 自定义解包/封包（支持 Lua 或 C++ 函数）
    Custom,
    // RFC 6455 WebSocket（Boost.Beast 文本帧）
    WebSocket,
};

class CircularBuffer;

// C++ 自定义解包函数签名：
// 输入环形缓冲区，成功解包则消费缓冲区数据、填入 out_payload 与 out_len 并返回 true；若缓冲区数据不足或无完整包则返回 false
using CustomParserFn = std::function<bool(CircularBuffer &buf, const char *&out_payload, uint32_t &out_len)>;

// C++ 自定义封包编码函数签名：
// 输入业务数据与长度，将打包后（含头/分隔符）的完整数据写入 buf
using CustomEncoderFn = std::function<void(CircularBuffer &buf, const char *data, size_t len)>;

struct NetConfig {
    std::string ip = "127.0.0.1";
    uint16_t port = 8888;
    // 默认连接数（可按需通过 maxconn 加大）
    int max_conn = 64;
    // 默认 64KB 发送缓冲
    int send_buf_size = 64 * 1024;
    // 略大于 max_packet_len，容纳 4 字节长度头
    int recv_buf_size = 65 * 1024;
    // 默认单包上限 64KB
    int max_packet_len = 64 * 1024;
    // 当 framer == FramerType::FixedLength 时使用
    int fixed_packet_len = 0;
    int wait_timeout_ms = 1;
    int backlog = 128;
    // Lua 仍可传 nonblocking；C++ 忽略。Windows 上不能 socket.non_blocking(true)。
    bool non_blocking = true;
    bool no_delay = true;
    bool keep_alive = true;
    FramerType framer = FramerType::Header4BigEndian;
    // Lua 自定义解包函数名（返回 packet_str 或 nil）
    std::string custom_parser_name;
    // C++ 自定义解包函数
    CustomParserFn custom_parser_fn;
    // C++ 自定义编码函数
    CustomEncoderFn custom_encoder_fn;
    // WebSocket 握手路径（默认 /）
    std::string ws_path = "/";
    // 客户端 Host 头（默认可由 ip:port 推导）
    std::string ws_host;
    // 客户端 Origin 头（可选）
    std::string ws_origin;
    // WebSocket over TLS (wss). Server needs tls_cert/tls_key; client uses tls_verify/tls_ca.
    bool tls = false;
    bool tls_verify = true;
    std::string tls_cert;
    std::string tls_key;
    std::string tls_ca;
};

void NetInit();
void NetShutdown();

int GetLastSocketError();
bool WouldBlock(int err);
void CloseSocket(socket_t fd);
bool SetNonBlocking(socket_t fd);
void SetSocketOptions(socket_t fd, const NetConfig &cfg);
bool FillSockaddr(struct ::sockaddr_in &addr, const std::string &ip, uint16_t port);

}// namespace fakelua::net
