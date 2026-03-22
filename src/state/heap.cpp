#include "heap.h"
#include "State.h"
#include "util/common.h"

#include <ranges>

namespace fakelua {

void *Heap::Alloc(size_t size) {
    if (blocks_.empty() || current_block_offset_ + size > BLOCK_SIZE) {
        // allocate a new block
        void *new_block = malloc(std::max(BLOCK_SIZE, size));
        if (!new_block) {
            throw std::runtime_error("failed to allocate memory");
        }
        blocks_.emplace_back(new_block);
        current_block_index_ = blocks_.size() - 1;
        current_block_offset_ = 0;
    }

    void *ptr = static_cast<char *>(blocks_[current_block_index_]) + current_block_offset_;
    current_block_offset_ += size;
    return ptr;
}

void Heap::Reset() {
    current_block_index_ = 0;
    current_block_offset_ = 0;
    for (const auto &block: blocks_) {
        free(block);
    }
    blocks_.clear();
}

}// namespace fakelua
