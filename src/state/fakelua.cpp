#include "fakelua.h"
#include "compile/c_runtime_header.h"
#include "interp/func_proto.h"
#include "interp/interpreter.h"
#include "jit/jit_error_boundary.h"
#include "native/event/native_event.h"
#include "native/http/native_http.h"
#include "native/io/native_io.h"
#include "native/mysql/native_mysql.h"
#include "native/net/native_net.h"
#include "native/object/native_object.h"
#include "native/redis/native_redis.h"
#include "native/sqlite/native_sqlite.h"
#include "native/table/native_table.h"
#include "native/timer/native_timer.h"
#include "state/state.h"
#include "util/common.h"
#include "util/dispatch_macro.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"

#include <cmath>

namespace fakelua {

namespace inter {

template<typename T>
CVar NativeToFakeluaIntHelper(T val) {
    Var ret;
    ret.SetInt(static_cast<int64_t>(val));
    return ret;
}

CVar NativeToFakeluaNil(State *state) {
    return Var{};
}

CVar NativeToFakeluaBool(State *state, bool val) {
    Var ret;
    ret.SetBool(val);
    return ret;
}

CVar NativeToFakeluaChar(State *state, char val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUchar(State *state, unsigned char val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaShort(State *state, short val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUshort(State *state, unsigned short val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaInt(State *state, long long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUint(State *state, unsigned int val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaLong(State *state, long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUlong(State *state, unsigned long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaLonglong(State *state, long long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUlonglong(State *state, unsigned long long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaFloat(State *state, double val) {
    Var ret;
    ret.SetFloat(val);
    return ret;
}

CVar NativeToFakeluaDouble(State *state, double val) {
    Var ret;
    ret.SetFloat(val);
    return ret;
}

CVar NativeToFakeluaCstr(State *state, const char *val) {
    Var ret;
    // 防御性处理：外部传入 nullptr 时视为空字符串，避免 string_view 构造崩溃
    ret.SetTempString(state, val ? val : "");
    return ret;
}

CVar NativeToFakeluaStr(State *state, char *val) {
    return NativeToFakeluaCstr(state, val);
}

CVar NativeToFakeluaString(State *state, const std::string &val) {
    Var ret;
    ret.SetTempString(state, val);
    return ret;
}

CVar NativeToFakeluaStringView(State *state, const std::string_view &val) {
    Var ret;
    ret.SetTempString(state, val);
    return ret;
}

bool FakeluaToNativeBool(State *state, CVar val) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::Bool)) {
        return var_val.GetBool();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeBool failed, type is {}", VarTypeToString(var_val.Type())));
}

template<typename T>
T FakeluaToNativeIntHelper(CVar val, const char *func_name) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::Int)) {
        return static_cast<T>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("{} failed, type is {}", func_name, VarTypeToString(var_val.Type())));
}

char FakeluaToNativeChar(State *state, CVar val) {
    return FakeluaToNativeIntHelper<char>(val, "FakeluaToNativeChar");
}

unsigned char FakeluaToNativeUchar(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned char>(val, "FakeluaToNativeUchar");
}

short FakeluaToNativeShort(State *state, CVar val) {
    return FakeluaToNativeIntHelper<short>(val, "FakeluaToNativeShort");
}

unsigned short FakeluaToNativeUshort(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned short>(val, "FakeluaToNativeUshort");
}

int FakeluaToNativeInt(State *state, CVar val) {
    return FakeluaToNativeIntHelper<int>(val, "FakeluaToNativeInt");
}

unsigned int FakeluaToNativeUint(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned int>(val, "FakeluaToNativeUint");
}

long FakeluaToNativeLong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<long>(val, "FakeluaToNativeLong");
}

unsigned long FakeluaToNativeUlong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned long>(val, "FakeluaToNativeUlong");
}

long long FakeluaToNativeLonglong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<long long>(val, "FakeluaToNativeLonglong");
}

unsigned long long FakeluaToNativeUlonglong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned long long>(val, "FakeluaToNativeUlonglong");
}

template<typename T>
T FakeluaToNativeFloatHelper(CVar val, const char *func_name) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::Float)) {
        return static_cast<T>(var_val.GetFloat());
    }
    if (UNLIKELY(var_val.Type() == VarType::Int)) {
        return static_cast<T>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("{} failed, type is {}", func_name, VarTypeToString(var_val.Type())));
}

