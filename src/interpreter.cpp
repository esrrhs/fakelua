#include "interpreter.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

//////////////////////////////////////////////////////////////////////////

void interpreter::call(const variant & func)
{
    fake * fk = m_fk;
	const funcunion * f = m_fk->fm.get_func(func);
	if (UNLIKE(!f))
	{
		FKERR("fkrun no func %s fail", vartostring(&func).c_str());
		seterror(m_fk, efk_run_no_func_error, "fkrun no func %s fail", vartostring(&func).c_str());
		m_isend = true;
		return;
	}

	// 常规函数
	if (f->havefb)
	{
		const func_binary * fb = &f->fb;

		// 空函数处理
		if (UNLIKE(!FUNC_BINARY_CMDSIZE(*fb)))
		{
			// 所有都完
			if (ARRAY_EMPTY(m_stack_list))
			{
				FKLOG("call stack empty end");
				m_isend = true;
			}
			return;
		}

		// 压栈
		if (UNLIKE(ARRAY_SIZE(m_stack_list) >= ARRAY_MAX_SIZE(m_stack_list)))
		{
			int newsize = ARRAY_MAX_SIZE(m_stack_list) + 1 + ARRAY_MAX_SIZE(m_stack_list) * m_fk->cfg.array_grow_speed / 100;
			ARRAY_GROW(m_stack_list, newsize, stack);
		}
		ARRAY_PUSH_BACK(m_stack_list);
		stack & s = ARRAY_BACK(m_stack_list);
		m_cur_stack = &s;
		STACK_INI(s, m_fk, fb);
		if (UNLIKE(FUNC_BINARY_MAX_STACK(*fb) > (int)ARRAY_MAX_SIZE(s.m_stack_variant_list)))
		{
			ARRAY_GROW(s.m_stack_variant_list, FUNC_BINARY_MAX_STACK(*fb), variant);
		}

		// 记录profile
		beginfuncprofile();
		
		paramstack * ps = getps(m_fk);

		if (UNLIKE((int)ps->m_variant_list_num != FUNC_BINARY_PARAMNUM(*fb)))
		{
			FKERR("call func %s param not match", vartostring(&func).c_str());
			seterror(m_fk, efk_run_param_error, "call func %s param not match", vartostring(&func).c_str());
			m_isend = true;
			return;
		}

		assert(FUNC_BINARY_PARAMNUM(*fb) <= (int)ARRAY_MAX_SIZE(s.m_stack_variant_list));

		// 分配栈空间
		for (int i = 0; i < (int)FUNC_BINARY_PARAMNUM(*fb); i++)
		{
			SET_STACK(&(ps->m_variant_list[i]), s, i);
			FKLOG("call set %s to pos %d", (vartostring(&(ps->m_variant_list[i]))).c_str(), i);
		}
		
		ps->clear();

		// 重置ret
		V_SET_NIL(&m_ret[0]);

		// 标记
		FUNC_BINARY_USE(*fb)++;

		return;
	}

	// 记录profile
	uint32_t s = 0;
	if (m_fk->pf.isopen())
	{
		s = fkgetmstick();
	}

	// 绑定函数
	if (f->haveff)
	{
		BIND_FUNC_CALL(f, this);
		FKLOG("call C func %s", vartostring(&func).c_str());
	}
	// 内置函数
	else if (f->havebif)
	{
		BUILDIN_FUNC_CALL(f, this);
		FKLOG("call buildin func %s", vartostring(&func).c_str());
	}
	else
	{
		assert(0);
		FKERR("fkrun no inter func %s fail", vartostring(&func).c_str());
		seterror(m_fk, efk_run_no_func_error, "fkrun no inter func %s fail", vartostring(&func).c_str());
		m_isend = true;
		return;
	}

	// 返回值
	paramstack * theps = getps(m_fk);
    bool err = false;
	USE(err);

	// 这种情况是直接跳过脚本调用了C函数
	if (UNLIKE(ARRAY_EMPTY(m_stack_list)))
	{
    	variant * cret;
    	PS_POP_AND_GET(*theps, cret);
		m_isend = true;
		// 直接塞返回值
		m_ret[0] = *cret;
	}
	// 否则塞到当前堆栈上
	else
	{
		// 塞返回值
		m_cur_stack = &ARRAY_BACK(m_stack_list);
		const func_binary & fb = *m_cur_stack->m_fb;
		for (int i = 0; i < m_cur_stack->m_retnum; i++)
		{
			variant * ret;
			GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_retvpos[i]);
			
        	variant * cret;
        	PS_GET(*theps, cret, i);
        	
			*ret = *cret;
		}
	}
    if (UNLIKE(err))
    {
        m_isend = true;
    }
    
	if (m_fk->pf.isopen())
	{
	    bool err = false;
		const char * name = 0;
		V_GET_STRING(&func, name);
		m_fk->pf.add_func_sample(name, fkgetmstick() - s);
	}

	return;
}

void interpreter::call(const variant & callpos, int calltype)
{
    if (LIKE(calltype == CALL_NORMAL))
    {
        call(callpos);
    }
    else
    {
		m_processor->start_routine(callpos);
    }
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
		m_fk->pf.add_func_sample(FUNC_BINARY_NAME(*m_cur_stack->m_fb), fkgetmstick() - m_cur_stack->m_calltime);
    }
}

const char * interpreter::get_running_call_stack() const
{
    m_fk->rn.curcallstack.clear();
    int deps = 0;
    for (int i = ARRAY_SIZE(m_stack_list) - 1; i >= 0; i--)
    {
        m_fk->rn.curcallstack += "#";
        m_fk->rn.curcallstack += fkitoa(deps);
        m_fk->rn.curcallstack += "    ";
        stack & s = ARRAY_GET(m_stack_list, i);
        m_fk->rn.curcallstack += s.m_fb->m_name;
        m_fk->rn.curcallstack += " at ";
        m_fk->rn.curcallstack += s.m_fb->m_filename;
        m_fk->rn.curcallstack += ":";
        m_fk->rn.curcallstack += fkitoa(GET_CMD_LINENO((*s.m_fb), s.m_pos));
        m_fk->rn.curcallstack += "\n";
        for (int j = 0; j < s.m_fb->m_maxstack && j < (int)ARRAY_MAX_SIZE(s.m_stack_variant_list); j++)
        {
            m_fk->rn.curcallstack += "        [";
            m_fk->rn.curcallstack += fkitoa(j);
            m_fk->rn.curcallstack += "]\t";
            m_fk->rn.curcallstack += vartostring(&ARRAY_GET(s.m_stack_variant_list, j));
            m_fk->rn.curcallstack += "\n";
        }
        deps++;
    }
    return m_fk->rn.curcallstack.c_str();
}

uint32_t interpreter::get_max_stack() const
{
    return m_fk->cfg.stack_deps;
}
