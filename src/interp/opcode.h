#pragma once

#include <cstdint>

namespace fakelua {

// 寄存器字节码指令。布局参考 fakescript 的三地址形式，操作数以寄存器槽为主。
enum class Op : uint8_t {
    MOVE,     // R(A) = R(B)
    LOADK,    // R(A) = K(Bx)
    LOADNIL,  // R(A) = nil
    LOADBOOL, // R(A) = (B != 0)
    GETGLOBAL,// R(A) = G[Kstr(Bx)]
    SETGLOBAL,// G[Kstr(Bx)] = R(A)
    GETUPVAL, // R(A) = UV(B)
    SETUPVAL, // UV(B) = R(A)
    GETTABLE, // R(A) = R(B)[R(C)]
    SETTABLE, // R(A)[R(B)] = R(C)
    NEWTABLE, // R(A) = {}
    NEWBOX,   // boxes[A] = alloc CVar*; *boxes[A] = R(A)
    ADD,
    SUB,
    MUL,
    DIV,
    IDIV,
    MOD,
    POW,
    BAND,
    BXOR,
    BOR,
    SHL,
    SHR,
    CONCAT, // R(A) = R(B) .. R(C)
    UNM,    // R(A) = -R(B)
    NOT,    // R(A) = not R(B)
    LEN,    // R(A) = #R(B)
    BNOT,   // R(A) = ~R(B)
    EQ,     // R(A) = (R(B) == R(C))
    LT,
    LE,
    TESTJMP, // if IsTrue(R(A)) == (B!=0) then pc += sbx
    JMP,     // pc += sbx
    CALL,    // R(A) = call R(A) with B args at R(A+1)..; C=0 keep multi, C=1 unbox
    CALLNAME,// R(A) = CallByName(Kstr(Bx), B args at R(A)..); C as CALL
    RETURN,  // return R(A); B=0 keep multi, B=1 single
    CLOSURE,   // R(A) = closure(proto Bx)
    SETLIST,   // expand multi R(C) into table R(A) starting at int B
    TABENT,    // R(B), R(C) = table_entry(R(A), Bx); sbx unused
    TABCOUNT,  // R(A) = Int(table_entry_count(R(B)))
    UNBOX,     // R(A) = UnboxMulti(R(B), C)
    MAKEMULTI, // R(A) = MakeMulti(R(A) .. R(A+B-1))
    COMBINE,   // R(A) = CombineMulti(prefix R(A)..R(A+B-1), last R(A+B))
    THROW,     // throw string constant K(sbx)
    FORCHECK,  // R(A) = Bool(step>0); validate R(B) as numeric for-step
    FORADVANCE,// R(A) += R(B); if int overflow, pc += sbx
};

struct Inst {
    Op op = Op::MOVE;
    uint16_t a = 0;
    uint16_t b = 0;
    uint16_t c = 0;
    int32_t sbx = 0;
};

struct UpvalDesc {
    bool in_stack = false; // true: enclosing function register; false: enclosing upvalue
    uint16_t idx = 0;
};

}// namespace fakelua