float FakeluaToNativeFloat(State *state, CVar val) {
    return FakeluaToNativeFloatHelper<float>(val, "FakeluaToNativeFloat");
}

double FakeluaToNativeDouble(State *state, CVar val) {
    return FakeluaToNativeFloatHelper<double>(val, "FakeluaToNativeDouble");
}

std::string FakeluaToNativeString(State *state, CVar val) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::String || var_val.Type() == VarType::StringId)) {
        return std::string(var_val.GetString()->Str());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeString failed, type is {}", VarTypeToString(var_val.Type())));
}

std::string_view FakeluaToNativeStringView(State *state, CVar val) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::String || var_val.Type() == VarType::StringId)) {
        return var_val.GetString()->Str();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStringview failed, type is {}", VarTypeToString(var_val.Type())));
}

int64_t CVarToInteger(const CVar &v, int64_t default_val) {
    const auto &var = AsVar(v);
    if (var.Type() == VarType::Int) return var.GetInt();
    if (var.Type() == VarType::Bool) return var.GetBool() ? 1 : 0;
    if (var.Type() == VarType::Float) {
        const double d = var.GetFloat();
        // NaN/Inf 以及超出 int64 范围的 float 强转是 UB，返回默认值让调用方决定。
        if (!std::isfinite(d) || d < static_cast<double>(INT64_MIN) || d >= static_cast<double>(INT64_MAX) + 1.0) {
            return default_val;
        }
        return static_cast<int64_t>(d);
    }
    if (var.Type() == VarType::String || var.Type() == VarType::StringId) {
        std::string_view sv = var.GetString()->Str();
        if (!sv.empty()) {
            try {
                size_t pos = 0;
                std::string s(sv);
                int64_t ival = std::stoll(s, &pos, 0);
                if (pos == s.size()) return ival;
            } catch (...) {
            }
            try {
                size_t pos = 0;
                std::string s(sv);
                double dval = std::stod(s, &pos);
                if (pos == s.size()) {
                    if (!std::isfinite(dval) || dval < static_cast<double>(INT64_MIN) || dval >= static_cast<double>(INT64_MAX) + 1.0) {
                        return default_val;
                    }
                    return static_cast<int64_t>(dval);
                }
            } catch (...) {
            }
        }
    }
    return default_val;
}

double CVarToNumber(const CVar &v, double default_val) {
    const auto &var = AsVar(v);
    if (var.Type() == VarType::Float) return var.GetFloat();
    if (var.Type() == VarType::Int) return static_cast<double>(var.GetInt());
    if (var.Type() == VarType::String || var.Type() == VarType::StringId) {
        std::string_view sv = var.GetString()->Str();
        if (!sv.empty()) {
            try {
                size_t pos = 0;
                double dval = std::stod(std::string(sv), &pos);
                if (pos == sv.size()) return dval;
            } catch (...) {
            }
        }
    }
    return default_val;
}

bool CVarToBool(const CVar &v, bool default_val) {
    const auto &var = AsVar(v);
    if (var.Type() == VarType::Nil) return default_val;
    if (var.Type() == VarType::Bool) return var.GetBool();
    // 遵循 Lua 真假值规则：除 nil 与 false 外的一切值（包括 0、0.0、空字符串等）均为 true。
    return true;
}

std::string CVarToString(const CVar &v, const char *default_val) {
    const auto &var = AsVar(v);
    if (var.Type() == VarType::String || var.Type() == VarType::StringId) {
        return std::string(var.GetString()->Str());
    }
    return default_val != nullptr ? std::string(default_val) : std::string();
}

