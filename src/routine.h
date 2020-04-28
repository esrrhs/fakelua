#pragma once

#include "types.h"
#include "variant.h"
#include "interpreter.h"

struct fake;

class binary;

struct paramstack;
struct routine {
    fake *m_fk;
    int m_id;
    // ½âÊÍÆ÷
    interpreter m_interpreter;
};

#define ROUTINE_INI(rou, fk, id) (rou).m_fk = fk;\
    (rou).m_id = id;\
    INTER_INI((rou).m_interpreter, fk)

#define ROUTINE_ID(rou) ((rou).m_id)

#define ROUTINE_DELETE(rou) INTER_DELETE((rou).m_interpreter)

#define ROUTINE_CLEAR(rou) INTER_CLEAR((rou).m_interpreter)

#define ROUTINE_ENTRY(rou, func, retnum, retpos) (rou).m_interpreter.call(func, retnum, retpos)

#define ROUTINE_ISEND(rou) (rou).m_interpreter.isend()

#define ROUTINE_RUN(rou, cmdnum) (rou).m_interpreter.run(cmdnum)

#define ROUTINE_GETRET(rou) (rou).m_interpreter.getret()

#define ROUTINE_SET_PRO(rou, pro) INTER_SET_PRO((rou).m_interpreter, pro)
