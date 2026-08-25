#include "native/net/net_buffer.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace fakelua::net {

CircularBuffer::CircularBuffer(size_t capacity) : buf_(capacity) {}

CircularBuffer::~CircularBuffer() = default;

void CircularBuffer::clear() {
    head_ = tail_ = size_ = 0;
}

size_t CircularBuffer::write(const char *data, size_t len) {
    size_t cap = buf_.size();
    size_t avail = cap - size_;
    len = std::min(len, avail);
    if (len == 0) return 0;
    size_t first = std::min(len, cap - tail_);
    std::memcpy(buf_.data() + tail_, data, first);
    size_t second = len - first;
    if (second > 0) {
        std::memcpy(buf_.data(), data + first, second);
    }
    tail_ = (tail_ + len) % cap;
    size_ += len;
    return len;
}

size_t CircularBuffer::read(char *dst, size_t len) {
    len = std::min(len, size_);
    if (len == 0) return 0;
    size_t cap = buf_.size();
    size_t first = std::min(len, cap - head_);
    std::memcpy(dst, buf_.data() + head_, first);
    size_t second = len - first;
    if (second > 0) {
        std::memcpy(dst + first, buf_.data(), second);
    }
    head_ = (head_ + len) % cap;
    size_ -= len;
    return len;
}

size_t CircularBuffer::peek(char *dst, size_t len) const {
    len = std::min(len, size_);
    if (len == 0) return 0;
    size_t cap = buf_.size();
    size_t first = std::min(len, cap - head_);
    std::memcpy(dst, buf_.data() + head_, first);
    size_t second = len - first;
    if (second > 0) {
        std::memcpy(dst + first, buf_.data(), second);
    }
    return len;
}

size_t CircularBuffer::skip(size_t len) {
    len = std::min(len, size_);
    head_ = (head_ + len) % buf_.size();
    size_ -= len;
    return len;
}

std::pair<char *, size_t> CircularBuffer::writable_region() {
    size_t cap = buf_.size();
    if (size_ >= cap) return {nullptr, 0};
    size_t end = (head_ > tail_) ? head_ : cap;
    return {buf_.data() + tail_, end - tail_};
}

void CircularBuffer::commit_write(size_t bytes) {
    size_t cap = buf_.size();
    tail_ = (tail_ + bytes) % cap;
    size_ += bytes;
}

std::pair<const char *, size_t> CircularBuffer::readable_region() {
    size_t cap = buf_.size();
    if (size_ == 0) return {nullptr, 0};
    size_t end = (tail_ > head_) ? tail_ : cap;
    return {buf_.data() + head_, end - head_};
}

void CircularBuffer::commit_read(size_t bytes) {
    size_t cap = buf_.size();
    head_ = (head_ + bytes) % cap;
    size_ -= bytes;
}

void write_packet_header(CircularBuffer &buf, uint32_t payload_len) {
    char header[kPacketHeaderSize];
    header[0] = static_cast<char>((payload_len >> 24) & 0xFF);
    header[1] = static_cast<char>((payload_len >> 16) & 0xFF);
    header[2] = static_cast<char>((payload_len >> 8) & 0xFF);
    header[3] = static_cast<char>(payload_len & 0xFF);
    buf.write(header, kPacketHeaderSize);
}

// 计算某 framer 编码 len 字节 payload 所需的总字节数（含头/分隔符/填充）。
// 返回值以 size_t 表示，避免 uint32 溢出。
static size_t encoded_packet_size(const NetConfig &cfg, size_t len) {
    switch (cfg.framer) {
        case FramerType::Header4BigEndian:
        case FramerType::Header4LittleEndian:
            return 4 + len;
        case FramerType::Header2BigEndian:
        case FramerType::Header2LittleEndian:
            return 2 + len;
        case FramerType::LineDelimiter:
            return len + 1;
        case FramerType::FixedLength:
            return (cfg.fixed_packet_len > 0) ? static_cast<size_t>(cfg.fixed_packet_len) : len;
        case FramerType::RawStream:
        case FramerType::Custom:
        default:
            return len;
    }
}

