#include "var_string_heap.h"
#include "fakelua.h"
#include "state.h"
#include "util/common.h"

namespace fakelua {

std::string_view var_string_heap::alloc(const std::string_view &str) {
    if (str.size() <= MAX_SHORT_STR_LEN) {
        // first, try to find the string in the short string map
        auto it = short_str_to_index_map_.find(str);
        if (it != short_str_to_index_map_.end()) {
            // found. return
            return *it->second;
        }

        // not found.
        auto s = std::make_shared<std::string>(str.data(), str.size());
        auto key = std::string_view(*s);
        short_str_to_index_map_.emplace(key, s);
        return *s;
    } else {
        // long string
        auto s = std::make_shared<std::string>(str.data(), str.size());
        long_str_vec_.push_back(s);
        return *s;
    }
}

void var_string_heap::reset() {
    // remove all string except the used ones in the vm
    std::unordered_set<const char *> used;
    auto &vm = dynamic_cast<state *>(state_)->get_vm();
    for (auto &pair: vm.get_functions()) {
        auto func = pair.second;
        auto handle = func->get_gcc_jit_handle();
        auto &str_container_map = handle->get_str_container_map();
        std::transform(str_container_map.begin(), str_container_map.end(), std::inserter(used, used.end()),
                       [](const auto &s) { return s.data(); });
    }

    std::erase_if(short_str_to_index_map_, [&used](const auto &pair) { return used.find(pair.second->c_str()) == used.end(); });

    std::erase_if(long_str_vec_, [&used](const auto &str) { return used.find(str->c_str()) == used.end(); });
}

}// namespace fakelua
