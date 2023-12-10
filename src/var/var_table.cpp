#include "var_table.h"
#include "fakelua.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

var *var_table::get(var *key) {
    auto type = key->type();
    switch (type) {
        case var_type::VAR_STRING: {
            if (!key->is_short_string()) {
                throw std::runtime_error("table get: long string index not supported");
            }
            auto str = key->get_string();
            pair_type ret;
            bool ok = str_table_.get(reinterpret_cast<int64_t>(str.data()), ret);
            if (!ok) {
                return &const_null_var;
            }
            return ret.second;
        }
        case var_type::VAR_INT: {
            auto k = key->get_int();
            pair_type ret;
            bool ok = int_table_.get(k, ret);
            if (!ok) {
                return &const_null_var;
            }
            return ret.second;
        }
        default: {
            throw std::runtime_error("table get: invalid key type");
        }
    }
}

var *var_table::get_by_int(int64_t key) {
    pair_type ret;
    bool ok = int_table_.get(key, ret);
    if (!ok) {
        return &const_null_var;
    }
    return ret.second;
}

var *var_table::get_by_str(const std::string_view &key) {
    if (key.length() > MAX_SHORT_STR_LEN) {
        throw std::runtime_error("table get: long string index not supported");
    }
    pair_type ret;
    bool ok = str_table_.get(reinterpret_cast<int64_t>(key.data()), ret);
    if (!ok) {
        return &const_null_var;
    }
    return ret.second;
}

void var_table::set(var *key, var *val, bool can_be_nil) {
    auto type = key->type();
    if (!can_be_nil && (!val || val->type() == var_type::VAR_NIL)) {
        switch (type) {
            case var_type::VAR_STRING: {
                if (!key->is_short_string()) {
                    throw std::runtime_error("table set nil: long string index not supported");
                }
                auto str = key->get_string();
                str_table_.remove(reinterpret_cast<int64_t>(str.data()));
                break;
            }
            case var_type::VAR_INT: {
                auto k = key->get_int();
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
            if (!key->is_short_string()) {
                throw std::runtime_error("table set: long string index not supported");
            }
            auto str = key->get_string();
            str_table_.set(reinterpret_cast<int64_t>(str.data()), std::make_pair(key, val));
            break;
        }
        case var_type::VAR_INT: {
            auto k = key->get_int();
            int_table_.set(k, std::make_pair(key, val));
            break;
        }
        default: {
            throw std::runtime_error("table set: invalid key type");
        }
    }
}

void var_table::range(std::function<void(var *, var *)> iter) {
    str_table_.range([&](const int64_t &key, const pair_type &kv) {
        iter(kv.first, kv.second);
        return true;
    });

    int_table_.range([&](const int64_t &key, const pair_type &kv) {
        iter(kv.first, kv.second);
        return true;
    });
}

}// namespace fakelua
