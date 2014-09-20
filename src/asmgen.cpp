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

void asmgen::copy_const(variant * p, size_t num, int start)
{
    for (int i = 0; i < (int)num; i++)
    {
        int typeoff = -1 * start * variant_size + variant_type_off;
        int dataoff = -1 * start * variant_size + variant_data_off;
        mov_l_rbp(p[i].type, typeoff);
        mov_ll_rax(*(int64_t*)&p[i].data);
        mov_rax_rbp(dataoff);
    }
}

