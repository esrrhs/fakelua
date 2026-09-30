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
    // spec 块字节数。0 表示没有需要搬迁的 spec（spec 指针为空）。
    // 与 c_runtime_header.h 的 VarTable 保持同布局。
    uint32_t spec_bytes;
    // >0 时 spec 块是连续 spec_cvars 个 CVar（JIT 表特化）。0 表示不透明块（如 NativeObjectSpec）。
    uint32_t spec_cvars;
};

}// namespace fakelua