bool write_packet(CircularBuffer &buf, const NetConfig &cfg, const char *data, size_t len) {
    if (cfg.custom_encoder_fn) {
        size_t room = buf.capacity() - buf.size();
        CircularBuffer tmp(room + 1);
        cfg.custom_encoder_fn(tmp, data, len);
        if (tmp.size() > room) return false;
        size_t n = tmp.size();
        if (n > 0) {
            std::vector<char> tmp_data(n);
            tmp.read(tmp_data.data(), n);
            buf.write(tmp_data.data(), n);
        }
        return true;
    }

    // 整包必须能完整写入，否则拒绝（禁止截断半个 length-header 包）。
    size_t needed = encoded_packet_size(cfg, len);
    if (needed > buf.capacity() - buf.size()) return false;

    switch (cfg.framer) {
        case FramerType::Header4BigEndian: {
            write_packet_header(buf, static_cast<uint32_t>(len));
            buf.write(data, len);
            break;
        }
        case FramerType::Header4LittleEndian: {
            char header[4];
            uint32_t l = static_cast<uint32_t>(len);
            header[0] = static_cast<char>(l & 0xFF);
            header[1] = static_cast<char>((l >> 8) & 0xFF);
            header[2] = static_cast<char>((l >> 16) & 0xFF);
            header[3] = static_cast<char>((l >> 24) & 0xFF);
            buf.write(header, 4);
            buf.write(data, len);
            break;
        }
        case FramerType::Header2BigEndian: {
            if (len > 0xFFFF) return false; // 超出 uint16 表示范围，拒绝
            char header[2];
            uint16_t l = static_cast<uint16_t>(len);
            header[0] = static_cast<char>((l >> 8) & 0xFF);
            header[1] = static_cast<char>(l & 0xFF);
            buf.write(header, 2);
            buf.write(data, len);
            break;
        }
        case FramerType::Header2LittleEndian: {
            if (len > 0xFFFF) return false; // 超出 uint16 表示范围，拒绝
            char header[2];
            uint16_t l = static_cast<uint16_t>(len);
            header[0] = static_cast<char>(l & 0xFF);
            header[1] = static_cast<char>((l >> 8) & 0xFF);
            buf.write(header, 2);
            buf.write(data, len);
            break;
        }
        case FramerType::LineDelimiter: {
            buf.write(data, len);
            buf.write("\n", 1);
            break;
        }
        case FramerType::FixedLength: {
            // 如果不足 fixed_packet_len，补 0；如果超出则截断
            size_t target_len = (cfg.fixed_packet_len > 0) ? static_cast<size_t>(cfg.fixed_packet_len) : len;
            if (len >= target_len) {
                buf.write(data, target_len);
            } else {
                buf.write(data, len);
                std::vector<char> pad(target_len - len, '\0');
                buf.write(pad.data(), pad.size());
            }
            break;
        }
        case FramerType::RawStream:
        case FramerType::Custom:
        default: {
            buf.write(data, len);
            break;
        }
    }
    return true;
}

// 校验 length-header framer 读出的 payload_len 是否合法。
// 使用 size_t 做全部比较，杜绝 uint32 加法溢出。
// 返回 true 表示合法（可能数据不足，由调用方继续等）；
// 返回 false 且 *out_error=true 表示协议违规，调用方应关闭连接。
static bool validate_payload_len(const NetConfig &cfg, size_t payload_len, size_t header_size,
                                 size_t buf_size, size_t buf_capacity, bool &out_error) {
    size_t total = header_size + payload_len; // size_t，不会溢出
    if (payload_len > static_cast<size_t>(cfg.max_packet_len)) {
        out_error = true;
        return false;
    }
    if (total > buf_capacity) {
        // 声明长度超过缓冲区容量，永远装不下 → 协议违规
        out_error = true;
        return false;
    }
    if (buf_size < total) {
        // 数据还没到齐，继续等待（不是错误）
        return false;
    }
    return true;
}

