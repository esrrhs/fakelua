#include "interp/interpreter.h"
#include "interp/func_proto.h"
#include "interp/runtime.h"
#include "jit/jit_error_boundary.h"
#include "jit/vm.h"
#include "native/table/native_table.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/exception.h"
#include "util/macro.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include <algorithm>
#include <format>
#include <string_view>
#include <vector>

namespace fakelua {

namespace {

std::string_view ConstToName(const CVar &c) {
    if (c.type_ == static_cast<int>(VarType::StringId)) {
        return ConstString::GetString(c.data_.i);
    }
    if (c.type_ == static_cast<int>(VarType::String) && c.data_.s) {
        return c.data_.s->Str();
    }
    return {};
}

CVar LookupGlobal(FuncProto *proto, std::string_view name) {
    if (name == "_VERSION") {
        CVar v{};
        v.type_ = static_cast<int>(VarType::StringId);
        v.data_.i = proto->unit->state->GetConstString().Alloc(std::string("Fakelua ") + FAKELUA_VERSION_STRING);
        return v;
    }
    const auto it = proto->unit->globals.find(name);
    if (it == proto->unit->globals.end()) {
        return interp_rt::Nil();
    }
    return it->second;
}

void SetGlobal(FuncProto *proto, std::string_view name, CVar v) {
    // init 内 NewTable 已带 CONST_FLAG，只禁止 SETTABLE 改内容。
    // 重绑定（g = {x=1}）对齐 CGen 对 static CVar 的直接赋值，必须放行（编译期已拒绝用户再赋）。
    auto &g = proto->unit->globals;
    if (auto it = g.find(name); it != g.end()) {
        it->second = v;
        return;
    }
    g.emplace(std::string(name), v);
}

thread_local std::vector<CVar> g_interp_stack;
thread_local std::vector<CVar *> g_interp_boxes;
thread_local size_t g_interp_top = 0;
thread_local int g_interp_boxed_frames = 0;

// 出错指令的行号：取值时 ip 已自增，异常抛出点对应的就是刚取出的那条指令。
// 只在这一层补位置，内层帧已经补过的不再覆盖，于是位置总是最内层那条 Lua 指令。
void AnnotateInterpLocation(FuncProto *proto, const Inst *ip, FakeluaException &e) {
    if (e.HasLoc() || proto == nullptr || proto->unit == nullptr || ip == nullptr || proto->code.empty()) {
        return;
    }
    const auto offset = ip - proto->code.data();
    if (offset <= 0 || static_cast<size_t>(offset) > proto->lineinfo.size()) {
        return;
    }
    const int line = proto->lineinfo[static_cast<size_t>(offset) - 1];
    if (line <= 0) {
        return;
    }
    e.SetLoc(std::format("{}:{}", proto->unit->file_name, line));
}

// 解释器帧内再调解释器：当前引擎通常已是 INTERP，错误位置走 lineinfo，
// 不必再套 DispatchCall 的 JitContextScope / LuaLocScope。若从尚未切换引擎
// 的入口进来（例如 init），先把 CurrentJit 设成 INTERP 再执行。
CVar InterpExecuteInContext(State *s, FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl) {
    if (s->CurrentJit() != JIT_INTERP) {
        State::JitContextScope jit_scope(s, JIT_INTERP);
        return InterpreterExecute(s, proto, args, arg_count, cl);
    }
    return InterpreterExecute(s, proto, args, arg_count, cl);
}

// 文件级 / 全局命名调用的快路径：签名匹配且目标是 INTERP 原型时直接 InterpreterExecute，
// 跳过 CallByName 的 VmFunction 拷贝、GuardJitEntry 与参数再装箱。
// 变参、末参 Multi、参数个数不一致、原生函数或无 INTERP 地址时返回 false，由调用方走 CallByNameArgs。
bool TryInterpNamedCall(State *s, std::string_view name, const CVar *args, int nargs, CVar &ret) {
    const VmFunction *fn = s->GetVM().FindFunction(name);
    if (!fn) {
        return false;
    }
    void *addr = fn->GetAddr(JIT_INTERP);
    if (!addr || fn->IsVararg()) {
        return false;
    }
    const int expected = fn->GetArgCount();
    const bool last_is_multi = (nargs > 0 && args[nargs - 1].type_ == static_cast<int>(VarType::Multi));
    if (last_is_multi || nargs != expected) {
        return false;
    }
    ret = InterpExecuteInContext(s, AsInterpProto(addr), args, nargs, nullptr);
    return true;
}

// 尾调用跨 InterpreterExecuteT 模板（无 boxes 的帧尾调到 uses_boxes 函数）
// 或需要在根帧重新 reserve 时，通过此槽把目标原型交还给 InterpreterExecute 的外层循环。
// 不增加 C 栈深度。
struct InterpTailJob {
    bool pending = false;
    FuncProto *proto = nullptr;
    VarClosure *cl = nullptr;
    int arg_count = 0;
    CVar args[kMaxFunctionInputParams]{};
};
thread_local InterpTailJob g_interp_tail;

void RequestTailTrampoline(FuncProto *new_proto, VarClosure *new_cl, const CVar *args, int arg_count) {
    g_interp_tail.pending = true;
    g_interp_tail.proto = new_proto;
    g_interp_tail.cl = new_cl;
    g_interp_tail.arg_count = arg_count;
    for (int i = 0; i < arg_count; ++i) {
        g_interp_tail.args[i] = args[i];
    }
}

int FlattenByNameArgs(const CVar *raw, int arg_num, CVar *flat) {
    int flat_count = 0;
    for (int i = 0; i < arg_num; ++i) {
        if (i == arg_num - 1 && raw[i].type_ == static_cast<int>(VarType::Multi)) {
            VarMulti *m = raw[i].data_.m;
            const int extra = m ? static_cast<int>(m->GetCount()) : 0;
            if (flat_count + extra > static_cast<int>(kMaxFunctionInputParams)) {
                ThrowFakeluaException(std::format("interpreter: too many arguments ({}), max is {}", flat_count + extra, kMaxFunctionInputParams));
            }
            if (m) {
                for (uint32_t j = 0; j < m->GetCount(); ++j) {
                    flat[flat_count++] = m->GetVars()[j];
                }
            }
        } else if (raw[i].type_ == static_cast<int>(VarType::Multi)) {
            VarMulti *m = raw[i].data_.m;
            if (flat_count >= static_cast<int>(kMaxFunctionInputParams)) {
                ThrowFakeluaException(std::format("interpreter: too many arguments, max is {}", kMaxFunctionInputParams));
            }
            flat[flat_count++] = (m && m->GetCount() > 0) ? m->GetVars()[0] : interp_rt::Nil();
        } else {
            if (flat_count >= static_cast<int>(kMaxFunctionInputParams)) {
                ThrowFakeluaException(std::format("interpreter: too many arguments, max is {}", kMaxFunctionInputParams));
            }
            flat[flat_count++] = raw[i];
        }
    }
    return flat_count;
}

// 按闭包签名装箱（与 DispatchCallClosure 一致）：变参打进最后的 Multi，缺参补 nil，多余丢掉。
int PackClosureCallArgs(State *s, VarClosure *cl, const CVar *flat, int flat_count, CVar *packed) {
    const int expected = cl->expected_arg_count;
    if (expected < 0 || expected > static_cast<int>(kMaxFunctionInputParams)) {
        ThrowFakeluaException("too many arguments");
    }
    if (cl->is_vararg) {
        const int fixed = (expected > 0) ? (expected - 1) : 0;
        for (int i = 0; i < fixed; ++i) {
            packed[i] = (i < flat_count) ? flat[i] : interp_rt::Nil();
        }
        const int extra = (flat_count > fixed) ? (flat_count - fixed) : 0;
        CVar multi = inter::AllocMultiCVar(s, extra);
        for (int i = 0; i < extra; ++i) {
            inter::SetMultiCVarElement(multi, i, flat[fixed + i]);
        }
        packed[fixed] = multi;
        return (expected > 0) ? expected : 1;
    }
    for (int i = 0; i < expected; ++i) {
        packed[i] = (i < flat_count) ? flat[i] : interp_rt::Nil();
    }
    return expected;
}

// 按 CallByName 规则装箱：非变参且末参不是 Multi 时实参个数必须精确匹配。
int PackNamedCallArgs(State *s, std::string_view name, bool is_vararg, int expected, const CVar *raw, int raw_n, const CVar *flat, int flat_count, CVar *packed) {
    if (expected < 0 || expected > static_cast<int>(kMaxFunctionInputParams)) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' declares too many parameters ({}), max is {}", name, expected, kMaxFunctionInputParams));
    }
    const int fixed = is_vararg ? std::max(0, expected - 1) : expected;
    const bool last_is_multi = (raw_n > 0 && raw[raw_n - 1].type_ == static_cast<int>(VarType::Multi));
    if (is_vararg) {
        for (int i = 0; i < fixed; ++i) {
            packed[i] = (i < flat_count) ? flat[i] : interp_rt::Nil();
        }
        const int vararg_count = std::max(0, flat_count - fixed);
        CVar multi = inter::AllocMultiCVar(s, vararg_count);
        for (int i = 0; i < vararg_count; ++i) {
            inter::SetMultiCVarElement(multi, i, flat[fixed + i]);
        }
        packed[fixed] = multi;
        return expected;
    }
    if (!last_is_multi && flat_count != expected) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' expects {} argument(s), got {}", name, expected, flat_count));
    }
    for (int i = 0; i < expected; ++i) {
        packed[i] = (i < flat_count) ? flat[i] : interp_rt::Nil();
    }
    return expected;
}