// VarToVi: read CVar into VarInterface* bridge type.
static void VarToVi(State *state, const CVar &src, VarInterface *dst, std::unordered_set<const VarTable *> &active_tables) {
    const auto &var_val = AsVar(src);
    DEBUG_ASSERT(var_val.Type() >= VarType::Min && var_val.Type() <= VarType::Max);
    switch (var_val.Type()) {
        case VarType::Nil:
            dst->ViSetNil();
            break;
        case VarType::Bool:
            dst->ViSetBool(var_val.GetBool());
            break;
        case VarType::Int:
            dst->ViSetInt(var_val.GetInt());
            break;
        case VarType::Float:
            dst->ViSetFloat(var_val.GetFloat());
            break;
        case VarType::String:
        case VarType::StringId:
            dst->ViSetString(var_val.GetString()->Str());
            break;
        case VarType::Table: {
            const auto table = var_val.GetTable();
            if (!table) {
                dst->ViSetNil();
                break;
            }
            if (!active_tables.insert(table).second) {
                ThrowFakeluaException("VarToVi failed: cyclic table is not supported");
            }
            struct ActiveTableGuard {
                std::unordered_set<const VarTable *> &set;
                const VarTable *tbl;
                ~ActiveTableGuard() {
                    set.erase(tbl);
                }
            } active_guard{active_tables, table};

            EnsureNativeSpecSnapshot(table);

            const uint32_t count = table->count_;
            std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
            struct KvsGuard {
                std::vector<std::pair<VarInterface *, VarInterface *>> &items;
                ~KvsGuard() {
                    for (auto &[k, v]: items) {
                        delete k;
                        delete v;
                    }
                }
            } kvs_guard{kvs};
            kvs.reserve(table->spec_count + table->arr_size_ + count);
            const auto &new_func = GetVarInterfaceNewFunc(state);
            auto alloc_vi = [&new_func]() -> std::unique_ptr<VarInterface> {
                return std::unique_ptr<VarInterface>(new_func ? new_func() : new SimpleVarImpl());
            };

            if (table->spec_count > 0 && table->spec_keys && table->spec_vals) {
                const auto *sk = reinterpret_cast<const Var *>(table->spec_keys);
                const auto *sv = reinterpret_cast<const Var *>(table->spec_vals);
                for (uint32_t i = 0; i < table->spec_count; ++i) {
                    if (sk[i].Type() == VarType::Nil || sv[i].Type() == VarType::Nil) {
                        continue;
                    }
                    auto key_item = alloc_vi();
                    auto val_item = alloc_vi();
                    VarToVi(state, sk[i], key_item.get(), active_tables);
                    VarToVi(state, sv[i], val_item.get(), active_tables);
                    kvs.emplace_back(key_item.release(), val_item.release());
                }
            }
            for (uint32_t i = 0; i < table->arr_size_; ++i) {
                const auto &av = AsVar(table->arr_[i]);
                if (av.Type() == VarType::Nil) {
                    continue;
                }
                auto key_item = alloc_vi();
                auto val_item = alloc_vi();
                key_item->ViSetInt(static_cast<int64_t>(i) + 1);
                VarToVi(state, table->arr_[i], val_item.get(), active_tables);
                kvs.emplace_back(key_item.release(), val_item.release());
            }
            if (const uint32_t *al = table->active_list_; al == nullptr) {
                for (uint32_t i = 0; i < count; ++i) {
                    const auto &e = table->quick_data_[i];
                    auto key_item = alloc_vi();
                    auto val_item = alloc_vi();
                    VarToVi(state, e.key, key_item.get(), active_tables);
                    VarToVi(state, e.val, val_item.get(), active_tables);
                    kvs.emplace_back(key_item.release(), val_item.release());
                }
            } else {
                for (uint32_t i = 0; i < count; ++i) {
                    const auto &e = table->nodes_[al[i]].entry;
                    auto key_item = alloc_vi();
                    auto val_item = alloc_vi();
                    VarToVi(state, e.key, key_item.get(), active_tables);
                    VarToVi(state, e.val, val_item.get(), active_tables);
                    kvs.emplace_back(key_item.release(), val_item.release());
                }
            }
            dst->ViSetTable(kvs);
            break;
        }
        case VarType::Closure:
            ThrowFakeluaException("VarToVi failed: Closure type is not supported");
            break;
        case VarType::Multi:
            ThrowFakeluaException("VarToVi failed: Multi-return type is not supported");
            break;
    }
}

VarInterface *FakeluaToNativeObj(State *state, CVar val) {
    const auto &new_func = GetVarInterfaceNewFunc(state);
    std::unique_ptr<VarInterface> ret(new_func ? new_func() : new SimpleVarImpl());
    std::unordered_set<const VarTable *> active_tables;
    VarToVi(state, val, ret.get(), active_tables);
    return ret.release();
}

CVar NativeToFakeluaNativeObject(State *state, const NativeObject *obj) {
    if (!obj || !obj->Alive()) {
        return NativeToFakeluaNil(state);
    }
    return obj->Wrap(state);
}

