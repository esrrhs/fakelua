#include "interpreter.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

void stack::grow(size_t size)
{
	assert(m_fk);
	// 新空间
	int newsize = size;
	assert(newsize > (int)m_stack_variant_list_num);
	variant * newbuff = (variant *)safe_fkmalloc(m_fk, (newsize * sizeof(variant)));
	
	// 复制
	if (m_stack_variant_list)
	{
		memcpy(newbuff, m_stack_variant_list, m_stack_variant_list_num * sizeof(variant));
	}

	// 构造剩余的
	memset(&newbuff[m_stack_variant_list_num], 0, (newsize - m_stack_variant_list_num) * sizeof(variant));

	// 删除
	safe_fkfree(m_fk, m_stack_variant_list);

	m_stack_variant_list = newbuff;
	m_stack_variant_list_num = newsize;
}

//////////////////////////////////////////////////////////////////////////

void interpreter::call(binary * bin, const char * func, paramstack * ps)
{
    const func_binary * fb = bin->get_func(func);
    if (!fb)
    {
        // 去C函数看看，C函数只支持一个返回值
        if (m_fk->bf.call(func))
        {
            FKLOG("call C func %s", func);
            
            // 塞返回值
    		bool err = false;
			m_cur_stack = &m_stack_list[m_stack_list_num - 1];
			const func_binary & fb = *m_cur_stack->m_fb;
    		variant * ret;
			do { GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_retvpos[0]); } while (0);

            paramstack * theps = getps(m_fk);
            variant * cret;
        	PS_POP_AND_GET(*theps, cret);
        	
            *ret = *cret;
        }
        else
        {
            FKERR("fkrun bin %p no func %s fail", bin, func);
            seterror(m_fk, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, func);
            m_isend = true;
        }
        
        return;
    }

    // 空函数处理
    if (!fb->cmdsize())
    {
        // 所有都完
    	if (!m_stack_list_num)
        {
            FKLOG("call stack empty end");
            m_isend = true;
        }
        return;
    }

    // 压栈
	if (m_stack_list_num >= m_stack_list_max_num)
	{
		grow();
	}
	m_stack_list_num++;
	stack & s = m_stack_list[m_stack_list_num - 1];
    m_cur_stack = &s;
	s.m_fk = m_fk;
	s.m_fb = fb;
	s.clear();
	if (fb->maxstack() > s.m_stack_variant_list_num)
	{
	    s.grow(fb->maxstack());
	}

	// 记录profile
	beginfuncprofile();

    // 分配栈空间
    for (int i = 0; i < (int)ps->m_variant_list_num; i++)
    {
		SET_STACK(&(ps->m_variant_list[i]), s, i);
		FKLOG("call set %s to pos %d", (vartostring(&(ps->m_variant_list[i]))).c_str(), i);
    }
    ps->clear();

	// 重置ret
	V_SET_NIL(&m_ret[0]);
}

void interpreter::grow()
{
	assert(m_fk);
	// 新空间
	int newsize = m_stack_list_max_num + 1 + m_stack_list_max_num * m_fk->cfg.stack_list_grow_speed / 100;
	assert(newsize > (int)m_stack_list_max_num);
	stack * newbuff = (stack *)safe_fkmalloc(m_fk, (newsize * sizeof(stack)));

	// 复制
	if (m_stack_list)
	{
		memcpy(newbuff, m_stack_list, m_stack_list_max_num * sizeof(stack));
	}

	// 构造剩余的
	memset(&newbuff[m_stack_list_max_num], 0, (newsize - m_stack_list_max_num) * sizeof(stack));

	// 删除
	safe_fkfree(m_fk, m_stack_list);

	m_stack_list = newbuff;
	m_stack_list_max_num = newsize;
}

void interpreter::call(fake * fk, const variant * callpos, paramstack * ps)
{
    const char * name = 0;
    V_GET_STRING(callpos, name);

    call(&m_fk->bin, name, ps);
}

void interpreter::beginfuncprofile()
{
    if (m_fk->pf.isopen())
    {
        m_cur_stack->m_calltime = fkgetmstick();
    }
}

void interpreter::endfuncprofile()
{
    if (m_fk->pf.isopen())
    {
        m_fk->pf.add_func_sample(m_cur_stack->m_fb->getname(), fkgetmstick() - m_cur_stack->m_calltime);
    }
}

