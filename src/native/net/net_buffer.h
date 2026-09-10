#pragma once

#include "native/net/net_common.h"

#include <cstddef>
#include <cstdint>
#include <string>
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

    // 复用的线性暂存区。环形缓冲的可读区会绕回，想交出一段连续的 const char* 就得先拷到
    // 一块连续内存里，这两块就是干这个的，免得每次解包都做一次堆分配。
    //
    // 按缓冲区各存一份，而不是用 thread_local：一个连接只被它所属的 State 单线程访问，
    // 所以这里既没有竞争，还顺带把 out_payload 的有效期从"直到本线程下次调用"收紧成
    // "直到同一个缓冲区上的下次调用" —— 前者意味着解析另一条连接会让先前的 payload 失效。
    //
    // 分两块是因为 ws 帧解析要同时用：一块 peek 头部，一块存包体。
    [[nodiscard]] std::vector<char> &header_scratch() {
        return header_scratch_;
    }

    [[nodiscard]] std::vector<char> &payload_scratch() {
        return payload_scratch_;
    }

private:
    std::vector<char> buf_;
    std::vector<char> header_scratch_;
    std::vector<char> payload_scratch_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};

// 封包打包函数：依据配置向 send_buf 写入打包后的数据（含长度头/分隔符）
// 若缓冲区剩余空间不足以容纳完整编码后的包，则不写入任何内容并返回 false（禁止截断）
bool write_packet(CircularBuffer &buf, const NetConfig &cfg, const char *data, size_t len);

// 封包解析函数：依据配置从 recv_buf 中尝试解析出一个完整的数据包
// out_error: 当声明长度非法（超过 max_packet_len / 超过缓冲区容量 / uint32 溢出）时为 true，
//           调用方应关闭该连接；返回 false 且 out_error=false 仅表示数据未齐，需继续等待
// out_payload: 指向 buf 自己的复用暂存区（CircularBuffer::payload_scratch），只在下一次
//              对同一个 buf 调用本函数之前有效，调用方必须在那之前把数据拷走。

bool try_parse_packet(CircularBuffer &buf, const NetConfig &cfg, const char *&out_payload, uint32_t &out_len,
                      bool &out_error);

// 兼容旧接口的 4 字节大端打包
void write_packet_header(CircularBuffer &buf, uint32_t payload_len);

} // namespace fakelua::net
