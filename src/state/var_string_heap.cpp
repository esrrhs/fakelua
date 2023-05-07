#include "var_string_heap.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

var_string var_string_heap::alloc(const std::string_view &str) {
    if (str.size() <= MAX_SHORT_STR_LEN) {
        // short string
        uint32_t index;
        auto ok = short_str_to_index_map_.get_by_other_type(str, index);
        if (ok) {
            return var_string(index);
        }
        index = short_str_index_.fetch_add(1);
        if (!index) {
            index = short_str_index_.fetch_add(1);
        }
        auto str_container = std::make_shared<std::string>(str);
        short_str_to_index_map_.set(str_container, index);
        index_to_short_str_map_.set(index, str_container);
        return var_string(index);
    } else {
        // long string
        auto ptr = std::make_shared<std::string>(str);
        long_str_vec_.push_back(ptr);
        return var_string(*ptr);
    }
}

std::string_view var_string_heap::get(const var_string &str) const {
    if (str.short_string_index()) {
        auto index = str.short_string_index();
        str_container_ptr str_container;
        auto ok = index_to_short_str_map_.get(index, str_container);
        if (ok) {
            return *str_container;
        }
        return {};
    } else {
        return str.long_string_view();
    }
}

}// namespace fakelua
