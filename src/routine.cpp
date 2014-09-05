#include "routine.h"
#include "fuck.h"

void routine::clear()
{
    m_ret = variant(m_fk);
    m_cmdpos = 0;
    m_isend = false;
}

const variant & routine::getret() const
{
    return m_ret;
}

bool routine::isend() const
{
    return m_isend;
}

int routine::run(int cmdnum)
{
    int curcmdnum = 0;

    for (int i = 0; i < cmdnum && !isend(); i++)
    {
        runcmd();
        curcmdnum++;
    }
    
    return curcmdnum;
}    

void routine::runcmd()
{
    // TODO
}
