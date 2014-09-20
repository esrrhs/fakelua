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

#define V_TYPE_OFF(stackpos) (-1 * (stackpos + 1) * variant_size + variant_type_off)
#define V_DATA_OFF(stackpos) (-1 * (stackpos + 1) * variant_size + variant_data_off)

void asmgen::copy_const(variant * p, size_t num, int start)
{
    for (int i = 0; i < (int)num; i++)
    {
        int typeoff = V_TYPE_OFF(start);
        int dataoff = V_DATA_OFF(start);
        mov_l_rbp(p[i].type, typeoff);
        mov_ll_rax(*(int64_t*)&p[i].data);
        mov_rax_rbp(dataoff);
    }
}

void asmgen::variant_assign(int leftpos, int rightpos)
{
    int lefttypeoff = V_TYPE_OFF(leftpos);
    int leftdataoff = V_DATA_OFF(leftpos);
    int rightpostypeoff = V_TYPE_OFF(rightpos);
    int rightposdataoff = V_DATA_OFF(rightpos);
    mov_rbp_rax(rightpostypeoff);
    mov_rax_rbp(lefttypeoff);
    mov_rbp_rax(rightposdataoff);
    mov_rax_rbp(leftdataoff);
}

