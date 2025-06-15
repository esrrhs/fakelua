#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

namespace inter {

cvar native_to_fakelua_nil(const fakelua_state_ptr &s) {
    return var{};
}

cvar native_to_fakelua_bool(const fakelua_state_ptr &s, bool v) {
    var ret;
    ret.set_bool(v);
    return ret;
}

cvar native_to_fakelua_char(const fakelua_state_ptr &s, char v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_uchar(const fakelua_state_ptr &s, unsigned char v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_short(const fakelua_state_ptr &s, short v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_ushort(const fakelua_state_ptr &s, unsigned short v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_int(const fakelua_state_ptr &s, int v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_uint(const fakelua_state_ptr &s, unsigned int v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_long(const fakelua_state_ptr &s, long v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_ulong(const fakelua_state_ptr &s, unsigned long v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_longlong(const fakelua_state_ptr &s, long long v) {
    var ret;
    ret.set_int(v);
    return ret;
}

cvar native_to_fakelua_ulonglong(const fakelua_state_ptr &s, unsigned long long v) {
    var ret;
    ret.set_int(static_cast<int64_t>(v));
    return ret;
}

cvar native_to_fakelua_float(const fakelua_state_ptr &s, float v) {
    var ret;
    ret.set_float(v);
    return ret;
}

cvar native_to_fakelua_double(const fakelua_state_ptr &s, double v) {
    var ret;
    ret.set_float(v);
    return ret;
}

cvar native_to_fakelua_cstr(const fakelua_state_ptr &s, const char *v) {
    var ret;
    ret.set_string(s, v);
    return ret;
}

cvar native_to_fakelua_str(const fakelua_state_ptr &s, char *v) {
    var ret;
    ret.set_string(s, v);
    return ret;
}

cvar native_to_fakelua_string(const fakelua_state_ptr &s, const std::string &v) {
    var ret;
    ret.set_string(s, v);
    return ret;
}

cvar native_to_fakelua_stringview(const fakelua_state_ptr &s, const std::string_view &v) {
    var ret;
    ret.set_string(s, v);
    return ret;
}

var vi_to_var(const fakelua_state_ptr &s, const var_interface *src) {
    DEBUG_ASSERT(src->vi_get_type() >= var_interface::type::MIN && src->vi_get_type() <= var_interface::type::MAX);
    var ret;
    switch (src->vi_get_type()) {
        case var_interface::type::NIL:
            ret.set_nil();
            break;
        case var_interface::type::BOOL:
            ret.set_bool(src->vi_get_bool());
            break;
        case var_interface::type::INT:
            ret.set_int(src->vi_get_int());
            break;
        case var_interface::type::FLOAT:
            ret.set_float(src->vi_get_float());
            break;
        case var_interface::type::STRING:
            ret.set_string(s, src->vi_get_string());
            break;
        case var_interface::type::TABLE:
            ret.set_table(s);
            for (int i = 0; i < static_cast<int>(src->vi_get_table_size()); ++i) {
                const auto [fst, snd] = src->vi_get_table_kv(i);
                auto k = vi_to_var(s, fst);
                auto v = vi_to_var(s, snd);
                ret.get_table()->set(k, v, true);
            }
            break;
    }
    return ret;
}

cvar native_to_fakelua_obj(const fakelua_state_ptr &s, const var_interface *v) {
    return vi_to_var(s, v);
}

bool fakelua_to_native_bool(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_BOOL) {
        return vv.get_bool();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_bool failed, type is {}", magic_enum::enum_name(vv.type())));
}

char fakelua_to_native_char(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<char>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_char failed, type is {}", magic_enum::enum_name(vv.type())));
}

unsigned char fakelua_to_native_uchar(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<unsigned char>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_uchar failed, type is {}", magic_enum::enum_name(vv.type())));
}

short fakelua_to_native_short(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<short>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_short failed, type is {}", magic_enum::enum_name(vv.type())));
}

unsigned short fakelua_to_native_ushort(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<unsigned short>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ushort failed, type is {}", magic_enum::enum_name(vv.type())));
}

int fakelua_to_native_int(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<int>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_int failed, type is {}", magic_enum::enum_name(vv.type())));
}

unsigned int fakelua_to_native_uint(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<unsigned int>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_uint failed, type is {}", magic_enum::enum_name(vv.type())));
}

long fakelua_to_native_long(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return vv.get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_long failed, type is {}", magic_enum::enum_name(vv.type())));
}

unsigned long fakelua_to_native_ulong(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return vv.get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ulong failed, type is {}", magic_enum::enum_name(vv.type())));
}

long long fakelua_to_native_longlong(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return vv.get_int();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_longlong failed, type is {}", magic_enum::enum_name(vv.type())));
}

unsigned long long fakelua_to_native_ulonglong(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<unsigned long long>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_ulonglong failed, type is {}", magic_enum::enum_name(vv.type())));
}

float fakelua_to_native_float(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_FLOAT) {
        return static_cast<float>(vv.get_float());
    }
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<float>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_float failed, type is {}", magic_enum::enum_name(vv.type())));
}

double fakelua_to_native_double(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_FLOAT) {
        return vv.get_float();
    }
    if (vv.type() == var_type::VAR_INT) {
        return static_cast<double>(vv.get_int());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_double failed, type is {}", magic_enum::enum_name(vv.type())));
}

const char *fakelua_to_native_cstr(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_STRING) {
        return vv.get_string()->str().data();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_cstr failed, type is {}", magic_enum::enum_name(vv.type())));
}

const char *fakelua_to_native_str(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_STRING) {
        return vv.get_string()->str().data();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_str failed, type is {}", magic_enum::enum_name(vv.type())));
}

std::string fakelua_to_native_string(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_STRING) {
        return std::string(vv.get_string()->str());
    }
    throw_fakelua_exception(std::format("fakelua_to_native_string failed, type is {}", magic_enum::enum_name(vv.type())));
}

std::string_view fakelua_to_native_stringview(const fakelua_state_ptr &s, cvar v) {
    const auto vv = static_cast<var &>(v);
    if (vv.type() == var_type::VAR_STRING) {
        return vv.get_string()->str();
    }
    throw_fakelua_exception(std::format("fakelua_to_native_stringview failed, type is {}", magic_enum::enum_name(vv.type())));
}

void var_to_vi(const fakelua_state_ptr &s, cvar src, var_interface *dst) {
    const auto vv = static_cast<var &>(src);
    DEBUG_ASSERT(vv.type() >= var_type::VAR_MIN && vv.type() <= var_type::VAR_MAX);
    switch (vv.type()) {
        case var_type::VAR_NIL:
            dst->vi_set_nil();
            break;
        case var_type::VAR_BOOL:
            dst->vi_set_bool(vv.get_bool());
            break;
        case var_type::VAR_INT:
            dst->vi_set_int(vv.get_int());
            break;
        case var_type::VAR_FLOAT:
            dst->vi_set_float(vv.get_float());
            break;
        case var_type::VAR_STRING:
            dst->vi_set_string(vv.get_string()->str());
            break;
        case var_type::VAR_TABLE: {
            std::vector<std::pair<var_interface *, var_interface *>> kvs;
            for (size_t i = 0; i < vv.get_table()->size(); ++i) {
                const auto k = vv.get_table()->key_at(i);
                const auto v = vv.get_table()->value_at(i);
                auto ki = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
                auto vi = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
                var_to_vi(s, k, ki);
                var_to_vi(s, v, vi);
                kvs.emplace_back(ki, vi);
            }
            dst->vi_set_table(kvs);
            break;
        }
    }
}

var_interface *fakelua_to_native_obj(const fakelua_state_ptr &s, cvar v) {
    const auto ret = std::dynamic_pointer_cast<state>(s)->get_var_interface_new_func()();
    var_to_vi(s, v, ret);
    return ret;
}

cvar fakelua_get_var_by_index(const fakelua_state_ptr &s, cvar ret, size_t i) {
    const auto vv = static_cast<var &>(ret);
    if (vv.type() == var_type::VAR_TABLE && vv.is_variadic()) {
        const var tmp(static_cast<int64_t>(i));
        return vv.get_table()->get(tmp);
    } else {
        if (i == 1) {
            return ret;
        } else {
            return const_null_var;
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

cvar make_variadic_table(const fakelua_state_ptr &s, int start, int n, const cvar *args) {
    var ret;
    ret.set_table(s);
    for (int i = 0; i < n - start; i++) {
        var key(static_cast<int64_t>(i + 1));
        const auto v = args[start + i];
        ret.get_table()->set(key, static_cast<const var &>(v), true);
    }
    ret.set_variadic(true);
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
    set_log_level(static_cast<log_level>(level));
}

}// namespace fakelua
