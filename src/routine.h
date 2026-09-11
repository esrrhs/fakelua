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
    // 解释器
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

#define ROUTINE_RUN(rou) (rou).m_interpreter.run()
#define ROUTINE_STEP(rou) (rou).m_interpreter.step()

#define ROUTINE_GETRET(rou) (rou).m_interpreter.getret()

#define ROUTINE_SET_PRO(rou, pro) INTER_SET_PRO((rou).m_interpreter, pro)
