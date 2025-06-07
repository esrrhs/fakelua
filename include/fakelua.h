#pragma once

#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <string>

namespace fakelua {

// use to describe the complex type of lua.
// communication between fakelua and native.
// so it can transfer complex type like table.
struct var_interface {
    enum class type {
        MIN,
        NIL = MIN,
        BOOL,
        INT,
        FLOAT,
        STRING,
        TABLE,
        MAX = TABLE,
    };

    virtual ~var_interface() = default;

    [[nodiscard]] virtual type vi_get_type() const = 0;

    virtual void vi_set_nil() = 0;

    virtual void vi_set_bool(bool v) = 0;

    virtual void vi_set_int(int64_t v) = 0;

    virtual void vi_set_float(double v) = 0;

    virtual void vi_set_string(const std::string_view &v) = 0;

    virtual void vi_set_table(const std::vector<std::pair<var_interface *, var_interface *>> &kv) = 0;

    [[nodiscard]] virtual bool vi_get_bool() const = 0;

    [[nodiscard]] virtual int64_t vi_get_int() const = 0;

    [[nodiscard]] virtual double vi_get_float() const = 0;

    [[nodiscard]] virtual std::string_view vi_get_string() const = 0;

    [[nodiscard]] virtual size_t vi_get_table_size() const = 0;

    [[nodiscard]] virtual std::pair<var_interface *, var_interface *> vi_get_table_kv(int index) const = 0;

    [[nodiscard]] virtual std::string vi_to_string(int tab) const = 0;
};

// simple var implement, just for simple use.
struct simple_var_impl final : public var_interface {
    simple_var_impl() = default;

    ~simple_var_impl() override = default;

    [[nodiscard]] type vi_get_type() const override {
        return type_;
    }

    void vi_set_nil() override {
        type_ = type::NIL;
    }

    void vi_set_bool(bool v) override {
        type_ = type::BOOL;
        bool_ = v;
    }

    void vi_set_int(int64_t v) override {
        type_ = type::INT;
        int_ = v;
    }

    void vi_set_float(double v) override {
        type_ = type::FLOAT;
        float_ = v;
    }

    void vi_set_string(const std::string_view &v) override {
        type_ = type::STRING;
        string_ = v;
    }

    void vi_set_table(const std::vector<std::pair<var_interface *, var_interface *>> &kv) override {
        type_ = type::TABLE;
        table_ = kv;
    }

    [[nodiscard]] bool vi_get_bool() const override {
        return bool_;
    }

    [[nodiscard]] int64_t vi_get_int() const override {
        return int_;
    }

    [[nodiscard]] double vi_get_float() const override {
        return float_;
    }

    [[nodiscard]] std::string_view vi_get_string() const override {
        return string_;
    }

    [[nodiscard]] size_t vi_get_table_size() const override {
        return table_.size();
    }

    [[nodiscard]] std::pair<var_interface *, var_interface *> vi_get_table_kv(int index) const override {
        return table_[index];
    }

    [[nodiscard]] std::string vi_to_string(int tab) const override {
        std::string ret;
        switch (type_) {
            case type::NIL:
                ret = "nil";
                break;
            case type::BOOL:
                ret = bool_ ? "true" : "false";
                break;
            case type::INT:
                ret = std::to_string(int_);
                break;
            case type::FLOAT:
                ret = std::to_string(float_);
                break;
            case type::STRING:
                ret = std::format("\"{}\"", string_);
                break;
            case type::TABLE:
                ret = "table:";
                for (auto &kv: table_) {
                    ret += std::format("\n{}[{}] = {}", std::string(tab + 1, '\t'), kv.first->vi_to_string(tab + 1),
                                       kv.second->vi_to_string(tab + 1));
                }
                break;
        }

        return ret;
    }

    // sort table by key, just for debug
    void vi_sort_table() {
        std::sort(table_.begin(), table_.end(), [](const auto &a, const auto &b) {
            if (a.first->vi_get_type() != b.first->vi_get_type()) {
                return a.first->vi_get_type() < b.first->vi_get_type();
            }
            switch (a.first->vi_get_type()) {
                case type::NIL:
                    return false;
                case type::BOOL:
                    return a.first->vi_get_bool() < b.first->vi_get_bool();
                case type::INT:
                    return a.first->vi_get_int() < b.first->vi_get_int();
                case type::FLOAT:
                    return a.first->vi_get_float() < b.first->vi_get_float();
                case type::STRING:
                    return a.first->vi_get_string() < b.first->vi_get_string();
                case type::TABLE:
                    return a.first->vi_get_table_size() < b.first->vi_get_table_size();
                default:
                    return false;
            }
        });
        for (const auto &val: table_ | std::views::values) {
            if (val->vi_get_type() == type::TABLE) {
                dynamic_cast<simple_var_impl *>(val)->vi_sort_table();
            }
        }
    }

