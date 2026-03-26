#include "heap.h"
#include "state.h"
#include "util/common.h"

namespace fakelua {

HeapAllocator::~HeapAllocator() {
    current_block_index_ = 0;
    current_block_offset_ = 0;
    for (const auto &block: blocks_) {
        free(block);
    }
    blocks_.clear();
}

void *HeapAllocator::Alloc(size_t size) {
    if (size > BLOCK_SIZE) {
        ThrowFakeluaException(std::format("requested size {} exceeds block size {}", size, BLOCK_SIZE));
    }

    if (current_block_offset_ + size > BLOCK_SIZE) {
        current_block_offset_ = 0;
        current_block_index_++;
    }

    if (current_block_index_ >= blocks_.size()) {
        // allocate a new block
        void *new_block = malloc(BLOCK_SIZE);
        if (!new_block) {
            throw std::runtime_error("failed to allocate memory");
        }
        blocks_.emplace_back(new_block);
    }

    void *ptr = static_cast<char *>(blocks_[current_block_index_]) + current_block_offset_;
    current_block_offset_ += size;
    return ptr;
}

void HeapAllocator::Reset() {
    current_block_index_ = 0;
    current_block_offset_ = 0;
}

size_t HeapAllocator::Size() const {
    return current_block_index_ * BLOCK_SIZE + current_block_offset_;
}

}// namespace fakelua
