#include "bindfunc.h"
#include "fake.h"
#include "fakescript.h"

// print, very slow
void buildinprint(fake * fk)
{
    String str;
    for (int i = 0; i < (int)fk->ps.m_variant_list_num; i++)
    {
        str += vartostring(&fk->ps.m_variant_list[i]);
    }
    // printf
    printf("%s\n", str.c_str());
    // ret
    fkpspush<int>(fk, (int)str.size());
}

void buildinfunc::inifunc()
{
    m_shh.add("print", buildinprint);
}

bool buildinfunc::call(const char * name)
{
    if (m_shh.empty())
    {
        inifunc();
    }

    bifunc * p = m_shh.get(name);
    if (!p)
    {
        return false;
    }
    (*p)(m_fk);
    return true;
}
