#include "native/net/net_websocket.h"

#include "native/crypto/hash.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <random>
#include <string_view>
#include <vector>

namespace fakelua::net {

namespace {

constexpr const char *kWsGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

std::string to_lower_ascii(std::string s) {
    for (char &c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool icontains(std::string_view hay, std::string_view needle) {
    if (needle.empty()) return true;
    std::string h = to_lower_ascii(std::string(hay));
    std::string n = to_lower_ascii(std::string(needle));
    return h.find(n) != std::string::npos;
}

bool find_header_value(std::string_view headers, std::string_view name, std::string &out) {
    std::string key = to_lower_ascii(std::string(name)) + ":";
    size_t pos = 0;
    while (pos < headers.size()) {
        size_t line_end = headers.find('\n', pos);
        if (line_end == std::string::npos) line_end = headers.size();
        std::string_view line = headers.substr(pos, line_end - pos);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        std::string lower_line = to_lower_ascii(std::string(line));
        if (lower_line.rfind(key, 0) == 0) {
            size_t val_start = key.size();
            while (val_start < lower_line.size() && lower_line[val_start] == ' ') val_start++;
            out.assign(line.substr(val_start));
            while (!out.empty() && (out.back() == ' ' || out.back() == '\r')) out.pop_back();
            return true;
        }
        if (line_end == headers.size()) break;
        pos = line_end + 1;
    }
    return false;
}

std::string ws_accept_key(std::string_view client_key) {
    std::string concat(client_key);
    concat += kWsGuid;
    auto digest = fakelua::crypto::sha1(concat);
    return fakelua::crypto::base64_encode(digest.data(), digest.size());
}

bool parse_http_request_line(std::string_view req, std::string &method, std::string &path) {
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) return false;
    std::string_view line = req.substr(0, line_end);
    size_t sp1 = line.find(' ');
    if (sp1 == std::string::npos) return false;
    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return false;
    method.assign(line.substr(0, sp1));
    path.assign(line.substr(sp1 + 1, sp2 - sp1 - 1));
    return true;
}

std::string random_ws_key() {
    std::array<uint8_t, 16> bytes{};
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto &b : bytes) b = static_cast<uint8_t>(dist(rng));
    return fakelua::crypto::base64_encode(bytes.data(), bytes.size());
}

void apply_mask(char *data, size_t len, const uint8_t mask[4]) {
    for (size_t i = 0; i < len; ++i) {
        data[i] = static_cast<char>(static_cast<uint8_t>(data[i]) ^ mask[i % 4]);
    }
}

size_t ws_encoded_size(size_t payload_len) {
    size_t header = 2;
    if (payload_len >= 126) {
        header += (payload_len >= 65536) ? 8 : 2;
    }
    return header + payload_len;
}

} // namespace

bool try_ws_server_handshake(CircularBuffer &buf, const NetConfig &cfg, std::string &out_response, bool &out_need_more,
                             bool &out_error) {
    out_need_more = false;
    out_error = false;
    out_response.clear();

    if (buf.size() < 16) {
        out_need_more = true;
        return false;
    }

    static thread_local std::vector<char> peek;
    size_t total = std::min(buf.size(), static_cast<size_t>(8192));
    if (peek.size() < total) peek.resize(total);
    buf.peek(peek.data(), total);
    std::string_view req(peek.data(), total);

    size_t header_end = req.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        if (total >= 8192) {
            out_error = true;
            return false;
        }
        out_need_more = true;
        return false;
    }

    std::string method;
    std::string path;
    if (!parse_http_request_line(req, method, path)) {
        out_error = true;
        return false;
    }
    if (method != "GET") {
        out_error = true;
        return false;
    }

    const std::string &expected_path = cfg.ws_path.empty() ? "/" : cfg.ws_path;
    if (path != expected_path) {
        out_error = true;
        return false;
    }

    std::string_view headers = req.substr(req.find('\n') + 1, header_end - req.find('\n') - 1);
    std::string upgrade;
    std::string connection;
    std::string ws_key;
    std::string ws_version;
    if (!find_header_value(headers, "Upgrade", upgrade) || !icontains(upgrade, "websocket")) {
        out_error = true;
        return false;
    }
    if (!find_header_value(headers, "Connection", connection) || !icontains(connection, "upgrade")) {
        out_error = true;
        return false;
    }
    if (!find_header_value(headers, "Sec-WebSocket-Key", ws_key) || ws_key.empty()) {
        out_error = true;
        return false;
    }
    if (!find_header_value(headers, "Sec-WebSocket-Version", ws_version) || ws_version != "13") {
        out_error = true;
        return false;
    }

