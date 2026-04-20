#include "heap.h"
#include "state.h"
#include "util/common.h"

namespace fakelua {

HeapAllocator::~HeapAllocator() {
    Reset();
    for (const auto &block: blocks_) {
        free(block);
    }
    blocks_.clear();
}

void *HeapAllocator::Alloc(size_t size) {
    constexpr size_t alignment = alignof(std::max_align_t);

    // 仅检查 size 本身是否超出块容量。padding 在同一块内才有意义，
    // 跨块时 offset 会重置为 0 导致 padding 为 0，因此不参与上限判断。
    if (size > BLOCK_SIZE) {
        ThrowFakeluaException(std::format("Alloc failed, requested size {} exceeds block size {}", size, BLOCK_SIZE));
    }

    size_t padding = (alignment - (current_block_offset_ % alignment)) % alignment;

    if (current_block_offset_ + padding + size > BLOCK_SIZE) {
        current_block_offset_ = 0;
        current_block_index_++;
        padding = 0;
    }

    if (current_block_index_ >= blocks_.size()) {
        // 分配一个新的内存块
        void *new_block = malloc(BLOCK_SIZE);
        if (!new_block) {
            throw std::runtime_error("failed to allocate memory");
        }
        blocks_.emplace_back(new_block);
    }

    current_block_offset_ += padding;
    void *ptr = static_cast<char *>(blocks_[current_block_index_]) + current_block_offset_;
    current_block_offset_ += size;
    return ptr;
}

void HeapAllocator::Reset() {
    for (auto it = destructors_.rbegin(); it != destructors_.rend(); ++it) {
        it->destroyer(it->ptr);
    }
    destructors_.clear();
    current_block_index_ = 0;
    current_block_offset_ = 0;
}

size_t HeapAllocator::Size() const {
    return current_block_index_ * BLOCK_SIZE + current_block_offset_;
}

}// namespace fakelua
