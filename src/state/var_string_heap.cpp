#include "var_string_heap.h"
#include "fakelua.h"
#include "state.h"
#include "util/common.h"

namespace fakelua {

const std::string_view &var_string_heap::alloc(const std::string_view &str, bool is_const) {
    // first, try to find the string in the const string map
    auto it = const_str_map_.find(str);
    if (it != const_str_map_.end()) {
        // found. return
        return it->second->str();
    }

    if (is_const) {
        // alloc const but found in tmp map, need to move it to const map
        it = tmp_str_map_.find(str);
        if (it != tmp_str_map_.end()) {
            // move it to const map
            auto s = it->second;
            tmp_str_map_.erase(it);
            const auto &key = s->str();
            const_str_map_.emplace(key, s);
            return key;
        }

        // not found.
        auto s = var_string::make_var_string(str.data(), static_cast<int>(str.size()));
        const auto &key = s->str();
        const_str_map_.emplace(key, s);
        return key;
    } else {
        it = tmp_str_map_.find(str);
        if (it != tmp_str_map_.end()) {
            // found.
            return it->second->str();
        }

        // not found.
        auto s = var_string::make_var_string(str.data(), static_cast<int>(str.size()));
        const auto &key = s->str();
        tmp_str_map_.emplace(key, s);
        return key;
    }
}

void var_string_heap::reset() {
    for (auto &iter: const_str_map_) {
        free(iter.second);
    }
    const_str_map_.clear();
}

}// namespace fakelua
