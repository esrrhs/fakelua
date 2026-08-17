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

void write_packet(CircularBuffer &buf, const NetConfig &cfg, const char *data, size_t len) {
    if (cfg.custom_encoder_fn) {
        cfg.custom_encoder_fn(buf, data, len);
        return;
    }

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
            char header[2];
            uint16_t l = static_cast<uint16_t>(len);
            header[0] = static_cast<char>((l >> 8) & 0xFF);
            header[1] = static_cast<char>(l & 0xFF);
            buf.write(header, 2);
            buf.write(data, len);
            break;
        }
        case FramerType::Header2LittleEndian: {
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
}

bool try_parse_packet(CircularBuffer &buf, const NetConfig &cfg, const char *&out_payload, uint32_t &out_len) {
    if (cfg.custom_parser_fn) {
        return cfg.custom_parser_fn(buf, out_payload, out_len);
    }

    static thread_local std::vector<char> parse_tmp;

    switch (cfg.framer) {
        case FramerType::Header4BigEndian: {
            if (buf.size() < 4) return false;
            char header[4];
            buf.peek(header, 4);
            uint32_t payload_len = (static_cast<uint8_t>(header[0]) << 24) |
                                   (static_cast<uint8_t>(header[1]) << 16) |
                                   (static_cast<uint8_t>(header[2]) << 8) |
                                   static_cast<uint8_t>(header[3]);
            if (buf.size() < 4 + payload_len) return false;
            buf.skip(4);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = payload_len;
            return true;
        }
        case FramerType::Header4LittleEndian: {
            if (buf.size() < 4) return false;
            char header[4];
            buf.peek(header, 4);
            uint32_t payload_len = static_cast<uint8_t>(header[0]) |
                                   (static_cast<uint8_t>(header[1]) << 8) |
                                   (static_cast<uint8_t>(header[2]) << 16) |
                                   (static_cast<uint8_t>(header[3]) << 24);
            if (buf.size() < 4 + payload_len) return false;
            buf.skip(4);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = payload_len;
            return true;
        }
        case FramerType::Header2BigEndian: {
            if (buf.size() < 2) return false;
            char header[2];
            buf.peek(header, 2);
            uint32_t payload_len = (static_cast<uint8_t>(header[0]) << 8) |
                                   static_cast<uint8_t>(header[1]);
            if (buf.size() < 2 + payload_len) return false;
            buf.skip(2);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = payload_len;
            return true;
        }
        case FramerType::Header2LittleEndian: {
            if (buf.size() < 2) return false;
            char header[2];
            buf.peek(header, 2);
            uint32_t payload_len = static_cast<uint8_t>(header[0]) |
                                   (static_cast<uint8_t>(header[1]) << 8);
            if (buf.size() < 2 + payload_len) return false;
            buf.skip(2);
            if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
            buf.read(parse_tmp.data(), payload_len);
            out_payload = parse_tmp.data();
            out_len = payload_len;
            return true;
        }
        case FramerType::LineDelimiter: {
            if (buf.empty()) return false;
            size_t total = buf.size();
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
            if (buf.size() < fixed_len) return false;
            if (parse_tmp.size() < fixed_len) parse_tmp.resize(fixed_len);
            buf.read(parse_tmp.data(), fixed_len);
            out_payload = parse_tmp.data();
            out_len = fixed_len;
            return true;
        }
        case FramerType::RawStream: {
            if (buf.empty()) return false;
            size_t n = buf.size();
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