void *GetFuncAddr(State *state, JITType type, const std::string_view &name, int &arg_count, bool &is_vararg) {
    const auto ret = state->GetVM().GetFunction({name.data(), name.size()});
    if (UNLIKELY(ret.Empty())) {
        return nullptr;
    }
    arg_count = ret.GetArgCount();
    is_vararg = ret.IsVararg();
    return ret.GetAddr(type);
}

[[noreturn]] void ThrowInterFakeluaException(const std::string &msg) {
    ThrowFakeluaException(msg);
}

int GetReentrantCount(State *state) {
    return state->GetReentrantCount();
}

void AddReentrantCount(State *state) {
    state->AddReentrantCount();
}

void SubReentrantCount(State *state) {
    state->SubReentrantCount();
}

void Reset(State *state) {
    state->Reset();
}

}// namespace inter

State *FakeluaNewState(const StateConfig &cfg) {
    return new State(cfg);
}

void FakeluaDeleteState(State *state) {
    if (!state) return;
    net::OnStateDeleted(state);
    http::OnStateDeleted(state);
    redis::OnStateDeleted(state);
    mysql::OnStateDeleted(state);
    sqlite::OnStateDeleted(state);
    io::OnStateDeleted(state);
    delete state;
}

void CompileFile(State *state, const std::string &filename, const CompileConfig &cfg) {
    state->GetCompiler().CompileFile(filename, cfg);
}

void CompileString(State *state, const std::string &str, const CompileConfig &cfg) {
    state->GetCompiler().CompileString(str, cfg);
}

std::string GetLastRecordedCCode(State *state) {
    return state->GetCompiler().GetLastRecordedCCode();
}

void SetVarInterfaceNewFunc(State *state, const std::function<VarInterface *()> &func) {
    state->SetVarInterfaceNewFunc(func);
}

std::function<VarInterface *()> &GetVarInterfaceNewFunc(State *state) {
    return state->GetVarInterfaceNewFunc();
}

void SetDebugLogLevel(State *s, int level) {
    SetLogLevel(s, static_cast<LogLevel>(level));
}

void SetDebugLogLevel(State *s, LogLevel level) {
    SetLogLevel(s, level);
}

// RegisterNativeFunction — 注册 C++ 函数供 lua 侧通过名字调用
void RegisterNativeFunction(State *s, const std::string &name, int arg_count, bool is_vararg, NativeFuncCallback callback) {
    s->GetVM().RegisterNativeFunction(name, arg_count, is_vararg, std::move(callback));
}

void RegisterNativeVarFunction(State *s, const std::string &name, int arg_count, bool is_vararg, NativeVarFuncCallback callback) {
    auto safe_cb = std::move(callback);
    RegisterNativeFunction(s, name, arg_count, is_vararg, [safe_cb](State *state, CVar *args, int n) -> CVar {
        if (!safe_cb) {
            return inter::NativeToFakeluaNil(state);
        }
        std::vector<VarInterface *> vi_args;
        struct ViArgsGuard {
            std::vector<VarInterface *> &items;
            ~ViArgsGuard() {
                for (auto *vi: items) {
                    delete vi;
                }
            }
        } args_guard{vi_args};
        vi_args.reserve(n);
        for (int i = 0; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            vi_args.push_back(inter::FakeluaToNativeObj(state, arg_i));
        }
        std::unique_ptr<VarInterface> res_vi(safe_cb(state, vi_args));
        return inter::NativeToFakeluaVarInterface(state, res_vi.get());
    });
}

