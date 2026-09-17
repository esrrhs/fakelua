#pragma once

#include "fakelua.h"
#include "interp/vm_ops.h"

namespace fakelua {

class State;
struct VarClosure;

namespace interp_rt {

bool IsEqual(const CVar &a, const CVar &b);

CVar BinConcat(State *s, const CVar &a, const CVar &b);
CVar UnLen(const CVar &a);

CVar GetTable(State *s, CVar t, CVar k);
void SetTable(State *s, CVar t, CVar k, CVar v);
CVar NewTable(State *s);
void TableExpandMulti(State *s, CVar t, int64_t start_idx, CVar v);
bool TableEntry(CVar t, uint32_t idx, CVar &k, CVar &v);
uint32_t TableEntryCount(CVar t);

CVar CombineMulti(State *s, const CVar *prefix, uint32_t prefix_count, CVar last);
CVar MakeMulti(State *s, const CVar *vals, uint32_t count);

CVar MakeClosure(State *s, void *func_ptr, int upvalue_count, int expected_arg_count, bool is_vararg, CVar **upvals);

void *Alloc(State *s, size_t size);

}// namespace interp_rt

CVar InterpreterExecute(State *s, struct FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl);

}// namespace fakelua
