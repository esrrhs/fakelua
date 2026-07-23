#include "var_table.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

// Get() is needed by C++ test code to read JIT-created tables.
// The JIT C code creates tables using c_runtime_header.h; this provides
// C++ accessors for verification.  The full Set/Rehash implementations
// are no longer needed (production uses the JIT code path).

Var VarTable::Get(const Var &key) const {
    if (UNLIKELY(key.Type() == VarType::Nil)) {
        ThrowFakeluaException("VarTable Get failed, table index is nil");
    }
    // Normalize float keys that are actually integer-valued.
    Var lookup_key = key;
    if (UNLIKELY(key.Type() == VarType::Float)) {
        const auto result = TryConvertDoubleToInt64(key.GetFloat());
        if (result) {
            lookup_key.SetInt(*result);
        }
    }

    // Check spec fields first.
    if (spec_count > 0) {
        const auto *sk = reinterpret_cast<const Var *>(spec_keys);
        const auto *sv = reinterpret_cast<const Var *>(spec_vals);
        for (uint32_t i = 0; i < spec_count; ++i) {
            if (sk[i].Equal(lookup_key)) {
                return sv[i];
            }
        }
    }

    if (UNLIKELY(count_ == 0)) {
        return const_null_var;
    }

    const auto hash_val = static_cast<uint32_t>(lookup_key.Hash());

    if (LIKELY(bucket_count_ == 0)) {
        for (uint32_t i = 0; i < count_; ++i) {
            if (quick_data_[i].hash == hash_val && quick_data_[i].key.Equal(lookup_key)) {
                return quick_data_[i].val;
            }
        }
        return const_null_var;
    }

    const uint32_t mask = bucket_count_ - 1;
    uint32_t curr_idx = hash_val & mask;
    const TableNode *curr = &nodes_[curr_idx];

    if (UNLIKELY(curr->entry.key.Type() == VarType::Nil)) {
        return const_null_var;
    }

    for (;;) {
        if (curr->entry.hash == hash_val && curr->entry.key.Equal(lookup_key)) {
            return curr->entry.val;
        }
        curr_idx = curr->next;
        if (curr_idx == INVALID_INDEX) {
            break;
        }
        curr = &nodes_[curr_idx];
    }
    return const_null_var;
}

}// namespace fakelua
