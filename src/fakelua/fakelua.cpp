#include "fakelua.h"
#ifdef __linux__
#include "gperftools/profiler.h"
#endif
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

namespace inter {

var *native_to_fakelua_bool(const fakelua_state_ptr &s, bool v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_bool(v);
    return ret;
}

var *native_to_fakelua_char(const fakelua_state_ptr &s, char v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_uchar(const fakelua_state_ptr &s, unsigned char v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_short(const fakelua_state_ptr &s, short v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ushort(const fakelua_state_ptr &s, unsigned short v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_int(const fakelua_state_ptr &s, int v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_uint(const fakelua_state_ptr &s, unsigned int v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_long(const fakelua_state_ptr &s, long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ulong(const fakelua_state_ptr &s, unsigned long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_longlong(const fakelua_state_ptr &s, long long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_ulonglong(const fakelua_state_ptr &s, unsigned long long v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_int(v);
    return ret;
}

var *native_to_fakelua_float(const fakelua_state_ptr &s, float v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_float(v);
    return ret;
}

var *native_to_fakelua_double(const fakelua_state_ptr &s, double v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_float(v);
    return ret;
}

var *native_to_fakelua_cstr(const fakelua_state_ptr &s, const char *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_str(const fakelua_state_ptr &s, char *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_string(const fakelua_state_ptr &s, const std::string &v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_stringview(const fakelua_state_ptr &s, const std::string_view &v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    ret->set_string(s, v);
    return ret;
}

var *native_to_fakelua_obj(const fakelua_state_ptr &s, void *v) {
    auto ret = std::dynamic_pointer_cast<state>(s)->get_var_pool().alloc();
    // TODO
    return ret;
}

bool fakelua_to_native_bool(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_BOOL) {
        return v->get_bool();
    }
    throw std::runtime_error(std::format("fakelua_to_native_bool failed, type is {}", magic_enum::enum_name(v->type())));
}

char fakelua_to_native_char(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_char failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned char fakelua_to_native_uchar(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_uchar failed, type is {}", magic_enum::enum_name(v->type())));
}

short fakelua_to_native_short(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_short failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned short fakelua_to_native_ushort(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_ushort failed, type is {}", magic_enum::enum_name(v->type())));
}

int fakelua_to_native_int(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_int failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned int fakelua_to_native_uint(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_uint failed, type is {}", magic_enum::enum_name(v->type())));
}

long fakelua_to_native_long(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_long failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned long fakelua_to_native_ulong(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_ulong failed, type is {}", magic_enum::enum_name(v->type())));
}

long long fakelua_to_native_longlong(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_longlong failed, type is {}", magic_enum::enum_name(v->type())));
}

unsigned long long fakelua_to_native_ulonglong(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_INT) {
        return v->get_int();
    }
    throw std::runtime_error(std::format("fakelua_to_native_ulonglong failed, type is {}", magic_enum::enum_name(v->type())));
}

float fakelua_to_native_float(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_FLOAT) {
        return v->get_float();
    }
    throw std::runtime_error(std::format("fakelua_to_native_float failed, type is {}", magic_enum::enum_name(v->type())));
}

double fakelua_to_native_double(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_FLOAT) {
        return v->get_float();
    }
    throw std::runtime_error(std::format("fakelua_to_native_double failed, type is {}", magic_enum::enum_name(v->type())));
}

const char *fakelua_to_native_cstr(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string().data();
    }
    throw std::runtime_error(std::format("fakelua_to_native_cstr failed, type is {}", magic_enum::enum_name(v->type())));
}

const char *fakelua_to_native_str(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string().data();
    }
    throw std::runtime_error(std::format("fakelua_to_native_str failed, type is {}", magic_enum::enum_name(v->type())));
}

std::string fakelua_to_native_string(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return std::string(v->get_string());
    }
    throw std::runtime_error(std::format("fakelua_to_native_string failed, type is {}", magic_enum::enum_name(v->type())));
}

std::string_view fakelua_to_native_stringview(const fakelua_state_ptr &s, var *v) {
    if (v->type() == var_type::VAR_STRING) {
        return v->get_string();
    }
    throw std::runtime_error(std::format("fakelua_to_native_stringview failed, type is {}", magic_enum::enum_name(v->type())));
}

var *fakelua_get_var_by_index(const fakelua_state_ptr &s, var *ret, size_t i) {
    if (ret->type() == var_type::VAR_TABLE) {
        var tmp;
        tmp.set_int(i);
        return ret->get_table().get(tmp);
    }
    throw std::runtime_error(std::format("fakelua_get_var_by_index failed, type is {}", magic_enum::enum_name(ret->type())));
}

}// namespace inter

fakelua_state_ptr fakelua_newstate() {
    LOG(INFO) << "fakelua_newstate";
    return std::make_shared<state>();
}

void open_profiler(const std::string &fname) {
#ifdef __linux__
    LOG(INFO) << "open_profiler";
    ProfilerStart(fname.c_str());
#endif
}

void stop_profiler() {
#ifdef __linux__
    LOG(INFO) << "stop_profiler";
    ProfilerStop();
#endif
}

}// namespace fakelua