// 在本 InterpreterExecuteT 帧内切到新的 INTERP 原型；需要换 HasBoxes 模板或扩容时走 trampoline。
// 成功且应继续分发返回 true（已更新 proto/cl/ip/stk）；应 return ret 则返回 false。
template<bool HasBoxes>
bool ApplyInterpTail(State *s, FuncProto *&proto, VarClosure *&cl, CVar *const stk, CVar **const boxp, size_t saved, const Inst *&ip, FuncProto *new_proto, VarClosure *new_cl, const CVar *args, int arg_count, CVar &ret) {
    if (!new_proto) {
        ThrowFakeluaException("interpreter: null function prototype");
    }
    CVar copy[kMaxFunctionInputParams];
    for (int i = 0; i < arg_count; ++i) {
        copy[i] = args[i];
    }
    if (new_proto->uses_boxes && !HasBoxes) {
        RequestTailTrampoline(new_proto, new_cl, copy, arg_count);
        ret = {};
        return false;
    }
    const size_t new_slots = static_cast<size_t>(std::max(new_proto->max_stack, new_proto->param_count)) + 8;
    const size_t need = saved + new_slots;
    // 不在本帧 realloc：父帧可能仍持有 data() 指针。容量不够就 trampoline（根帧）或报溢出。
    // 因此 stk/boxp 地址不变，热路径可以把它们当成不变指针。
    if (need > g_interp_stack.capacity()) {
        if (saved == 0) {
            RequestTailTrampoline(new_proto, new_cl, copy, arg_count);
            ret = {};
            return false;
        }
        ThrowFakeluaException("interpreter stack overflow");
    }
    if (HasBoxes && need > g_interp_boxes.capacity()) {
        if (g_interp_boxed_frames > 1) {
            ThrowFakeluaException("interpreter stack overflow");
        }
        RequestTailTrampoline(new_proto, new_cl, copy, arg_count);
        ret = {};
        return false;
    }
    if (need > g_interp_top) {
        g_interp_stack.resize(need);
    }
    g_interp_top = need;
    const int nparam = new_proto->param_count;
    // 形参马上覆盖，不必先清；只清剩余寄存器，避免脏局部漏到下一帧。
    if (new_slots > static_cast<size_t>(nparam)) {
        std::fill(stk + nparam, stk + new_slots, CVar{});
    }
    if (HasBoxes) {
        if (need > g_interp_boxes.size()) {
            g_interp_boxes.resize(need);
        }
        std::fill(boxp, boxp + new_slots, nullptr);
    }
    for (int i = 0; i < nparam; ++i) {
        stk[static_cast<size_t>(i)] = (i < arg_count) ? copy[i] : interp_rt::Nil();
    }
    proto = new_proto;
    cl = new_cl;
    if (proto->code.empty()) {
        ret = interp_rt::Nil();
        return false;
    }
    ip = proto->code.data();
    return true;
}


