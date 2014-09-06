#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void interpreter::clear()
{
    m_isend = false;
    m_ret = variant(m_fk);
    m_stack_list.clear();
}

bool interpreter::isend() const
{
    return m_isend;
}

const variant & interpreter::getret() const
{
    return m_ret;
}

void interpreter::call(binary * bin, const String & func, paramstack * ps)
{
    const func_binary * fb = bin->get_func(func);
    if (!fb)
    {
        FKLOG("fkrun bin %p no func %s fail", bin, func.c_str());
        m_fk->seterror(m_ei, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, func.c_str());
        m_isend = true;
        return;
    }

    // —π’ª
    m_stack_list.push_back(stack(m_fk, m_ei, fb));
    stack & s = m_stack_list.back();

    // ∑÷≈‰’ªø’º‰
    for (int i = 0; i < (int)ps->size(); i++)
    {
        s.set_stack_variant((*ps)[i], i);
    }
}

int interpreter::run(int cmdnum)
{
    // TODO


    return 0;
}
