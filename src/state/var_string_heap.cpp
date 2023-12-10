#include "var_string_heap.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

std::string_view var_string_heap::alloc(const std::string_view &str) {
    if (str.size() <= MAX_SHORT_STR_LEN) {
        // first, try to find the string in the short string map
        auto it = short_str_to_index_map_.find(str);
        if (it != short_str_to_index_map_.end()) {
            // found. return
            return it->second;
        }

        // not found. try to alloc size from the str_mem_
        if (str_mem_index_ + str.size() + 1 <= str_mem_.size()) {
            // alloc success
            memcpy(&str_mem_[str_mem_index_], str.data(), str.size());
            str_mem_[str_mem_index_ + str.size()] = '\0';
            auto ret = std::string_view(&str_mem_[str_mem_index_], str.size());
            short_str_to_index_map_.emplace(str, ret);
            str_mem_index_ += str.size() + 1;
            return ret;
        } else {
            // alloc failed. use the short_str_tmp_ to store the new short strings.
            auto s = std::make_shared<std::string>(str.data(), str.size());
            auto ret = std::string_view(s->data(), s->size());
            short_str_tmp_.push_back(s);
            short_str_to_index_map_.emplace(str, ret);
            return true, ret;
        }
    } else {
        // long string
        auto s = std::make_shared<std::string>(str.data(), str.size());
        long_str_vec_.push_back(s);
        auto ret = std::string_view(s->data(), s->size());
        return ret;
    }
}

void var_string_heap::reset() {
    str_mem_index_ = 0;
    auto tmp_size = 0;
    for (auto &s: short_str_tmp_) {
        tmp_size += s->size() + 1;
    }
    short_str_tmp_.clear();
    long_str_vec_.clear();
    short_str_to_index_map_.clear();
    if (tmp_size > 0) {
        str_mem_.resize(str_mem_.size() + tmp_size * 2);
    }
}

}// namespace fakelua