bool try_parse_packet(CircularBuffer &buf, const NetConfig &cfg, const char *&out_payload, uint32_t &out_len,
                      bool &out_error) {
    out_error = false;

    if (cfg.custom_parser_fn) {
        return cfg.custom_parser_fn(buf, out_payload, out_len);
    }

    static thread_local std::vector<char> parse_tmp;

    switch (cfg.framer) {
        case FramerType::Header4BigEndian: {
            if (buf.size() < 4) return false;
            char header[4];
            buf.peek(header, 4);
            size_t payload_len = (static_cast<size_t>(static_cast<uint8_t>(header[0])) << 24) |
                                 (static_cast<size_t>(static_cast<uint8_t>(header[1])) << 16) |
                                 (static_cast<size_t>(static_cast<uint8_t>(header[2])) << 8) |
                                 static_cast<size_t>(static_cast<uint8_t>(header[3]));
            if (!validate_payload_len(cfg, payload_len, 4, buf.size(), buf.capacity(), out_error)) {
                return false;
            }
            buf.skip(4);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = static_cast<uint32_t>(payload_len);
            return true;
        }
        case FramerType::Header4LittleEndian: {
            if (buf.size() < 4) return false;
            char header[4];
            buf.peek(header, 4);
            size_t payload_len = static_cast<size_t>(static_cast<uint8_t>(header[0])) |
                                 (static_cast<size_t>(static_cast<uint8_t>(header[1])) << 8) |
                                 (static_cast<size_t>(static_cast<uint8_t>(header[2])) << 16) |
                                 (static_cast<size_t>(static_cast<uint8_t>(header[3])) << 24);
            if (!validate_payload_len(cfg, payload_len, 4, buf.size(), buf.capacity(), out_error)) {
                return false;
            }
            buf.skip(4);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = static_cast<uint32_t>(payload_len);
            return true;
        }
        case FramerType::Header2BigEndian: {
            if (buf.size() < 2) return false;
            char header[2];
            buf.peek(header, 2);
            size_t payload_len = (static_cast<size_t>(static_cast<uint8_t>(header[0])) << 8) |
                                 static_cast<size_t>(static_cast<uint8_t>(header[1]));
            if (!validate_payload_len(cfg, payload_len, 2, buf.size(), buf.capacity(), out_error)) {
                return false;
            }
            buf.skip(2);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = static_cast<uint32_t>(payload_len);
            return true;
        }
        case FramerType::Header2LittleEndian: {
            if (buf.size() < 2) return false;
            char header[2];
            buf.peek(header, 2);
            size_t payload_len = static_cast<size_t>(static_cast<uint8_t>(header[0])) |
                                 (static_cast<size_t>(static_cast<uint8_t>(header[1])) << 8);
            if (!validate_payload_len(cfg, payload_len, 2, buf.size(), buf.capacity(), out_error)) {
                return false;
            }
            buf.skip(2);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = static_cast<uint32_t>(payload_len);
            return true;
        }
        case FramerType::LineDelimiter: {
            if (buf.empty()) return false;
            size_t total = buf.size();
            if (total > static_cast<size_t>(cfg.max_packet_len)) {
                // 超过 max_packet_len 仍未见到换行符，视为恶意/异常连接
                out_error = true;
                return false;
            }
            if (parse_tmp.size() < total) parse_tmp.resize(total);
            buf.peek(parse_tmp.data(), total);

            // 查找 '\n'
            size_t line_end = 0;
            bool found = false;
            for (size_t i = 0; i < total; ++i) {
                if (parse_tmp[i] == '\n') {
                    line_end = i;
                    found = true;
                    break;
                }
            }
            if (!found) return false;

            // 消费包含 '\n' 在内的所有字节
            buf.read(parse_tmp.data(), line_end + 1);

            // 去除末尾可选的 '\r'
            size_t payload_len = line_end;
            if (payload_len > 0 && parse_tmp[payload_len - 1] == '\r') {
                payload_len--;
            }
            out_payload = parse_tmp.data();
            out_len = static_cast<uint32_t>(payload_len);
            return true;
        }
        case FramerType::FixedLength: {
            if (cfg.fixed_packet_len <= 0) return false;
            uint32_t fixed_len = static_cast<uint32_t>(cfg.fixed_packet_len);
            if (fixed_len > buf.capacity() ||
                (cfg.max_packet_len > 0 && fixed_len > static_cast<uint32_t>(cfg.max_packet_len))) {
                out_error = true;
                return false;
            }
            if (buf.size() < fixed_len) return false;
            if (parse_tmp.size() < fixed_len) parse_tmp.resize(fixed_len);
            buf.read(parse_tmp.data(), fixed_len);
            out_payload = parse_tmp.data();
            out_len = fixed_len;
            return true;
        }
        case FramerType::RawStream: {
            if (buf.empty()) return false;
            // 原始流也做上限，避免单 tick 转发无界数据
            size_t n = std::min(buf.size(), static_cast<size_t>(cfg.max_packet_len));
            if (parse_tmp.size() < n) parse_tmp.resize(n);
            buf.read(parse_tmp.data(), n);
            out_payload = parse_tmp.data();
            out_len = static_cast<uint32_t>(n);
            return true;
        }
        case FramerType::Custom:
        default:
            return false;
    }
}

} // namespace fakelua::net
