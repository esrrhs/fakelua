#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var_string.h"

namespace fakelua {

class HeapAllocator {
public:
    ~HeapAllocator();

    // 分配一块连续的临时内存，分配后的指针地址不会变化
    [[nodiscard]] void *Alloc(size_t size);

    // 重置，释放所有临时内存
    void Reset();

    // 当前临时内存使用
    [[nodiscard]] size_t Size() const;

private:
    static constexpr size_t BLOCK_SIZE = 1024 * 1024;// 每块内存的大小，默认1MB
    std::vector<void *> blocks_;                     // 大块内存的地址
    size_t current_block_index_ = 0;                 // 当前使用的大块内存索引
    size_t current_block_offset_ = 0;                // 当前使用的大块内存偏移
};

// 从大块内存中切出一小块内存使用，释放的时候一口气全释放
class Heap {
public:
    ~Heap();

    HeapAllocator &GetTempAllocator() {
        return temp_allocator_;
    }

    HeapAllocator &GetConstAllocator() {
        return const_allocator_;
    }

    void Reset() {
        temp_allocator_.Reset();
        // const_allocator_不重置，常量内存一直保留
    }

private:
    HeapAllocator temp_allocator_; // 临时内存分配器，编译过程中使用，编译结束后重置
    HeapAllocator const_allocator_;// 常量内存分配器，编译过程中使用，编译结束后不重置
};

}// namespace fakelua
