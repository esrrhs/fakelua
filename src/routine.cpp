#include "routine.h"
#include "fuck.h"

void routine::clear()
{
    m_bin = 0;
    m_func.clear();
    m_ps = 0;
    m_ret = variant(m_fk);
    m_interpreter.clear();
}

void routine::entry(binary * bin, const String & func, paramstack * ps)
{
    m_bin = bin;
    m_func = func;
    m_ps = ps;
}
    
const variant & routine::getret() const
{
    return m_ret;
}

bool routine::isend() const
{
    return m_interpreter.isend();
}

int routine::run(int cmdnum)
{
    return m_interpreter.run(cmdnum);
}    

