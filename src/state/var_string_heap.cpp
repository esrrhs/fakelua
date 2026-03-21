#include "var_string_heap.h"
#include "fakelua.h"
#include "state.h"
#include "util/common.h"

#include <ranges>

namespace fakelua {

var_string *var_string_heap::alloc(const std::string_view &str) {
    auto it = tmp_str_map_.find(str);
    if (it != tmp_str_map_.end()) {
        // found.
        return it->second;
    }

    // not found.
    auto s = var_string::make_var_string(str.data(), static_cast<int>(str.size()));
    const auto &key = s->str();
    tmp_str_map_.emplace(key, s);
    return s;
}

void var_string_heap::reset() {
    for (const auto &val: tmp_str_map_ | std::views::values) {
        var_string::free_var_string(val);
    }
    tmp_str_map_.clear();
}

}// namespace fakelua
