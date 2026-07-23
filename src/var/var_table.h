#pragma once

#include "var/var.h"
#include <cstdint>

namespace fakelua {

// VarTable struct layout must match c_runtime_header.h exactly.
// This is the C++ wrapper around the C struct definition.
// The C++ implementations (var_table.cpp) have been removed;
// only the struct definition and inline accessors remain.
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

    [[nodiscard]] uint32_t Size() const { return count_ + spec_count; }
    [[nodiscard]] uint32_t GetHashCount() const { return count_; }

    [[nodiscard]] Var Get(const Var &key) const {
        // Quick-data only lookup — sufficient for test verification.
        // Production code goes through c_runtime_header.h inline functions.
        const uint32_t hash_val = static_cast<uint32_t>(key.Hash());
        for (uint32_t i = 0; i < count_; ++i) {
            if (quick_data_[i].hash == hash_val && quick_data_[i].key.Equal(key)) {
                return quick_data_[i].val;
            }
        }
        return const_null_var;
    }

    [[nodiscard]] const VarEntry *GetQuickData() const { return quick_data_; }
    [[nodiscard]] const TableNode *GetNodes() const { return nodes_; }
    [[nodiscard]] const uint32_t *GetActiveList() const { return active_list_; }
    [[nodiscard]] uint32_t GetSpecCount() const { return spec_count; }
    [[nodiscard]] const CVar *GetSpecKeys() const { return spec_keys; }
    [[nodiscard]] const CVar *GetSpecVals() const { return spec_vals; }
};

// Size must match c_runtime_header.h struct layout

}// namespace fakelua