    const size_t consume = header_end + 4;
    buf.skip(consume);

    std::string accept = ws_accept_key(ws_key);
    out_response = "HTTP/1.1 101 Switching Protocols\r\n"
                   "Upgrade: websocket\r\n"
                   "Connection: Upgrade\r\n"
                   "Sec-WebSocket-Accept: " +
                   accept + "\r\n\r\n";
    return true;
}

bool build_ws_client_handshake_request(const NetConfig &cfg, std::string &out_request, std::string &out_key) {
    out_key = random_ws_key();
    std::string host = cfg.ws_host.empty() ? (cfg.ip + ":" + std::to_string(cfg.port)) : cfg.ws_host;
    const std::string &path = cfg.ws_path.empty() ? "/" : cfg.ws_path;
    out_request = "GET " + path + " HTTP/1.1\r\n"
                  "Host: " +
                  host + "\r\n"
                         "Upgrade: websocket\r\n"
                         "Connection: Upgrade\r\n"
                         "Sec-WebSocket-Key: " +
                  out_key + "\r\n"
                            "Sec-WebSocket-Version: 13\r\n";
    if (!cfg.ws_origin.empty()) {
        out_request += "Origin: " + cfg.ws_origin + "\r\n";
    }
    out_request += "\r\n";
    return true;
}

bool try_ws_client_handshake(CircularBuffer &buf, bool &out_done, bool &out_need_more, bool &out_error) {
    out_done = false;
    out_need_more = false;
    out_error = false;

    if (buf.size() < 16) {
        out_need_more = true;
        return false;
    }

    static thread_local std::vector<char> peek;
    size_t total = std::min(buf.size(), static_cast<size_t>(4096));
    if (peek.size() < total) peek.resize(total);
    buf.peek(peek.data(), total);
    std::string_view resp(peek.data(), total);

    size_t header_end = resp.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        if (total >= 4096) {
            out_error = true;
            return false;
        }
        out_need_more = true;
        return false;
    }

    std::string_view status_line = resp.substr(0, resp.find("\r\n"));
    if (status_line.find("101") == std::string_view::npos) {
        out_error = true;
        return false;
    }

    std::string_view headers = resp.substr(resp.find('\n') + 1, header_end - resp.find('\n') - 1);
    std::string upgrade;
    std::string connection;
    if (!find_header_value(headers, "Upgrade", upgrade) || !icontains(upgrade, "websocket")) {
        out_error = true;
        return false;
    }
    if (!find_header_value(headers, "Connection", connection) || !icontains(connection, "upgrade")) {
        out_error = true;
        return false;
    }

    buf.skip(header_end + 4);
    out_done = true;
    return true;
}

