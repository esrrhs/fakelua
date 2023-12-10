#pragma once

#include <memory>
#include <string>

namespace fakelua {

class var;

// fake_lua state interface, every state can only run in one thread, just like Lua.
// every state has its own running environment. there could be many states in one process.
class fakelua_state : public std::enable_shared_from_this<fakelua_state> {
public:
    fakelua_state() {
    }

    virtual ~fakelua_state() {
    }

    // compile file, the file is a lua file.
    virtual void compile_file(const std::string &filename) = 0;

    // compile string, the string is the content of a file.
    virtual void compile_string(const std::string &str) = 0;

    template<typename... Rets, typename... Args>
    void call(const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args);

protected:
    virtual void *get_func_addr(const std::string &name) = 0;
};

using fakelua_state_ptr = std::shared_ptr<fakelua_state>;

namespace inter {

// native to fakelua
var *native_to_fakelua_bool(const fakelua_state_ptr &s, bool v);
var *native_to_fakelua_char(const fakelua_state_ptr &s, char v);
var *native_to_fakelua_uchar(const fakelua_state_ptr &s, unsigned char v);
var *native_to_fakelua_short(const fakelua_state_ptr &s, short v);
var *native_to_fakelua_ushort(const fakelua_state_ptr &s, unsigned short v);
var *native_to_fakelua_int(const fakelua_state_ptr &s, int v);
var *native_to_fakelua_uint(const fakelua_state_ptr &s, unsigned int v);
var *native_to_fakelua_long(const fakelua_state_ptr &s, long v);
var *native_to_fakelua_ulong(const fakelua_state_ptr &s, unsigned long v);
var *native_to_fakelua_longlong(const fakelua_state_ptr &s, long long v);
var *native_to_fakelua_ulonglong(const fakelua_state_ptr &s, unsigned long long v);
var *native_to_fakelua_float(const fakelua_state_ptr &s, float v);
var *native_to_fakelua_double(const fakelua_state_ptr &s, double v);
var *native_to_fakelua_cstr(const fakelua_state_ptr &s, const char *v);
var *native_to_fakelua_str(const fakelua_state_ptr &s, char *v);
var *native_to_fakelua_string(const fakelua_state_ptr &s, const std::string &v);
var *native_to_fakelua_stringview(const fakelua_state_ptr &s, const std::string_view &v);
var *native_to_fakelua_obj(const fakelua_state_ptr &s, void *v);

template<typename T>
var *native_to_fakelua(const fakelua_state_ptr &s, T v) {
    // check if T is bool
    if constexpr (std::is_same<T, bool>::value) {
        return native_to_fakelua_bool(s, v);
    }
    // check if T is char
    else if constexpr (std::is_same<T, char>::value) {
        return native_to_fakelua_char(s, v);
    }
    // check if T is unsigned char
    else if constexpr (std::is_same<T, unsigned char>::value) {
        return native_to_fakelua_uchar(s, v);
    }
    // check if T is short
    else if constexpr (std::is_same<T, short>::value) {
        return native_to_fakelua_short(s, v);
    }
    // check if T is unsigned short
    else if constexpr (std::is_same<T, unsigned short>::value) {
        return native_to_fakelua_ushort(s, v);
    }
    // check if T is int
    else if constexpr (std::is_same<T, int>::value) {
        return native_to_fakelua_int(s, v);
    }
    // check if T is unsigned int
    else if constexpr (std::is_same<T, unsigned int>::value) {
        return native_to_fakelua_uint(s, v);
    }
    // check if T is long
    else if constexpr (std::is_same<T, long>::value) {
        return native_to_fakelua_long(s, v);
    }
    // check if T is unsigned long
    else if constexpr (std::is_same<T, unsigned long>::value) {
        return native_to_fakelua_ulong(s, v);
    }
    // check if T is long long
    else if constexpr (std::is_same<T, long long>::value) {
        return native_to_fakelua_longlong(s, v);
    }
    // check if T is unsigned long long
    else if constexpr (std::is_same<T, unsigned long long>::value) {
        return native_to_fakelua_ulonglong(s, v);
    }
    // check if T is float
    else if constexpr (std::is_same<T, float>::value) {
        return native_to_fakelua_float(s, v);
    }
    // check if T is double
    else if constexpr (std::is_same<T, double>::value) {
        return native_to_fakelua_double(s, v);
    }
    // check if T is const char *
    else if constexpr (std::is_same<T, const char *>::value) {
        return native_to_fakelua_cstr(s, v);
    }
    // check if T is char *
    else if constexpr (std::is_same<T, char *>::value) {
        return native_to_fakelua_str(s, v);
    }
    // check if T is std::string
    else if constexpr (std::is_same<T, std::string>::value) {
        return native_to_fakelua_string(s, v);
    }
    // check if T is std::string_view
    else if constexpr (std::is_same<T, std::string_view>::value) {
        return native_to_fakelua_stringview(s, v);
    } else {
        static_assert(std::is_pointer<T>::value, "T should be pointer");
        return native_to_fakelua_obj(s, v);
    }
}

// fakelua to native
bool fakelua_to_native_bool(const fakelua_state_ptr &s, var *v);
char fakelua_to_native_char(const fakelua_state_ptr &s, var *v);
unsigned char fakelua_to_native_uchar(const fakelua_state_ptr &s, var *v);
short fakelua_to_native_short(const fakelua_state_ptr &s, var *v);
unsigned short fakelua_to_native_ushort(const fakelua_state_ptr &s, var *v);
int fakelua_to_native_int(const fakelua_state_ptr &s, var *v);
unsigned int fakelua_to_native_uint(const fakelua_state_ptr &s, var *v);
long fakelua_to_native_long(const fakelua_state_ptr &s, var *v);
unsigned long fakelua_to_native_ulong(const fakelua_state_ptr &s, var *v);
long long fakelua_to_native_longlong(const fakelua_state_ptr &s, var *v);
unsigned long long fakelua_to_native_ulonglong(const fakelua_state_ptr &s, var *v);
float fakelua_to_native_float(const fakelua_state_ptr &s, var *v);
double fakelua_to_native_double(const fakelua_state_ptr &s, var *v);
const char *fakelua_to_native_cstr(const fakelua_state_ptr &s, var *v);
const char *fakelua_to_native_str(const fakelua_state_ptr &s, var *v);
std::string fakelua_to_native_string(const fakelua_state_ptr &s, var *v);
std::string_view fakelua_to_native_stringview(const fakelua_state_ptr &s, var *v);

template<typename T>
T fakelua_to_native(const fakelua_state_ptr &s, var *v) {
    if constexpr (std::is_same<T, bool>::value) {
        return fakelua_to_native_bool(s, v);
    } else if constexpr (std::is_same<T, char>::value) {
        return fakelua_to_native_char(s, v);
    } else if constexpr (std::is_same<T, unsigned char>::value) {
        return fakelua_to_native_uchar(s, v);
    } else if constexpr (std::is_same<T, short>::value) {
        return fakelua_to_native_short(s, v);
    } else if constexpr (std::is_same<T, unsigned short>::value) {
        return fakelua_to_native_ushort(s, v);
    } else if constexpr (std::is_same<T, int>::value) {
        return fakelua_to_native_int(s, v);
    } else if constexpr (std::is_same<T, unsigned int>::value) {
        return fakelua_to_native_uint(s, v);
    } else if constexpr (std::is_same<T, long>::value) {
        return fakelua_to_native_long(s, v);
    } else if constexpr (std::is_same<T, unsigned long>::value) {
        return fakelua_to_native_ulong(s, v);
    } else if constexpr (std::is_same<T, long long>::value) {
        return fakelua_to_native_longlong(s, v);
    } else if constexpr (std::is_same<T, unsigned long long>::value) {
        return fakelua_to_native_ulonglong(s, v);
    } else if constexpr (std::is_same<T, float>::value) {
        return fakelua_to_native_float(s, v);
    } else if constexpr (std::is_same<T, double>::value) {
        return fakelua_to_native_double(s, v);
    } else if constexpr (std::is_same<T, const char *>::value) {
        return fakelua_to_native_cstr(s, v);
    } else if constexpr (std::is_same<T, char *>::value) {
        return fakelua_to_native_str(s, v);
    } else if constexpr (std::is_same<T, std::string>::value) {
        return fakelua_to_native_string(s, v);
    } else if constexpr (std::is_same<T, std::string_view>::value) {
        return fakelua_to_native_stringview(s, v);
    } else {
        static_assert(std::is_pointer<T>::value, "T should be pointer");
        return fakelua_to_native_obj<T>(s, v);
    }
}

var *fakelua_get_var_by_index(const fakelua_state_ptr &s, var *ret, size_t i);

template<size_t I = 0, typename... Rets>
inline typename std::enable_if<I == sizeof...(Rets), void>::type fakelua_func_ret_helper(const fakelua_state_ptr &s, var *ret,
                                                                                         std::tuple<Rets &...> &rets) {
}

template<size_t I = 0, typename... Rets>
        inline typename std::enable_if <
        I<sizeof...(Rets), void>::type fakelua_func_ret_helper(const fakelua_state_ptr &s, var *ret, std::tuple<Rets &...> &rets) {
    typedef typename std::remove_reference<std::tuple_element_t<I, std::tuple<Rets &...>>>::type t;
    auto v = fakelua_get_var_by_index(s, ret, I);
    std::get<I>(rets) = fakelua_to_native<t>(s, v);
    fakelua_func_ret_helper<I + 1, Rets...>(s, ret, rets);
}

}// namespace inter

// call funtion by name
template<typename... Rets, typename... Args>
void fakelua_state::call(const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args) {
    auto addr = get_func_addr(name);
    if (!addr) {
        throw std::runtime_error("function not found " + name);
    }
    // change every input args to var * by native_to_var() function
    // and change every output args to native type by var_to_native() function
    // the var * is the internal type of fakelua
    // the native type is the type of c++
    auto ret_var = reinterpret_cast<var *(*) (const fakelua_state_ptr &, var *...)>(addr)(
            shared_from_this(), inter::native_to_fakelua(std::forward<Args>(args))...);
    inter::fakelua_func_ret_helper(shared_from_this(), ret_var, rets);
}

// create fake_lua state
fakelua_state_ptr fakelua_newstate();

// open global profiler by gperftools
void open_profiler(const std::string_view &fname);

// stop global profiler by gperftools
void stop_profiler();

}// namespace fakelua
