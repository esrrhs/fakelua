#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

namespace inter {

var *native_to_fakelua_nil(const fakelua_state_ptr &s) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_nil();
    return ret;
}

var *native_to_fakelua_bool(const fakelua_state_ptr &s, bool v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_bool(v);
    return ret;
}

var *native_to_fakelua_char(const fakelua_state_ptr &s, char v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_uchar(const fakelua_state_ptr &s, unsigned char v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_short(const fakelua_state_ptr &s, short v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ushort(const fakelua_state_ptr &s, unsigned short v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_int(const fakelua_state_ptr &s, int v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_uint(const fakelua_state_ptr &s, unsigned int v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_long(const fakelua_state_ptr &s, long v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ulong(const fakelua_state_ptr &s, unsigned long v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_longlong(const fakelua_state_ptr &s, long long v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ulonglong(const fakelua_state_ptr &s, unsigned long long v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_float(const fakelua_state_ptr &s, float v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_float(v);
    return ret;
}

var *native_to_fakelua_double(const fakelua_state_ptr &s, double v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_float(v);
    return ret;
}

var *native_to_fakelua_cstr(const fakelua_state_ptr &s, const char *v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_str(const fakelua_state_ptr &s, char *v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_string(const fakelua_state_ptr &s, const std::string &v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_stringview(const fakelua_state_ptr &s, const std::string_view &v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

void vi_to_var(const fakelua_state_ptr &s, const var_interface *src, var *dst) {
    DEBUG_ASSERT(src->vi_get_type() >= var_interface::type::MIN && src->vi_get_type() <= var_interface::type::MAX);
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
            dst->set_table(s);
            for (int i = 0; i < src->vi_get_table_size(); ++i) {
                const auto [fst, snd] = src->vi_get_table_kv(i);
                const auto k = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
                const auto v = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
                vi_to_var(s, fst, k);
                vi_to_var(s, snd, v);
                dst->get_table()->set(*k, *v);
            }
            break;
    }
}

var *native_to_fakelua_obj(const fakelua_state_ptr &s, const var_interface *v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    vi_to_var(s, v, ret);
    return ret;
}

bool fakelua_to_native_bool(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_BOOL) {
        return v->get_bool();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_bool failed, type is {}", magic_enum::enum_name(v->type())));
}

char fakelua_to_native_char(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_char failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned char fakelua_to_native_uchar(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_uchar failed, type is {}", magic_enum::enum_name(v->type())));
}

short fakelua_to_native_short(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_short failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned short fakelua_to_native_ushort(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ushort failed, type is {}", magic_enum::enum_name(v->type())));
}

int fakelua_to_native_int(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_int failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned int fakelua_to_native_uint(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_uint failed, type is {}", magic_enum::enum_name(v->type())));
}

long fakelua_to_native_long(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_long failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned long fakelua_to_native_ulong(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ulong failed, type is {}", magic_enum::enum_name(v->type())));
}

long long fakelua_to_native_longlong(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_longlong failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned long long fakelua_to_native_ulonglong(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ulonglong failed, type is {}", magic_enum::enum_name(v->type())));
}

float fakelua_to_native_float(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_FLOAT) {
        return v->get_float();
    }
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_float failed, type is {}", magic_enum::enum_name(v->type())));
}

double fakelua_to_native_double(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_FLOAT) {
        return v->get_float();
    }
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_double failed, type is {}", magic_enum::enum_name(v->type())));
}

const char *fakelua_to_native_cstr(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string().data();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_cstr failed, type is {}", magic_enum::enum_name(v->type())));
}

const char *fakelua_to_native_str(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string().data();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_str failed, type is {}", magic_enum::enum_name(v->type())));
}

std::string fakelua_to_native_string(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return std::string(v->get_string());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_string failed, type is {}", magic_enum::enum_name(v->type())));
}

std::string_view fakelua_to_native_stringview(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_stringview failed, type is {}", magic_enum::enum_name(v->type())));
}

void var_to_vi(const fakelua_state_ptr &s, var *src, var_interface *dst) {
    DEBUG_ASSERT(src->type() >= var_type::VAR_MIN && src->type() <= var_type::VAR_MAX);
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
    }
}

var_interface *fakelua_to_native_obj(const fakelua_state_ptr &s, var *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
    var_to_vi(s, v, ret);
    return ret;
}

var *fakelua_get_var_by_index(const fakelua_state_ptr &s, var *ret, size_t i) {
    DEBUG_ASSERT(ret);
    if (ret->type() == var_type::VAR_TABLE && ret->is_variadic()) {
        var tmp;
        tmp.set_int(i);
        return ret->get_table().get(&tmp);
    } else {
        if (i == 1) {
            return ret;
        } else {
            return &const_null_var;
        }
    }
}

void *get_func_addr(const fakelua_state_ptr &s, const std::string &name, int &arg_count, bool &is_variadic) {
    if (const auto func = std::dynamic_pointer_cast<state>(s)->get_vm().get_function(name)) {
        arg_count = func->get_arg_count();
        is_variadic = func->is_variadic();
        return func->get_addr();
    }
    return nullptr;
}

var *make_variadic_table(const fakelua_state_ptr &s, int start, int n, var **args) {
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

void reset(const fakelua_state_ptr &s) {
    std::dynamic_pointer_cast<state>(s)->reset();
}

[[noreturn]] void throw_inter_fakelua_exception(const std::string &msg) {
    throw_fakelua_exception(msg);
}

}// namespace inter

fakelua_state_ptr fakelua_newstate() {
    LOG_INFO("fakelua_newstate");
    return std::make_shared<state>();
}

void fakelua_state::set_debug_log_level(int level) {
    set_log_level((log_level) level);
}

}// namespace fakelua
