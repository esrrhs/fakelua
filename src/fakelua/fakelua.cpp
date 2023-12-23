#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

namespace inter {

var *native_to_fakelua_bool(fakelua_state_ptr s, bool v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_bool(v);
    return ret;
}

var *native_to_fakelua_char(fakelua_state_ptr s, char v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_uchar(fakelua_state_ptr s, unsigned char v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_short(fakelua_state_ptr s, short v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ushort(fakelua_state_ptr s, unsigned short v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_int(fakelua_state_ptr s, int v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_uint(fakelua_state_ptr s, unsigned int v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_long(fakelua_state_ptr s, long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ulong(fakelua_state_ptr s, unsigned long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_longlong(fakelua_state_ptr s, long long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ulonglong(fakelua_state_ptr s, unsigned long long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_float(fakelua_state_ptr s, float v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_float(v);
    return ret;
}

var *native_to_fakelua_double(fakelua_state_ptr s, double v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_float(v);
    return ret;
}

var *native_to_fakelua_cstr(fakelua_state_ptr s, const char *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_str(fakelua_state_ptr s, char *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_string(fakelua_state_ptr s, const std::string &v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_stringview(fakelua_state_ptr s, const std::string_view &v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

void vi_to_var(fakelua_state_ptr s, var_interface *src, var *dst) {
    switch (src->vi_get_type()) {
        case var_interface::type::NIL:
            dst->set_nil();
            break;
        case var_interface::type::BOOL:
            dst->set_bool(src->vi_get_bool());
            break;
        case var_interface::type::INT:
            dst->set_int(src->vi_get_int());
            break;
        case var_interface::type::FLOAT:
            dst->set_float(src->vi_get_float());
            break;
        case var_interface::type::STRING:
            dst->set_string(s, src->vi_get_string());
            break;
        case var_interface::type::TABLE:
            dst->set_table();
            for (int i = 0; i < src->vi_get_table_size(); ++i) {
                auto kv = src->vi_get_table_kv(i);
                auto k = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
                auto v = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
                vi_to_var(s, kv.first, k);
                vi_to_var(s, kv.second, v);
                dst->get_table().set(k, v);
            }
            break;
        default:
            throw_fakelua_exception(std::format("vi_to_var failed, type is {}", (int) src->vi_get_type()));
    }
}

var *native_to_fakelua_obj(fakelua_state_ptr s, var_interface *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    vi_to_var(s, v, ret);
    return ret;
}

bool fakelua_to_native_bool(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_BOOL) {
        return v->get_bool();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_bool failed, type is {}", magic_enum::enum_name(v->type())));
}

char fakelua_to_native_char(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_char failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned char fakelua_to_native_uchar(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_uchar failed, type is {}", magic_enum::enum_name(v->type())));
}

short fakelua_to_native_short(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_short failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned short fakelua_to_native_ushort(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ushort failed, type is {}", magic_enum::enum_name(v->type())));
}

int fakelua_to_native_int(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_int failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned int fakelua_to_native_uint(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_uint failed, type is {}", magic_enum::enum_name(v->type())));
}

long fakelua_to_native_long(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_long failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned long fakelua_to_native_ulong(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ulong failed, type is {}", magic_enum::enum_name(v->type())));
}

long long fakelua_to_native_longlong(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_longlong failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned long long fakelua_to_native_ulonglong(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ulonglong failed, type is {}", magic_enum::enum_name(v->type())));
}

float fakelua_to_native_float(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_FLOAT) {
        return v->get_float();
    }
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_float failed, type is {}", magic_enum::enum_name(v->type())));
}

double fakelua_to_native_double(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_FLOAT) {
        return v->get_float();
    }
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_double failed, type is {}", magic_enum::enum_name(v->type())));
}

const char *fakelua_to_native_cstr(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string().data();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_cstr failed, type is {}", magic_enum::enum_name(v->type())));
}

const char *fakelua_to_native_str(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string().data();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_str failed, type is {}", magic_enum::enum_name(v->type())));
}

std::string fakelua_to_native_string(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return std::string(v->get_string());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_string failed, type is {}", magic_enum::enum_name(v->type())));
}

std::string_view fakelua_to_native_stringview(fakelua_state_ptr s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_stringview failed, type is {}", magic_enum::enum_name(v->type())));
}

void var_to_vi(fakelua_state_ptr s, var *src, var_interface *dst) {
    switch (src->type()) {
        case var_type::VAR_NIL:
            dst->vi_set_nil();
            break;
        case var_type::VAR_BOOL:
            dst->vi_set_bool(src->get_bool());
            break;
        case var_type::VAR_INT:
            dst->vi_set_int(src->get_int());
            break;
        case var_type::VAR_FLOAT:
            dst->vi_set_float(src->get_float());
            break;
        case var_type::VAR_STRING:
            dst->vi_set_string(src->get_string());
            break;
        case var_type::VAR_TABLE: {
            std::vector<std::pair<var_interface *, var_interface *>> kvs;
            src->get_table().range([&](var *k, var *v) {
                auto ki = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
                auto vi = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
                var_to_vi(s, k, ki);
                var_to_vi(s, v, vi);
                kvs.emplace_back(ki, vi);
            });
            dst->vi_set_table(kvs);
            break;
        }
        default:
            throw_fakelua_exception(std::format("var_to_vi failed, type is {}", (int) src->type()));
    }
}

var_interface *fakelua_to_native_obj(fakelua_state_ptr s, var *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
    var_to_vi(s, v, ret);
    return ret;
}

var *fakelua_get_var_by_index(fakelua_state_ptr s, var *ret, size_t i) {
    if (ret->type() == var_type::VAR_TABLE) {
        var tmp;
        tmp.set_int(i);
        return ret->get_table().get(&tmp);
    }
    throw_fakelua_exception(std::format("fakelua_get_var_by_index failed, type is {}", magic_enum::enum_name(ret->type())));
}

void *get_func_addr(fakelua_state_ptr s, const std::string &name, int &arg_count, bool &is_variadic) {
    auto func = std::dynamic_pointer_cast<state>(s)->get_vm().get_function(name);
    if (func) {
        arg_count = func->get_arg_count();
        is_variadic = func->is_variadic();
        return func->get_addr();
    }
    return nullptr;
}

var *make_variadic_table(fakelua_state_ptr s, int start, int n, var **args) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_table();
    for (int i = 0; i < n - start; i++) {
        auto key = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
        key->set_int(i + 1);
        auto v = args[start + i];
        ret->get_table().set(key, v, true);
    }
    ret->set_variadic(true);
    return ret;
}

void throw_inter_fakelua_exception(const std::string &msg) {
    throw_fakelua_exception(msg);
}

}// namespace inter

fakelua_state_ptr fakelua_newstate() {
    LOG_INFO("fakelua_newstate");
    return std::make_shared<state>();
}

}// namespace fakelua
