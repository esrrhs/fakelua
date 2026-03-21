#include "State.h"
#include "var_string_heap.h"
#include "fakelua.h"
#include "util/common.h"

#include <ranges>

namespace fakelua {

VarString *VarStringHeap::alloc(const std::string_view &str) {
    auto it = tmp_str_map_.find(str);
    if (it != tmp_str_map_.end()) {
        // found.
        return it->second;
    }

    // not found.
    auto s = VarString::MakeVarString(str.data(), static_cast<int>(str.size()));
    const auto &key = s->Str();
    tmp_str_map_.emplace(key, s);
    return s;
}

void VarStringHeap::reset() {
    for (const auto &val: tmp_str_map_ | std::views::values) {
        VarString::FreeVarString(val);
    }
    tmp_str_map_.clear();
}

}// namespace fakelua
