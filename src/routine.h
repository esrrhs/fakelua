#pragma once

#include "types.h"
#include "variant.h"
#include "interpreter.h"

struct fuck;
class fkerrorinfo;
class binary;
class paramstack;
class routine
{
public:
    routine(fuck * fk, fkerrorinfo * ei) : m_fk(fk), m_ei(ei), m_interpreter(fk, ei)
    {
        clear();
    }
    ~routine()
    {
    }

    void clear();

    void entry(binary * bin, const String & func, paramstack * ps);
    const variant & getret() const;

    bool isend() const;
    
    int run(int cmdnum);

private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    binary * m_bin;
    String m_func;
    paramstack * m_ps;
    variant m_ret;
    // ½âÊÍÆ÷
    interpreter m_interpreter;
};

