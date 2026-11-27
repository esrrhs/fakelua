#include "heap.h"

#include <ranges>

#include "state.h"
#include "util/common.h"

namespace fakelua {

HeapAllocator::~HeapAllocator() {
    Reset();
    if (slot_) {
        slot_->curr = nullptr;
        slot_->end = nullptr;
    }
    for (const auto &block: blocks_) {
#if defined(_WIN32)
        _aligned_free(block);
#else
        free(block);
#endif
    }
    blocks_.clear();
}

void *HeapAllocator::Alloc(size_t size) {
    constexpr size_t alignment = alignof(std::max_align_t);

    // JIT FlFastAlloc bumps slot_->curr without updating current_block_offset_.
    // Pull the bump pointer back before every C++ allocation so the two paths
    // never hand out overlapping ranges in the same block.
    if (slot_ && slot_->curr && current_block_index_ < blocks_.size()) {
        char *base = static_cast<char *>(blocks_[current_block_index_]);
        char *end = base + BLOCK_SIZE;
        if (slot_->curr >= base && slot_->curr <= end) {
            current_block_offset_ = static_cast<size_t>(slot_->curr - base);
        }
    }

    // 保持 current_block_offset_ 与 slot_->curr 始终按 alignment（16 字节）对齐，
    // 否则 C++ 侧分配任意非 16 倍数字节后，JIT 快速路径 FlFastAlloc 直接使用 hs->curr
    // 会发出非对齐的 VarTable / CVar / VarClosure 指针。
    const size_t aligned_size = ((size == 0 ? 1 : size) + alignment - 1) & ~(alignment - 1);
    if (UNLIKELY(aligned_size > BLOCK_SIZE)) {
        ThrowFakeluaException(std::format("Alloc failed, requested size {} exceeds block size {}", size, BLOCK_SIZE));
    }

    size_t padding = (alignment - (current_block_offset_ % alignment)) % alignment;

    if (UNLIKELY(current_block_offset_ + padding + aligned_size > BLOCK_SIZE)) {
        current_block_offset_ = 0;
        current_block_index_++;
        padding = 0;
    }

    if (UNLIKELY(current_block_index_ >= blocks_.size())) {
        // 分配一个新的内存块，确保块起始地址按 alignment 对齐，
        // 这样当 current_block_offset_=0 时返回的地址天然满足对齐要求。
        // BLOCK_SIZE 是 alignment 的整数倍（1MB 是 16 的倍数），满足 aligned_alloc 的要求。
#if defined(_WIN32)
        void *new_block = _aligned_malloc(BLOCK_SIZE, alignment);
#else
        void *new_block = std::aligned_alloc(alignment, BLOCK_SIZE);
#endif
        if (!new_block) {
            throw std::runtime_error("failed to allocate memory");
        }
        blocks_.emplace_back(new_block);
    }

    current_block_offset_ += padding;
    void *ptr = static_cast<char *>(blocks_[current_block_index_]) + current_block_offset_;
    current_block_offset_ += aligned_size;
    if (slot_) {
        slot_->curr = static_cast<char *>(blocks_[current_block_index_]) + current_block_offset_;
        slot_->end = static_cast<char *>(blocks_[current_block_index_]) + BLOCK_SIZE;
    }
    return ptr;
}

void HeapAllocator::Reset() {
    for (const auto &destructor_info: std::views::reverse(destructors_)) {
        destructor_info.destroyer(destructor_info.ptr);
    }
    destructors_.clear();
    current_block_index_ = 0;
    current_block_offset_ = 0;
    if (slot_) {
        if (!blocks_.empty()) {
            slot_->curr = static_cast<char *>(blocks_[0]);
            slot_->end = static_cast<char *>(blocks_[0]) + BLOCK_SIZE;
        } else {
            slot_->curr = nullptr;
            slot_->end = nullptr;
        }
    }
}

size_t HeapAllocator::Size() const {
    size_t offset = current_block_offset_;
    if (slot_ && slot_->curr && current_block_index_ < blocks_.size()) {
        const char *base = static_cast<const char *>(blocks_[current_block_index_]);
        const char *end = base + BLOCK_SIZE;
        if (slot_->curr >= base && slot_->curr <= end) {
            offset = static_cast<size_t>(slot_->curr - base);
        }
    }
    return current_block_index_ * BLOCK_SIZE + offset;
}

}// namespace fakelua
