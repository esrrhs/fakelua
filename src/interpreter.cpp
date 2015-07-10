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
	    if (m_fk->pf.isopen())
	    {
	        m_cur_stack->m_calltime = fkgetmstick();
	    }
		
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

int interpreter::run(int cmdnum)
{
	fake * fk = m_fk;
	int num = 0;

	// 栈溢出检查
	if (UNLIKE((int)ARRAY_SIZE(m_stack_list) > m_fk->cfg.stack_deps))
	{	
		seterror(fk, efk_run_inter_error, "stack too deep %d", ARRAY_SIZE(m_stack_list));
		m_isend = true;
		return 0;
	}

	// 切换检查
	if (UNLIKE(m_sleeping))
	{
		if (LIKE(m_yieldtime))
		{
			m_yieldtime--;
			return 0;
		}
		else if (LIKE(fkgetmstick() < m_wakeuptime))
		{
			return 0;
		}
		else
		{
			m_wakeuptime = 0;
		}
	}
	
	for (int i = 0; i < cmdnum; i++)
	{
		bool err = false;

		// next
		const func_binary & fb = *m_cur_stack->m_fb;
		int pos = m_cur_stack->m_pos;
		
		// 当前函数走完
		if (UNLIKE(pos >= (int)FUNC_BINARY_CMDSIZE(fb)))
		{
			FKLOG("pop stack %s", FUNC_BINARY_NAME(fb));
			// 记录profile
		    if (m_fk->pf.isopen())
		    {
				m_fk->pf.add_func_sample(FUNC_BINARY_NAME(*m_cur_stack->m_fb), fkgetmstick() - m_cur_stack->m_calltime);
		    }
			// 标记
			FUNC_BINARY_USE(fb)--;
			// 更新
			if (UNLIKE(!FUNC_BINARY_USE(fb) && FUNC_BINARY_BACKUP(fb)))
			{
				FUNC_BINARY_BACKUP_MOVE(fb);
			}
			// 出栈
			ARRAY_POP_BACK(m_stack_list);
			// 所有都完
			if (ARRAY_EMPTY(m_stack_list))
			{
				FKLOG("stack empty end");
				m_isend = true;
				break;
			}
			// 塞返回值
			else
			{
				m_cur_stack = &ARRAY_BACK(m_stack_list);
				const func_binary & fb = *m_cur_stack->m_fb;
				for (int i = 0; i < m_cur_stack->m_retnum; i++)
				{
					variant * ret;
					GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_retvpos[i]);
					*ret = m_ret[i];
				}
			}
			continue;
		}

		command cmd = GET_CMD(fb, pos);
		int type = COMMAND_TYPE(cmd);
		int code = COMMAND_CODE(cmd);

		USE(type);
		FKLOG("next %d %d %s", type, code, OpCodeStr(code));
			
		assert (type == COMMAND_OPCODE);

		m_cur_stack->m_pos++;

		if (UNLIKE(m_fk->pf.isopen()))
		{
			m_fk->pf.add_code_sample(code);
		}

		// 执行对应命令，放一起switch效率更高，cpu有缓存
		switch (code)
		{
		case OPCODE_ASSIGN:
			{
				// 赋值dest，必须为栈上或容器内
				if (UNLIKE(!(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos))) == ADDR_STACK || 
					ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos))) == ADDR_CONTAINER)))
				{	
					err = true;
					seterror(fk, efk_run_inter_error, "interpreter assign error, dest is not stack or container, type %s", vartypetostring((int)ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos)))));
					break;
				}

				variant * varv = 0;
				LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "var");
				GET_VARIANT(*m_cur_stack, fb, varv, m_cur_stack->m_pos);
				m_cur_stack->m_pos++;
				
				// 赋值来源
				const variant * valuev = 0;
				LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "value");
				GET_VARIANT(*m_cur_stack, fb, valuev, m_cur_stack->m_pos);
				m_cur_stack->m_pos++;

				// 赋值
				*varv = *valuev;

				FKLOG("assign %s to %s", (vartostring(valuev)).c_str(), (vartostring(varv)).c_str());
			}
			break;
		case OPCODE_PLUS:
			{
				MATH_OPER(*m_cur_stack, fb, PLUS);
			}
			break;
		case OPCODE_MINUS:
			{
				MATH_OPER(*m_cur_stack, fb, MINUS);
			}
			break;
		case OPCODE_MULTIPLY:
			{
				MATH_OPER(*m_cur_stack, fb, MULTIPLY);
			}
			break;
		case OPCODE_DIVIDE:
			{
				MATH_OPER(*m_cur_stack, fb, DIVIDE);
			}
			break;
		case OPCODE_DIVIDE_MOD:
			{
				MATH_OPER(*m_cur_stack, fb, DIVIDE_MOD);
			}
			break;
		case OPCODE_AND:
			{
				MATH_OPER(*m_cur_stack, fb, AND);
			}
			break;
		case OPCODE_OR:
			{
				MATH_OPER(*m_cur_stack, fb, OR);
			}
			break;
		case OPCODE_LESS:
			{
				MATH_OPER(*m_cur_stack, fb, LESS);
			}
			break;
		case OPCODE_MORE:
			{
				MATH_OPER(*m_cur_stack, fb, MORE);
			}
			break;
		case OPCODE_EQUAL:
			{
				MATH_OPER(*m_cur_stack, fb, EQUAL);
			}
			break;
		case OPCODE_MOREEQUAL:
			{
				MATH_OPER(*m_cur_stack, fb, MOREEQUAL);
			}
			break;
		case OPCODE_LESSEQUAL:
			{
				MATH_OPER(*m_cur_stack, fb, LESSEQUAL);
			}
			break;
		case OPCODE_NOTEQUAL:
			{
				MATH_OPER(*m_cur_stack, fb, NOTEQUAL);
			}
			break;
		case OPCODE_NOT:
			{
				MATH_SINGLE_OPER(*m_cur_stack, fb, NOT);
			}
			break;
		case OPCODE_JNE:
			{
				const variant * cmp = 0;
				LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "cmp");
				GET_VARIANT(*m_cur_stack, fb, cmp, m_cur_stack->m_pos);
				m_cur_stack->m_pos++;

				int pos = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
				m_cur_stack->m_pos++;
				
				if (!(V_ISBOOL(cmp)))
				{
					FKLOG("jne %d", pos);
					
					m_cur_stack->m_pos = pos;
				}
				else
				{
					FKLOG("not jne %d", pos);
				}
			}
			break;
		case OPCODE_JMP:
			{
				int pos = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
				m_cur_stack->m_pos++;
				
				FKLOG("jmp %d", pos);

				m_cur_stack->m_pos = pos;
			}
			break;
		case OPCODE_PLUS_ASSIGN:
			{
				MATH_ASSIGN_OPER(*m_cur_stack, fb, PLUS);
			}
			break;
		case OPCODE_MINUS_ASSIGN:
			{
				MATH_ASSIGN_OPER(*m_cur_stack, fb, MINUS);
			}
			break;
		case OPCODE_MULTIPLY_ASSIGN:
			{
				MATH_ASSIGN_OPER(*m_cur_stack, fb, MULTIPLY);
			}
			break;
		case OPCODE_DIVIDE_ASSIGN:
			{
				MATH_ASSIGN_OPER(*m_cur_stack, fb, DIVIDE);
			}
			break;
		case OPCODE_DIVIDE_MOD_ASSIGN:
			{
				MATH_ASSIGN_OPER(*m_cur_stack, fb, DIVIDE_MOD);
			}
			break;
		case OPCODE_CALL:
			{
				int calltype = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
				m_cur_stack->m_pos++;

				const variant * callpos = 0;
				LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "callpos");
				GET_VARIANT(*m_cur_stack, fb, callpos, m_cur_stack->m_pos);
				m_cur_stack->m_pos++;

				int retnum = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
				m_cur_stack->m_retnum = retnum; 
				m_cur_stack->m_pos++;

				for (int i = 0; i < retnum; i++)
				{
					assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos))) == ADDR_STACK);
					m_cur_stack->m_retvpos[i] = m_cur_stack->m_pos;
					m_cur_stack->m_pos++;
				}
				
				int argnum = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
				m_cur_stack->m_pos++;

				paramstack & ps = *getps(m_fk);
				ps.clear();
				for (int i = 0; i < argnum; i++)
				{
					variant * arg = 0;
					LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "arg");
					GET_VARIANT(*m_cur_stack, fb, arg, m_cur_stack->m_pos);
					m_cur_stack->m_pos++;

					variant * argdest = 0;
					PS_PUSH_AND_GET(ps, argdest);
					*argdest = *arg;
				}
				
			    if (LIKE(calltype == CALL_NORMAL))
			    {
			        call(*callpos);
			    }
			    else
			    {
					m_processor->start_routine(*callpos);
			    }
			}
			break;
		case OPCODE_RETURN:
			{
				int returnnum = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
				if (UNLIKE(!returnnum))
				{
					FKLOG("return empty");
					m_cur_stack->m_pos = fb.m_size;
					break;
				}
				m_cur_stack->m_pos++;

				// 塞给ret
				for (int i = 0; i < returnnum; i++)
				{
					const variant * ret = 0;
					LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "ret");
					GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_pos);
					m_cur_stack->m_pos++;

					m_ret[i] = *ret;

					FKLOG("return %s", (vartostring(&m_ret[i])).c_str());
				}
				
				m_cur_stack->m_pos = fb.m_size;
			}
			break;
		case OPCODE_SLEEP:
			{
				const variant * time = 0;
				LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "time");
				GET_VARIANT(*m_cur_stack, fb, time, m_cur_stack->m_pos);
				m_cur_stack->m_pos++;

				uint32_t sleeptime = 0;
				V_GET_REAL(time, sleeptime);

				m_wakeuptime = fkgetmstick() + sleeptime;
				m_sleeping = true;
				return num;
			}
			break;
		case OPCODE_YIELD:
			{
				const variant * time = 0;
				LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "time");
				GET_VARIANT(*m_cur_stack, fb, time, m_cur_stack->m_pos);
				m_cur_stack->m_pos++;

				V_GET_REAL(time, m_yieldtime);
				m_sleeping = true;
				return num;
			}
			break;
		default:
			assert(0);
			FKERR("next err code %d %s", code, OpCodeStr(code));
			break;
		}

		if (UNLIKE(err))
		{
			// 发生错误
			m_isend = true;
		}
		num++;
		if (UNLIKE(m_isend))
		{
			break;
		}
	}

	return num;
}

