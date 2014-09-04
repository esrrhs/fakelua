#include "codegen.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "variant.h"

int codegen::getconst(const String & name, bool isstr)
{
	variant v;
	if (isstr)
	{
		v = variant(name, m_fk);
	}
	else
	{
		v = variant(fkatol(&name), m_fk);
	}
	
	for (int i = 0; i < (int)m_const_list.size(); i++)
	{
		variant & vv = m_const_list[i];
		if (vv == v)
		{
			return i;
		}
	}

	int pos = m_const_list.size();
	m_const_list.push_back(v);
	return pos;
}

void codegen::output(String name, func_binary * bin)
{
    bin->m_name = name;
    bin->m_const_list = m_const_list;
    bin->m_size = m_byte_code_list.size();
    bin->m_buff = (command *)m_fk->m_fkmalloc(bin->m_size * sizeof(command));
    memcpy(bin->m_buff, &m_byte_code_list[0], bin->m_size * sizeof(command));
}
