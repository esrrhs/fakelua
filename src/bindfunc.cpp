#include "bindfunc.h"
#include "fake.h"

void bindfunc::addfunc(const variant & name, const fkfunctor & ff)
{
	if (m_fk->fm.get_func(name) && m_fk->fm.get_func(name)->haveff)
    {
		seterror(m_fk, efk_reg_memfunc_double_name, "addfunc double name %s", vartostring(&name).c_str());
		FKERR("addfunc double name %s", vartostring(&name).c_str());
        return;
    }

	m_fk->fm.add_bind_func(name, ff);

	FKLOG("add bind func %s", vartostring(&name).c_str());
}
