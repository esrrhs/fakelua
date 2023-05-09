#include "var_string_heap.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

var_string var_string_heap::alloc(const std::string_view &str) {
    if (str.size() <= MAX_SHORT_STR_LEN) {
        // short string
        uint32_t index;

        // first, try to find the string in the short string map
        auto ok = short_str_to_index_map_.get_by_other_type(str, index);
        if (ok) {
            // found. return the index
            return var_string(true, index);
        }

        // not found. try to insert the string into the short string map
        index = short_str_index_.fetch_add(1);
        if (!index) {
            // index is 0. we need to reserve the index 0
            index = short_str_index_.fetch_add(1);
        }

        // alloc the string
        auto str_container = std::make_shared<std::string>(str);
        // try to insert the string with the index. maybe other thread has inserted the string.
        // if the string is already inserted, str_container and index will be set to the existing value.
        short_str_to_index_map_.get_or_set(str_container, index);
        // set the string to the vector at index
        short_str_vec_.set(index, str_container);
        return var_string(true, index);
    } else {
        // long string
        auto ptr = std::make_shared<std::string>(str);
        uint32_t index = long_str_vec_.push_back(ptr);
        return var_string(false, index);
    }
}

std::string_view var_string_heap::get(const var_string &str) const {
    auto string_index = str.string_index();
    if (string_index) {
        if (is_short_string_index(string_index)) {
            // short string
            auto index = get_short_string_index(string_index);
            str_container_ptr str_container;
            auto ok = short_str_vec_.get(index, str_container);
            if (ok) {
                return *str_container;
            }
        } else {
            // long string
            auto index = get_long_string_index(string_index);
            str_container_ptr str_container;
            auto ok = long_str_vec_.get(index, str_container);
            if (ok) {
                return *str_container;
            }
        }
    }
    return {};
}

}// namespace fakelua
