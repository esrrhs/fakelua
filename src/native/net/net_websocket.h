#pragma once

#include "native/net/net_buffer.h"
#include "native/net/net_common.h"

#include <cstdint>
#include <string>

namespace fakelua::net {

enum class WsState : uint8_t {
    None = 0,
    Handshake,
    Open,
    Failed,
};

enum class WsOpcode : uint8_t {
    Continuation = 0x0,
    Text = 0x1,
    Binary = 0x2,
    Close = 0x8,
    Ping = 0x9,
    Pong = 0xA,
};

// 服务端：从 recv_buf 解析 HTTP Upgrade 请求，成功则生成 101 响应并消费请求字节。
// out_need_more=true 表示数据不足；out_error=true 表示协议错误应关闭连接。
bool try_ws_server_handshake(CircularBuffer &buf, const NetConfig &cfg, std::string &out_response, bool &out_need_more,
                             bool &out_error);

// 客户端：生成握手请求（首次调用），并从 recv_buf 解析 101 响应。
bool build_ws_client_handshake_request(const NetConfig &cfg, std::string &out_request, std::string &out_key);

bool try_ws_client_handshake(CircularBuffer &buf, bool &out_done, bool &out_need_more, bool &out_error);

// 解析单帧 WebSocket 数据。成功返回 true 且 out_opcode 为 Text/Binary 等。
bool try_parse_ws_frame(CircularBuffer &buf, const NetConfig &cfg, bool from_client, const char *&out_payload,
                        uint32_t &out_len, WsOpcode &out_opcode, bool &out_error);

// 写入 WebSocket 帧。from_client=true 时按 RFC 6455 加 mask。
bool write_ws_frame(CircularBuffer &buf, const NetConfig &cfg, bool from_client, WsOpcode opcode, const char *data,
                    size_t len);

// 便捷：写入 Pong 响应 Ping。
bool write_ws_pong(CircularBuffer &buf, const NetConfig &cfg, bool from_client, const char *payload, size_t len);

} // namespace fakelua::net
