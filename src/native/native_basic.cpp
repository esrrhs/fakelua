#include "native/native_basic.h"
#include "compile/c_runtime_header.h"
#include "native/native_object.h"
#include "native/native_string.h"
#include "native/native_table.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include <charconv>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace fakelua {

// ─── pairs 迭代器状态 ───
struct PairIterState {
    CVar table;
    CVar last_key;// nil 表示刚开始
};

// ─── ipairs 迭代器状态 ───
struct IpairsState {
    CVar table;
    int64_t next_idx;
};

// ─── pairs 迭代器原生函数 ───
// 闭包签名：CVar (*)(VarClosure *cl, CVar s, CVar var)
// upvalues[0] = State* (as int)
// upvalues[1] = PairIterState* (as int)
// 辅助：比较 key 是否相等
static bool keys_equal(CVar a, CVar b) {
    if (a.type_ == b.type_) {
        if (b.type_ == static_cast<int>(VarType::Int) || b.type_ == static_cast<int>(VarType::Bool)) return a.data_.i == b.data_.i;
        if (b.type_ == static_cast<int>(VarType::Float)) return a.data_.f == b.data_.f;
        if (b.type_ == static_cast<int>(VarType::StringId)) return a.data_.i == b.data_.i;
        if (b.type_ == static_cast<int>(VarType::String)) {
            if (!a.data_.s || !b.data_.s) return a.data_.s == b.data_.s;
            return a.data_.s->Str() == b.data_.s->Str();
        }
        return a.data_.i == b.data_.i;
    }
    // 跨 Int 和 Float 比较
    if ((a.type_ == static_cast<int>(VarType::Int) || a.type_ == static_cast<int>(VarType::Float)) && (b.type_ == static_cast<int>(VarType::Int) || b.type_ == static_cast<int>(VarType::Float))) {
        double va = (a.type_ == static_cast<int>(VarType::Int)) ? static_cast<double>(a.data_.i) : a.data_.f;
        double vb = (b.type_ == static_cast<int>(VarType::Int)) ? static_cast<double>(b.data_.i) : b.data_.f;
        return va == vb;
    }
    // 跨 String 和 StringId 比较
    if ((a.type_ == static_cast<int>(VarType::String) || a.type_ == static_cast<int>(VarType::StringId)) &&
        (b.type_ == static_cast<int>(VarType::String) || b.type_ == static_cast<int>(VarType::StringId))) {
        return KeyToStringView(a) == KeyToStringView(b);
    }
    return false;
}

extern "C" CVar BasicPairsIterator(VarClosure *cl, CVar /*s*/, CVar /*var*/) {
    if (!cl || cl->upvalue_count < 2) {
        return CVar{static_cast<int>(VarType::Nil)};
    }
    State *state = reinterpret_cast<State *>(cl->upvalues[0]->data_.i);
    auto *st = reinterpret_cast<PairIterState *>(cl->upvalues[1]->data_.i);
    if (!state || !st) {
        return CVar{static_cast<int>(VarType::Nil)};
    }

    CVar tbl = st->table;
    CVar last = st->last_key;

    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
        return inter::NativeToFakeluaNil(state);
    }
    VarTable *t = tbl.data_.t;

    // 遍历查找 last_key 的下一个
    bool found_last = (last.type_ == static_cast<int>(VarType::Nil));// nil 表示刚开始
    CVar next_key{static_cast<int>(VarType::Nil)};
    CVar next_val{static_cast<int>(VarType::Nil)};
    bool has_next = false;

    // 遍历 spec_keys
    if (t->spec_keys && t->spec_vals && t->spec_count > 0) {
        for (uint32_t i = 0; i < t->spec_count && !has_next; ++i) {
            if (t->spec_keys[i].type_ == static_cast<int>(VarType::Nil)) continue;
            if (found_last) {
                next_key = t->spec_keys[i];
                next_val = t->spec_vals[i];
                has_next = true;
            } else if (keys_equal(t->spec_keys[i], last)) {
                found_last = true;
            }
        }
    }
    // 遍历 quick_data
    for (const auto &qd: t->quick_data_) {
        if (has_next) break;
        if (qd.key.type_ == static_cast<int>(VarType::Nil)) continue;
        if (found_last) {
            next_key = qd.key;
            next_val = qd.val;
            has_next = true;
        } else if (keys_equal(qd.key, last)) {
            found_last = true;
        }
    }
    // 遍历 nodes
    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_ && !has_next; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (entry.key.type_ == static_cast<int>(VarType::Nil)) continue;
            if (found_last) {
                next_key = entry.key;
                next_val = entry.val;
                has_next = true;
            } else if (keys_equal(entry.key, last)) {
                found_last = true;
            }
        }
    }

    if (!has_next) return inter::NativeToFakeluaNil(state);

    st->last_key = next_key;

    CVar multi = inter::AllocMultiCVar(state, 2);
    inter::SetMultiCVarElement(multi, 0, next_key);
    inter::SetMultiCVarElement(multi, 1, next_val);
    return multi;
}

