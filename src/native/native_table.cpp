#include "native/native_table.h"
#include "native/native_common.h"
#include "compile/c_runtime_header.h"
#include "native/native_object.h"
#include "native/native_string.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include <algorithm>
#include <string>
#include <vector>

namespace fakelua {

bool TableHelper::VarKeyEqualInt(CVar k, int64_t idx) {
    if (k.type_ == static_cast<int>(VarType::Int)) {
        return k.data_.i == idx;
    }
    if (k.type_ == static_cast<int>(VarType::Float)) {
        return k.data_.f == static_cast<double>(idx);
    }
    return false;
}

int64_t TableHelper::GetTableLen(CVar tbl) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return 0;
    VarTable *t = tbl.data_.t;
    int64_t max_idx = static_cast<int64_t>(t->spec_count);
    for (const auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Int)) {
            if (qd.key.data_.i > max_idx) max_idx = qd.key.data_.i;
        }
    }
    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (entry.key.type_ == static_cast<int>(VarType::Int)) {
                if (entry.key.data_.i > max_idx) max_idx = entry.key.data_.i;
            }
        }
    }
    return max_idx;
}

CVar TableHelper::GetTableInt(State *s, CVar tbl, int64_t idx) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return CVar{static_cast<int>(VarType::Nil)};
    VarTable *t = tbl.data_.t;

    if (t->spec_get) {
        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
        auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
        CVar key_cvar{static_cast<int>(VarType::Int)};
        key_cvar.data_.i = idx;
        bool finish = false;
        CVar r = get_fn(t, key_cvar, &finish);
        if (finish) return r;
    }

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (VarKeyEqualInt(t->spec_keys[i], idx)) {
                return t->spec_vals[i];
            }
        }
    }

    for (const auto &qd: t->quick_data_) {
        if (VarKeyEqualInt(qd.key, idx)) {
            return qd.val;
        }
    }

    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (VarKeyEqualInt(entry.key, idx)) {
                return entry.val;
            }
        }
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

CVar TableHelper::GetTableStrId(State *s, CVar tbl, const char *str_key) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t || !str_key) return CVar{static_cast<int>(VarType::Nil)};
    VarTable *t = tbl.data_.t;
    std::string_view target_key(str_key);

    auto var_key_match_str = [](CVar k, std::string_view target) -> bool {
        if (k.type_ == static_cast<int>(VarType::StringId)) {
            if (!k.data_.i) return false;
            const char *ptr = reinterpret_cast<const char *>(k.data_.i);
            int sz = *reinterpret_cast<const int *>(ptr);
            return static_cast<size_t>(sz) == target.size() && std::memcmp(ptr + 8, target.data(), target.size()) == 0;
        }
        if (k.type_ == static_cast<int>(VarType::String)) {
            if (!k.data_.s) return false;
            return k.data_.s->Str() == target;
        }
        return false;
    };

    if (t->spec_get) {
        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
        auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
        CVar key_cvar = inter::NativeToFakeluaString(s, std::string(target_key));
        bool finish = false;
        CVar r = get_fn(t, key_cvar, &finish);
        if (finish && r.type_ != static_cast<int>(VarType::Nil)) return r;
    }

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (var_key_match_str(t->spec_keys[i], target_key)) {
                return t->spec_vals[i];
            }
        }
    }

    for (const auto &qd: t->quick_data_) {
        if (var_key_match_str(qd.key, target_key)) {
            return qd.val;
        }
    }

    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (var_key_match_str(entry.key, target_key)) {
                return entry.val;
            }
        }
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

