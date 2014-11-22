#include "interpreter.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

//////////////////////////////////////////////////////////////////////////

void interpreter::call(binary * bin, const variant & func, paramstack * ps)
{
	const funcunion * f = m_fk->fm.get_func(func);
	if (!f)
	{
		FKERR("fkrun bin %p no func %s fail", bin, vartostring(&func).c_str());
		seterror(m_fk, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, vartostring(&func).c_str());
		m_isend = true;
	}

	// 常规函数
	if (f->havefb)
	{
		const func_binary * fb = &f->fb;

		// 空函数处理
		if (!FUNC_BINARY_CMDSIZE(*fb))
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
		if (ARRAY_SIZE(m_stack_list) >= ARRAY_MAX_SIZE(m_stack_list))
		{
			int newsize = ARRAY_MAX_SIZE(m_stack_list) + 1 + ARRAY_MAX_SIZE(m_stack_list) * m_fk->cfg.array_grow_speed / 100;
			ARRAY_GROW(m_stack_list, newsize, stack);
		}
		ARRAY_PUSH_BACK(m_stack_list);
		stack & s = ARRAY_BACK(m_stack_list);
		m_cur_stack = &s;
		STACK_INI(s, m_fk, fb);
		if (FUNC_BINARY_MAX_STACK(*fb) > (int)ARRAY_MAX_SIZE(s.m_stack_variant_list))
		{
			ARRAY_GROW(s.m_stack_variant_list, FUNC_BINARY_MAX_STACK(*fb), variant);
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
	}

	// 返回值
	paramstack * theps = getps(m_fk);
	variant * cret;
	PS_POP_AND_GET(*theps, cret);

	// 这种情况是直接跳过脚本调用了C函数
	if (ARRAY_EMPTY(m_stack_list))
	{
		m_isend = true;
		// 直接塞返回值
		m_ret[0] = *cret;
	}
	// 否则塞到当前堆栈上
	else
	{
		// 塞返回值
		bool err = false;
		m_cur_stack = &ARRAY_BACK(m_stack_list);
		const func_binary & fb = *m_cur_stack->m_fb;
		variant * ret;
		do { GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_retvpos[0]); } while (0);

		*ret = *cret;
	}

	if (m_fk->pf.isopen())
	{
		const char * name = 0;
		V_GET_STRING(&func, name);
		m_fk->pf.add_func_sample(name, fkgetmstick() - s);
	}

	return;
}

void interpreter::call(fake * fk, const variant & callpos, paramstack * ps, int calltype)
{
    if (calltype == CALL_NORMAL)
    {
        call(&m_fk->bin, callpos, ps);
    }
    else
    {
		m_processor->start_routine(&m_fk->bin, callpos, ps);
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