// 精确自尾调用：复用当前帧与 proto/cl，不做查表、装箱、整帧 memset。
// 参数先拷到 TLS（可能与 R(0..) 重叠），再写形参并清空其余寄存器。
// 不用 InterpreterExecuteT 的栈上大数组，以免抬高非尾调用的 C 帧。
template<bool HasBoxes>
#if defined(__GNUC__)
__attribute__((always_inline))
#endif
inline bool ReuseFrameSelfTail(FuncProto *proto, CVar *const stk, CVar **const boxp, const Inst *&ip, int arg_base, int nargs) {
    CVar *const tmp = g_interp_tail.args;
    for (int i = 0; i < nargs; ++i) {
        tmp[i] = interp_rt::VmSlot<HasBoxes>(stk, boxp, arg_base + i);
    }
    const int nparam = proto->param_count;
    const int live = std::max(proto->max_stack, nparam);
    if (live > nparam) {
        std::fill(stk + nparam, stk + live, CVar{});
    }
    if (HasBoxes && boxp) {
        std::fill(boxp, boxp + live, nullptr);
    }
    for (int i = 0; i < nargs; ++i) {
        stk[static_cast<size_t>(i)] = tmp[i];
    }
    for (int i = nargs; i < nparam; ++i) {
        stk[static_cast<size_t>(i)] = interp_rt::Nil();
    }
    if (proto->code.empty()) {
        return false;
    }
    ip = proto->code.data();
    return true;
}

