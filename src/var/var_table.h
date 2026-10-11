#pragma once

#include "var/var.h"
#include <cstdint>

namespace fakelua {

// VarTable struct layout must match c_runtime_header.h exactly.
// No methods — this is a pure struct definition for ABI compatibility
// with JIT-compiled C code. All table logic lives in c_runtime_header.h.
struct VarTable {
    static constexpr uint32_t QUICK_DATA_SIZE = 8;
    static constexpr uint32_t INVALID_INDEX = 0xFFFFFFFFu;

    struct VarEntry {
        Var key;
        Var val;
        uint32_t hash;
    };

    struct TableNode {
        VarEntry entry;
        uint32_t next;
        uint32_t active_pos;
    };

    uint32_t count_;
    uint32_t bucket_count_;
    TableNode *nodes_;
    uint32_t *active_list_;
    VarEntry quick_data_[QUICK_DATA_SIZE];
    uint32_t free_list_idx_;
    void *spec;
    void *spec_get;
    void *spec_set;
    CVar *spec_keys;
    CVar *spec_vals;
    uint32_t spec_count;
    // 连续整数键前缀长度（# 运算符结果）的缓存。seq_len_valid_ == 0 代表缓存无效，
    // 需要重算——因此把 VarTable 整体清零的分配路径天然落在安全的重算分支上。
    uint32_t seq_len_valid_;
    int64_t seq_len_;
    // 数组部分：整数键 1..arr_size_ 只存放在 arr_ 中（允许 nil 空洞），哈希部分不含
    // [1, arr_size_+1] 内的整数键。arr_size_ 只增不减。必须与 c_runtime_header.h 一致。
    CVar *arr_;
    uint32_t arr_size_;
    uint32_t arr_cap_;
};

}// namespace fakelua
