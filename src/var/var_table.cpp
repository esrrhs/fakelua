#include "var_table.h"
#include "fakelua.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

var *var_table::get(const var &key) {
    auto type = key.type();
    switch (type) {
        case var_type::VAR_STRING: {
            if (key.is_short_string()) {
                throw std::runtime_error("table get: long string index not supported");
            }
            auto str = key.get_string();
            var *ret = nullptr;
            bool ok = str_table_.get(reinterpret_cast<int64_t>(str.data()), ret);
            if (!ok) {
                return &const_null_var;
            }
            return ret;
        }
        case var_type::VAR_INT: {
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

void var_table::set(const var &key, var *val) {
    auto type = key.type();
    if (!val || val->type() == var_type::VAR_NIL) {
        switch (type) {
            case var_type::VAR_STRING: {
                if (key.is_short_string()) {
                    throw std::runtime_error("table set nil: long string index not supported");
                }
                auto str = key.get_string();
                str_table_.remove(reinterpret_cast<int64_t>(str.data()));
                break;
            }
            case var_type::VAR_INT: {
                auto k = key.get_int();
                int_table_.remove(k);
                break;
            }
            default: {
                throw std::runtime_error("table set nil: invalid key type");
            }
        }
        return;
    }

    switch (type) {
        case var_type::VAR_STRING: {
            if (key.is_short_string()) {
                throw std::runtime_error("table set: long string index not supported");
            }
            auto str = key.get_string();
            str_table_.set(reinterpret_cast<int64_t>(str.data()), val);
            break;
        }
        case var_type::VAR_INT: {
            auto k = key.get_int();
            int_table_.set(k, val);
            break;
        }
        default: {
            throw std::runtime_error("table set: invalid key type");
        }
    }
}

}// namespace fakelua