// 尾调用实现单独成函数，避免把 InterpreterExecuteT 热路径胀大、扰乱寄存器分配。
template<bool HasBoxes>
#if defined(__GNUC__)
__attribute__((noinline))
#endif
bool InterpDoTail(State *s, FuncProto *&proto, VarClosure *&cl, CVar *const stk, CVar **const boxp, size_t saved, const Inst *&ip, const Inst &inst, CVar &ret) {
    CVar fn = interp_rt::VmSlot<HasBoxes>(stk, boxp, inst.a);
    if (fn.type_ != static_cast<int>(VarType::Closure) || !fn.data_.cl) {
        ThrowFakeluaException("attempt to call a non-function value");
    }
    CVar call_args[kMaxFunctionInputParams];
    const int nargs = inst.b;
    if (nargs < 0 || nargs > static_cast<int>(kMaxFunctionInputParams)) {
        ThrowFakeluaException(std::format("interpreter: too many arguments ({}), max is {}", nargs, kMaxFunctionInputParams));
    }
    int nflat = 0;
    for (int i = 0; i < nargs; ++i) {
        CVar av = interp_rt::VmSlot<HasBoxes>(stk, boxp, inst.a + 1 + i);
        if (i == nargs - 1 && av.type_ == static_cast<int>(VarType::Multi)) {
            VarMulti *m = av.data_.m;
            const int extra = m ? static_cast<int>(m->GetCount()) : 0;
            if (nflat + extra > static_cast<int>(kMaxFunctionInputParams)) {
                ThrowFakeluaException(std::format("interpreter: too many arguments ({}), max is {}", nflat + extra, kMaxFunctionInputParams));
            }
            for (uint32_t j = 0; j < m->GetCount(); ++j) {
                call_args[nflat++] = m->GetVars()[j];
            }
        } else {
            call_args[nflat++] = interp_rt::UnboxMulti(av, 0);
        }
    }
    VarClosure *callee = fn.data_.cl;
    if (!IsInterpClosure(callee)) {
        ret = inter::DispatchCallClosure(s, callee, call_args, nflat, JIT_INTERP);
        return false;
    }
    CVar packed[kMaxFunctionInputParams];
    const int n_dispatch = PackClosureCallArgs(s, callee, call_args, nflat, packed);
    return ApplyInterpTail<HasBoxes>(s, proto, cl, stk, boxp, saved, ip, AsInterpProto(callee->func_ptr), callee, packed, n_dispatch, ret);
}

template<bool HasBoxes>
#if defined(__GNUC__)
__attribute__((noinline))
#endif
bool InterpDoTailName(State *s, FuncProto *&proto, VarClosure *&cl, CVar *const stk, CVar **const boxp, size_t saved, const Inst *&ip, const Inst &inst, CVar &ret) {
    const std::string_view name = ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]);
    CVar raw[kMaxFunctionInputParams];
    const int nargs = inst.b;
    if (nargs < 0 || nargs > static_cast<int>(kMaxFunctionInputParams)) {
        ThrowFakeluaException(std::format("interpreter: too many arguments ({}) passed for function '{}', max is {}", nargs, name, kMaxFunctionInputParams));
    }
    for (int i = 0; i < nargs; ++i) {
        raw[i] = interp_rt::VmSlot<HasBoxes>(stk, boxp, inst.a + i);
    }
    const VmFunction *fn = s->GetVM().FindFunction(name);
    if (fn) {
        void *addr = fn->GetAddr(JIT_INTERP);
        const bool is_vararg = fn->IsVararg();
        const int expected = fn->GetArgCount();
        CVar flat[kMaxFunctionInputParams];
        const int nflat = FlattenByNameArgs(raw, nargs, flat);
        CVar packed[kMaxFunctionInputParams];
        const int n_dispatch = PackNamedCallArgs(s, name, is_vararg, expected, raw, nargs, flat, nflat, packed);
        if (addr) {
            return ApplyInterpTail<HasBoxes>(s, proto, cl, stk, boxp, saved, ip, AsInterpProto(addr), nullptr, packed, n_dispatch, ret);
        }
        ret = CallByNameArgs(s, JIT_INTERP, name, nargs, raw);
        return false;
    }
    if (!s->GetVM().FindNativeFunction(name)) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' not found", name));
    }
    ret = CallByNameArgs(s, JIT_INTERP, name, nargs, raw);
    return false;
}

constexpr size_t kInterpStackReserve = 262144;// 4 MiB of CVar; nested realloc would invalidate parent pointers

struct InterpFrame {
    size_t saved;
    CVar *stk;
    CVar **boxp;
    InterpFrame(size_t nslots, bool want_boxes) {
        saved = g_interp_top;
        const size_t need = saved + nslots;
        if (g_interp_stack.capacity() < kInterpStackReserve) {
            g_interp_stack.reserve(kInterpStackReserve);
        }
        if (need > g_interp_stack.capacity()) {
            if (saved != 0) {
                ThrowFakeluaException("interpreter stack overflow");
            }
            g_interp_stack.reserve(std::max(need, kInterpStackReserve));
        }
        g_interp_stack.resize(need);
        stk = g_interp_stack.data() + saved;
        std::fill(stk, stk + nslots, CVar{});
        if (want_boxes) {
            // 父帧没开 boxes 时可以 realloc；有活着的 boxp 时只能用已 reserve 的容量。
            if (need > g_interp_boxes.capacity()) {
                if (g_interp_boxed_frames != 0) {
                    ThrowFakeluaException("interpreter stack overflow");
                }
                g_interp_boxes.reserve(std::max(need, kInterpStackReserve));
            }
            g_interp_boxes.resize(need);
            boxp = g_interp_boxes.data() + saved;
            std::fill(boxp, boxp + nslots, nullptr);
            ++g_interp_boxed_frames;
        } else {
            boxp = nullptr;
        }
        g_interp_top = need;
    }
    ~InterpFrame() {
        if (boxp) {
            --g_interp_boxed_frames;
        }
        g_interp_top = saved;
    }
};