void TableHelper::SetTableInt(State *s, CVar tbl, int64_t idx, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;

    if (t->spec_set) {
        using SpecSetFn = void (*)(VarTable *, CVar, CVar, bool *);
        auto set_fn = reinterpret_cast<SpecSetFn>(t->spec_set);
        CVar key_cvar{static_cast<int>(VarType::Int)};
        key_cvar.data_.i = idx;
        bool finish = false;
        set_fn(t, key_cvar, val, &finish);
        if (finish) return;
    }

    CVar key{static_cast<int>(VarType::Int)};
    key.data_.i = idx;
    uint32_t hash = static_cast<uint32_t>(idx ^ (idx >> 32));

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (VarKeyEqualInt(t->spec_keys[i], idx)) {
                t->spec_vals[i] = val;
                return;
            }
        }
    }

    for (auto &qd: t->quick_data_) {
        if (VarKeyEqualInt(qd.key, idx)) {
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            return;
        }
    }

    for (auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Nil)) {
            static_cast<CVar &>(qd.key) = key;
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            t->count_++;
            return;
        }
    }

    std::string key_str = std::to_string(idx);
    SetTableStrId(s, tbl, key_str.c_str(), val);
}

void TableHelper::SetTableStrId(State *s, CVar tbl, const char *str_key, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;
    int64_t id = s->GetConstString().Alloc(str_key);
    auto *vs = reinterpret_cast<const VarString *>(id);
    uint32_t hash = vs->Hash();

    CVar key{static_cast<int>(VarType::StringId)};
    key.data_.i = id;

    if (t->bucket_count_ > 0 && t->nodes_) {
        uint32_t mask = t->bucket_count_ - 1;
        uint32_t idx = hash & mask;
        if (t->nodes_[idx].entry.key.type_ == static_cast<int>(VarType::StringId) && t->nodes_[idx].entry.hash == hash) {
            static_cast<CVar &>(t->nodes_[idx].entry.val) = val;
            return;
        }
        if (t->nodes_[idx].entry.key.type_ == static_cast<int>(VarType::Nil)) {
            static_cast<CVar &>(t->nodes_[idx].entry.key) = key;
            static_cast<CVar &>(t->nodes_[idx].entry.val) = val;
            t->nodes_[idx].entry.hash = hash;
            t->nodes_[idx].next = VarTable::INVALID_INDEX;
            t->count_++;
            return;
        }
        uint32_t cur = idx;
        while (t->nodes_[cur].next != VarTable::INVALID_INDEX) {
            uint32_t nxt = t->nodes_[cur].next;
            if (t->nodes_[nxt].entry.key.type_ == static_cast<int>(VarType::StringId) && t->nodes_[nxt].entry.hash == hash) {
                static_cast<CVar &>(t->nodes_[nxt].entry.val) = val;
                return;
            }
            cur = nxt;
        }
        for (uint32_t probe = 1; probe < t->bucket_count_; ++probe) {
            uint32_t i = (idx + probe) & mask;
            if (t->nodes_[i].entry.key.type_ == static_cast<int>(VarType::Nil)) {
                static_cast<CVar &>(t->nodes_[i].entry.key) = key;
                static_cast<CVar &>(t->nodes_[i].entry.val) = val;
                t->nodes_[i].entry.hash = hash;
                t->nodes_[i].next = VarTable::INVALID_INDEX;
                t->nodes_[cur].next = i;
                t->count_++;
                return;
            }
        }
        return;
    }

    for (auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::StringId) && qd.key.data_.i == id) {
            static_cast<CVar &>(qd.val) = val;
            return;
        }
    }

    for (auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Nil)) {
            static_cast<CVar &>(qd.key) = key;
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            t->count_++;
            return;
        }
    }
}

// Use shared CheckNumberArg from native_common.h

// ─── Helper: create an empty table with arena allocator ───
static CVar CreateEmptyTable(State *state) {
    VarTable *vtbl = static_cast<VarTable *>(FakeluaAlloc(state, sizeof(VarTable), false));
    *vtbl = VarTable{};
    for (auto &qd: vtbl->quick_data_) {
        qd.key.type_ = static_cast<int>(VarType::Nil);
        qd.val.type_ = static_cast<int>(VarType::Nil);
    }
    vtbl->free_list_idx_ = VarTable::INVALID_INDEX;
    CVar tbl_cvar{};
    tbl_cvar.type_ = static_cast<int>(VarType::Table);
    tbl_cvar.data_.t = vtbl;
    return tbl_cvar;
}

void RegisterTableLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "table.insert", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.insert' (table expected)");
        }
        int64_t len = TableHelper::GetTableLen(tbl);

        if (n == 1) return inter::NativeToFakeluaNil(state);
        if (n == 2) {
            CVar val = inter::GetNativeArg(state, args, n, 1);
            TableHelper::SetTableInt(state, tbl, len + 1, val);
        } else {
            CVar pos_var = inter::GetNativeArg(state, args, n, 1);
            CVar val = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(pos_var, 2, "table.insert");
            int64_t pos = inter::CVarToInteger(pos_var, 1);
            if (pos < 1 || pos > len + 1) return inter::NativeToFakeluaNil(state);
            for (int64_t i = len; i >= pos; --i) {
                CVar item = TableHelper::GetTableInt(state, tbl, i);
                TableHelper::SetTableInt(state, tbl, i + 1, item);
            }
            TableHelper::SetTableInt(state, tbl, pos, val);
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "table.remove", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.remove' (table expected)");
        }
        int64_t len = TableHelper::GetTableLen(tbl);
        int64_t pos = len;
        if (n >= 2) {
            CVar pos_var = inter::GetNativeArg(state, args, n, 1);
            CheckNumberArg(pos_var, 2, "table.remove");
            pos = inter::CVarToInteger(pos_var, len);
        }
        if (pos < 1 || pos > len) return inter::NativeToFakeluaNil(state);

        CVar removed = TableHelper::GetTableInt(state, tbl, pos);
        for (int64_t i = pos; i < len; ++i) {
            CVar next_val = TableHelper::GetTableInt(state, tbl, i + 1);
            TableHelper::SetTableInt(state, tbl, i, next_val);
        }
        TableHelper::SetTableInt(state, tbl, len, inter::NativeToFakeluaNil(state));
        return removed;
    });

    RegisterNativeFunction(s, "table.concat", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        // 标准 Lua：table.concat 第一个参数必须是 table，否则抛出异常
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.concat' (table expected)");
        }
        std::string sep = "";
        if (n >= 2) {
            CVar sep_var = inter::GetNativeArg(state, args, n, 1);
            // 标准 Lua：table.concat 的 sep 必须是 string（数字会被转换为字符串，这是标准行为）
            CheckStringArg(sep_var, 2, "table.concat");
            std::string temp_sep;
            sep = std::string(GetStringArgView(sep_var, temp_sep));
        }
        int64_t start_i = 1;
        if (n >= 3) {
            CVar start_var = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(start_var, 3, "table.concat");
            start_i = inter::CVarToInteger(start_var, 1);
        }
        int64_t end_j = TableHelper::GetTableLen(tbl);
        if (n >= 4) {
            CVar end_var = inter::GetNativeArg(state, args, n, 3);
            CheckNumberArg(end_var, 4, "table.concat");
            end_j = inter::CVarToInteger(end_var, end_j);
        }

        std::string res;
        for (int64_t idx = start_i; idx <= end_j; ++idx) {
            if (idx > start_i) res += sep;
            CVar item = TableHelper::GetTableInt(state, tbl, idx);
            if (item.type_ == static_cast<int>(VarType::Int)) {
                res += std::to_string(item.data_.i);
            } else if (item.type_ == static_cast<int>(VarType::Float)) {
                // 标准 Lua：table.concat 会将 float 转换为字符串
                res += std::format("{}", item.data_.f);
            } else if (item.type_ == static_cast<int>(VarType::String) || item.type_ == static_cast<int>(VarType::StringId)) {
                auto sv = KeyToStringView(item);
                res += std::string(sv);
            } else {
                ThrowFakeluaException("invalid value in table for 'concat'");
            }
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "table.unpack", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'unpack' (table expected)");
        }
        int64_t start_i = 1;
        if (n >= 2) {
            CVar start_var = inter::GetNativeArg(state, args, n, 1);
            CheckNumberArg(start_var, 2, "table.unpack");
            start_i = inter::CVarToInteger(start_var, 1);
        }
        int64_t end_j = TableHelper::GetTableLen(tbl);
        if (n >= 3) {
            CVar end_var = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(end_var, 3, "table.unpack");
            end_j = inter::CVarToInteger(end_var, end_j);
        }

        if (start_i > end_j) return inter::AllocMultiCVar(state, 0);
        int64_t diff = end_j - start_i + 1;
        if (diff <= 0 || diff > 1000000) return inter::AllocMultiCVar(state, 0);
        int count = static_cast<int>(diff);
        CVar multi = inter::AllocMultiCVar(state, count);
        for (int i = 0; i < count; ++i) {
            CVar item = TableHelper::GetTableInt(state, tbl, start_i + i);
            inter::SetMultiCVarElement(multi, i, item);
        }
        return multi;
    });

    RegisterNativeFunction(s, "table.pack", 0, true, [](State *state, CVar *args, int n) -> CVar {
        CVar tbl_cvar = CreateEmptyTable(state);

        for (int i = 0; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            TableHelper::SetTableInt(state, tbl_cvar, i + 1, arg_i);
        }

        TableHelper::SetTableStrId(state, tbl_cvar, "n", inter::NativeToFakeluaInt(state, n));
        return tbl_cvar;
    });

    RegisterNativeFunction(s, "table.move", 4, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 4) return inter::NativeToFakeluaNil(state);
        CVar a1 = inter::GetNativeArg(state, args, n, 0);
        CVar f_var = inter::GetNativeArg(state, args, n, 1);
        CVar e_var = inter::GetNativeArg(state, args, n, 2);
        CVar t_var = inter::GetNativeArg(state, args, n, 3);
        CVar a2 = (n >= 5) ? inter::GetNativeArg(state, args, n, 4) : a1;
        if (a2.type_ == static_cast<int>(VarType::Nil)) {
            a2 = a1;
        }
        if (a1.type_ != static_cast<int>(VarType::Table) || !a1.data_.t || a2.type_ != static_cast<int>(VarType::Table) || !a2.data_.t) {
            ThrowFakeluaException("bad argument to 'table.move' (table expected)");
        }
        CheckNumberArg(f_var, 2, "table.move");
        CheckNumberArg(e_var, 3, "table.move");
        CheckNumberArg(t_var, 4, "table.move");

        int64_t f = inter::CVarToInteger(f_var, 1);
        int64_t e = inter::CVarToInteger(e_var, 0);
        int64_t t = inter::CVarToInteger(t_var, 1);

        if (e >= f) {
            uint64_t count = static_cast<uint64_t>(e) - static_cast<uint64_t>(f) + 1;
            if (count > 10000000ULL) return a2;
            int64_t icount = static_cast<int64_t>(count);
            bool same_table = (a1.type_ == static_cast<int>(VarType::Table) && a2.type_ == static_cast<int>(VarType::Table) && a1.data_.t == a2.data_.t);
            if (!same_table || t <= f || t > e) {
                for (int64_t i = 0; i < icount; ++i) {
                    CVar val = TableHelper::GetTableInt(state, a1, f + i);
                    TableHelper::SetTableInt(state, a2, t + i, val);
                }
            } else {
                for (int64_t i = icount - 1; i >= 0; --i) {
                    CVar val = TableHelper::GetTableInt(state, a1, f + i);
                    TableHelper::SetTableInt(state, a2, t + i, val);
                }
            }
        }
        return a2;
    });

    RegisterNativeFunction(s, "table.sort", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.sort' (table expected)");
        }
        int64_t len = TableHelper::GetTableLen(tbl);
        if (len <= 1) return inter::NativeToFakeluaNil(state);

        std::vector<CVar> vec(len);
        for (int64_t i = 0; i < len; ++i) {
            vec[i] = TableHelper::GetTableInt(state, tbl, i + 1);
        }

        CVar comp = (n >= 2) ? inter::GetNativeArg(state, args, n, 1) : CVar{static_cast<int>(VarType::Nil)};
        if (comp.type_ != static_cast<int>(VarType::Nil) && comp.type_ != static_cast<int>(VarType::Closure)) {
            ThrowBadArgument(2, "table.sort", "function expected");
        }

        if (comp.type_ == static_cast<int>(VarType::Closure) && comp.data_.cl) {
            VarClosure *cl = comp.data_.cl;
            auto comp_func = [&](const CVar &a, const CVar &b) -> bool {
                CVar res{static_cast<int>(VarType::Nil)};
                if (cl->func_ptr) {
                    void *addr = cl->func_ptr;
                    res = reinterpret_cast<CVar (*)(VarClosure *, CVar, CVar)>(addr)(cl, a, b);
                } else if (cl->code_str) {
                    CVar args_arr[2] = {a, b};
                    res = FlEvalLoadClosure(state, cl, 2, args_arr);
                }
                bool is_true = !(res.type_ == static_cast<int>(VarType::Nil) || (res.type_ == static_cast<int>(VarType::Bool) && !res.data_.b));
                return is_true;
            };
            std::stable_sort(vec.begin(), vec.end(), comp_func);
        } else {
            auto default_comp = [](const CVar &a, const CVar &b) -> bool {
                if (a.type_ == static_cast<int>(VarType::Int) && b.type_ == static_cast<int>(VarType::Int)) {
                    return a.data_.i < b.data_.i;
                }
                if (a.type_ == static_cast<int>(VarType::Float) && b.type_ == static_cast<int>(VarType::Float)) {
                    return a.data_.f < b.data_.f;
                }
                if (a.type_ == static_cast<int>(VarType::Int) && b.type_ == static_cast<int>(VarType::Float)) {
                    return static_cast<double>(a.data_.i) < b.data_.f;
                }
                if (a.type_ == static_cast<int>(VarType::Float) && b.type_ == static_cast<int>(VarType::Int)) {
                    return a.data_.f < static_cast<double>(b.data_.i);
                }
                // fakelua 扩展：允许 number 与 string 混合比较（转换为 string）
                // 标准 Lua 5.3 只允许 string-string 或 number-number，但 fakelua 支持混合
                bool a_is_str = (a.type_ == static_cast<int>(VarType::String) || a.type_ == static_cast<int>(VarType::StringId));
                bool b_is_str = (b.type_ == static_cast<int>(VarType::String) || b.type_ == static_cast<int>(VarType::StringId));
                bool a_is_num = (a.type_ == static_cast<int>(VarType::Int) || a.type_ == static_cast<int>(VarType::Float));
                bool b_is_num = (b.type_ == static_cast<int>(VarType::Int) || b.type_ == static_cast<int>(VarType::Float));
                if (a_is_str && b_is_str) {
                    return std::string(KeyToStringView(a)) < std::string(KeyToStringView(b));
                }
                // number 与 string 混合：都转换为 string比较（fakelua 扩展）
                if ((a_is_num && b_is_str) || (a_is_str && b_is_num)) {
                    std::string sa = a_is_str ? std::string(KeyToStringView(a)) : AsVar(a).ToString(/*has_quote=*/false, /*has_postfix=*/false);
                    std::string sb = b_is_str ? std::string(KeyToStringView(b)) : AsVar(b).ToString(/*has_quote=*/false, /*has_postfix=*/false);
                    return sa < sb;
                }
                // 其他类型组合（bool 等）抛出异常
                ThrowFakeluaException("attempt to compare two values");
                return false; // unreachable
            };
            std::stable_sort(vec.begin(), vec.end(), default_comp);
        }

        for (int64_t i = 0; i < len; ++i) {
            TableHelper::SetTableInt(state, tbl, i + 1, vec[i]);
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "table.create", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return CreateEmptyTable(state);
        CVar seq_var = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(seq_var, 1, "table.create");
        int64_t count = inter::CVarToInteger(seq_var, 0);
        if (count < 0) count = 0;

        CVar val = (n >= 2) ? inter::GetNativeArg(state, args, n, 1) : CVar{static_cast<int>(VarType::Nil)};
        CVar tbl_cvar = CreateEmptyTable(state);
        if (val.type_ != static_cast<int>(VarType::Nil)) {
            for (int64_t i = 1; i <= count; ++i) {
                TableHelper::SetTableInt(state, tbl_cvar, i, val);
            }
        }
        return tbl_cvar;
    });
}

}// namespace fakelua
