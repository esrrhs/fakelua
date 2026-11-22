#pragma once

#include "fakelua.h"
#include "interp/func_proto.h"

#include <cstddef>
#include <vector>

namespace fakelua {

class State;

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

// 解释器单 State 运行期执行状态（寄存器栈、box 指针表、栈顶与尾调用 trampoline 槽）。
// 挂在 State 上随实例创建、Reset 与销毁，避免多个 State 在同一线程上共享或残留状态。
struct InterpState {
    std::vector<CVar> stack;
    std::vector<CVar *> boxes;
    size_t top = 0;
    int boxed_frames = 0;
    InterpTailJob tail;

    void Reset() {
        stack.clear();
        boxes.clear();
        top = 0;
        boxed_frames = 0;
        tail.pending = false;
        tail.proto = nullptr;
        tail.cl = nullptr;
        tail.arg_count = 0;
    }
};

// 解释执行一个函数原型。args 已按形参加减（与 DispatchCall 一致）。
CVar InterpreterExecute(State *s, FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl);

// JIT 生成的 FlCallClosure 在碰到解释器闭包时走这条 C 入口。
extern "C" CVar FakeluaInterpCall(State *state, VarClosure *cl, int arg_num, const CVar *args);

}// namespace fakelua
