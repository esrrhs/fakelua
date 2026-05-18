#include "compile/compile_common.h"

namespace fakelua {

InferredType InferExpType(const SyntaxTreeInterfacePtr &exp, const InferExpContext &ctx) {
    if (!exp || exp->Type() != SyntaxTreeType::Exp) {
        return T_DYNAMIC;
    }
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();

    if (exp_kind == ExpKind::kNumber) {
        const auto node_type = ctx.lookup_node(e.get());
        if (node_type == T_INT || node_type == T_FLOAT) {
            return node_type;
        }
        // 回退：解析字面量字符串以确定整数/浮点类型。
        const auto &val = e->ExpValue();
        if (val.find('.') == std::string::npos &&
            val.find('e') == std::string::npos &&
            val.find('E') == std::string::npos) {
            return T_INT;
        }
        return T_FLOAT;
    }

    if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (!pe) {
            return T_DYNAMIC;
        }

        if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            if (!var || var->GetVarKind() != VarKind::kSimple) {
                return T_DYNAMIC;
            }
            // parent_exp_node = e.get()（即外层 kPrefixExp 的 Exp 节点），
            // 供 EvalReturnExpType 适配器在 spec_ctx 未命中时回退到快照查找。
            return ctx.lookup_var(var->GetName(), e.get());
        }

        if (pe->GetPrefixKind() == PrefixExpKind::kExp) {
            return InferExpType(pe->GetValue(), ctx);
        }

        if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
            if (!fc) {
                return T_DYNAMIC;
            }
            const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
            if (!callee_pe || callee_pe->GetPrefixKind() != PrefixExpKind::kVar) {
                return T_DYNAMIC;
            }
            const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
            if (!callee_var || callee_var->GetVarKind() != VarKind::kSimple) {
                return T_DYNAMIC;
            }
            const auto &callee_name = callee_var->GetName();
            const auto *math_params = ctx.lookup_math_params(callee_name);
            if (!math_params || math_params->empty()) {
                return T_DYNAMIC;
            }
            const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
            if (!args_ptr) {
                return T_DYNAMIC;
            }
            const auto raw_args = ExtractCallRawArgs(args_ptr);
            int bitmask = 0;
            for (int i = 0; i < static_cast<int>(math_params->size()); ++i) {
                const int param_pos = (*math_params)[static_cast<size_t>(i)];
                if (param_pos >= static_cast<int>(raw_args.size())) {
                    return T_DYNAMIC;
                }
                const auto &arg = raw_args[static_cast<size_t>(param_pos)];
                if (!arg || arg->Type() != SyntaxTreeType::Exp) {
                    return T_DYNAMIC;
                }
                const auto t = InferExpType(arg, ctx);
                if (t == T_DYNAMIC) {
                    return T_DYNAMIC;
                }
                if (t == T_FLOAT) {
                    bitmask |= (1 << i);
                }
            }
            return ctx.lookup_return(callee_name, bitmask);
        }

        return T_DYNAMIC;
    }

    if (exp_kind == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
        if (!op) {
            return T_DYNAMIC;
        }
        const auto lt = InferExpType(e->Left(), ctx);
        const auto rt = InferExpType(e->Right(), ctx);
        return InferNumericBinopResultType(op->GetOpKind(), lt, rt);
    }

    if (exp_kind == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
        if (!op) {
            return T_DYNAMIC;
        }
        if (op->GetOpKind() == UnOpKind::kMinus) {
            return InferExpType(e->Right(), ctx);
        }
        if (op->GetOpKind() == UnOpKind::kBitNot) {
            const auto t = InferExpType(e->Right(), ctx);
            if (t == T_INT) {
                return T_INT;
            }
            return T_DYNAMIC;
        }
        if (op->GetOpKind() == UnOpKind::kNumberSign) {
            // # 运算符始终返回整数（字符串字节数或表元素数）。
            return T_INT;
        }
        return T_DYNAMIC;
    }

    return T_DYNAMIC;
}

}// namespace fakelua
