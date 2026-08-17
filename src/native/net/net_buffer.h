#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace fakelua::net {

constexpr size_t kPacketHeaderSize = 4;

class CircularBuffer {
public:
    explicit CircularBuffer(size_t capacity);
    ~CircularBuffer();

    [[nodiscard]] size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }
    [[nodiscard]] bool full() const { return size_ >= buf_.size(); }
    [[nodiscard]] size_t capacity() const { return buf_.size(); }

    // 写入数据，返回实际写入字节数
    size_t write(const char *data, size_t len);
    // 读取数据（消费），返回实际读取字节数
    size_t read(char *dst, size_t len);
    // 查看数据但不消费
    size_t peek(char *dst, size_t len) const;
    // 丢弃指定字节数
    size_t skip(size_t len);

    // 获取可写入的连续缓冲区（用于直接 recv 到缓冲区）
    [[nodiscard]] std::pair<char *, size_t> writable_region();
    void commit_write(size_t bytes);
    // 获取可读的连续缓冲区（用于直接 send）
    [[nodiscard]] std::pair<const char *, size_t> readable_region();
    void commit_read(size_t bytes);

    void clear();

private:
    std::vector<char> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};

void write_packet_header(CircularBuffer &buf, uint32_t payload_len);
bool try_parse_packet(CircularBuffer &buf, const char *&out_payload, uint32_t &out_len);

} // namespace fakelua::net