    type type_ = type::NIL;
    bool bool_ = false;
    int64_t int_ = 0;
    double float_ = 0;
    std::string_view string_;
    std::vector<std::pair<var_interface *, var_interface *>> table_;
};

class var;

// control the compiler behavior
struct compile_config {
    // skip jit compile. just lex and parse.
    bool skip_jit = false;
    // debug mode. if true, the jit code will be dumped to file.
    bool debug_mode = true;
};

// fake_lua state interface, every state can only run in one thread, just like Lua.
// every state has its own running environment. there could be many states in one process.
class fakelua_state : public std::enable_shared_from_this<fakelua_state> {
public:
    fakelua_state() = default;

    virtual ~fakelua_state() = default;

    // compile file, the file is a lua file.
    virtual void compile_file(const std::string &filename, compile_config cfg) = 0;

    // compile string, the string is the content of a file.
    virtual void compile_string(const std::string &str, compile_config cfg) = 0;

    // call function by name
    template<typename... Rets, typename... Args>
    void call(const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args);

    // set var_interface new instance function
    void set_var_interface_new_func(const std::function<var_interface *()> &func) {
        var_interface_new_func_ = func;
    }

    // get var_interface new instance function
    std::function<var_interface *()> &get_var_interface_new_func() {
        return var_interface_new_func_;
    }

    // set global debug log level, note all state will be set.
    // 0: off, 1: error, 2: info, default is error.
    void set_debug_log_level(int level);

private:
    std::function<var_interface *()> var_interface_new_func_;
    int reentrant_count_ = 0; // used to reset
};

using fakelua_state_ptr = std::shared_ptr<fakelua_state>;

namespace inter {

// native to fakelua
var *native_to_fakelua_nil(fakelua_state_ptr s);
var *native_to_fakelua_bool(fakelua_state_ptr s, bool v);
var *native_to_fakelua_char(fakelua_state_ptr s, char v);
var *native_to_fakelua_uchar(fakelua_state_ptr s, unsigned char v);
var *native_to_fakelua_short(fakelua_state_ptr s, short v);
var *native_to_fakelua_ushort(fakelua_state_ptr s, unsigned short v);
var *native_to_fakelua_int(fakelua_state_ptr s, int v);
var *native_to_fakelua_uint(fakelua_state_ptr s, unsigned int v);
var *native_to_fakelua_long(fakelua_state_ptr s, long v);
var *native_to_fakelua_ulong(fakelua_state_ptr s, unsigned long v);
var *native_to_fakelua_longlong(fakelua_state_ptr s, long long v);
var *native_to_fakelua_ulonglong(fakelua_state_ptr s, unsigned long long v);
var *native_to_fakelua_float(fakelua_state_ptr s, float v);
var *native_to_fakelua_double(fakelua_state_ptr s, double v);
var *native_to_fakelua_cstr(fakelua_state_ptr s, const char *v);
var *native_to_fakelua_str(fakelua_state_ptr s, char *v);
var *native_to_fakelua_string(fakelua_state_ptr s, const std::string &v);
var *native_to_fakelua_stringview(fakelua_state_ptr s, const std::string_view &v);
var *native_to_fakelua_obj(fakelua_state_ptr s, var_interface *v);

template<typename T>
var *native_to_fakelua(fakelua_state_ptr s, T v) {
    // check if T is nil
    if constexpr (std::is_same<T, std::nullptr_t>::value) {
        return native_to_fakelua_nil(s);
    }
    // check if T is bool
    else if constexpr (std::is_same<T, bool>::value) {
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
    }
    // check if T is var*
    else if constexpr (std::is_same<T, var *>::value) {
        return v;
    } else {
        // static_assert T should be var_interface* or implement var_interface
        static_assert(std::is_pointer<T>::value, "T should be pointer");
        static_assert(std::is_base_of<var_interface, std::remove_pointer_t<T>>::value, "T should be var_interface");
        return native_to_fakelua_obj(s, v);
    }
}

// fakelua to native
bool fakelua_to_native_bool(fakelua_state_ptr s, var *v);
char fakelua_to_native_char(fakelua_state_ptr s, var *v);
unsigned char fakelua_to_native_uchar(fakelua_state_ptr s, var *v);
short fakelua_to_native_short(fakelua_state_ptr s, var *v);
unsigned short fakelua_to_native_ushort(fakelua_state_ptr s, var *v);
int fakelua_to_native_int(fakelua_state_ptr s, var *v);
unsigned int fakelua_to_native_uint(fakelua_state_ptr s, var *v);
long fakelua_to_native_long(fakelua_state_ptr s, var *v);
unsigned long fakelua_to_native_ulong(fakelua_state_ptr s, var *v);
long long fakelua_to_native_longlong(fakelua_state_ptr s, var *v);
unsigned long long fakelua_to_native_ulonglong(fakelua_state_ptr s, var *v);
float fakelua_to_native_float(fakelua_state_ptr s, var *v);
double fakelua_to_native_double(fakelua_state_ptr s, var *v);
const char *fakelua_to_native_cstr(fakelua_state_ptr s, var *v);
const char *fakelua_to_native_str(fakelua_state_ptr s, var *v);
std::string fakelua_to_native_string(fakelua_state_ptr s, var *v);
std::string_view fakelua_to_native_stringview(fakelua_state_ptr s, var *v);
var_interface *fakelua_to_native_obj(fakelua_state_ptr s, var *v);

template<typename T>
T fakelua_to_native(fakelua_state_ptr s, var *v) {
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
        return (char *) fakelua_to_native_str(s, v);
    } else if constexpr (std::is_same<T, std::string>::value) {
        return fakelua_to_native_string(s, v);
    } else if constexpr (std::is_same<T, std::string_view>::value) {
        return fakelua_to_native_stringview(s, v);
    } else if constexpr (std::is_same<T, var *>::value) {
        return v;
    } else {
        // static_assert T should be var_interface* or implement var_interface
        static_assert(std::is_pointer<T>::value, "T should be pointer");
        static_assert(std::is_base_of<var_interface, std::remove_pointer_t<T>>::value, "T should be var_interface");
        return fakelua_to_native_obj(s, v);
    }
}

var *fakelua_get_var_by_index(fakelua_state_ptr s, var *ret, size_t i);

template<size_t I = 0, typename... Rets>
inline typename std::enable_if<I == sizeof...(Rets), void>::type fakelua_func_ret_helper(fakelua_state_ptr s, var *ret,
                                                                                         std::tuple<Rets &...> &rets) {
}

template<size_t I = 0, typename... Rets>
        inline typename std::enable_if <
        I<sizeof...(Rets), void>::type fakelua_func_ret_helper(fakelua_state_ptr s, var *ret, std::tuple<Rets &...> &rets) {
    typedef typename std::remove_reference<std::tuple_element_t<I, std::tuple<Rets &...>>>::type t;
    auto v = fakelua_get_var_by_index(s, ret, I + 1);
    std::get<I>(rets) = fakelua_to_native<t>(s, v);
    fakelua_func_ret_helper<I + 1, Rets...>(s, ret, rets);
}

template<typename Func, typename T, std::size_t N, std::size_t... Is>
var *call_variadic_helper(Func func, const T (&array)[N], std::index_sequence<Is...>) {
    return func(array[Is]...);
}

template<typename Func, typename T, std::size_t N>
var *call_variadic_helper(Func func, const T (&array)[N]) {
    return call_variadic_helper(func, array, std::make_index_sequence<N>{});
}

void *get_func_addr(const fakelua_state_ptr& s, const std::string &name, int &arg_count, bool &is_variadic);

var *make_variadic_table(fakelua_state_ptr s, int start, int n, var **args);

void reset(const fakelua_state_ptr& s);

[[noreturn]] void throw_inter_fakelua_exception(const std::string &msg);

class reentry_counter {
public:
    explicit reentry_counter(int &counter) : counter_(counter) {
        ++counter_;
    }

