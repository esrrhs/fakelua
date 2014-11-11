#include "bindfunc.h"
#include "fuck.h"

void bindfunc::addfunc(const char * name, const fkfunctor & ff)
{
    if (m_shh.get(name) != 0)
    {
        seterror(m_fk, efk_reg_memfunc_double_name, "addfunc double name %s", name);
        FKERR("addfunc double name %s", name);
        return;
    }
    
    bindfuncele ele;
    ele.ff = ff;
    ele.callnum = 0;
    m_shh.add(name, ele);

    FKLOG("add bind func %s", name);
}