// ─── ipairs 迭代器原生函数 ───
extern "C" CVar BasicIpairsIterator(VarClosure *cl, CVar /*s*/, CVar /*var*/) {
    if (!cl || cl->upvalue_count < 2) {
        return CVar{static_cast<int>(VarType::Nil)};
    }
    State *state = reinterpret_cast<State *>(cl->upvalues[0]->data_.i);
    auto *st = reinterpret_cast<IpairsState *>(cl->upvalues[1]->data_.i);
    if (!state || !st) {
        return CVar{static_cast<int>(VarType::Nil)};
    }

    CVar tbl = st->table;
    int64_t idx = st->next_idx;

    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
        return inter::NativeToFakeluaNil(state);
    }

    CVar val = TableHelper::GetTableInt(state, tbl, idx);
    if (val.type_ == static_cast<int>(VarType::Nil)) {
        return inter::NativeToFakeluaNil(state);
    }

    st->next_idx = idx + 1;

    CVar multi = inter::AllocMultiCVar(state, 2);
    inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaInt(state, idx));
    inter::SetMultiCVarElement(multi, 1, val);
    return multi;
}

void RegisterBasicLibraryApi(State *s) {
    if (!s) return;

    // ─── print(...) ───
    RegisterNativeFunction(s, "print", 0, true, [](State *state, CVar *args, int n) -> CVar {
        for (int i = 0; i < n; ++i) {
            if (i > 0) std::printf("\t");
            CVar arg = inter::GetNativeArg(state, args, n, i);
            std::string str = AsVar(arg).ToString();
            std::printf("%s", str.c_str());
        }
        std::printf("\n");
        std::fflush(stdout);
        return inter::NativeToFakeluaNil(state);
    });

    // ─── type(v) ───
    RegisterNativeFunction(s, "type", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        switch (static_cast<VarType>(a0.type_)) {
            case VarType::Nil:
                return inter::NativeToFakeluaStringView(state, "nil");
            case VarType::Bool:
                return inter::NativeToFakeluaStringView(state, "boolean");
            case VarType::Int:
            case VarType::Float:
                return inter::NativeToFakeluaStringView(state, "number");
            case VarType::String:
            case VarType::StringId:
                return inter::NativeToFakeluaStringView(state, "string");
            case VarType::Table:
                return inter::NativeToFakeluaStringView(state, "table");
            case VarType::Closure:
                return inter::NativeToFakeluaStringView(state, "function");
            default:
                return inter::NativeToFakeluaStringView(state, "userdata");
        }
    });

    // ─── tostring(v) ───
    RegisterNativeFunction(s, "tostring", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        const auto &v = AsVar(a0);
        if (v.Type() == VarType::String || v.Type() == VarType::StringId) {
            return a0;
        }
        std::string str = v.ToString(/*has_quote=*/false, /*has_postfix=*/false);
        return inter::NativeToFakeluaString(state, str);
    });

    // ─── tonumber(e [, base]) ───
    RegisterNativeFunction(s, "tonumber", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        const auto &v = AsVar(a0);

        if (v.Type() == VarType::Int) return a0;
        if (v.Type() == VarType::Float) return a0;
        if (v.Type() != VarType::String && v.Type() != VarType::StringId) {
            return inter::NativeToFakeluaNil(state);
        }

        std::string str(v.GetString()->Str());
        // Trim leading and trailing whitespace per Lua spec
        size_t start = str.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) return inter::NativeToFakeluaNil(state);
        size_t end = str.find_last_not_of(" \t\n\r\f\v");
        str = str.substr(start, end - start + 1);

        int base = 10;
        bool has_custom_base = false;
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ != static_cast<int>(VarType::Nil)) {
                if (a1.type_ == static_cast<int>(VarType::Bool) || a1.type_ == static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("bad argument #2 to 'tonumber' (number expected)");
                }
                if (a1.type_ == static_cast<int>(VarType::Float)) {
                    if (static_cast<double>(static_cast<int64_t>(a1.data_.f)) != a1.data_.f) {
                        return inter::NativeToFakeluaNil(state);
                    }
                }
                base = static_cast<int>(inter::CVarToInteger(a1, 10));
                has_custom_base = true;
            }
        }

        if (has_custom_base && (base < 2 || base > 36)) {
            return inter::NativeToFakeluaNil(state);
        }

        // Auto-detect 0x/0X prefix when no custom base is provided
        if (!has_custom_base && (str.rfind("0x", 0) == 0 || str.rfind("0X", 0) == 0 || str.rfind("-0x", 0) == 0 || str.rfind("-0X", 0) == 0 || str.rfind("+0x", 0) == 0 || str.rfind("+0X", 0) == 0)) {
            base = 16;
        }

        if (base == 10) {
            // 尝试整数解析 (先去除领先正号)
            std::string_view s_view = str;
            if (!s_view.empty() && s_view[0] == '+') {
                s_view.remove_prefix(1);
            }
            int64_t ival = 0;
            auto [ptr, ec] = std::from_chars(s_view.data(), s_view.data() + s_view.size(), ival);
            if (ec == std::errc{} && ptr == s_view.data() + s_view.size()) {
                return inter::NativeToFakeluaInt(state, ival);
            }
            // 尝试浮点解析
            try {
                size_t pos = 0;
                double dval = std::stod(str, &pos);
                if (pos == str.size()) {
                    return inter::NativeToFakeluaDouble(state, dval);
                }
            } catch (...) {
            }
            return inter::NativeToFakeluaNil(state);
        } else {
            if (base < 2 || base > 36) return inter::NativeToFakeluaNil(state);
            int64_t result = 0;
            bool negative = false;
            size_t i = 0;
            if (str[0] == '-') {
                negative = true;
                i = 1;
            } else if (str[0] == '+') {
                i = 1;
            }
            // Skip 0x/0X prefix for base 16
            if (base == 16 && i + 2 <= str.size() && str[i] == '0' && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
                i += 2;
            }
            if (i >= str.size()) return inter::NativeToFakeluaNil(state);
            for (; i < str.size(); ++i) {
                char c = str[i];
                int digit = -1;
                if (c >= '0' && c <= '9') digit = c - '0';
                else if (c >= 'a' && c <= 'z')
                    digit = c - 'a' + 10;
                else if (c >= 'A' && c <= 'Z')
                    digit = c - 'A' + 10;
                if (digit < 0 || digit >= base) return inter::NativeToFakeluaNil(state);
                result = result * base + digit;
            }
            if (negative) result = -result;
            return inter::NativeToFakeluaInt(state, result);
        }
    });

    // ─── select(n, ...) ───
    RegisterNativeFunction(s, "select", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        // 支持 select("#", ...) 返回总数
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);
        if (sv == "#") {
            return inter::NativeToFakeluaInt(state, n - 1);
        }
        int64_t idx = inter::CVarToInteger(a0, 1);
        int var_count = n - 1;
        if (idx < 0) {
            idx = var_count + idx + 1;
        }
        if (idx < 1 || idx > var_count) {
            return inter::NativeToFakeluaNil(state);
        }
        int start = static_cast<int>(idx);
        int count = var_count - start + 1;
        if (count <= 0) return inter::NativeToFakeluaNil(state);
        if (count == 1) {
            return inter::GetNativeArg(state, args, n, start);
        }
        CVar multi = inter::AllocMultiCVar(state, count);
        for (int i = 0; i < count; ++i) {
            inter::SetMultiCVarElement(multi, i, inter::GetNativeArg(state, args, n, start + i));
        }
        return multi;
    });

    // ─── error(message [, level]) ───
    RegisterNativeFunction(s, "error", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        const auto &v = AsVar(a0);
        std::string msg;
        if (v.Type() == VarType::String || v.Type() == VarType::StringId) {
            msg = std::string(v.GetString()->Str());
        } else {
            msg = v.ToString(/*has_quote=*/false, /*has_postfix=*/false);
        }
        ThrowFakeluaException(msg);
    });

    // ─── assert(v [, message]) ───
    RegisterNativeFunction(s, "assert", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        const auto &v = AsVar(a0);
        bool is_true = (v.Type() != VarType::Nil) && !(v.Type() == VarType::Bool && !v.GetBool());
        if (is_true) {
            if (n == 1) return a0;
            CVar multi = inter::AllocMultiCVar(state, n);
            for (int i = 0; i < n; ++i) {
                inter::SetMultiCVarElement(multi, i, inter::GetNativeArg(state, args, n, i));
            }
            return multi;
        }
        std::string msg = "assertion failed!";
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            const auto &m = AsVar(a1);
            if (m.Type() == VarType::String || m.Type() == VarType::StringId) {
                msg = std::string(m.GetString()->Str());
            } else {
                msg = m.ToString(/*has_quote=*/false, /*has_postfix=*/false);
            }
        }
        ThrowFakeluaException(msg);
    });

    // ─── pcall(f [, arg1, ...]) ───
    RegisterNativeFunction(s, "pcall", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar func = inter::GetNativeArg(state, args, n, 0);
        if (func.type_ != static_cast<int>(VarType::Closure) || !func.data_.cl) {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, false));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaStringView(state, "attempt to call a non-function"));
            return multi;
        }

        VarClosure *cl = func.data_.cl;
        int call_arg_count = n - 1;
        if (call_arg_count < 0) call_arg_count = 0;

        std::vector<CVar> call_args(call_arg_count);
        for (int i = 0; i < call_arg_count; ++i) {
            call_args[i] = inter::GetNativeArg(state, args, n, i + 1);
        }

        int expected = cl->expected_arg_count;
        if (cl->is_vararg) {
            int fixed = std::max(0, expected - 1);
            while (static_cast<int>(call_args.size()) < fixed) {
                call_args.push_back(inter::NativeToFakeluaNil(state));
            }
        }

        CVar result{static_cast<int>(VarType::Nil)};
        bool success = false;
        std::string err_msg;

        try {
            void *addr = cl->func_ptr;
            if (addr != nullptr) {
                result = inter::DispatchCall(addr, call_args.data(), static_cast<int>(call_args.size()));
            } else if (cl->code_str) {
                result = FlEvalLoadClosure(state, cl, static_cast<int>(call_args.size()), call_args.data());
            } else {
                ThrowFakeluaException("pcall: closure has no code");
            }
            success = true;
        } catch (const FakeluaException &e) {
            err_msg = e.what();
        } catch (const std::exception &e) {
            err_msg = e.what();
        } catch (...) {
            err_msg = "unknown error";
        }

        if (success) {
            if (result.type_ == static_cast<int>(VarType::Multi) && result.data_.m) {
                int count = result.data_.m->GetCount();
                CVar multi = inter::AllocMultiCVar(state, 1 + count);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
                for (int i = 0; i < count; ++i) {
                    inter::SetMultiCVarElement(multi, i + 1, inter::GetMultiCVarElement(result, i));
                }
                return multi;
            } else {
                CVar multi = inter::AllocMultiCVar(state, 2);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
                inter::SetMultiCVarElement(multi, 1, result);
                return multi;
            }
        } else {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, false));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, err_msg));
            return multi;
        }
    });

    // ─── xpcall(f, err [, arg1, ...]) ───
    RegisterNativeFunction(s, "xpcall", 2, true, [](State *state, CVar *args, int n) -> CVar {
        CVar func = inter::GetNativeArg(state, args, n, 0);
        CVar err_func = inter::GetNativeArg(state, args, n, 1);
        if (err_func.type_ != static_cast<int>(VarType::Closure) || !err_func.data_.cl) {
            ThrowFakeluaException("bad argument #2 to 'xpcall' (function expected)");
        }

        if (func.type_ != static_cast<int>(VarType::Closure) || !func.data_.cl) {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, false));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaStringView(state, "attempt to call a non-function"));
            return multi;
        }

        VarClosure *cl = func.data_.cl;
        int call_arg_count = n - 2;
        if (call_arg_count < 0) call_arg_count = 0;

        constexpr int kMaxArgs = 16;
        CVar call_args[kMaxArgs];
        int actual_arg_count = 0;
        for (int i = 0; i < call_arg_count && i < kMaxArgs; ++i) {
            call_args[i] = inter::GetNativeArg(state, args, n, i + 2);
            actual_arg_count++;
        }

        int expected = cl->expected_arg_count;
        if (cl->is_vararg) {
            int fixed = std::max(0, expected - 1);
            for (int i = actual_arg_count; i < fixed && i < kMaxArgs; ++i) {
                call_args[i] = inter::NativeToFakeluaNil(state);
                actual_arg_count++;
            }
        }

        CVar result{static_cast<int>(VarType::Nil)};
        bool success = false;
        std::string err_msg;

        try {
            void *addr = cl->func_ptr;
            if (addr != nullptr) {
                result = inter::DispatchCall(addr, call_args, actual_arg_count);
            } else if (cl->code_str) {
                result = FlEvalLoadClosure(state, cl, actual_arg_count, call_args);
            } else {
                ThrowFakeluaException("xpcall: closure has no code");
            }
            success = true;
        } catch (const FakeluaException &e) {
            err_msg = e.what();
        } catch (const std::exception &e) {
            err_msg = e.what();
        } catch (...) {
            err_msg = "unknown error";
        }

        if (success) {
            if (result.type_ == static_cast<int>(VarType::Multi) && result.data_.m) {
                int count = result.data_.m->GetCount();
                CVar multi = inter::AllocMultiCVar(state, 1 + count);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
                for (int i = 0; i < count; ++i) {
                    inter::SetMultiCVarElement(multi, i + 1, inter::GetMultiCVarElement(result, i));
                }
                return multi;
            } else {
                CVar multi = inter::AllocMultiCVar(state, 2);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
                inter::SetMultiCVarElement(multi, 1, result);
                return multi;
            }
        }

        // 调用错误处理函数
        if (err_func.type_ == static_cast<int>(VarType::Closure) && err_func.data_.cl) {
            VarClosure *err_cl = err_func.data_.cl;
            CVar err_arg = inter::NativeToFakeluaString(state, err_msg);
            try {
                void *addr = err_cl->func_ptr;
                CVar err_result{static_cast<int>(VarType::Nil)};
                if (addr != nullptr) {
                    err_result = inter::DispatchCall(addr, &err_arg, 1);
                } else if (err_cl->code_str) {
                    err_result = FlEvalLoadClosure(state, err_cl, 1, &err_arg);
                } else {
                    err_result = inter::NativeToFakeluaString(state, err_msg);
                }
                CVar multi = inter::AllocMultiCVar(state, 2);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, false));
                inter::SetMultiCVarElement(multi, 1, err_result);
                return multi;
            } catch (...) {
                CVar multi = inter::AllocMultiCVar(state, 2);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, false));
                inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, err_msg));
                return multi;
            }
        }

        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, false));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, err_msg));
        return multi;
    });

    // 注意：fakelua 没有元表，所以 rawequal/rawget/rawset/rawlen 不需要实现

    // ─── next(table [, index]) ───
    // 辅助：在表中查找 key 是否匹配
    auto key_matches = [](CVar key, CVar target) -> bool { return keys_equal(key, target); };

    // 辅助：遍历回调函数
    using NextCallback = std::function<void(CVar key, CVar val)>;
    auto traverse_table = [](VarTable *t, NextCallback cb) {
        // 遍历 spec_keys/spec_vals
        if (t->spec_keys && t->spec_vals && t->spec_count > 0) {
            for (uint32_t i = 0; i < t->spec_count; ++i) {
                if (t->spec_keys[i].type_ != static_cast<int>(VarType::Nil)) {
                    cb(t->spec_keys[i], t->spec_vals[i]);
                }
            }
        }
        // 遍历 quick_data
        for (const auto &qd: t->quick_data_) {
            if (qd.key.type_ != static_cast<int>(VarType::Nil)) {
                cb(qd.key, qd.val);
            }
        }
        // 遍历 nodes
        if (t->nodes_ && t->bucket_count_ > 0) {
            for (uint32_t i = 0; i < t->count_; ++i) {
                uint32_t node_idx = t->active_list_[i];
                const auto &entry = t->nodes_[node_idx].entry;
                if (entry.key.type_ != static_cast<int>(VarType::Nil)) {
                    cb(entry.key, entry.val);
                }
            }
        }
    };

    RegisterNativeFunction(s, "next", 1, true, [&](State *state, CVar *args, int n) -> CVar {
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'next' (table expected)");
        }
        VarTable *t = tbl.data_.t;

        bool has_index = (n >= 2);
        CVar index = has_index ? inter::GetNativeArg(state, args, n, 1) : CVar{static_cast<int>(VarType::Nil)};

        // 空表或 nil index 时返回第一个 key
        if (!has_index || index.type_ == static_cast<int>(VarType::Nil)) {
            CVar first_key{static_cast<int>(VarType::Nil)};
            CVar first_val{static_cast<int>(VarType::Nil)};
            bool found = false;
            traverse_table(t, [&](CVar k, CVar v) {
                if (!found) {
                    first_key = k;
                    first_val = v;
                    found = true;
                }
            });
            if (!found) return inter::NativeToFakeluaNil(state);
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, first_key);
            inter::SetMultiCVarElement(multi, 1, first_val);
            return multi;
        }

        // 查找 index 的下一个
        bool found_index = false;
        CVar next_key{static_cast<int>(VarType::Nil)};
        CVar next_val{static_cast<int>(VarType::Nil)};
        bool has_next = false;
        traverse_table(t, [&](CVar k, CVar v) {
            if (has_next) return;// 已经找到下一个
            if (found_index) {
                next_key = k;
                next_val = v;
                has_next = true;
                return;
            }
            if (key_matches(k, index)) {
                found_index = true;
            }
        });

        if (!has_next) return inter::NativeToFakeluaNil(state);
        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, next_key);
        inter::SetMultiCVarElement(multi, 1, next_val);
        return multi;
    });

    // ─── pairs(t) ───
    RegisterNativeFunction(s, "pairs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'pairs' (table expected)");
        }
        auto &alloc = state->GetHeap().GetAllocator(false);

        // 分配迭代器状态
        auto *st = static_cast<PairIterState *>(alloc.Alloc(sizeof(PairIterState)));
        st->table = tbl;
        st->last_key = CVar{static_cast<int>(VarType::Nil)};

        // upvalue 0: State* (用于分配 Multi)
        CVar *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv0->type_ = static_cast<int>(VarType::Int);
        uv0->data_.i = reinterpret_cast<int64_t>(state);

        // upvalue 1: PairIterState*
        CVar *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv1->type_ = static_cast<int>(VarType::Int);
        uv1->data_.i = reinterpret_cast<int64_t>(st);

        VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 2 * sizeof(CVar *)));
        cl->func_ptr = reinterpret_cast<void *>(BasicPairsIterator);
        cl->upvalue_count = 2;
        cl->expected_arg_count = 2;
        cl->is_vararg = false;
        cl->code_str = nullptr;
        cl->upvalues[0] = uv0;
        cl->upvalues[1] = uv1;

        CVar iter_closure{};
        iter_closure.type_ = static_cast<int>(VarType::Closure);
        iter_closure.data_.cl = cl;

        // 返回 next, tbl, nil
        CVar multi = inter::AllocMultiCVar(state, 3);
        inter::SetMultiCVarElement(multi, 0, iter_closure);
        inter::SetMultiCVarElement(multi, 1, tbl);
        inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaNil(state));
        return multi;
    });

    // ─── ipairs(t) ───
    RegisterNativeFunction(s, "ipairs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'ipairs' (table expected)");
        }
        auto &alloc = state->GetHeap().GetAllocator(false);

        auto *st = static_cast<IpairsState *>(alloc.Alloc(sizeof(IpairsState)));
        st->table = tbl;
        st->next_idx = 1;

        CVar *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv0->type_ = static_cast<int>(VarType::Int);
        uv0->data_.i = reinterpret_cast<int64_t>(state);

        CVar *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv1->type_ = static_cast<int>(VarType::Int);
        uv1->data_.i = reinterpret_cast<int64_t>(st);

        VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 2 * sizeof(CVar *)));
        cl->func_ptr = reinterpret_cast<void *>(BasicIpairsIterator);
        cl->upvalue_count = 2;
        cl->expected_arg_count = 2;
        cl->is_vararg = false;
        cl->code_str = nullptr;
        cl->upvalues[0] = uv0;
        cl->upvalues[1] = uv1;

        CVar iter_closure{};
        iter_closure.type_ = static_cast<int>(VarType::Closure);
        iter_closure.data_.cl = cl;

        CVar multi = inter::AllocMultiCVar(state, 3);
        inter::SetMultiCVarElement(multi, 0, iter_closure);
        inter::SetMultiCVarElement(multi, 1, tbl);
        inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, 0));
        return multi;
    });

    // ─── collectgarbage([opt [, arg]]) ───
    // fakelua 使用 Arena 分配器，无标准 GC。目前仅支持 "count"：
    //   返回当前临时 + 常量分配器总使用量（单位 KB，与 Lua 一致）。
    // 其他选项（"collect"/"step"/"stop"/"restart" 等）为 no-op，返回 0。
    RegisterNativeFunction(s, "collectgarbage", 0, true, [](State *state, CVar *args, int n) -> CVar {
        std::string_view opt = "count";
        std::string temp_opt;
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ == static_cast<int>(VarType::Bool) || a0.type_ == static_cast<int>(VarType::Table)) {
                ThrowFakeluaException("bad argument #1 to 'collectgarbage' (string expected)");
            }
            opt = GetStringArgView(a0, temp_opt);
            if (opt.empty()) opt = "count";
        }
        if (opt == "count") {
            // 返回内存使用量（KB）= (temp + const allocator bytes) / 1024
            const size_t total_bytes = state->GetHeap().GetAllocator(false /* temp */).Size() + state->GetHeap().GetAllocator(true /* const */).Size();
            double kb = static_cast<double>(total_bytes) / 1024.0;
            return inter::NativeToFakeluaDouble(state, kb);
        }
        // 其他选项：no-op，返回 0
        return inter::NativeToFakeluaInt(state, 0);
    });
}

}// namespace fakelua
