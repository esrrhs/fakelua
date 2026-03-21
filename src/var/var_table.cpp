#include "var_table.h"
#include "var.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

Var VarTable::Get(const Var &key) const {
    DEBUG_ASSERT(key.Type() >= VarType::Min && key.Type() <= VarType::Max);
    for (const auto &[fst, snd]: table_) {
        if (fst.Equal(key)) {
            return snd;
        }
    }
    return const_null_var;
}

void VarTable::Set(const Var &key, const Var &val, bool can_be_nil) {
    DEBUG_ASSERT(key.Type() >= VarType::Min && key.Type() <= VarType::Max);
    DEBUG_ASSERT(val.Type() >= VarType::Min && val.Type() <= VarType::Max);

    // set nil means delete
    if (val.Type() == VarType::Nil && !can_be_nil) {
        for (size_t index = 0; index < table_.size(); ++index) {
            if (const auto &[fst, snd] = table_[index]; fst.Equal(key)) {
                if (index < table_.size() - 1) {
                    // swap with the last element
                    table_[index] = table_.back();
                }
                // pop back
                table_.pop_back();
                break;
            }
        }
        return;
    }

    // find and update the value
    for (auto &[fst, snd]: table_) {
        if (fst.Equal(key)) {
            snd = val;
            return;
        }
    }

    // not found, insert a new key-value pair
    table_.emplace_back(key, val);
}

Var VarTable::KeyAt(size_t pos) const {
    if (pos >= table_.size()) {
        return const_null_var;
    }
    return table_[pos].first;
}

Var VarTable::ValueAt(size_t pos) const {
    if (pos >= table_.size()) {
        return const_null_var;
    }
    return table_[pos].second;
}

}// namespace fakelua