template<bool HasBoxes>
#if defined(__GNUC__)
__attribute__((hot))
#endif
CVar InterpreterExecuteT(State *s, FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl) {
    const int max_stack = std::max(proto->max_stack, proto->param_count);
    InterpFrame frame(static_cast<size_t>(max_stack) + 8, HasBoxes);
    CVar *const stk = frame.stk;
    CVar **const boxp [[maybe_unused]] = frame.boxp;
#define VM_R(r) ::fakelua::interp_rt::VmSlot<HasBoxes>(stk, boxp, (r))
#define VM_W(r, v) ::fakelua::interp_rt::VmWrite<HasBoxes>(stk, boxp, (r), (v))

    for (int i = 0; i < proto->param_count; ++i) {
        stk[static_cast<size_t>(i)] = (i < arg_count && args) ? args[i] : interp_rt::Nil();
    }

    const Inst *ip = proto->code.empty() ? nullptr : proto->code.data();
    Inst inst{};

    // ip 是本帧的局部量，catch 里读到的就是抛出时正在执行的那条指令，
    // 所以补位置不需要在热路径上为每个指令先存一份。
    try {

    // GNU/Clang：token-threading（computed goto）。字节码由 codegen 保证以 RETURN
    // 结尾、跳转落在函数内，热路径不再做 pc 越界 / 未知 opcode 检查。
#if defined(__GNUC__)
#define FAKELUA_OP_LABEL_ADDR(name) &&L_OP_##name,
    static const void *const kDispatch[] = {FAKELUA_FOR_EACH_OP(FAKELUA_OP_LABEL_ADDR)};
#undef FAKELUA_OP_LABEL_ADDR
    static_assert(sizeof(kDispatch) / sizeof(kDispatch[0]) == static_cast<size_t>(Op::COUNT));
    const void *const *const dispatch = kDispatch;
#define VM_DISPATCH_NEXT()                                                                                                                 \
    do {                                                                                                                                   \
        inst = *ip++;                                                                                                                      \
        goto *dispatch[static_cast<uint8_t>(inst.op)];                                                                                     \
    } while (0)
#define VM_CASE(name) L_OP_##name:
    if (!ip) {
        return interp_rt::Nil();
    }
    VM_DISPATCH_NEXT();
#else
#define VM_DISPATCH_NEXT() goto vm_switch_dispatch
#define VM_CASE(name) case Op::name:
vm_switch_dispatch:
    if (!ip) {
        return interp_rt::Nil();
    }
    inst = *ip++;
    switch (inst.op) {
#endif

    VM_CASE(MOVE) {
        VM_W(inst.a, VM_R(inst.b));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(LOADK) {
        VM_W(inst.a, proto->constants[static_cast<size_t>(inst.sbx)]);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(LOADNIL) {
        VM_W(inst.a, interp_rt::Nil());
        VM_DISPATCH_NEXT();
    }
    VM_CASE(LOADBOOL) {
        VM_W(inst.a, interp_rt::Bool(inst.b != 0));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(GETGLOBAL) {
        VM_W(inst.a, LookupGlobal(proto, ConstToName(proto->constants[static_cast<size_t>(inst.sbx)])));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SETGLOBAL) {
        SetGlobal(proto, ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]), VM_R(inst.a));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(GETUPVAL) {
        if (!cl || inst.b >= static_cast<uint16_t>(cl->upvalue_count) || !cl->upvalues[inst.b]) {
            VM_W(inst.a, interp_rt::Nil());
        } else {
            VM_W(inst.a, *cl->upvalues[inst.b]);
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SETUPVAL) {
        if (cl && inst.b < static_cast<uint16_t>(cl->upvalue_count) && cl->upvalues[inst.b]) {
            *cl->upvalues[inst.b] = VM_R(inst.a);
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(GETTABLE) {
        const CVar t = VM_R(inst.b);
        const CVar k = VM_R(inst.c);
        if (FL_VM_UNLIKELY(t.type_ != interp_rt::kTable)) {
            ThrowFakeluaException("attempt to index a non-table value");
        }
        if (FL_VM_UNLIKELY(k.type_ == interp_rt::kNil)) {
            ThrowFakeluaException("table index is nil");
        }
        if (k.type_ == interp_rt::kInt) {
            VM_W(inst.a, table::TableHelper::GetTableInt(s, t, k.data_.i));
        } else {
            VM_W(inst.a, table::TableHelper::GetTable(s, t, k));
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SETTABLE) {
        interp_rt::SetTable(s, VM_R(inst.a), VM_R(inst.b), VM_R(inst.c));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(NEWTABLE) {
        VM_W(inst.a, interp_rt::NewTable(s));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(NEWBOX) {
        if constexpr (!HasBoxes) {
            ThrowFakeluaException("interpreter: NEWBOX without boxes");
        } else {
            auto *box = static_cast<CVar *>(interp_rt::Alloc(s, sizeof(CVar)));
            *box = VM_R(inst.a);
            boxp[inst.a] = box;
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(ADD) {
        FL_VM_ARITH_INT(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, add, interp_rt::BinAdd);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SUB) {
        FL_VM_ARITH_INT(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, sub, interp_rt::BinSub);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(MUL) {
        FL_VM_ARITH_INT(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, mul, interp_rt::BinMul);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(DIV) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinDiv);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(IDIV) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinIdiv);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(MOD) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinMod);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(POW) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinPow);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(BAND) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinBand);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(BXOR) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinBxor);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(BOR) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinBor);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SHL) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinShl);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SHR) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::BinShr);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(CONCAT) {
        VM_W(inst.a, interp_rt::BinConcat(s, VM_R(inst.b), VM_R(inst.c)));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(UNM) {
        FL_VM_UNOP(HasBoxes, stk, boxp, inst.a, inst.b, interp_rt::UnMinus);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(NOT) {
        FL_VM_UNOP(HasBoxes, stk, boxp, inst.a, inst.b, interp_rt::UnNot);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(LEN) {
        FL_VM_UNOP(HasBoxes, stk, boxp, inst.a, inst.b, interp_rt::UnLen);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(BNOT) {
        FL_VM_UNOP(HasBoxes, stk, boxp, inst.a, inst.b, interp_rt::UnBnot);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(EQ) {
        const CVar &lb = VM_R(inst.b);
        const CVar &lc = VM_R(inst.c);
        if (FL_VM_LIKELY(lb.type_ == interp_rt::kInt && lc.type_ == interp_rt::kInt)) {
            VM_W(inst.a, interp_rt::Bool(lb.data_.i == lc.data_.i));
        } else {
            VM_W(inst.a, interp_rt::Bool(interp_rt::IsEqual(lb, lc)));
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(LT) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::CmpLt);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(LE) {
        FL_VM_BINOP(HasBoxes, stk, boxp, inst.a, inst.b, inst.c, interp_rt::CmpLe);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(TESTJMP) {
        if (interp_rt::IsTrue(VM_R(inst.a)) == (inst.b != 0)) {
            ip += inst.sbx;
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(JMP) {
        ip += inst.sbx;
        VM_DISPATCH_NEXT();
    }
    VM_CASE(CALL) {
        CVar fn = VM_R(inst.a);
        if (fn.type_ != static_cast<int>(VarType::Closure) || !fn.data_.cl) {
            ThrowFakeluaException("attempt to call a non-function value");
        }
        CVar call_args[kMaxFunctionInputParams];
        const int nargs = inst.b;
        if (nargs < 0 || nargs > static_cast<int>(kMaxFunctionInputParams)) {
            ThrowFakeluaException(std::format("interpreter: too many arguments ({}), max is {}", nargs, kMaxFunctionInputParams));
        }
        int nflat = 0;
        for (int i = 0; i < nargs; ++i) {
            CVar av = VM_R(inst.a + 1 + i);
            if (i == nargs - 1 && av.type_ == static_cast<int>(VarType::Multi)) {
                VarMulti *m = av.data_.m;
                const int extra = m ? static_cast<int>(m->GetCount()) : 0;
                if (nflat + extra > static_cast<int>(kMaxFunctionInputParams)) {
                    ThrowFakeluaException(std::format("interpreter: too many arguments ({}), max is {}", nflat + extra, kMaxFunctionInputParams));
                }
                for (uint32_t j = 0; j < m->GetCount(); ++j) {
                    call_args[nflat++] = m->GetVars()[j];
                }
            } else {
                call_args[nflat++] = interp_rt::UnboxMulti(av, 0);
            }
        }
        CVar ret;
        VarClosure *callee = fn.data_.cl;
        // 解释器闭包且非变参、实参个数正好等于形参时，直接 InterpreterExecute，
        // 跳过 DispatchCallClosure 的再装箱与 LuaLocScope。其余情况（变参、少参/多参、
        // load() 源码闭包、TCC/GCC 闭包）仍走原来的 DispatchCallClosure。
        if (IsInterpClosure(callee) && !callee->is_vararg &&
            callee->expected_arg_count == nflat &&
            nflat <= static_cast<int>(kMaxFunctionInputParams)) {
            ret = InterpExecuteInContext(s, AsInterpProto(callee->func_ptr), call_args, nflat, callee);
        } else {
            ret = inter::DispatchCallClosure(s, callee, call_args, nflat, JIT_INTERP);
        }
        VM_W(inst.a, inst.c == 0 ? ret : interp_rt::UnboxMulti(ret, 0));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(CALLNAME) {
        // ConstToName 返回 string_view（interned，不一定带 NUL）。成功路径不再堆分配
        // std::string；报错走 std::format 的 string_view，仅在 CallByNameImpl 的失败路径
        // 才可能分配。
        {
            const std::string_view name = ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]);
            CVar call_args[kMaxFunctionInputParams];
            const int nargs = inst.b;
            if (nargs < 0 || nargs > static_cast<int>(kMaxFunctionInputParams)) {
                ThrowFakeluaException(std::format("interpreter: too many arguments ({}) passed for function '{}', max is {}", nargs, name, kMaxFunctionInputParams));
            }
            for (int i = 0; i < nargs; ++i) {
                call_args[i] = VM_R(inst.a + i);
            }
            CVar ret;
            if (!TryInterpNamedCall(s, name, call_args, nargs, ret)) {
                ret = CallByNameArgs(s, JIT_INTERP, name, nargs, call_args);
            }
            VM_W(inst.a, inst.c == 0 ? ret : interp_rt::UnboxMulti(ret, 0));
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(TAIL) {
        if (inst.c == kTailSelfExact) {
            // 本函数精确自尾调用：保持 cl，直接重置 ip。
            if (ReuseFrameSelfTail<HasBoxes>(proto, stk, boxp, ip, inst.a + 1, inst.b)) {
                VM_DISPATCH_NEXT();
            }
            return interp_rt::Nil();
        }
        CVar ret{};
        const bool cont = InterpDoTail<HasBoxes>(s, proto, cl, stk, boxp, frame.saved, ip, inst, ret);
        if (cont) {
            VM_DISPATCH_NEXT();
        }
        return ret;
    }
    VM_CASE(TAILNAME) {
        if (inst.c == kTailSelfExact) {
            if (ReuseFrameSelfTail<HasBoxes>(proto, stk, boxp, ip, inst.a, inst.b)) {
                VM_DISPATCH_NEXT();
            }
            return interp_rt::Nil();
        }
        CVar ret{};
        const bool cont = InterpDoTailName<HasBoxes>(s, proto, cl, stk, boxp, frame.saved, ip, inst, ret);
        if (cont) {
            VM_DISPATCH_NEXT();
        }
        return ret;
    }
    VM_CASE(RETURN) {
        CVar ret = VM_R(inst.a);
        return inst.b == 0 ? ret : interp_rt::UnboxMulti(ret, 0);
    }
    VM_CASE(CLOSURE) {
        {
            FuncProto *child = proto->child_protos[static_cast<size_t>(inst.sbx)];
            const int nup = static_cast<int>(child->upvalues.size());
            std::vector<CVar *> upbufs(static_cast<size_t>(nup), nullptr);
            for (int i = 0; i < nup; ++i) {
                const auto &uv = child->upvalues[static_cast<size_t>(i)];
                if (uv.in_stack) {
                    if constexpr (!HasBoxes) {
                        ThrowFakeluaException("interpreter: captured local without boxes");
                    } else {
                        if (!boxp[uv.idx]) {
                            auto *box = static_cast<CVar *>(interp_rt::Alloc(s, sizeof(CVar)));
                            *box = VM_R(uv.idx);
                            boxp[uv.idx] = box;
                        }
                        upbufs[i] = boxp[uv.idx];
                    }
                } else {
                    upbufs[i] = (cl && uv.idx < static_cast<uint16_t>(cl->upvalue_count)) ? cl->upvalues[uv.idx] : nullptr;
                }
            }
            VM_W(inst.a, interp_rt::MakeClosure(s, TagInterpProto(child), nup, child->param_count, child->is_vararg, upbufs.empty() ? nullptr : upbufs.data()));
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(SETLIST) {
        interp_rt::TableExpandMulti(s, VM_R(inst.a), inst.sbx, VM_R(inst.c));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(TABENT) {
        CVar idxv = VM_R(inst.sbx);
        uint32_t idx = 0;
        if (idxv.type_ == static_cast<int>(VarType::Int) && idxv.data_.i >= 0) {
            idx = static_cast<uint32_t>(idxv.data_.i);
        }
        CVar k, v;
        if (!interp_rt::TableEntry(VM_R(inst.a), idx, k, v)) {
            k = interp_rt::Nil();
            v = interp_rt::Nil();
        }
        VM_W(inst.b, k);
        VM_W(inst.c, v);
        VM_DISPATCH_NEXT();
    }
    VM_CASE(TABCOUNT) {
        VM_W(inst.a, interp_rt::Int(static_cast<int64_t>(interp_rt::TableEntryCount(VM_R(inst.b)))));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(UNBOX) {
        VM_W(inst.a, interp_rt::UnboxMulti(VM_R(inst.b), inst.c));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(MAKEMULTI) {
        {
            const int n = inst.b;
            std::vector<CVar> vals(static_cast<size_t>(n));
            for (int i = 0; i < n; ++i) {
                vals[static_cast<size_t>(i)] = VM_R(inst.a + i);
            }
            VM_W(inst.a, interp_rt::MakeMulti(s, vals.data(), static_cast<uint32_t>(n)));
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(COMBINE) {
        {
            const int nprefix = inst.b;
            std::vector<CVar> prefix(static_cast<size_t>(nprefix));
            for (int i = 0; i < nprefix; ++i) {
                prefix[static_cast<size_t>(i)] = VM_R(inst.a + i);
            }
            VM_W(inst.a, interp_rt::CombineMulti(s, prefix.data(), static_cast<uint32_t>(nprefix), VM_R(inst.a + nprefix)));
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(THROW) {
        const CVar msg = proto->constants[static_cast<size_t>(inst.sbx)];
        ThrowFakeluaException(std::string(ConstToName(msg)));
    }
    VM_CASE(FORPREP) {
        const int base = inst.a;
        CVar ctrl = VM_R(base);
        CVar limit = VM_R(base + 1);
        CVar step = VM_R(base + 2);
        CVar loopv{};
        if (interp_rt::ForPrep(ctrl, limit, step, loopv)) {
            stk[base + 3] = loopv;
            interp_rt::VmClearBox<HasBoxes>(boxp, base + 3);
            ip += inst.sbx;
        } else {
            VM_W(base, ctrl);
            VM_W(base + 1, limit);
            VM_W(base + 2, step);
            stk[base + 3] = loopv;
            interp_rt::VmClearBox<HasBoxes>(boxp, base + 3);
        }
        VM_DISPATCH_NEXT();
    }
    VM_CASE(FORLOOP) {
        const int base = inst.a;
        const CVar &step = VM_R(base + 2);
        if (FL_VM_LIKELY(step.type_ == interp_rt::kInt)) {
            const uint64_t count = static_cast<uint64_t>(VM_R(base + 1).data_.i);
            if (count > 0) {
                interp_rt::VmWriteInt<HasBoxes>(stk, boxp, base + 1, static_cast<int64_t>(count - 1));
                const int64_t newidx = static_cast<int64_t>(static_cast<uint64_t>(VM_R(base).data_.i) + static_cast<uint64_t>(step.data_.i));
                interp_rt::VmWriteInt<HasBoxes>(stk, boxp, base, newidx);
                stk[base + 3] = stk[base];
                interp_rt::VmClearBox<HasBoxes>(boxp, base + 3);
                ip += inst.sbx;
            }
            VM_DISPATCH_NEXT();
        }
        CVar ctrl = VM_R(base);
        const CVar &limit = VM_R(base + 1);
        VM_W(base, interp_rt::BinAdd(ctrl, step));
        if (interp_rt::ForLoopInRange(VM_R(base), limit, step)) {
            stk[base + 3] = VM_R(base);
            interp_rt::VmClearBox<HasBoxes>(boxp, base + 3);
            ip += inst.sbx;
        }
        VM_DISPATCH_NEXT();
    }

#if !defined(__GNUC__)
    default:
        ThrowFakeluaException("interpreter: unknown opcode");
    }
#endif

    } catch (FakeluaException &e) {
        AnnotateInterpLocation(proto, ip, e);
        throw;
    }

#undef VM_DISPATCH_NEXT
#undef VM_CASE
#undef VM_R
#undef VM_W
}

}// namespace

CVar InterpreterExecute(State *s, FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl) {
    if (!proto) {
        ThrowFakeluaException("interpreter: null function prototype");
    }
    // 热路径与引入尾调用前相同：一次 ExecuteT 后直接返回。
    // 仅在需要换 HasBoxes 模板或根帧重新 reserve 时，才用 TLS 槽循环再进入（C 栈深度仍为 1）。
    CVar ret = proto->uses_boxes ? InterpreterExecuteT<true>(s, proto, args, arg_count, cl) : InterpreterExecuteT<false>(s, proto, args, arg_count, cl);
    while (UNLIKELY(g_interp_tail.pending)) {
        proto = g_interp_tail.proto;
        cl = g_interp_tail.cl;
        arg_count = g_interp_tail.arg_count;
        args = g_interp_tail.args;
        g_interp_tail.pending = false;
        ret = proto->uses_boxes ? InterpreterExecuteT<true>(s, proto, args, arg_count, cl) : InterpreterExecuteT<false>(s, proto, args, arg_count, cl);
    }
    return ret;
}

extern "C" __attribute__((used)) CVar FakeluaInterpCall(State *state, VarClosure *cl, int arg_num, const CVar *args) {
    return GuardJitEntry(state, [&]() -> CVar {
        if (!IsInterpClosure(cl) || !cl->func_ptr) {
            ThrowFakeluaException("attempt to call a non-function value");
        }
        return InterpreterExecute(state, AsInterpProto(cl->func_ptr), args, arg_num, cl);
    });
}

}// namespace fakelua
