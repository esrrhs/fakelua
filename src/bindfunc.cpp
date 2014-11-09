#include "bindfunc.h"
#include "fuck.h"

void bindfunc::addfunc(const char * name, const fkfunctor & ff)
{
    bindfuncele ele;
    ele.ff = ff;
    ele.callnum = 0;
    m_shh.add(name, ele);

    FKLOG("add bind func %s", name);
}
