#include "codegen.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "variant.h"

int codegen::add_stack_identifier(const String & name)
{
    assert(m_block_identifiers_stack.size() > 0);
    if (get_cur_variable_pos(name) != -1)
    {
        seterror(m_fk, efk_compile_stack_identifier_error, "double %s identifier error", name.c_str());
        return -1;
    }
    block_identifiers_list & list = m_block_identifiers_stack.back();
    list.push_back(block_identifiers(name, m_stackpos));
	int ret = m_stackpos;
    m_stackpos++;
    if (m_stackpos > m_maxstackpos)
    {
        m_maxstackpos = m_stackpos;
    }
	return ret;
}

int codegen::getconst(const variant & v)
{
	for (int i = 0; i < (int)m_const_list.size(); i++)
	{
		variant & vv = m_const_list[i];
		bool b = false;
		V_EQUAL_V(b, (&vv), (&v));
		if (b)
		{
			return i;
		}
	}

	int pos = m_const_list.size();
	m_const_list.push_back(v);
	return pos;
}

void codegen::output(const String & name, func_binary * bin)
{
    bin->m_name = name;

    bin->m_maxstack = m_maxstackpos;
    
    bin->m_size = m_byte_code_list.size();
    bin->m_buff = (command *)safe_fkmalloc(m_fk, (bin->m_size * sizeof(command)));
    memcpy(bin->m_buff, &m_byte_code_list[0], bin->m_size * sizeof(command));
    
    bin->m_const_list_num = m_const_list.size();
	if (bin->m_const_list_num > 0)
	{
		bin->m_const_list = (variant *)safe_fkmalloc(m_fk, (bin->m_const_list_num * sizeof(variant)));
		memcpy(bin->m_const_list, &m_const_list[0], bin->m_const_list_num * sizeof(variant));
    }

    FKLOG("codegen out %s %d", name.c_str(), m_maxstackpos);
}
