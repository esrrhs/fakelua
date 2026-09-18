#pragma once

#include "fakelua.h"
#include "interp/opcode.h"
#include "jit/jit_common.h"
#include "var/var_closure.h"
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

struct FuncProto;

// 一次编译单元：持有全部函数原型与文件级全局槽，生命周期绑定到 VmFunction 的 handle。
struct TransparentStringHash {
    using is_transparent = void;
    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
    std::size_t operator()(const std::string &s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
};

struct InterpUnit final : JITHandle {
    std::vector<std::unique_ptr<FuncProto>> protos;
    std::unordered_map<std::string, CVar, TransparentStringHash, std::equal_to<>> globals;
    std::unordered_set<std::string> const_global_names;
    FuncProto *init_proto = nullptr;
    State *state = nullptr;
};

struct FuncProto {
    InterpUnit *unit = nullptr;
    std::string name;
    int param_count = 0;
    bool is_vararg = false;
    int max_stack = 0;
    // 本函数有被捕获的局部（NEWBOX / CLOSURE in_stack）。无捕获时解释器走纯栈寄存器路径。
    bool uses_boxes = false;
    std::vector<Inst> code;
    std::vector<CVar> constants;
    std::vector<UpvalDesc> upvalues;
    std::vector<FuncProto *> child_protos;
    std::vector<int> lineinfo;
};

inline const char *const kInterpClosureMagic = reinterpret_cast<const char *>(static_cast<uintptr_t>(1));

inline void *TagInterpProto(FuncProto *p) {
    return p;
}

inline bool IsInterpClosure(const VarClosure *cl) {
    // code_str == 1 是解释器闭包标记。func_ptr 必须同时指向 FuncProto，
    // 避免 arena 复用后 native 方法闭包未清 code_str 时被误判（进而
    // 把 NativeMethodBridge 当成字节码入口）。
    return cl != nullptr && cl->code_str == kInterpClosureMagic && cl->func_ptr != nullptr;
}

inline FuncProto *AsInterpProto(void *p) {
    return reinterpret_cast<FuncProto *>(p);
}

}// namespace fakelua
