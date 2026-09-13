#pragma once

#include "fakelua.h"
#include "interp/func_proto.h"

namespace fakelua {

class State;

// 解释执行一个函数原型。args 已按形参加减（与 DispatchCall 一致）。
CVar InterpreterExecute(State *s, FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl);

// JIT 生成的 FlCallClosure 在碰到解释器闭包时走这条 C 入口。
extern "C" CVar FakeluaInterpCall(State *state, VarClosure *cl, int arg_num, const CVar *args);

}// namespace fakelua
