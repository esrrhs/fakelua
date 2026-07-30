#include "native/native_basic.h"
#include "native/native_object.h"
#include "native/native_table.h"
#include "compile/c_runtime_header.h"
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
    CVar last_key; // nil 表示刚开始
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

    constexpr int kMaxKeys = 256;
    CVar keys[kMaxKeys];
    int key_count = 0;

    // 收集 spec_keys (typed fields)
    if (t->spec_keys && t->spec_count > 0) {
        for (uint32_t i = 0; i < t->spec_count && key_count < kMaxKeys; ++i) {
            if (t->spec_keys[i].type_ == static_cast<int>(VarType::Nil)) continue;
            keys[key_count++] = t->spec_keys[i];
        }
    }
    // 收集 quick_data
    for (const auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Nil)) continue;
        if (key_count >= kMaxKeys) break;
        keys[key_count++] = qd.key;
    }
    // 收集 nodes
    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_ && key_count < kMaxKeys; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (entry.key.type_ == static_cast<int>(VarType::Nil)) continue;
            keys[key_count++] = entry.key;
        }
    }

    if (key_count == 0) return inter::NativeToFakeluaNil(state);

    int start = 0;
    if (last.type_ != static_cast<int>(VarType::Nil)) {
        bool found = false;
        for (int i = 0; i < key_count; ++i) {
            if (keys[i].type_ == last.type_ &&
                ((keys[i].type_ == static_cast<int>(VarType::Int) && keys[i].data_.i == last.data_.i) ||
                 (keys[i].type_ == static_cast<int>(VarType::StringId) && keys[i].data_.i == last.data_.i))) {
                start = i + 1;
                found = true;
                break;
            }
        }
        if (!found || start >= key_count) return inter::NativeToFakeluaNil(state);
    }

    CVar k = keys[start];
    CVar v{static_cast<int>(VarType::Nil)};

    if (k.type_ == static_cast<int>(VarType::Int)) {
        v = TableHelper::GetTableInt(state, tbl, k.data_.i);
    } else if (k.type_ == static_cast<int>(VarType::StringId)) {
        int64_t id = k.data_.i;
        // 先在 spec_keys 中查找
        if (t->spec_keys && t->spec_vals && t->spec_count > 0) {
            for (uint32_t i = 0; i < t->spec_count; ++i) {
                if (t->spec_keys[i].type_ == static_cast<int>(VarType::StringId) && t->spec_keys[i].data_.i == id) {
                    v = t->spec_vals[i];
                    break;
                }
            }
        }
        // 再在 quick_data 中查找
        if (v.type_ == static_cast<int>(VarType::Nil)) {
            for (const auto &qd: t->quick_data_) {
                if (qd.key.type_ == static_cast<int>(VarType::StringId) && qd.key.data_.i == id) {
                    v = qd.val;
                    break;
                }
            }
        }
        // 最后在 nodes 中查找
        if (v.type_ == static_cast<int>(VarType::Nil) && t->nodes_ && t->bucket_count_ > 0) {
            for (uint32_t i = 0; i < t->count_; ++i) {
                uint32_t node_idx = t->active_list_[i];
                const auto &entry = t->nodes_[node_idx].entry;
                if (entry.key.type_ == static_cast<int>(VarType::StringId) && entry.key.data_.i == id) {
                    v = entry.val;
                    break;
                }
            }
        }
    }

    st->last_key = k;

    CVar multi = inter::AllocMultiCVar(state, 2);
    inter::SetMultiCVarElement(multi, 0, k);
    inter::SetMultiCVarElement(multi, 1, v);
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
        if (str.empty()) return inter::NativeToFakeluaNil(state);

        int base = 10;
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ == static_cast<int>(VarType::Int)) {
                base = static_cast<int>(a1.data_.i);
            } else if (a1.type_ == static_cast<int>(VarType::Float)) {
                base = static_cast<int>(a1.data_.f);
            }
        }

        if (base == 10) {
            // 尝试整数解析
            int64_t ival = 0;
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), ival);
            if (ec == std::errc{} && ptr == str.data() + str.size()) {
                return inter::NativeToFakeluaInt(state, ival);
            }
            // 尝试浮点解析
            try {
                size_t pos = 0;
                double dval = std::stod(str, &pos);
                if (pos == str.size()) {
                    return inter::NativeToFakeluaFloat(state, static_cast<float>(dval));
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
            for (; i < str.size(); ++i) {
                char c = str[i];
                int digit = -1;
                if (c >= '0' && c <= '9') digit = c - '0';
                else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
                else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
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
        if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
            std::string_view sv = KeyToStringView(a0);
            if (sv == "#") {
                return inter::NativeToFakeluaInt(state, n - 1);
            }
        }
        int64_t idx = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i
                     : (a0.type_ == static_cast<int>(VarType::Float)) ? static_cast<int64_t>(a0.data_.f)
                     : 1;
        if (idx < 1) idx = 1;
        int start = static_cast<int>(idx);
        int count = n - start;
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

        constexpr int kMaxArgs = 16;
        CVar call_args[kMaxArgs];
        int actual_arg_count = 0;
        for (int i = 0; i < call_arg_count && i < kMaxArgs; ++i) {
            call_args[i] = inter::GetNativeArg(state, args, n, i + 1);
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

        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, success));
        if (success) {
            inter::SetMultiCVarElement(multi, 1, result);
        } else {
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, err_msg));
        }
        return multi;
    });

    // ─── xpcall(f, err [, arg1, ...]) ───
    RegisterNativeFunction(s, "xpcall", 2, true, [](State *state, CVar *args, int n) -> CVar {
        CVar func = inter::GetNativeArg(state, args, n, 0);
        CVar err_func = inter::GetNativeArg(state, args, n, 1);

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
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
            inter::SetMultiCVarElement(multi, 1, result);
            return multi;
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
    RegisterNativeFunction(s, "next", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            return inter::NativeToFakeluaNil(state);
        }
        VarTable *t = tbl.data_.t;

        struct KvPair {
            CVar key;
            CVar val;
        };
        constexpr int kMaxPairs = 256;
        KvPair pairs[kMaxPairs];
        int pair_count = 0;

        // 收集 spec_keys/spec_vals (typed fields)
        if (t->spec_keys && t->spec_vals && t->spec_count > 0) {
            for (uint32_t i = 0; i < t->spec_count && pair_count < kMaxPairs; ++i) {
                if (t->spec_keys[i].type_ == static_cast<int>(VarType::Nil)) continue;
                pairs[pair_count].key = t->spec_keys[i];
                pairs[pair_count].val = t->spec_vals[i];
                pair_count++;
            }
        }
        // 收集 quick_data
        for (const auto &qd: t->quick_data_) {
            if (qd.key.type_ == static_cast<int>(VarType::Nil)) continue;
            if (pair_count >= kMaxPairs) break;
            pairs[pair_count].key = qd.key;
            pairs[pair_count].val = qd.val;
            pair_count++;
        }
        // 收集 nodes (hash table)
        if (t->nodes_ && t->bucket_count_ > 0) {
            for (uint32_t i = 0; i < t->count_ && pair_count < kMaxPairs; ++i) {
                uint32_t node_idx = t->active_list_[i];
                const auto &entry = t->nodes_[node_idx].entry;
                if (entry.key.type_ == static_cast<int>(VarType::Nil)) continue;
                pairs[pair_count].key = entry.key;
                pairs[pair_count].val = entry.val;
                pair_count++;
            }
        }

        if (pair_count == 0) {
            return inter::NativeToFakeluaNil(state);
        }

        bool has_index = (n >= 2);
        CVar index = has_index ? inter::GetNativeArg(state, args, n, 1) : CVar{static_cast<int>(VarType::Nil)};

        if (!has_index || index.type_ == static_cast<int>(VarType::Nil)) {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, pairs[0].key);
            inter::SetMultiCVarElement(multi, 1, pairs[0].val);
            return multi;
        }

        int found_pos = -1;
        for (int i = 0; i < pair_count; ++i) {
            if (pairs[i].key.type_ == index.type_) {
                if (index.type_ == static_cast<int>(VarType::Int) && pairs[i].key.data_.i == index.data_.i) {
                    found_pos = i;
                    break;
                } else if (index.type_ == static_cast<int>(VarType::Float) && pairs[i].key.data_.f == index.data_.f) {
                    found_pos = i;
                    break;
                } else if (index.type_ == static_cast<int>(VarType::StringId) && pairs[i].key.data_.i == index.data_.i) {
                    found_pos = i;
                    break;
                }
            }
        }

        if (found_pos >= 0 && found_pos + 1 < pair_count) {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, pairs[found_pos + 1].key);
            inter::SetMultiCVarElement(multi, 1, pairs[found_pos + 1].val);
            return multi;
        }

        return inter::NativeToFakeluaNil(state);
    });

    // ─── pairs(t) ───
    RegisterNativeFunction(s, "pairs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
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

}

}// namespace fakelua
