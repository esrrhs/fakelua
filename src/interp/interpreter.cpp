#include "interp/interpreter.h"
#include "interp/func_proto.h"
#include "interp/runtime.h"
#include "jit/jit_error_boundary.h"
#include "jit/vm.h"
#include "native/table/native_table.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/exception.h"
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
    // 文件级 local 表会打 CONST_FLAG，只禁止 SETTABLE 改内容。
    // 重绑定（g = {x=1}）对齐 CGen 对 static CVar 的直接赋值，必须放行。
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
        CVar ret = inter::DispatchCallClosure(s, fn.data_.cl, call_args, nflat, JIT_INTERP);
        VM_W(inst.a, inst.c == 0 ? ret : interp_rt::UnboxMulti(ret, 0));
        VM_DISPATCH_NEXT();
    }
    VM_CASE(CALLNAME) {
        // std::string 必须在 computed-goto 之前析构。
        {
            const std::string name(ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]));
            CVar call_args[kMaxFunctionInputParams];
            const int nargs = inst.b;
            if (nargs < 0 || nargs > static_cast<int>(kMaxFunctionInputParams)) {
                ThrowFakeluaException(std::format("interpreter: too many arguments ({}) passed for function '{}', max is {}", nargs, name, kMaxFunctionInputParams));
            }
            for (int i = 0; i < nargs; ++i) {
                call_args[i] = VM_R(inst.a + i);
            }
            CVar ret = CallByNameArgs(s, JIT_INTERP, name.c_str(), nargs, call_args);
            VM_W(inst.a, inst.c == 0 ? ret : interp_rt::UnboxMulti(ret, 0));
        }
        VM_DISPATCH_NEXT();
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
    if (proto->uses_boxes) {
        return InterpreterExecuteT<true>(s, proto, args, arg_count, cl);
    }
    return InterpreterExecuteT<false>(s, proto, args, arg_count, cl);
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
