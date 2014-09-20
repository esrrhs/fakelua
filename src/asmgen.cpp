#include "asmgen.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "variant.h"

void asmgen::output(const String & name, func_native * nt)
{
    nt->m_name = name;

    nt->m_size = m_asm_code_list.size();
    nt->m_buff = (char *)safe_fkmalloc(m_fk, (nt->m_size * sizeof(char)));
    memcpy(nt->m_buff, &m_asm_code_list[0], nt->m_size * sizeof(char));
}

void asmgen::copy_const(variant * p, size_t num)
{
}