bool try_parse_ws_frame(CircularBuffer &buf, const NetConfig &cfg, bool from_client, const char *&out_payload,
                        uint32_t &out_len, WsOpcode &out_opcode, bool &out_error) {
    out_error = false;
    if (buf.size() < 2) return false;

    static thread_local std::vector<char> hdr;
    size_t peek_len = std::min(buf.size(), static_cast<size_t>(14));
    if (hdr.size() < peek_len) hdr.resize(peek_len);
    buf.peek(hdr.data(), peek_len);

    uint8_t b0 = static_cast<uint8_t>(hdr[0]);
    uint8_t b1 = static_cast<uint8_t>(hdr[1]);

    bool fin = (b0 & 0x80) != 0;
    WsOpcode opcode = static_cast<WsOpcode>(b0 & 0x0F);
    bool masked = (b1 & 0x80) != 0;
    uint64_t payload_len = b1 & 0x7F;

    size_t header_len = 2;
    if (payload_len == 126) {
        if (buf.size() < 4) return false;
        payload_len = (static_cast<uint64_t>(static_cast<uint8_t>(hdr[2])) << 8) |
                      static_cast<uint64_t>(static_cast<uint8_t>(hdr[3]));
        header_len = 4;
    } else if (payload_len == 127) {
        if (buf.size() < 10) return false;
        if (hdr.size() < 10) hdr.resize(10);
        if (peek_len < 10) {
            buf.peek(hdr.data(), 10);
        }
        payload_len = 0;
        for (int i = 0; i < 8; ++i) {
            payload_len = (payload_len << 8) | static_cast<uint64_t>(static_cast<uint8_t>(hdr[2 + i]));
        }
        header_len = 10;
    }

    if (payload_len > static_cast<uint64_t>(cfg.max_packet_len)) {
        out_error = true;
        return false;
    }

    if (from_client && !masked) {
        out_error = true;
        return false;
    }
    if (!from_client && masked) {
        out_error = true;
        return false;
    }

    if (masked) header_len += 4;
    if (buf.size() < header_len + payload_len) return false;

    uint8_t mask[4] = {0, 0, 0, 0};
    if (masked) {
        if (hdr.size() < header_len) hdr.resize(header_len);
        if (peek_len < header_len) {
            buf.peek(hdr.data(), header_len);
        }
        std::memcpy(mask, hdr.data() + header_len - 4, 4);
    }

    static thread_local std::vector<char> payload;
    if (payload.size() < payload_len) payload.resize(static_cast<size_t>(payload_len));
    buf.skip(header_len);
    if (payload_len > 0) {
        buf.read(payload.data(), static_cast<size_t>(payload_len));
        if (masked) apply_mask(payload.data(), static_cast<size_t>(payload_len), mask);
    }

    if (!fin && opcode != WsOpcode::Continuation) {
        // 暂不支持分片帧：非 FIN 的控制/数据帧视为协议错误
        if (opcode != WsOpcode::Ping && opcode != WsOpcode::Pong && opcode != WsOpcode::Close) {
            out_error = true;
            return false;
        }
    }

    out_payload = payload.data();
    out_len = static_cast<uint32_t>(payload_len);
    out_opcode = opcode;
    return true;
}

bool write_ws_frame(CircularBuffer &buf, const NetConfig &cfg, bool from_client, WsOpcode opcode, const char *data,
                    size_t len) {
    if (len > static_cast<size_t>(cfg.max_packet_len)) return false;
    size_t needed = ws_encoded_size(len) + (from_client ? 4 : 0);
    if (needed > buf.capacity() - buf.size()) return false;

    uint8_t b0 = static_cast<uint8_t>(0x80 | static_cast<uint8_t>(opcode));
    uint8_t b1 = from_client ? 0x80 : 0x00;

    if (len < 126) {
        b1 |= static_cast<uint8_t>(len);
        buf.write(reinterpret_cast<const char *>(&b0), 1);
        buf.write(reinterpret_cast<const char *>(&b1), 1);
    } else if (len <= 0xFFFF) {
        b1 |= 126;
        uint16_t ext = static_cast<uint16_t>(len);
        uint8_t ext_bytes[2] = {static_cast<uint8_t>((ext >> 8) & 0xFF), static_cast<uint8_t>(ext & 0xFF)};
        buf.write(reinterpret_cast<const char *>(&b0), 1);
        buf.write(reinterpret_cast<const char *>(&b1), 1);
        buf.write(reinterpret_cast<const char *>(ext_bytes), 2);
    } else {
        b1 |= 127;
        uint64_t ext = len;
        uint8_t ext_bytes[8];
        for (int i = 7; i >= 0; --i) {
            ext_bytes[i] = static_cast<uint8_t>(ext & 0xFF);
            ext >>= 8;
        }
        buf.write(reinterpret_cast<const char *>(&b0), 1);
        buf.write(reinterpret_cast<const char *>(&b1), 1);
        buf.write(reinterpret_cast<const char *>(ext_bytes), 8);
    }

    if (from_client) {
        uint8_t mask[4];
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(0, 255);
        for (auto &m : mask) m = static_cast<uint8_t>(dist(rng));
        buf.write(reinterpret_cast<const char *>(mask), 4);
        if (len > 0) {
            static thread_local std::vector<char> masked;
            if (masked.size() < len) masked.resize(len);
            std::memcpy(masked.data(), data, len);
            apply_mask(masked.data(), len, mask);
            buf.write(masked.data(), len);
        }
    } else if (len > 0) {
        buf.write(data, len);
    }
    return true;
}

bool write_ws_pong(CircularBuffer &buf, const NetConfig &cfg, bool from_client, const char *payload, size_t len) {
    return write_ws_frame(buf, cfg, from_client, WsOpcode::Pong, payload, len);
}

} // namespace fakelua::net
