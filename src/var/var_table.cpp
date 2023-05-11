#include "var_table.h"
#include "fakelua.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

var *var_table::get(const var &key) {
    auto type = key.type();
    switch (type) {
        case var_type::STRING: {
            auto str = key.get_string();
            auto str_index = str.string_index();
            if (is_long_string_index(str_index)) {
                throw std::runtime_error("table get: long string index not supported");
            }
            var *ret = nullptr;
            bool ok = str_table_.get(str_index, ret);
            if (!ok) {
                return &const_null_var;
            }
            return ret;
        }
        case var_type::INT: {
            auto k = key.get_int();
            var *ret = nullptr;
            bool ok = int_table_.get(k, ret);
            if (!ok) {
                return &const_null_var;
            }
            return ret;
        }
        default: {
            throw std::runtime_error("table get: invalid key type");
        }
    }
}

}// namespace fakelua
