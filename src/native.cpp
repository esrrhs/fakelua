#include "native.h"
#include "fuck.h"

String func_native::dump() const
{
    String ret;

    // Ãû×Ö
    ret += "\n[";
    ret += m_name;
    ret += "]\n";

    ret += "\n\t////// code ";
    ret += fkitoa(m_size);
    ret += " //////\n\t";
    // code
    for (int i = 0; i < (int)m_size; i++)
    {
        unsigned char c = m_buff[i];
        ret += fkxtoa(c, 2);
        ret += ", ";
        if (i > 0 && i % 8 == 0)
        {
            ret += "\n\t";
        }
    }
    ret += "\n";
    return ret;
}

String native::dump() const
{
    String ret;
    for (int i = 0; i < (int)m_func_native_list.size(); i++)
    {
        const func_native & nt = m_func_native_list[i];
        ret += nt.dump();
    }
    return ret;
}