    ~reentry_counter() {
        --counter_;
    }

private:
    int &counter_;
};

}// namespace inter

// call funtion by name
template<typename... Rets, typename... Args>
void fakelua_state::call(const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args) {
    int arg_count = 0;
    bool is_variadic = false;
    const auto addr = inter::get_func_addr(shared_from_this(), name, arg_count, is_variadic);
    if (!addr) {
        inter::throw_inter_fakelua_exception(std::format("function {} not found", name));
    }

    if (!reentrant_count_) {
        inter::reset(shared_from_this());
    }
    inter::reentry_counter rc(reentrant_count_);

    // change every input args to var * by native_to_var() function
    // and change every output args to native type by var_to_native() function
    // the var * is the internal type of fakelua
    // the native type is the type of c++
    var *ret_var = nullptr;
    if (!is_variadic) {
        if (sizeof...(Args) != (size_t) arg_count) {
            inter::throw_inter_fakelua_exception(
                    std::format("function {} arg count not match, need {} get {}", name, arg_count, sizeof...(Args)));
        }
        ret_var = reinterpret_cast<var *(*) (...)>(addr)(inter::native_to_fakelua(shared_from_this(), std::forward<Args>(args))...);
    } else {
        if (sizeof...(Args) < (size_t) arg_count) {
            inter::throw_inter_fakelua_exception(
                    std::format("function {} arg count not match, need >= {} get {}", name, arg_count, sizeof...(Args)));
        }
        // save the variadic args to a table
        var *args_array[sizeof...(Args) + 1] = {nullptr, inter::native_to_fakelua(shared_from_this(), std::forward<Args>(args))...};
        args_array[0] = inter::make_variadic_table(shared_from_this(), arg_count + 1, sizeof...(Args) + 1, args_array);
        // call function by args array
        ret_var = inter::call_variadic_helper(reinterpret_cast<var *(*) (...)>(addr), args_array);
    }

    inter::fakelua_func_ret_helper(shared_from_this(), ret_var, rets);
}

// create fake_lua state
fakelua_state_ptr fakelua_newstate();

}// namespace fakelua
