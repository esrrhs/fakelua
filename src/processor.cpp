#include "processor.h"
#include "fake.h"

routine * processor::start_routine(const variant & func, int retnum, int * retpos)
{
	routine * n = 0;
	POOLLIST_POP(m_pl, n, routine);
	
	if (!m_entryroutine)
	{
		m_entryroutine = n;
	}
	
	// 初始化下
	ROUTINE_INI(*n, m_fk, m_genid);
	m_genid++;
	ROUTINE_CLEAR(*n);
	ROUTINE_SET_PRO(*n, this);
	ROUTINE_ENTRY(*n, func, retnum, retpos);
	
	m_routine_num++;
	if (!m_curroutine)
	{
		m_curroutine = n;
	}

	return n;
}
	
void processor::run()
{
	if (UNLIKE(m_fk->rn.stepmod))
	{
		if (UNLIKE(ARRAY_EMPTY(m_pl.l)))
		{	
			return;
		}
		if (UNLIKE(!m_routine_num))
		{
			ARRAY_CLEAR(m_pl.l);
			return;
		}

		routine * n = ARRAY_GET(m_pl.l, m_lastroutine);
		assert(n);
		ROUTINE_RUN(*n, 1);
		m_lastroutine_runnum++;
		bool needupdate = false;
		if (UNLIKE(ROUTINE_ISEND(*n)))
		{
			POOL_PUSH(m_pl.p, n, routine);
			ARRAY_GET(m_pl.l, m_lastroutine) = 0;
			m_routine_num--;
			needupdate = true;
		}
		else if (UNLIKE(m_lastroutine_runnum >= m_fk->cfg.per_frame_cmd_num))
		{
			m_lastroutine_runnum = 0;
			needupdate = true;
		}
		if (UNLIKE(needupdate))
		{
			for (int i = 0; LIKE(i < (int)ARRAY_SIZE(m_pl.l)); i++)
			{
				int index = m_lastroutine + i + 1;
				if (UNLIKE(index >= (int)ARRAY_SIZE(m_pl.l)))
				{
					index = index % ARRAY_SIZE(m_pl.l);
				}
				
				routine * n = ARRAY_GET(m_pl.l, index);
				if (UNLIKE(!n))
				{
					continue;
				}
				
				m_curroutine = n;
				m_lastroutine = index;
				break;
			}
		}
		if (UNLIKE(!m_routine_num))
		{
			ARRAY_CLEAR(m_pl.l);
		}
	}
	else
	{
		while (LIKE(m_routine_num > 0))
		{
			for (int i = 0; LIKE(i < (int)ARRAY_SIZE(m_pl.l)); i++)
			{
				routine * n = ARRAY_GET(m_pl.l, i);
				if (UNLIKE(!n))
				{
					continue;
				}
				m_curroutine = n;
				// 注意:此函数内部可能会调用到add接口
				ROUTINE_RUN(*n, m_fk->cfg.per_frame_cmd_num);
				if (UNLIKE(ROUTINE_ISEND(*n)))
				{
					POOL_PUSH(m_pl.p, n, routine);
					ARRAY_GET(m_pl.l, i) = 0;
					m_routine_num--;
				}
			}
		}
		ARRAY_CLEAR(m_pl.l);
	}
}

routine * processor::get_routine_by_id(int id)
{
	for (int i = 0; LIKE(i < (int)ARRAY_SIZE(m_pl.l)); i++)
	{
		routine * n = ARRAY_GET(m_pl.l, i);
		if (UNLIKE(!n))
		{
			continue;
		}

		if (ROUTINE_ID(*n) == id)
		{
			return n;
		}
	}
	return 0;
}

const char * processor::get_routine_info()
{
	String tmp;
	for (int i = 0; i < get_routine_num(); i++)
	{
		tmp += get_routine_info_by_index(i);
	}
	m_fk->rn.cur_runinginfo = tmp;
	return m_fk->rn.cur_runinginfo.c_str();
}

const char * processor::get_routine_info_by_id(int id)
{
	int j = 0;
	for (int i = 0; LIKE(i < (int)ARRAY_SIZE(m_pl.l)); i++)
	{
		routine * n = ARRAY_GET(m_pl.l, i);
		if (UNLIKE(!n))
		{
			continue;
		}

		if (ROUTINE_ID(*n) == id)
		{
			m_fk->rn.cur_runinginfo.clear();
			
			m_fk->rn.cur_runinginfo += "#";
			m_fk->rn.cur_runinginfo += fkitoa(j);
			m_fk->rn.cur_runinginfo += "\tId:";
			m_fk->rn.cur_runinginfo += fkitoa(ROUTINE_ID(*n));
			m_fk->rn.cur_runinginfo += "\t";
			m_fk->rn.cur_runinginfo += n->m_interpreter.get_running_func_name();
			m_fk->rn.cur_runinginfo += "(";
			m_fk->rn.cur_runinginfo += n->m_interpreter.get_running_file_name();
			m_fk->rn.cur_runinginfo += ":";
			m_fk->rn.cur_runinginfo += fkitoa(n->m_interpreter.get_running_file_line());
			m_fk->rn.cur_runinginfo += ")\t";
			m_fk->rn.cur_runinginfo += ROUTINE_ISEND(*n) ? "Dead" : "Alive";
			m_fk->rn.cur_runinginfo += "\n";
			return m_fk->rn.cur_runinginfo.c_str();
		}
		j++;
	}
	return "";
}

const char * processor::get_routine_info_by_index(int index)
{
	int j = 0;
	for (int i = 0; LIKE(i < (int)ARRAY_SIZE(m_pl.l)); i++)
	{
		routine * n = ARRAY_GET(m_pl.l, i);
		if (UNLIKE(!n))
		{
			continue;
		}

		if (j >= index)
		{
			return get_routine_info_by_id(ROUTINE_ID(*n));
		}
		j++;
	}
	return "";
}

routine * processor::get_routine_by_index(int index)
{
	int j = 0;
	for (int i = 0; LIKE(i < (int)ARRAY_SIZE(m_pl.l)); i++)
	{
		routine * n = ARRAY_GET(m_pl.l, i);
		if (UNLIKE(!n))
		{
			continue;
		}

		if (j >= index)
		{
			return n;
		}
		j++;
	}
	return 0;
}

