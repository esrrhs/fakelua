#pragma once

#include <cstdint>

namespace fakelua {

// 寄存器字节码指令。布局参考 fakescript 的三地址形式，操作数以寄存器槽为主。
// X-macro 同时驱动 enum 与解释器 computed-goto 跳转表，新增 opcode 必须加在这里。
#define FAKELUA_FOR_EACH_OP(X) \
    X(MOVE)      /* R(A) = R(B) */ \
    X(LOADK)     /* R(A) = K(Bx) */ \
    X(LOADNIL)   /* R(A) = nil */ \
    X(LOADBOOL)  /* R(A) = (B != 0) */ \
    X(GETGLOBAL) /* R(A) = G[Kstr(Bx)] */ \
    X(SETGLOBAL) /* G[Kstr(Bx)] = R(A) */ \
    X(GETUPVAL)  /* R(A) = UV(B) */ \
    X(SETUPVAL)  /* UV(B) = R(A) */ \
    X(GETTABLE)  /* R(A) = R(B)[R(C)] */ \
    X(SETTABLE)  /* R(A)[R(B)] = R(C) */ \
    X(NEWTABLE)  /* R(A) = {} */ \
    X(NEWBOX)    /* boxes[A] = alloc CVar*; *boxes[A] = R(A) */ \
    X(ADD) \
    X(SUB) \
    X(MUL) \
    X(DIV) \
    X(IDIV) \
    X(MOD) \
    X(POW) \
    X(BAND) \
    X(BXOR) \
    X(BOR) \
    X(SHL) \
    X(SHR) \
    X(CONCAT)    /* R(A) = R(B) .. R(C) */ \
    X(UNM)       /* R(A) = -R(B) */ \
    X(NOT)       /* R(A) = not R(B) */ \
    X(LEN)       /* R(A) = #R(B) */ \
    X(BNOT)      /* R(A) = ~R(B) */ \
    X(EQ)        /* R(A) = (R(B) == R(C)) */ \
    X(LT) \
    X(LE) \
    X(TESTJMP)   /* if IsTrue(R(A)) == (B!=0) then pc += sbx */ \
    X(JMP)       /* pc += sbx */ \
    X(CALL)      /* R(A) = call R(A) with B args at R(A+1)..; C=0 keep multi, C=1 unbox */ \
    X(CALLNAME)  /* R(A) = CallByName(Kstr(Bx), B args at R(A)..); C as CALL */ \
    X(RETURN)    /* return R(A); B=0 keep multi, B=1 single */ \
    X(CLOSURE)   /* R(A) = closure(proto Bx) */ \
    X(SETLIST)   /* expand multi R(C) into table R(A) starting at int sbx */ \
    X(TABENT)    /* R(B), R(C) = table_entry(R(A), R(sbx)) */ \
    X(TABCOUNT)  /* R(A) = Int(table_entry_count(R(B))) */ \
    X(UNBOX)     /* R(A) = UnboxMulti(R(B), C) */ \
    X(MAKEMULTI) /* R(A) = MakeMulti(R(A) .. R(A+B-1)) */ \
    X(COMBINE)   /* R(A) = CombineMulti(prefix R(A)..R(A+B-1), last R(A+B)) */ \
    X(THROW)     /* throw string constant K(sbx) */ \
    X(FORPREP)   /* int: R(A+1)=count; float: keep limit; skip if empty (pc+=sbx); R(A+3)=R(A) */ \
    X(FORLOOP)   /* int: if --count>=0: R(A)+=step, R(A+3)=R(A), pc+=sbx; float: add+compare */

enum class Op : uint8_t {
#define FAKELUA_OP_ENUM(name) name,
    FAKELUA_FOR_EACH_OP(FAKELUA_OP_ENUM)
#undef FAKELUA_OP_ENUM
    COUNT
};

struct Inst {
    Op op = Op::MOVE;
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    int32_t sbx = 0;
};
static_assert(sizeof(Inst) == 8, "Inst should be 8 bytes for denser dispatch");

struct UpvalDesc {
    bool in_stack = false; // true: enclosing function register; false: enclosing upvalue
    uint16_t idx = 0;
};

}// namespace fakelua
