#pragma once

#include "fakelua.h"

namespace fakelua {

class State;
struct VarClosure;

namespace interp_rt {

CVar Nil();
CVar Bool(bool v);
CVar Int(int64_t v);
CVar Float(double v);

bool IsTrue(const CVar &v);
bool IsEqual(const CVar &a, const CVar &b);

void CheckNum(const CVar &v);
int64_t CheckInt(const CVar &v);
double ToDouble(const CVar &v);

CVar BinAdd(const CVar &a, const CVar &b);
CVar BinSub(const CVar &a, const CVar &b);
CVar BinMul(const CVar &a, const CVar &b);
CVar BinDiv(const CVar &a, const CVar &b);
CVar BinIdiv(const CVar &a, const CVar &b);
CVar BinMod(const CVar &a, const CVar &b);
CVar BinPow(const CVar &a, const CVar &b);
CVar BinBand(const CVar &a, const CVar &b);
CVar BinBxor(const CVar &a, const CVar &b);
CVar BinBor(const CVar &a, const CVar &b);
CVar BinShl(const CVar &a, const CVar &b);
CVar BinShr(const CVar &a, const CVar &b);
CVar BinConcat(State *s, const CVar &a, const CVar &b);
CVar UnMinus(const CVar &a);
CVar UnNot(const CVar &a);
CVar UnLen(const CVar &a);
CVar UnBnot(const CVar &a);
CVar CmpLt(const CVar &a, const CVar &b);
CVar CmpLe(const CVar &a, const CVar &b);

CVar GetTable(State *s, CVar t, CVar k);
void SetTable(State *s, CVar t, CVar k, CVar v);
CVar NewTable(State *s);
void TableExpandMulti(State *s, CVar t, int64_t start_idx, CVar v);
bool TableEntry(CVar t, uint32_t idx, CVar &k, CVar &v);
uint32_t TableEntryCount(CVar t);

CVar UnboxMulti(const CVar &v, uint32_t idx);
CVar CombineMulti(State *s, const CVar *prefix, uint32_t prefix_count, CVar last);
CVar MakeMulti(State *s, const CVar *vals, uint32_t count);

CVar MakeClosure(State *s, void *func_ptr, int upvalue_count, int expected_arg_count, bool is_vararg, CVar **upvals);

bool ForIntAdvance(int64_t *ctrl, int64_t step);
bool ForStepPositive(const CVar &step);

void *Alloc(State *s, size_t size);

}// namespace interp_rt

CVar InterpreterExecute(State *s, struct FuncProto *proto, const CVar *args, int arg_count, VarClosure *cl);

}// namespace fakelua