namespace inter {

static CVar ViToVar(State *state, VarInterface *src) {
    if (!src || src->ViGetType() == VarInterface::Type::NIL) {
        return NativeToFakeluaNil(state);
    }
    switch (src->ViGetType()) {
        case VarInterface::Type::BOOL:
            return NativeToFakeluaBool(state, src->ViGetBool());
        case VarInterface::Type::INT:
            return NativeToFakeluaInt(state, src->ViGetInt());
        case VarInterface::Type::FLOAT:
            return NativeToFakeluaDouble(state, src->ViGetFloat());
        case VarInterface::Type::STRING:
            return NativeToFakeluaStringView(state, src->ViGetString());
        case VarInterface::Type::TABLE: {
            // 必须走 TableHelper 的哈希写入：此前这里把第 i 个键值对直接写在 nodes_[i]，
            // 既不按 hash & mask 落槽也不填 entry.hash，导致这样构造出来的表在 JIT 侧
            // （FlGetTable/FlGetTableInt 都按哈希定位）根本索引不到。
            const size_t count = src->ViGetTableSize();
            CVar tbl_cvar = table::TableHelper::CreateTable(state);
            for (size_t i = 0; i < count; ++i) {
                auto kv = src->ViGetTableKv(static_cast<int>(i));
                CVar k = ViToVar(state, kv.first);
                CVar v = ViToVar(state, kv.second);
                table::TableHelper::SetTable(state, tbl_cvar, k, v);
            }
            return tbl_cvar;
        }
        default:
            break;
    }
    return NativeToFakeluaNil(state);
}

CVar NativeToFakeluaVarInterface(State *state, VarInterface *v) {
    return ViToVar(state, v);
}

void ThrowIfMultiCVar(const CVar &v) {
    if (UNLIKELY(v.type_ == static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("NativeToFakelua: CVar with Multi type is not allowed, use raw values instead");
    }
}

static CVar DispatchCallRaw(void *addr, const CVar *arg_arr, int arg_count, VarClosure *cl);

CVar DispatchCall(State *s, void *addr, const CVar *arg_arr, int arg_count, JITType type, VarClosure *cl) {
    State::ActiveDispatchScope active_dispatch_scope(s);
    const bool use_interp = cl ? IsInterpClosure(cl) : (type == JIT_INTERP);
    const JITType effective_type = use_interp ? JIT_INTERP : (IsGccClosure(cl) ? JIT_GCC : type);
    // 进入目标引擎代码：按闭包真实的创建引擎切换上下文（例如 table.sort / xpcall 借用
    // JIT_TCC 标记请求 setjmp 边界，但被调若是 GCC/解释器闭包，CurrentJit 仍应是真实引擎）。
    State::JitContextScope jit_scope(s, effective_type);
    // 本次调用期间才承认「位置槽里是当前位置」，返回后恢复主叫方的值。
    LuaLocScope loc_scope(s);
    // 生成代码出错时 FL_LUA_POP 不会执行。深度在这里恢复；调用栈文本已留在异常里。
    LuaStackScope stack_scope(s);
    if (use_interp) {
        void *proto = (cl && cl->func_ptr) ? cl->func_ptr : addr;
        return InterpreterExecute(s, AsInterpProto(proto), arg_arr, arg_count, cl);
    }
    // GCC 后端产出的动态库带 .eh_frame，C++ 异常能正常穿过它的帧；
    // 若调用方显式传 JIT_TCC 请求边界，或闭包本身来自 TCC，则保留 setjmp 边界。
    if (effective_type == JIT_GCC && type == JIT_GCC) {
        return DispatchCallRaw(addr, arg_arr, arg_count, cl);
    }
    return RunWithJitErrorBoundary(s, [&] { return DispatchCallRaw(addr, arg_arr, arg_count, cl); });
}

CVar DispatchCallClosure(State *state, VarClosure *cl, const CVar *args, int arg_count, JITType type) {
    if (!cl) {
        ThrowFakeluaException("closure has no code");
    }
    // load() 返回的是源码闭包：func_ptr 为空，code_str 指向待编译文本。
    if (!cl->func_ptr) {
        if (IsSourceLoadClosure(cl)) {
            return FlEvalLoadClosure(state, cl, arg_count, args);
        }
        ThrowFakeluaException("closure has no code");
    }
    if (!args || arg_count < 0) arg_count = 0;
    const int expected = cl->expected_arg_count;
    if (expected < 0 || expected > static_cast<int>(kMaxFunctionInputParams)) {
        ThrowFakeluaException("too many arguments");
    }
    CVar padded[kMaxFunctionInputParams];
    const bool is_vararg = cl->is_vararg;
    if (is_vararg) {
        // expected_arg_count 对 vararg 函数的含义：
        //   expected == 0: 纯 vararg（如 function(...))，签名 (VarClosure*, CVar/*multi*/)
        //   expected == N (N>0): N-1 个固定参数 + 1 个 vararg 槽，签名 (VarClosure*, a0…a_{N-2}, multi)
        // 因此固定参数槽数 fixed = max(0, expected-1)，vararg 打包后放在 padded[fixed]，
        // 实际派发参数数量 n_dispatch = max(1, expected)（始终至少传 1 个 vararg 包）。
        const int fixed = (expected > 0) ? (expected - 1) : 0;
        for (int i = 0; i < fixed; ++i) {
            padded[i] = (i < arg_count && args) ? args[i] : NativeToFakeluaNil(state);
        }
        const int extra = (arg_count > fixed) ? (arg_count - fixed) : 0;
        CVar multi = AllocMultiCVar(state, extra);
        for (int i = 0; i < extra; ++i) {
            SetMultiCVarElement(multi, i, args[fixed + i]);
        }
        // vararg 包放在最后一个槽：fixed 号位置（0-based），即第 expected 个参数槽
        padded[fixed] = multi;
        const int n_dispatch = (expected > 0) ? expected : 1;
        return DispatchCall(state, cl->func_ptr, padded, n_dispatch, type, cl);
    }
    for (int i = 0; i < expected; ++i) {
        padded[i] = (i < arg_count && args) ? args[i] : NativeToFakeluaNil(state);
    }
    return DispatchCall(state, cl->func_ptr, padded, expected, type, cl);
}

static CVar DispatchCallRaw(void *addr, const CVar *arg_arr, int arg_count, VarClosure *cl) {
    switch (arg_count) {
#define DCASE(N)                                                                                                                                                                                       \
    case N:                                                                                                                                                                                            \
        return reinterpret_cast<CVar (*)(VarClosure * DISPATCH_CVAR_##N)>(addr)(cl DISPATCH_ARG_##N);

        DCASE(0)
        DCASE(1)
        DCASE(2)
        DCASE(3)
        DCASE(4)
        DCASE(5)
        DCASE(6)
        DCASE(7)
        DCASE(8)
        DCASE(9)
        DCASE(10)
        DCASE(11)
        DCASE(12)
        DCASE(13)
        DCASE(14)
        DCASE(15)
        DCASE(16)
        DCASE(17)
        DCASE(18)
        DCASE(19)
        DCASE(20)
        DCASE(21)
        DCASE(22)
        DCASE(23)
        DCASE(24)
        DCASE(25)
        DCASE(26)
        DCASE(27)
        DCASE(28)
        DCASE(29)
        DCASE(30)
        DCASE(31)
        DCASE(32)

#undef DCASE
#include "util/dispatch_macro_undef.h"
        default:
            ThrowFakeluaException(std::format("DispatchCall: arg_count {} out of range", arg_count));
    }
    __builtin_unreachable();
}

CVar AllocMultiCVar(State *s, int count) {
    VarMulti *m = VarMulti::AllocTemp(s, count);
    for (int i = 0; i < count; ++i) {
        m->GetVars()[i] = CVar{static_cast<int>(VarType::Nil)};
    }
    CVar result;
    result.type_ = static_cast<int>(VarType::Multi);
    result.flag_ = 0;
    result.data_.m = m;
    return result;
}

void SetMultiCVarElement(CVar &multi, int idx, CVar val) {
    if (UNLIKELY(multi.type_ != static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("SetMultiCVarElement: CVar is not Multi");
    }
    VarMulti *m = multi.data_.m;
    if (UNLIKELY(idx < 0 || idx >= static_cast<int>(m->GetCount()))) {
        ThrowFakeluaException(std::format("SetMultiCVarElement: idx {} out of range", idx));
    }
    m->GetVars()[idx] = val;
}

CVar GetMultiCVarElement(const CVar &multi, int idx) {
    if (UNLIKELY(multi.type_ != static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("GetMultiCVarElement: CVar is not Multi");
    }
    VarMulti *m = multi.data_.m;
    if (UNLIKELY(idx < 0 || idx >= static_cast<int>(m->GetCount()))) {
        ThrowFakeluaException(std::format("GetMultiCVarElement: idx {} out of range", idx));
    }
    return m->GetVars()[idx];
}

int GetMultiCVarCount(const CVar &multi) {
    if (UNLIKELY(multi.type_ != static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("GetMultiCVarCount: CVar is not Multi");
    }
    return static_cast<int>(multi.data_.m->GetCount());
}

CVar GetReturnElement(State *s, const CVar &ret_var, std::size_t idx) {
    if (ret_var.type_ == static_cast<int>(VarType::Multi)) {
        VarMulti *m = ret_var.data_.m;
        if (m && idx < m->GetCount()) {
            return m->GetVars()[idx];
        }
        return NativeToFakeluaNil(s);
    }
    return idx == 0 ? ret_var : NativeToFakeluaNil(s);
}

CVar UnboxFirstReturn(State *s, const CVar &ret_var) {
    return GetReturnElement(s, ret_var, 0);
}

PreservedCVarArg PreserveCVarBeforeReset(const CVar &v, bool will_reset) {
    PreservedCVarArg out;
    out.raw = v;
    if (will_reset && v.type_ == static_cast<int>(VarType::String) && v.data_.s != nullptr) {
        out.has_saved_str = true;
        out.saved_str.assign(v.data_.s->Str());
    }
    return out;
}

CVar MaterializePreservedCVar(State *s, const PreservedCVarArg &p) {
    if (p.has_saved_str) {
        return NativeToFakeluaStringView(s, p.saved_str);
    }
    return p.raw;
}

CVar GetNativeArg(State *s, CVar *args, int n, int idx) {
    (void) s;
    CVar res{static_cast<int>(VarType::Nil)};
    if (n > 0 && args[0].type_ == static_cast<int>(VarType::Multi)) {
        VarMulti *m = args[0].data_.m;
        if (m && idx < static_cast<int>(m->GetCount())) {
            res = m->GetVars()[idx];
        }
    } else if (idx < n) {
        res = args[idx];
    }
    while (res.type_ == static_cast<int>(VarType::Multi)) {
        VarMulti *m = res.data_.m;
        if (m && m->GetCount() > 0) {
            res = m->GetVars()[0];
        } else {
            res = CVar{static_cast<int>(VarType::Nil)};
            break;
        }
    }
    return res;
}

}// namespace inter

int64_t GetNativeIntArg(State *s, CVar *args, int n, int idx, int64_t default_val) {
    return inter::CVarToInteger(inter::GetNativeArg(s, args, n, idx), default_val);
}

double GetNativeFloatArg(State *s, CVar *args, int n, int idx, double default_val) {
    return inter::CVarToNumber(inter::GetNativeArg(s, args, n, idx), default_val);
}

bool GetNativeBoolArg(State *s, CVar *args, int n, int idx, bool default_val) {
    return inter::CVarToBool(inter::GetNativeArg(s, args, n, idx), default_val);
}

std::string GetNativeStringArg(State *s, CVar *args, int n, int idx, const char *default_val) {
    return inter::CVarToString(inter::GetNativeArg(s, args, n, idx), default_val);
}

CVar NativeNil(State *s) {
    return inter::NativeToFakeluaNil(s);
}

CVar NativeBool(State *s, bool val) {
    return inter::NativeToFakeluaBool(s, val);
}

CVar NativeInt(State *s, int64_t val) {
    return inter::NativeToFakeluaLonglong(s, static_cast<long long>(val));
}

CVar NativeFloat(State *s, double val) {
    return inter::NativeToFakeluaDouble(s, val);
}

CVar NativeString(State *s, const std::string &val) {
    return inter::NativeToFakeluaString(s, val);
}

bool HasEntryFunction(State *s, JITType type, const std::string_view &name) {
    int argc = 0;
    bool is_vararg = false;
    return inter::GetFuncAddr(s, type, name, argc, is_vararg) != nullptr;
}

namespace {

constexpr int kPromoteConstFlagBit = 0x1;
constexpr int kPromoteConstBoxFlagBit = 0x2;

struct ConstPromoteMemo {
    std::unordered_map<const VarTable *, CVar> tables;
    std::unordered_map<const VarClosure *, CVar> closures;
    std::unordered_map<const CVar *, CVar *> boxes;
};

CVar PromoteConstBoxValueImpl(State *s, CVar val, ConstPromoteMemo &memo) {
    if (!s) {
        val.flag_ &= ~kPromoteConstBoxFlagBit;
        return val;
    }
    switch (static_cast<VarType>(val.type_)) {
        case VarType::Nil:
        case VarType::Bool:
        case VarType::Int:
        case VarType::Float:
        case VarType::StringId:
            val.flag_ &= ~kPromoteConstBoxFlagBit;
            return val;
        case VarType::String: {
            if (!val.data_.s) {
                return CVar{static_cast<int>(VarType::Nil)};
            }
            State::ConstAllocScope const_alloc(s);
            CVar out{};
            out.type_ = static_cast<int>(VarType::String);
            out.flag_ = 0;
            out.data_.s = VarString::AllocTemp(s, val.data_.s->Str());
            return out;
        }
        case VarType::Table: {
            if (!val.data_.t) {
                return CVar{static_cast<int>(VarType::Nil)};
            }
            if ((val.flag_ & kPromoteConstFlagBit) != 0) {
                val.flag_ &= ~kPromoteConstBoxFlagBit;
                return val;
            }
            if (NativeObject *nobj = NativeObject::Unwrap(val)) {
                State::ConstAllocScope const_alloc(s);
                CVar wrapped = nobj->Wrap(s);
                wrapped.flag_ &= ~kPromoteConstBoxFlagBit;
                return wrapped;
            }
            if (const auto it = memo.tables.find(val.data_.t); it != memo.tables.end()) {
                return it->second;
            }
            State::ConstAllocScope const_alloc(s);
            CVar cloned = table::TableHelper::CreateTable(s);
            memo.tables.emplace(val.data_.t, cloned);
            table::TableHelper::ForEachKV(val, [&](CVar k, CVar elem) {
                CVar pk = PromoteConstBoxValueImpl(s, k, memo);
                CVar pv = PromoteConstBoxValueImpl(s, elem, memo);
                table::TableHelper::SetTable(s, cloned, pk, pv);
            });
            return cloned;
        }
        case VarType::Closure: {
            VarClosure *cl = val.data_.cl;
            if (!cl) {
                return CVar{static_cast<int>(VarType::Nil)};
            }
            if (const auto it = memo.closures.find(cl); it != memo.closures.end()) {
                return it->second;
            }
            if (cl->func_ptr != nullptr && !IsInterpClosure(cl)) {
                s->GetVM().RetainHandleForCodeAddr(cl->func_ptr);
            }
            auto &const_arena = s->GetHeap().GetAllocator(true);
            const int uv_cnt = cl->upvalue_count > 0 ? cl->upvalue_count : 0;
            auto *new_cl = static_cast<VarClosure *>(const_arena.Alloc(sizeof(VarClosure) + static_cast<size_t>(uv_cnt) * sizeof(CVar *)));
            new_cl->func_ptr = cl->func_ptr;
            new_cl->upvalue_count = cl->upvalue_count;
            new_cl->expected_arg_count = cl->expected_arg_count;
            new_cl->is_vararg = cl->is_vararg;
            new_cl->code_str = cl->code_str;
            for (int i = 0; i < uv_cnt; ++i) {
                new_cl->upvalues[i] = nullptr;
            }
            CVar out{};
            out.type_ = static_cast<int>(VarType::Closure);
            out.flag_ = 0;
            out.data_.cl = new_cl;
            memo.closures.emplace(cl, out);
            for (int i = 0; i < uv_cnt; ++i) {
                CVar *orig_box = cl->upvalues[i];
                if (!orig_box) {
                    continue;
                }
                if ((orig_box->flag_ & kPromoteConstBoxFlagBit) != 0) {
                    new_cl->upvalues[i] = orig_box;
                    continue;
                }
                if (const auto bit = memo.boxes.find(orig_box); bit != memo.boxes.end()) {
                    new_cl->upvalues[i] = bit->second;
                    continue;
                }
                auto *promoted_box = static_cast<CVar *>(const_arena.Alloc(sizeof(CVar)));
                *promoted_box = CVar{static_cast<int>(VarType::Nil)};
                promoted_box->flag_ = kPromoteConstBoxFlagBit;
                memo.boxes.emplace(orig_box, promoted_box);
                CVar inner = PromoteConstBoxValueImpl(s, *orig_box, memo);
                inner.flag_ |= kPromoteConstBoxFlagBit;
                *promoted_box = inner;
                new_cl->upvalues[i] = promoted_box;
            }
            return out;
        }
        default:
            val.flag_ &= ~kPromoteConstBoxFlagBit;
            return val;
    }
}

}// namespace

CVar PromoteConstBoxValue(State *state, CVar val) {
    ConstPromoteMemo memo;
    return PromoteConstBoxValueImpl(state, val, memo);
}

#if defined(__GNUC__) || defined(__clang__)
extern "C" __attribute__((used)) CVar FakeluaPromoteConstBoxValue(State *state, CVar val) {
#else
extern "C" CVar FakeluaPromoteConstBoxValue(State *state, CVar val) {
#endif
    return GuardJitEntry(state, [&] { return PromoteConstBoxValue(state, val); });
}

}// namespace fakelua
