#include "var_string_heap.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

var_string var_string_heap::alloc(const std::string &str) {
    if (str.size() <= MAX_SHORT_STR_LEN) {
        // short string
        var_string ret;
        auto ok = short_str_map_.get(str, ret);
        if (ok) {
            return ret;
        }
        auto index = short_str_index_.fetch_add(1);
        if (!index) {
            index = short_str_index_.fetch_add(1);
        }
        ret = var_string(index);
        short_str_map_.set(str, ret);
        return ret;
    } else {
        // long string
        auto ptr = std::make_shared<std::string>(str);
        long_str_vec_.push_back(ptr);
        return var_string(*ptr);
    }
}

}// namespace fakelua
