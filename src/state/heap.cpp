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
    size_t padding = (alignment - (current_block_offset_ % alignment)) % alignment;

    if (size + padding > BLOCK_SIZE) {
        ThrowFakeluaException(std::format("requested size {} with padding {} exceeds block size {}", size, padding, BLOCK_SIZE));
    }

    if (current_block_offset_ + padding + size > BLOCK_SIZE) {
        current_block_offset_ = 0;
        current_block_index_++;
        padding = 0;
    }

    if (current_block_index_ >= blocks_.size()) {
        // allocate a new block
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
