#pragma once

#include "var/var.h"
#include "util/common.h"
#include <cstdint>

namespace fakelua {

// VarTable struct layout must match c_runtime_header.h exactly.
// Use function pointer typedefs matching the C struct to ensure ABI compatibility.
typedef CVar (*SpecGetFn)(void *tbl, CVar k, bool *finish);
typedef void (*SpecSetFn)(void *tbl, CVar k, CVar v, bool *finish);

// Lightweight C++ wrapper: struct + inline Size() + inline accessors only.
// All table logic is in c_runtime_header.h (used by JIT-compiled C code).
// The simple inline Get() iterates active_list for test verification.
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
    SpecGetFn spec_get;
    SpecSetFn spec_set;
    CVar *spec_keys;
    CVar *spec_vals;
    uint32_t spec_count;

    [[nodiscard]] uint32_t Size() const { return count_ + spec_count; }
    [[nodiscard]] uint32_t GetHashCount() const { return count_; }

    [[nodiscard]] Var Get(const Var &key) const {
        return GetImpl(key);
    }

    [[nodiscard]] const VarEntry *GetQuickData() const { return quick_data_; }
    [[nodiscard]] const TableNode *GetNodes() const { return nodes_; }
    [[nodiscard]] const uint32_t *GetActiveList() const { return active_list_; }
    [[nodiscard]] uint32_t GetSpecCount() const { return spec_count; }
    [[nodiscard]] const CVar *GetSpecKeys() const { return spec_keys; }
    [[nodiscard]] const CVar *GetSpecVals() const { return spec_vals; }

private:
    [[nodiscard]] Var GetImpl(const Var &key) const {
        if (UNLIKELY(count_ == 0)) {
            return const_null_var;
        }
        // Quick-data mode: linear scan
        if (bucket_count_ == 0) {
            for (uint32_t i = 0; i < count_; ++i) {
                if (quick_data_[i].key.Equal(key)) {
                    return quick_data_[i].val;
                }
            }
            return const_null_var;
        }
        // Hash mode: iterate active_list for all valid entries
        for (uint32_t i = 0; i < count_; ++i) {
            const auto &e = nodes_[active_list_[i]].entry;
            if (e.key.Equal(key)) {
                return e.val;
            }
        }
        return const_null_var;
    }
};

}// namespace fakelua
