#include "native.h"
#include "fake.h"

String func_native::dump() const
{
    String ret;

    // Ãû×Ö
    ret += "\n[";
    ret += m_name;
    ret += "]\n";

    ret += "\n\t////// code ";
    ret += fkitoa(m_size);
    ret += " //////";
    // code
    for (int i = 0; i < (int)m_size; i++)
    {
        if (i % 8 == 0)
        {
            ret += "\n\t[";
            ret += fkitoa(i);
            ret += "]\t";
        }
        unsigned char c = m_buff[i];
        ret += fkxtoa(c, 2);
        ret += ", ";
    }
    ret += "\n\n";
    return ret;
}

String native::dump() const
{
    String ret;
    for (const stringhashmap::ele * p = m_shh.first(); p != 0; p = m_shh.next())
    {
        const func_native & nt = *p->t;
        ret += nt.dump();
    }
    return ret;
}

