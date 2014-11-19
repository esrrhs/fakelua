#pragma once

#include "types.h"
#include "variant.h"
#include "interpreter.h"

struct fake;
class binary;
class paramstack;
struct routine
{
    fake * m_fk;
    // ½âÊÍÆ÷
    interpreter m_interpreter;
};

#define ROUTINE_INI(rou, fk) (rou).m_fk = fk;\
    INTER_INI((rou).m_interpreter, fk)

#define ROUTINE_DELETE(rou) INTER_DELETE((rou).m_interpreter)

#define ROUTINE_CLEAR(rou) INTER_CLEAR((rou).m_interpreter)

#define ROUTINE_ENTRY(rou, bin, func, ps) (rou).m_interpreter.call(bin, func, ps)

#define ROUTINE_ISEND(rou) (rou).m_interpreter.isend()

#define ROUTINE_RUN(rou, cmdnum) (rou).m_interpreter.run(cmdnum)

#define ROUTINE_GETRET(rou) (rou).m_interpreter.getret()

