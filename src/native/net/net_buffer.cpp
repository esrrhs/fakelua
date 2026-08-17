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

bool try_parse_packet(CircularBuffer &buf, const char *&out_payload, uint32_t &out_len) {
    if (buf.size() < kPacketHeaderSize) return false;

    char header[kPacketHeaderSize];
    buf.peek(header, kPacketHeaderSize);

    uint32_t payload_len = (static_cast<uint8_t>(header[0]) << 24) |
                           (static_cast<uint8_t>(header[1]) << 16) |
                           (static_cast<uint8_t>(header[2]) << 8) |
                           static_cast<uint8_t>(header[3]);

    if (buf.size() < kPacketHeaderSize + payload_len) return false;

    buf.skip(kPacketHeaderSize);

    // 拷贝 payload（环形缓冲区可能回绕，需要线性缓冲区）
    static thread_local std::vector<char> parse_tmp;
    if (parse_tmp.size() < payload_len) parse_tmp.resize(payload_len);
    buf.read(parse_tmp.data(), payload_len);
    out_payload = parse_tmp.data();
    out_len = payload_len;
    return true;
}

} // namespace fakelua::net
