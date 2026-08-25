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
    Header4BigEndian = 0, // 4 字节大端整数长度头（默认）
    Header4LittleEndian,  // 4 字节小端整数长度头
    Header2BigEndian,     // 2 字节大端整数长度头
    Header2LittleEndian,  // 2 字节小端整数长度头
    LineDelimiter,        // \n 换行符（或 \r\n）分隔（自动剥离末尾换行符）
    FixedLength,          // 固定长度封包（由 fixed_packet_len 指定）
    RawStream,            // 原始流透传（直接转发收到的字节）
    Custom,               // 自定义解包/封包（支持 Lua 或 C++ 函数）
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
    int max_conn = 64;                    // 默认连接数（可按需通过 maxconn 加大）
    int send_buf_size = 64 * 1024;        // 默认 64KB 发送缓冲
    int recv_buf_size = 65 * 1024;        // 略大于 max_packet_len，容纳 4 字节长度头
    int max_packet_len = 64 * 1024;       // 默认单包上限 64KB
    int fixed_packet_len = 0; // 当 framer == FramerType::FixedLength 时使用
    int wait_timeout_ms = 1;
    int backlog = 128;
    bool non_blocking = true;
    bool no_delay = true;
    bool keep_alive = true;
    FramerType framer = FramerType::Header4BigEndian;
    std::string custom_parser_name; // Lua 自定义解包函数名（返回 packet_str 或 nil）
    CustomParserFn custom_parser_fn; // C++ 自定义解包函数
    CustomEncoderFn custom_encoder_fn; // C++ 自定义编码函数
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
