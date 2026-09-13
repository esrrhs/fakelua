#include "interp/interpreter.h"
#include "interp/func_proto.h"
#include "interp/runtime.h"
#include "jit/jit_error_boundary.h"
#include "jit/vm.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/exception.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include <vector>

namespace fakelua {

namespace {

std::string ConstToName(const CVar &c) {
    if (c.type_ == static_cast<int>(VarType::StringId)) {
        return std::string(ConstString::GetString(c.data_.i));
    }
    if (c.type_ == static_cast<int>(VarType::String) && c.data_.s) {
        return std::string(c.data_.s->Str());
    }
    return {};
}

CVar LookupGlobal(FuncProto *proto, const std::string &name) {
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

void SetGlobal(FuncProto *proto, const std::string &name, CVar v) {
    // 文件级 local 表会打 CONST_FLAG，只禁止 SETTABLE 改内容。
    // 重绑定（g = {x=1}）对齐 CGen 对 static CVar 的直接赋值，必须放行。
    proto->unit->globals[name] = v;
}

}// namespace

CVar InterpreterExecute(State *s, FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl) {
    if (!proto) {
        ThrowFakeluaException("interpreter: null function prototype");
    }
    const int max_stack = std::max(proto->max_stack, proto->param_count);
    std::vector<CVar> stack(static_cast<size_t>(max_stack) + 8);
    std::vector<CVar *> boxes(static_cast<size_t>(max_stack) + 8, nullptr);
    auto write = [&](int r, CVar v) {
        stack[static_cast<size_t>(r)] = v;
        if (boxes[static_cast<size_t>(r)]) {
            *boxes[static_cast<size_t>(r)] = v;
        }
    };
    auto read = [&](int r) -> CVar {
        if (boxes[static_cast<size_t>(r)]) return *boxes[static_cast<size_t>(r)];
        return stack[static_cast<size_t>(r)];
    };

    for (int i = 0; i < proto->param_count; ++i) {
        stack[static_cast<size_t>(i)] = (i < arg_count && args) ? args[i] : interp_rt::Nil();
    }

    int pc = 0;
    const auto &code = proto->code;
    using Op = Op;
    while (pc < static_cast<int>(code.size())) {
        const Inst inst = code[static_cast<size_t>(pc++)];
        switch (inst.op) {
            case Op::MOVE:
                write(inst.a, read(inst.b));
                break;
            case Op::LOADK:
                write(inst.a, proto->constants[static_cast<size_t>(inst.sbx)]);
                break;
            case Op::LOADNIL:
                write(inst.a, interp_rt::Nil());
                break;
            case Op::LOADBOOL:
                write(inst.a, interp_rt::Bool(inst.b != 0));
                break;
            case Op::GETGLOBAL: {
                const std::string name = ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]);
                write(inst.a, LookupGlobal(proto, name));
                break;
            }
            case Op::SETGLOBAL: {
                const std::string name = ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]);
                SetGlobal(proto, name, read(inst.a));
                break;
            }
            case Op::GETUPVAL:
                if (!cl || inst.b >= static_cast<uint16_t>(cl->upvalue_count) || !cl->upvalues[inst.b]) {
                    write(inst.a, interp_rt::Nil());
                } else {
                    write(inst.a, *cl->upvalues[inst.b]);
                }
                break;
            case Op::SETUPVAL:
                if (cl && inst.b < static_cast<uint16_t>(cl->upvalue_count) && cl->upvalues[inst.b]) {
                    *cl->upvalues[inst.b] = read(inst.a);
                }
                break;
            case Op::GETTABLE:
                write(inst.a, interp_rt::GetTable(s, read(inst.b), read(inst.c)));
                break;
            case Op::SETTABLE:
                interp_rt::SetTable(s, read(inst.a), read(inst.b), read(inst.c));
                break;
            case Op::NEWTABLE:
                write(inst.a, interp_rt::NewTable(s));
                break;
            case Op::NEWBOX: {
                auto *box = static_cast<CVar *>(interp_rt::Alloc(s, sizeof(CVar)));
                *box = read(inst.a);
                boxes[inst.a] = box;
                break;
            }
            case Op::ADD:
                write(inst.a, interp_rt::BinAdd(read(inst.b), read(inst.c)));
                break;
            case Op::SUB:
                write(inst.a, interp_rt::BinSub(read(inst.b), read(inst.c)));
                break;
            case Op::MUL:
                write(inst.a, interp_rt::BinMul(read(inst.b), read(inst.c)));
                break;
            case Op::DIV:
                write(inst.a, interp_rt::BinDiv(read(inst.b), read(inst.c)));
                break;
            case Op::IDIV:
                write(inst.a, interp_rt::BinIdiv(read(inst.b), read(inst.c)));
                break;
            case Op::MOD:
                write(inst.a, interp_rt::BinMod(read(inst.b), read(inst.c)));
                break;
            case Op::POW:
                write(inst.a, interp_rt::BinPow(read(inst.b), read(inst.c)));
                break;
            case Op::BAND:
                write(inst.a, interp_rt::BinBand(read(inst.b), read(inst.c)));
                break;
            case Op::BXOR:
                write(inst.a, interp_rt::BinBxor(read(inst.b), read(inst.c)));
                break;
            case Op::BOR:
                write(inst.a, interp_rt::BinBor(read(inst.b), read(inst.c)));
                break;
            case Op::SHL:
                write(inst.a, interp_rt::BinShl(read(inst.b), read(inst.c)));
                break;
            case Op::SHR:
                write(inst.a, interp_rt::BinShr(read(inst.b), read(inst.c)));
                break;
            case Op::CONCAT:
                write(inst.a, interp_rt::BinConcat(s, read(inst.b), read(inst.c)));
                break;
            case Op::UNM:
                write(inst.a, interp_rt::UnMinus(read(inst.b)));
                break;
            case Op::NOT:
                write(inst.a, interp_rt::UnNot(read(inst.b)));
                break;
            case Op::LEN:
                write(inst.a, interp_rt::UnLen(read(inst.b)));
                break;
            case Op::BNOT:
                write(inst.a, interp_rt::UnBnot(read(inst.b)));
                break;
            case Op::EQ:
                write(inst.a, interp_rt::Bool(interp_rt::IsEqual(read(inst.b), read(inst.c))));
                break;
            case Op::LT:
                write(inst.a, interp_rt::CmpLt(read(inst.b), read(inst.c)));
                break;
            case Op::LE:
                write(inst.a, interp_rt::CmpLe(read(inst.b), read(inst.c)));
                break;
            case Op::TESTJMP:
                if (interp_rt::IsTrue(read(inst.a)) == (inst.b != 0)) {
                    pc += inst.sbx;
                }
                break;
            case Op::JMP:
                pc += inst.sbx;
                break;
            case Op::CALL: {
                CVar fn = read(inst.a);
                if (fn.type_ != static_cast<int>(VarType::Closure) || !fn.data_.cl) {
                    ThrowFakeluaException("attempt to call a non-function value");
                }
                CVar call_args[kMaxFunctionInputParams];
                const int nargs = inst.b;
                int nflat = 0;
                for (int i = 0; i < nargs && nflat < static_cast<int>(kMaxFunctionInputParams); ++i) {
                    CVar av = read(inst.a + 1 + i);
                    if (i == nargs - 1 && av.type_ == static_cast<int>(VarType::Multi)) {
                        VarMulti *m = av.data_.m;
                        for (uint32_t j = 0; j < m->GetCount() && nflat < static_cast<int>(kMaxFunctionInputParams); ++j) {
                            call_args[nflat++] = m->GetVars()[j];
                        }
                    } else {
                        call_args[nflat++] = interp_rt::UnboxMulti(av, 0);
                    }
                }
                CVar ret = inter::DispatchCallClosure(s, fn.data_.cl, call_args, nflat, JIT_INTERP);
                write(inst.a, inst.c == 0 ? ret : interp_rt::UnboxMulti(ret, 0));
                break;
            }
            case Op::CALLNAME: {
                const std::string name = ConstToName(proto->constants[static_cast<size_t>(inst.sbx)]);
                CVar call_args[kMaxFunctionInputParams];
                const int nargs = inst.b;
                for (int i = 0; i < nargs; ++i) {
                    call_args[i] = read(inst.a + i);
                }
                CVar ret = CallByNameArgs(s, JIT_INTERP, name.c_str(), nargs, call_args);
                write(inst.a, inst.c == 0 ? ret : interp_rt::UnboxMulti(ret, 0));
                break;
            }
            case Op::RETURN: {
                CVar ret = read(inst.a);
                return inst.b == 0 ? ret : interp_rt::UnboxMulti(ret, 0);
            }
            case Op::CLOSURE: {
                FuncProto *child = proto->child_protos[static_cast<size_t>(inst.sbx)];
                const int nup = static_cast<int>(child->upvalues.size());
                CVar *upbufs[kMaxFunctionInputParams];
                for (int i = 0; i < nup; ++i) {
                    const auto &uv = child->upvalues[static_cast<size_t>(i)];
                    if (uv.in_stack) {
                        if (!boxes[uv.idx]) {
                            auto *box = static_cast<CVar *>(interp_rt::Alloc(s, sizeof(CVar)));
                            *box = read(uv.idx);
                            boxes[uv.idx] = box;
                        }
                        upbufs[i] = boxes[uv.idx];
                    } else {
                        upbufs[i] = (cl && uv.idx < static_cast<uint16_t>(cl->upvalue_count)) ? cl->upvalues[uv.idx] : nullptr;
                    }
                }
                write(inst.a, interp_rt::MakeClosure(s, TagInterpProto(child), nup, child->param_count, child->is_vararg, upbufs));
                break;
            }
            case Op::SETLIST:
                interp_rt::TableExpandMulti(s, read(inst.a), inst.b, read(inst.c));
                break;
            case Op::TABENT: {
                CVar idxv = read(inst.sbx);
                uint32_t idx = 0;
                if (idxv.type_ == static_cast<int>(VarType::Int) && idxv.data_.i >= 0) {
                    idx = static_cast<uint32_t>(idxv.data_.i);
                }
                CVar k, v;
                if (!interp_rt::TableEntry(read(inst.a), idx, k, v)) {
                    k = interp_rt::Nil();
                    v = interp_rt::Nil();
                }
                write(inst.b, k);
                write(inst.c, v);
                break;
            }
            case Op::TABCOUNT:
                write(inst.a, interp_rt::Int(static_cast<int64_t>(interp_rt::TableEntryCount(read(inst.b)))));
                break;
            case Op::UNBOX:
                write(inst.a, interp_rt::UnboxMulti(read(inst.b), inst.c));
                break;
            case Op::MAKEMULTI: {
                const int n = inst.b;
                std::vector<CVar> vals(static_cast<size_t>(n));
                for (int i = 0; i < n; ++i) {
                    vals[static_cast<size_t>(i)] = read(inst.a + i);
                }
                write(inst.a, interp_rt::MakeMulti(s, vals.data(), static_cast<uint32_t>(n)));
                break;
            }
            case Op::COMBINE: {
                const int nprefix = inst.b;
                std::vector<CVar> prefix(static_cast<size_t>(nprefix));
                for (int i = 0; i < nprefix; ++i) {
                    prefix[static_cast<size_t>(i)] = read(inst.a + i);
                }
                write(inst.a, interp_rt::CombineMulti(s, prefix.data(), static_cast<uint32_t>(nprefix), read(inst.a + nprefix)));
                break;
            }
            case Op::THROW: {
                const CVar msg = proto->constants[static_cast<size_t>(inst.sbx)];
                ThrowFakeluaException(ConstToName(msg));
            }
            case Op::FORCHECK:
                write(inst.a, interp_rt::Bool(interp_rt::ForStepPositive(read(inst.b))));
                break;
            case Op::FORADVANCE: {
                CVar ctrl = read(inst.a);
                CVar step = read(inst.b);
                if (ctrl.type_ == static_cast<int>(VarType::Int) && step.type_ == static_cast<int>(VarType::Int)) {
                    int64_t v = ctrl.data_.i;
                    if (!interp_rt::ForIntAdvance(&v, step.data_.i)) {
                        pc += inst.sbx;
                        break;
                    }
                    ctrl.data_.i = v;
                    write(inst.a, ctrl);
                } else {
                    write(inst.a, interp_rt::BinAdd(ctrl, step));
                }
                break;
            }
            default:
                ThrowFakeluaException("interpreter: unknown opcode");
        }
    }
    return interp_rt::Nil();
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
