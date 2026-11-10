#include "interp/codegen.h"

#include "interp/interpreter.h"
#include "jit/vm.h"
#include "state/state.h"
#include "util/exception.h"

namespace fakelua {

InterpCodegen::InterpCodegen(State *s) : FunctionRegisterUsage(s) {
}

void InterpCodegen::RegisterAndInit() {
    if (!unit_hold_ || !unit_) {
        ThrowFakeluaException("interpreter: RegisterAndInit called before Build");
    }

    const auto handle = std::static_pointer_cast<JITHandle>(unit_hold_);
    for (const auto &func: all_funcs_) {
        if (func->parent != nullptr || !func->proto) continue;
        const std::string &c_name = func->unique_c_name;
        const std::string &lua_name = (!func->name.empty()) ? func->name : c_name;
        void *addr = TagInterpProto(func->proto);
        s_->GetVM().RegisterFunction(VmFunction(lua_name, func->proto->param_count, JIT_INTERP, addr, handle, func->is_vararg));
        if (!cur_package_name_.empty() && c_name != kInitFunctionName && !func->name.empty()) {
            const std::string pkg = cur_package_name_ + "." + func->name;
            s_->GetVM().RegisterFunction(VmFunction(pkg, func->proto->param_count, JIT_INTERP, addr, handle, func->is_vararg));
        }
        if (c_name == kInitFunctionName) {
            unit_->init_proto = func->proto;
        }
    }

    if (unit_->init_proto) {
        // ConstAllocScope：init 期间 NewTable 创建即带 CONST_FLAG（与 JIT SET_TABLE 对齐）。
        State::ConstAllocScope const_alloc(s_);
        InterpreterExecute(s_, unit_->init_proto, nullptr, 0, nullptr);
    }
}

void InterpCodegen::Generate(const ParseResult &pr, const AnalysisResult &ar, const CompileConfig &cfg) {
    if (!cfg.disable_jit[JIT_INTERP]) {
        Build(pr, ar);
        RegisterAndInit();
    } else {
        // 高峰检查归属语义分析；此便捷路径在未走 Compiler::Compile 时仍要门禁。
        CheckFunctionRegisterLimits(pr, ar);
    }
}


}// namespace fakelua
