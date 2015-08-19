#include "compiler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fake.h"
#include "binary.h"

void compiler::clear()
{
	m_cur_addr = 0;
	memset(m_cur_addrs, 0, sizeof(m_cur_addrs));
	m_loop_break_pos_stack.clear();
	m_loop_continue_pos_stack.clear();
	m_continue_end_pos_stack.clear();
	m_func_ret_num = 1;
	m_cur_compile_func.clear();
	m_new_var = false;
	m_cmp_deps = 0;
	m_cmp_jne = false;
}

void compiler::compile_seterror(syntree_node * node, fake * fk, efkerror err, const char *fmt, ...)
{
	char errorstr[128];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(errorstr, sizeof(errorstr) - 1, fmt, ap);
	va_end(ap);
	errorstr[sizeof(errorstr) - 1] = 0;
	seterror(fk, err, m_mf->getfilename(), node->lineno(), "compile func(%s), %s", m_mf->getfilename(), node->lineno(), m_cur_compile_func.c_str(), errorstr);
}

bool compiler::compile()
{
	if (!compile_const_head())
	{
		FKERR("[compiler] compile_const_head fail");
		return false;
	}

	if (!compile_body())
	{
		FKERR("[compiler] compile_body fail");
		return false;
	}
	
	return true;
}

bool compiler::compile_const_head()
{
	FKLOG("[compiler] compile_const_head");
	// do nothing
	return true;
}

bool compiler::compile_body()
{
	myflexer * mf = m_mf;
	func_desc_list & funclist = mf->get_func_list();
	FKLOG("[compiler] compile_body funclist %d", funclist.size());

	for (int i = 0; i < (int)funclist.size(); i++)
	{
		func_desc_node * funcnode = funclist[i];
		if (!compile_func(funcnode))
		{   
			FKERR("[compiler] compile_body %s fail", funcnode->funcname.c_str());
			return false;
		}
	}
	
	FKLOG("[compiler] compile_body funclist %d ok dump \n%s", funclist.size(), m_fk->bin.dump().c_str());

	FKLOG("[compiler] compile_body funcmap %d ok dump \n%s", m_fk->fm.size(), m_fk->fm.dump().c_str());

	return true;
}

bool compiler::compile_func(func_desc_node * funcnode)
{
	FKLOG("[compiler] compile_func func %s", funcnode->funcname.c_str());

	m_cur_compile_func = funcnode->funcname;
	
	// 不检测重名，直接替换掉老的
	codegen cg(m_fk);
	func_binary bin;
	FUNC_BINARY_INI(bin);
	bin.m_end_lineno = funcnode->endline;
	
	// 压栈
	cg.push_stack_identifiers();
		
	// 参数入栈
	if (funcnode->arglist)
	{
		func_desc_arglist & arglist = funcnode->arglist->arglist;
		for (int i = 0; i < (int)arglist.size(); i++)
		{
			const String & arg = arglist[i];
			if (cg.add_stack_identifier(arg, funcnode->arglist->lineno()) == -1)
			{
				FKERR("[compile] compile_func %s arg error %s", funcnode->funcname.c_str(), arg.c_str());
				compile_seterror(funcnode->arglist, m_fk, efk_compile_stack_identifier_error, "double %s identifier error", arg.c_str());
				return false;
			}
		}
		bin.m_paramnum = arglist.size();
	}
	
	// 编译函数体
	if (funcnode->block)
	{
		if (!compile_block(cg, funcnode->block))
		{
			FKERR("[compile] compile_func compile_block %s fail", funcnode->funcname.c_str());
			return false;
		}
	}

	// break必须为空
	if (!m_loop_break_pos_stack.empty())
	{
		FKERR("[compile] compile_func compile_block extra break");
		compile_seterror(funcnode, m_fk, efk_compile_loop_error, "compile extra break error");
		return false;
	}
	
	// 编译成功
	String funcname = fkgen_package_name(m_mf->get_package(), funcnode->funcname);
	cg.output(m_mf->getfilename(), m_mf->get_package(), funcname.c_str(), &bin);

	// 看立即更新还是延迟更新
	variant fv = m_fk->sh.allocsysstr(funcname.c_str());
	m_fk->bin.add_func(fv, bin);
	
	FKLOG("[compiler] compile_func func %s size = %d OK", funcname.c_str(), FUNC_BINARY_SIZE(bin));
	
	return true;
}

bool compiler::compile_block(codegen & cg, block_node * blocknode)
{
	FKLOG("[compiler] compile_block block_node %p", blocknode);

	for (int i = 0; i < (int)blocknode->stmtlist.size(); i++)
	{
		syntree_node * stmt = blocknode->stmtlist[i];
		if (!compile_node(cg, stmt))
		{
			FKERR("[compiler] compile_block compile_node %p fail %s", blocknode, stmt->gettypename());
			return false;
		}
	}
	
	FKLOG("[compiler] compile_block block_node %p OK", blocknode);
	
	return true;
}

bool compiler::compile_node(codegen & cg, syntree_node * node)
{
	FKLOG("[compiler] compile_node %p %s", node, node->gettypename());

	esyntreetype type = node->gettype();
	switch (type)
	{
	case est_block:
		{
			block_node * bn = dynamic_cast<block_node *>(node);
			if (!compile_block(cg, bn))
			{
				FKERR("[compiler] compile_node block_node error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_while_stmt:
		{
			while_stmt * ws = dynamic_cast<while_stmt *>(node);
			if (!compile_while_stmt(cg, ws))
			{
				FKERR("[compiler] compile_node while_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_for_stmt:
		{
			for_stmt * fs = dynamic_cast<for_stmt *>(node);
			if (!compile_for_stmt(cg, fs))
			{
				FKERR("[compiler] compile_node for_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_multi_assign_stmt:
		{
			multi_assign_stmt * as = dynamic_cast<multi_assign_stmt *>(node);
			if (!compile_multi_assign_stmt(cg, as))
			{
				FKERR("[compiler] compile_node multi_assign_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_cmp_stmt:
		{
			cmp_stmt * cs = dynamic_cast<cmp_stmt *>(node);
			if (!compile_cmp_stmt(cg, cs))
			{
				FKERR("[compiler] compile_node cmp_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_if_stmt:
		{
			if_stmt * is = dynamic_cast<if_stmt *>(node);
			if (!compile_if_stmt(cg, is))
			{
				FKERR("[compiler] compile_node compile_if_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_explicit_value:
		{
			explicit_value_node * ev = dynamic_cast<explicit_value_node *>(node);
			if (!compile_explicit_value(cg, ev))
			{
				FKERR("[compiler] compile_node compile_if_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_return_stmt:
		{
			return_stmt * rs = dynamic_cast<return_stmt *>(node);
			if (!compile_return_stmt(cg, rs))
			{
				FKERR("[compiler] compile_node compile_return_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_return_value_list:
		{
			return_value_list_node * rn = dynamic_cast<return_value_list_node *>(node);
			if (!compile_return_value_list(cg, rn))
			{
				FKERR("[compiler] compile_node compile_return_value_list error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_assign_stmt:
		{
			assign_stmt * as = dynamic_cast<assign_stmt *>(node);
			if (!compile_assign_stmt(cg, as))
			{
				FKERR("[compiler] compile_node compile_assign_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_math_assign_stmt:
		{
			math_assign_stmt * ms = dynamic_cast<math_assign_stmt *>(node);
			if (!compile_math_assign_stmt(cg, ms))
			{
				FKERR("[compiler] compile_node compile_math_assign_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_variable:
		{
			variable_node * vn = dynamic_cast<variable_node *>(node);
			if (!compile_variable_node(cg, vn))
			{
				FKERR("[compiler] compile_node compile_variable_node error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_var:
		{
			var_node * vn = dynamic_cast<var_node *>(node);
			if (!compile_var_node(cg, vn))
			{
				FKERR("[compiler] compile_node compile_var_node error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_function_call:
		{
			function_call_node * fn = dynamic_cast<function_call_node *>(node);
			if (!compile_function_call_node(cg, fn))
			{
				FKERR("[compiler] compile_node function_call_node error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_break:
		{
			break_stmt * bs = dynamic_cast<break_stmt *>(node);
			if (!compile_break_stmt(cg, bs))
			{
				FKERR("[compiler] compile_node compile_break_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_continue:
		{
			continue_stmt * cs = dynamic_cast<continue_stmt *>(node);
			if (!compile_continue_stmt(cg, cs))
			{
				FKERR("[compiler] compile_node compile_continue_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_math_expr:
		{
			math_expr_node * mn = dynamic_cast<math_expr_node *>(node);
			if (!compile_math_expr_node(cg, mn))
			{
				FKERR("[compiler] compile_node math_expr_node error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_container_get:
		{
			container_get_node * cn = dynamic_cast<container_get_node *>(node);
			if (!compile_container_get(cg, cn))
			{
				FKERR("[compiler] compile_node compile_container_get error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_struct_pointer:
		{
			struct_pointer_node * cn = dynamic_cast<struct_pointer_node *>(node);
			if (!compile_struct_pointer(cg, cn))
			{
				FKERR("[compiler] compile_node compile_struct_pointer error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_sleep:
		{
			sleep_stmt * ss = dynamic_cast<sleep_stmt *>(node);
			if (!compile_sleep_stmt(cg, ss))
			{
				FKERR("[compiler] compile_node compile_sleep_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_yield:
		{
			yield_stmt * ys = dynamic_cast<yield_stmt *>(node);
			if (!compile_yield_stmt(cg, ys))
			{
				FKERR("[compiler] compile_node compile_yield_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_switch_stmt:
		{
			switch_stmt * ss = dynamic_cast<switch_stmt *>(node);
			if (!compile_switch_stmt(cg, ss))
			{
				FKERR("[compiler] compile_node compile_switch_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	case est_for_loop_stmt:
		{
			for_loop_stmt * fs = dynamic_cast<for_loop_stmt *>(node);
			if (!compile_for_loop_stmt(cg, fs))
			{
				FKERR("[compiler] compile_node compile_for_loop_stmt error %d %s", type, node->gettypename());
				return false;
			}
		}
		break;
	default:
		{
			FKERR("[compiler] compile_node type error %d %s", type, node->gettypename());
			compile_seterror(node, m_fk, efk_compile_stmt_type_error, "compile node type error %d", type);
			return false;
		}
		break;
	}

	FKLOG("[compiler] compile_node %p %s OK", node, node->gettypename());
	
	return true;
}

bool compiler::compile_while_stmt(codegen & cg, while_stmt * ws)
{
	FKLOG("[compiler] compile_while_stmt %p", ws);

	int startpos = 0;
	int jnepos = 0;

	m_loop_break_pos_stack.push_back(beak_pos_list());

	startpos = cg.byte_code_size();

	m_loop_continue_pos_stack.push_back(startpos);

	// 条件
	cg.push_stack_identifiers();
	if (!compile_node(cg, ws->cmp))
	{
		FKERR("[compiler] compile_while_stmt cmp fail");
		return false;
	}
	cg.pop_stack_identifiers();

	// cmp与jne结合
	if (m_cmp_jne)
	{
		cg.push(EMPTY_CMD, ws->cmp->lineno()); // 先塞个位置
		jnepos = cg.byte_code_size() - 1;
	}
	else
	{
		cg.push(MAKE_OPCODE(OPCODE_JNE), ws->lineno());
		cg.push(m_cur_addr, ws->lineno());
		cg.push(EMPTY_CMD, ws->lineno()); // 先塞个位置
		jnepos = cg.byte_code_size() - 1;
	}
	m_cmp_deps = 0;
	m_cmp_jne = false;
	
	// block块
	if (ws->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, ws->block))
		{
			FKERR("[compiler] compile_while_stmt block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	// 跳回判断地方
	cg.push(MAKE_OPCODE(OPCODE_JMP), ws->lineno());
	cg.push(MAKE_POS(startpos), ws->lineno());

	// 跳转出block块
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));

	// 替换掉break
	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	for (int i = 0; i < (int)bplist.size(); i++)
	{
		cg.set(bplist[i], MAKE_POS(cg.byte_code_size()));
	}
	m_loop_break_pos_stack.pop_back();

	m_loop_continue_pos_stack.pop_back();
	
	FKLOG("[compiler] compile_while_stmt %p OK", ws);
	
	return true;
}

bool compiler::compile_if_stmt(codegen & cg, if_stmt * is)
{
	FKLOG("[compiler] compile_if_stmt %p", is);

	int jnepos = 0;
	std::vector<int> jmpifpos;

	// 条件
	cg.push_stack_identifiers();
	if (!compile_node(cg, is->cmp))
	{
		FKERR("[compiler] compile_if_stmt cmp fail");
		return false;
	}
	cg.pop_stack_identifiers();

	// cmp与jne结合
	if (m_cmp_jne)
	{
		cg.push(EMPTY_CMD, is->cmp->lineno()); // 先塞个位置
		jnepos = cg.byte_code_size() - 1;
	}
	else
	{
		cg.push(MAKE_OPCODE(OPCODE_JNE), is->lineno());
		cg.push(m_cur_addr, is->lineno());
		cg.push(EMPTY_CMD, is->lineno()); // 先塞个位置
		jnepos = cg.byte_code_size() - 1;
	}
	m_cmp_deps = 0;
	m_cmp_jne = false;
	
	// if块
	if (is->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, is->block))
		{
			FKERR("[compiler] compile_if_stmt block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}
	
	// 跳出if块
	if (is->elseifs || (is->elses && is->elses->block))
	{
		cg.push(MAKE_OPCODE(OPCODE_JMP), is->lineno());
		cg.push(EMPTY_CMD, is->lineno()); // 先塞个位置
		jmpifpos.push_back(cg.byte_code_size() - 1);
	}
	
	// 开始处理elseif的
	if (is->elseifs)
	{
		stmt_node_list & list = is->elseifs->stmtlist;
		for (int i = 0; i < (int)list.size(); i++)
		{
			elseif_stmt * eis = dynamic_cast<elseif_stmt *>(list[i]);
			
			// 跳转到else if
			cg.set(jnepos, MAKE_POS(cg.byte_code_size()));
			
			// 条件
			cg.push_stack_identifiers();
			if (!compile_node(cg, eis->cmp))
			{
				FKERR("[compiler] compile_if_stmt cmp fail");
				return false;
			}
			cg.pop_stack_identifiers();

			// cmp与jne结合
			if (m_cmp_jne)
			{
				cg.push(EMPTY_CMD, eis->cmp->lineno()); // 先塞个位置
				jnepos = cg.byte_code_size() - 1;
			}
			else
			{
				cg.push(MAKE_OPCODE(OPCODE_JNE), eis->lineno());
				cg.push(m_cur_addr, eis->lineno());
				cg.push(EMPTY_CMD, eis->lineno()); // 先塞个位置
				jnepos = cg.byte_code_size() - 1;
			}
			m_cmp_deps = 0;
			m_cmp_jne = false;
			
			// else if块
			if (eis->block)
			{
				cg.push_stack_identifiers();
				if (!compile_node(cg, eis->block))
				{
					FKERR("[compiler] compile_if_stmt block fail");
					return false;
				}
				cg.pop_stack_identifiers();
			}
			
			// 跳出if块
			cg.push(MAKE_OPCODE(OPCODE_JMP), eis->lineno());
			cg.push(EMPTY_CMD, eis->lineno()); // 先塞个位置
			jmpifpos.push_back(cg.byte_code_size() - 1);
		}
	}
		
	// 跳转到else
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));
	
	// else块
	if (is->elses && is->elses->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, is->elses->block))
		{
			FKERR("[compiler] compile_if_stmt else block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}
	
	// 跳转到结束
	for (int i = 0; i < (int)jmpifpos.size(); i++)
	{
		cg.set(jmpifpos[i], MAKE_POS(cg.byte_code_size()));
	}
	
	FKLOG("[compiler] compile_if_stmt %p OK", is);
	
	return true;
}

bool compiler::compile_return_stmt(codegen & cg, return_stmt * rs)
{
	FKLOG("[compiler] compile_return_stmt %p", rs);

	if (rs->returnlist)
	{
		if (!compile_node(cg, rs->returnlist))
		{
			FKERR("[compiler] compile_return_stmt ret fail");
			return false;
		}

		cg.push(MAKE_OPCODE(OPCODE_RETURN), rs->lineno());
		cg.push(MAKE_POS(rs->returnlist->returnlist.size()), rs->lineno());
		for (int i = 0; i < (int)rs->returnlist->returnlist.size(); i++)
		{
			cg.push(m_cur_addrs[i], rs->lineno());
		}
	}
	else
	{
		cg.push(MAKE_OPCODE(OPCODE_RETURN), rs->lineno());
		cg.push(MAKE_POS(0), rs->lineno());
	}

	FKLOG("[compiler] compile_return_stmt %p OK", rs);
	
	return true;
}

bool compiler::compile_assign_stmt(codegen & cg, assign_stmt * as)
{
	FKLOG("[compiler] compile_assign_stmt %p", as);

	command var = 0;
	command value = 0;

	if (!compile_node(cg, as->value))
	{
		FKERR("[compiler] compile_assign_stmt value fail");
		return false;
	}
	value = m_cur_addr;
	FKLOG("[compiler] compile_assign_stmt value = %d", m_cur_addr);

	m_new_var = as->isnew;
	if (!compile_node(cg, as->var))
	{
		FKERR("[compiler] compile_assign_stmt var fail");
		return false;
	}
	m_new_var = false;
	var = m_cur_addr;
	FKLOG("[compiler] compile_assign_stmt var = %d", m_cur_addr);
	
	cg.push(MAKE_OPCODE(OPCODE_ASSIGN), as->lineno());
	cg.push(var, as->lineno());
	cg.push(value, as->lineno());
	FKLOG("[compiler] compile_assign_stmt %p OK", as);
	
	return true;
}

bool compiler::compile_math_assign_stmt(codegen & cg, math_assign_stmt * ms)
{
	FKLOG("[compiler] compile_math_assign_stmt %p", ms);

	command oper = 0;
	command var = 0;
	command value = 0;
	
	if (ms->oper == "+=")
	{
		oper = MAKE_OPCODE(OPCODE_PLUS_ASSIGN);
	}
	else if (ms->oper == "-=")
	{
		oper = MAKE_OPCODE(OPCODE_MINUS_ASSIGN);
	}
	else if (ms->oper == "*=")
	{
		oper = MAKE_OPCODE(OPCODE_MULTIPLY_ASSIGN);
	}
	else if (ms->oper == "/=")
	{
		oper = MAKE_OPCODE(OPCODE_DIVIDE_ASSIGN);
	}
	else if (ms->oper == "%=")
	{
		oper = MAKE_OPCODE(OPCODE_DIVIDE_MOD_ASSIGN);
	}
	else
	{
		FKERR("[compiler] compile_math_assign_stmt error oper type fail");
		compile_seterror(ms, m_fk, efk_compile_math_type_error, "compile math assign oper type %s error", ms->oper.c_str());
		return false;
	}

	// value
	if (!compile_node(cg, ms->value))
	{
		FKERR("[compiler] compile_math_assign_stmt value fail");
		return false;
	}
	value = m_cur_addr;
	FKLOG("[compiler] compile_math_assign_stmt value = %d", m_cur_addr);

	// var
	if (!compile_node(cg, ms->var))
	{
		FKERR("[compiler] compile_math_assign_stmt var fail");
		return false;
	}
	var = m_cur_addr;
	FKLOG("[compiler] compile_math_assign_stmt var = %d", m_cur_addr);

	cg.push(oper, ms->lineno());
	cg.push(var, ms->lineno());
	cg.push(value, ms->lineno());

	FKLOG("[compiler] compile_math_assign_stmt %p OK", ms);
	
	return true;
}

bool compiler::compile_break_stmt(codegen & cg, break_stmt * bs)
{
	FKLOG("[compiler] compile_break_stmt %p", bs);

	cg.push(MAKE_OPCODE(OPCODE_JMP), bs->lineno());
	cg.push(EMPTY_CMD, bs->lineno()); // 先塞个位置
	int jmppos = cg.byte_code_size() - 1;

	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	bplist.push_back(jmppos);

	FKLOG("[compiler] compile_break_stmt %p OK", bs);
	
	return true;
}

bool compiler::compile_continue_stmt(codegen & cg, continue_stmt * cs)
{
	FKLOG("[compiler] compile_continue_stmt %p", cs);

	if (m_loop_continue_pos_stack.empty())
	{
		FKERR("[compiler] compile_continue_stmt no loop to continue");
		compile_seterror(cs, m_fk, efk_compile_loop_error, "no loop to continue");
		return false;
	}

	int continuepos = m_loop_continue_pos_stack[m_loop_continue_pos_stack.size() - 1];
	
	cg.push(MAKE_OPCODE(OPCODE_JMP), cs->lineno());
	cg.push(MAKE_POS(continuepos), cs->lineno());
	
	if (continuepos == -1)
	{
		// 一会统一设置
		int pos = cg.byte_code_size() - 1;
		continue_end_pos_list & cplist = m_continue_end_pos_stack[m_continue_end_pos_stack.size() - 1];
		cplist.push_back(pos);
	}

	FKLOG("[compiler] compile_continue_stmt %p OK", cs);
	
	return true;
}

bool compiler::compile_cmp_stmt(codegen & cg, cmp_stmt * cs)
{
	FKLOG("[compiler] compile_cmp_stmt %p", cs);

	int deps = m_cmp_deps;
	m_cmp_deps++;

	command oper = 0;
	command left = 0;
	command right = 0;
	command dest = 0;   

	if (cs->cmp != "not")
	{
		// oper
		if (cs->cmp == "&&")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_AND_JNE) : MAKE_OPCODE(OPCODE_AND);
		}
		else if (cs->cmp == "||")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_OR_JNE) : MAKE_OPCODE(OPCODE_OR);
		}
		else if (cs->cmp == "<")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_LESS_JNE) : MAKE_OPCODE(OPCODE_LESS);
		}
		else if (cs->cmp == ">")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_MORE_JNE) : MAKE_OPCODE(OPCODE_MORE);
		}
		else if (cs->cmp == "==")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_EQUAL_JNE) : MAKE_OPCODE(OPCODE_EQUAL);
		}
		else if (cs->cmp == ">=")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_MOREEQUAL_JNE) : MAKE_OPCODE(OPCODE_MOREEQUAL);
		}
		else if (cs->cmp == "<=")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_LESSEQUAL_JNE) : MAKE_OPCODE(OPCODE_LESSEQUAL);
		}
		else if (cs->cmp == "!=")
		{
			oper = !deps ? MAKE_OPCODE(OPCODE_NOTEQUAL_JNE) : MAKE_OPCODE(OPCODE_NOTEQUAL);
		}
		else if (cs->cmp == "true")
		{
			variant v;
			V_SET_REAL((&v), 1);
			int pos = cg.getconst(v);
			m_cur_addr = MAKE_ADDR(ADDR_CONST, pos);
			
			m_cmp_deps--;
			m_cmp_jne = false;
			
			return true;
		}
		else if (cs->cmp == "false")
		{
			variant v;
			V_SET_REAL((&v), 0);
			int pos = cg.getconst(v);
			m_cur_addr = MAKE_ADDR(ADDR_CONST, pos);
			
			m_cmp_deps--;
			m_cmp_jne = false;
			
			return true;
		}
		else if (cs->cmp == "is")
		{
			// left
			if (!compile_node(cg, cs->left))
			{
				FKERR("[compiler] compile_cmp_stmt left fail");
				return false;
			}
			
			m_cmp_deps--;
			m_cmp_jne = false;
			
			return true;
		}
		else
		{
			FKERR("[compiler] compile_cmp_stmt cmp error %s", cs->cmp.c_str());
			compile_seterror(cs, m_fk, efk_compile_cmp_error, "cmp error %s", cs->cmp.c_str());
			return false;
		}
		
		// left
		if (!compile_node(cg, cs->left))
		{
			FKERR("[compiler] compile_cmp_stmt left fail");
			return false;
		}
		left = m_cur_addr;

		// right
		if (!compile_node(cg, cs->right))
		{
			FKERR("[compiler] compile_cmp_stmt right fail");
			return false;
		}
		right = m_cur_addr;

		// result
		int despos = cg.alloc_stack_identifier();
		dest = MAKE_ADDR(ADDR_STACK, despos);
		m_cur_addr = dest;
		
		cg.push(oper, cs->lineno());
		cg.push(left, cs->lineno());
		cg.push(right, cs->lineno());
		cg.push(dest, cs->lineno());
	}
	/* "not" */
	else
	{
		oper = !deps ? MAKE_OPCODE(OPCODE_NOT_JNE) : MAKE_OPCODE(OPCODE_NOT);
		
		// left
		if (!compile_node(cg, cs->left))
		{
			FKERR("[compiler] compile_cmp_stmt left fail");
			return false;
		}
		left = m_cur_addr;
	
		int despos = cg.alloc_stack_identifier();
		dest = MAKE_ADDR(ADDR_STACK, despos);
		m_cur_addr = dest;
	
		cg.push(oper, cs->lineno());
		cg.push(left, cs->lineno());
		cg.push(dest, cs->lineno());
	}
	
	m_cmp_deps--;
	if (!deps)
	{
		m_cmp_jne = true;
	}
	
	FKLOG("[compiler] compile_cmp_stmt %p OK", cs);
	
	return true;
}

bool compiler::compile_explicit_value(codegen & cg, explicit_value_node * ev)
{
	FKLOG("[compiler] compile_explicit_value %p %s", ev, ev->str.c_str());

	fake * fk = m_fk;

	variant v;
	switch (ev->getvaluetype())
	{
	case explicit_value_node::EVT_TRUE:
		V_SET_REAL((&v), 1);
		break;
	case explicit_value_node::EVT_FALSE:
		V_SET_REAL((&v), 0);
		break;
	case explicit_value_node::EVT_NUM:
		V_SET_REAL((&v), (fkatol(&ev->str)));
		break;
	case explicit_value_node::EVT_STR:
		v = fk->sh.allocsysstr(ev->str.c_str());
		break;
	case explicit_value_node::EVT_FLOAT:
		V_SET_REAL((&v), (fkatof(&ev->str)));
		break;
	case explicit_value_node::EVT_UUID:
		V_SET_UUID((&v), (fkatol(&ev->str)));
		break;
	default:
		FKERR("[compiler] compile_explicit_value type error %d %s", ev->getvaluetype(), ev->gettypename());
		compile_seterror(ev, m_fk, efk_compile_explicit_type_error, "compile explicit value type error %d", ev->getvaluetype());
		return false;
	}

	int pos = cg.getconst(v);
	m_cur_addr = MAKE_ADDR(ADDR_CONST, pos);

	FKLOG("[compiler] compile_explicit_value %p OK", ev);
	
	return true;
}

bool compiler::compile_variable_node(codegen & cg, variable_node * vn)
{
	FKLOG("[compiler] compile_variable_node %p", vn);

	// 看看是否是常量定义
	myflexer * mf = m_mf;
	explicit_value_map & evm = mf->get_const_map();
	if (evm.find(vn->str) != evm.end())
	{   
		explicit_value_node* ev = evm[vn->str];
		if (!compile_node(cg, ev))
		{
			FKERR("[compiler] compile_variable_node const fail %s", ev->str.c_str());
			return false;
		}
		FKLOG("[compiler] compile_variable_node %p OK", vn);
		return true;
	}

	// 从当前堆栈往上找
	int pos = cg.getvariable(vn->str);
	if (pos == -1)
	{
		// 是不是需要new出来
		if (m_new_var)
		{
			var_node tmp;
			tmp.str = vn->str;
			return compile_var_node(cg, &tmp);
		}
		else
		{
			FKERR("[compiler] compile_variable_node variable not found %s", vn->str.c_str());
			compile_seterror(vn, m_fk, efk_compile_variable_not_found, "variable %s not found", vn->str.c_str());
			return false;
		}
	}
	m_cur_addr = MAKE_ADDR(ADDR_STACK, pos);

	FKLOG("[compiler] compile_variable_node %p OK", vn);
	
	return true;
}

bool compiler::compile_var_node(codegen & cg, var_node * vn)
{
	FKLOG("[compiler] compile_var_node %p", vn);

	// 确保当前block没有
	if (cg.get_cur_variable_pos(vn->str) != -1)
	{
		FKERR("[compiler] compile_var_node variable has define %s", vn->str.c_str());
		compile_seterror(vn, m_fk, efk_compile_variable_has_define, "variable %s has define", vn->str.c_str());
		return false;
	}

	// 申请栈上空间
	int pos = cg.add_stack_identifier(vn->str, vn->lineno());
	if (pos == -1)
	{
		FKERR("[compiler] compile_var_node variable has define %s", vn->str.c_str());
		compile_seterror(vn, m_fk, efk_compile_stack_identifier_error, "double %s identifier error", vn->str.c_str());
		return false;
	}
	m_cur_addr = MAKE_ADDR(ADDR_STACK, pos);

	FKLOG("[compiler] compile_var_node %p OK", vn);
	
	return true;
}

bool compiler::compile_function_call_node(codegen & cg, function_call_node * fn)
{
	FKLOG("[compiler] compile_function_call_node %p", fn);

	fake * fk = m_fk;
	myflexer * mf = m_mf;

	int ret_num = m_func_ret_num;
	m_func_ret_num = 1;

	// 参数
	std::vector<command> arglist;
	if (fn->arglist)
	{
		for (int i = 0; i < (int)fn->arglist->arglist.size(); i++)
		{
			syntree_node* sn = fn->arglist->arglist[i];
			if (!compile_node(cg, sn))
			{
				FKERR("[compiler] compile_function_call_node arg fail");
				return false;
			}
			arglist.push_back(m_cur_addr);
		}
	}

	// 调用位置
	command callpos;
	String func = fn->fuc;
	// 1 检查变量
	int pos = cg.getvariable(func);
	if (pos != -1)
	{
		// 是用变量来调用函数
		callpos = MAKE_ADDR(ADDR_STACK, pos);
	}
	// 2 检查struct
	else if (mf->is_have_struct(func))
	{
		// 直接替换成map
		variant v;
		v = fk->sh.allocsysstr(MAP_FUNC_NAME);
		pos = cg.getconst(v);
		callpos = MAKE_ADDR(ADDR_CONST, pos);
	}
	// 3 检查本地函数
	else if (mf->is_have_func(func))
	{
		// 申请字符串变量
		variant v;
		// 拼上包名
		String pname = fkgen_package_name(mf->get_package(), func);
		v = fk->sh.allocsysstr(pname.c_str());
		pos = cg.getconst(v);
		callpos = MAKE_ADDR(ADDR_CONST, pos);
	}
	// 4 直接字符串使用
	else
	{
		// 申请字符串变量
		variant v;
		v = fk->sh.allocsysstr(func.c_str());
		pos = cg.getconst(v);
		callpos = MAKE_ADDR(ADDR_CONST, pos);
	}

	// oper
	command oper;
	oper = MAKE_OPCODE(OPCODE_CALL);

	// 调用类型
	command calltype;
	if (fn->fakecall)
	{
		calltype = MAKE_POS(CALL_FAKE);
	}
	else
	{
		calltype = MAKE_POS(CALL_NORMAL);
	}
	
	// 参数个数
	command argnum;
	argnum = MAKE_POS(arglist.size());
	if (arglist.size() > MAX_FAKE_PARAM_NUM)
	{
		FKERR("[compiler] compile_function_call_node arg too much");
		compile_seterror(fn, m_fk, efk_compile_variable_not_found, "arg too much");
		return false;
	}

	// 返回值个数
	command retnum;
	retnum = MAKE_POS(ret_num);
   
	// 返回值
	std::vector<command> ret;
	for (int i = 0; i < ret_num; i++)
	{
		int retpos = cg.alloc_stack_identifier();
		ret.push_back(MAKE_ADDR(ADDR_STACK, retpos));
		m_cur_addrs[i] = ret[i];
	}
	m_cur_addr = ret[0];
	
	cg.push(oper, fn->lineno());
	cg.push(calltype, fn->lineno());
	cg.push(callpos, fn->lineno());
	cg.push(retnum, fn->lineno());
	for (int i = 0; i < ret_num; i++)
	{
		cg.push(ret[i], fn->lineno());
	}
	cg.push(argnum, fn->lineno());
	for (int i = 0; i < (int)arglist.size(); i++)
	{
		cg.push(arglist[i], fn->lineno());
	}
	
	FKLOG("[compiler] compile_function_call_node %p OK", fn);
	
	return true;
}

bool compiler::compile_math_expr_node(codegen & cg, math_expr_node * mn)
{
	FKLOG("[compiler] compile_math_expr_node %p", mn);

	command oper = 0;
	command left = 0;
	command right = 0;
	command dest = 0;
	
	if (mn->oper == "+")
	{
		oper = MAKE_OPCODE(OPCODE_PLUS);
	}
	else if (mn->oper == "-")
	{
		oper = MAKE_OPCODE(OPCODE_MINUS);
	}
	else if (mn->oper == "*")
	{
		oper = MAKE_OPCODE(OPCODE_MULTIPLY);
	}
	else if (mn->oper == "/")
	{
		oper = MAKE_OPCODE(OPCODE_DIVIDE);
	}
	else if (mn->oper == "%")
	{
		oper = MAKE_OPCODE(OPCODE_DIVIDE_MOD);
	}
	else
	{
		FKERR("[compiler] compile_math_expr_node error oper type fail");
		compile_seterror(mn, m_fk, efk_compile_math_type_error, "compile math oper type %s error", mn->oper.c_str());
		return false;
	}

	// left
	if (!compile_node(cg, mn->left))
	{
		FKERR("[compiler] compile_math_expr_node left fail");
		return false;
	}
	left = m_cur_addr;

	// right
	if (!compile_node(cg, mn->right))
	{
		FKERR("[compiler] compile_math_expr_node left fail");
		return false;
	}
	right = m_cur_addr;

	// result
	int despos = cg.alloc_stack_identifier();
	dest = MAKE_ADDR(ADDR_STACK, despos);
	m_cur_addr = dest;
	
	cg.push(oper, mn->lineno());
	cg.push(left, mn->lineno());
	cg.push(right, mn->lineno());
	cg.push(dest, mn->lineno());

	FKLOG("[compiler] compile_math_expr_node %p OK", mn);
	
	return true;
}

bool compiler::compile_for_stmt(codegen & cg, for_stmt * fs)
{
	FKLOG("[compiler] compile_for_stmt %p", fs);

	int startpos = 0;
	int jnepos = 0;

	m_loop_break_pos_stack.push_back(beak_pos_list());

	// 开始语句，这个作用域是全for都有效的
	cg.push_stack_identifiers();
	if (fs->beginblock)
	{
		if (!compile_node(cg, fs->beginblock))
		{
			FKERR("[compiler] compile_for_stmt beginblock fail");
			return false;
		}
	}

	startpos = cg.byte_code_size();
	m_loop_continue_pos_stack.push_back(startpos);

	// 条件
	cg.push_stack_identifiers();
	if (!compile_node(cg, fs->cmp))
	{
		FKERR("[compiler] compile_for_stmt cmp fail");
		return false;
	}
	cg.pop_stack_identifiers();

	// cmp与jne结合
	if (m_cmp_jne)
	{
		cg.push(EMPTY_CMD, fs->cmp->lineno()); // 先塞个位置
		jnepos = cg.byte_code_size() - 1;
	}
	else
	{
		cg.push(MAKE_OPCODE(OPCODE_JNE), fs->lineno());
		cg.push(m_cur_addr, fs->lineno());
		cg.push(EMPTY_CMD, fs->lineno()); // 先塞个位置
		jnepos = cg.byte_code_size() - 1;
	}
	m_cmp_deps = 0;
	m_cmp_jne = false;
	
	// block块
	if (fs->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, fs->block))
		{
			FKERR("[compiler] compile_for_stmt block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	// 结束
	if (fs->endblock)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, fs->endblock))
		{
			FKERR("[compiler] compile_for_stmt endblock fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	// 跳回判断地方
	cg.push(MAKE_OPCODE(OPCODE_JMP), fs->lineno());
	cg.push(MAKE_POS(startpos), fs->lineno());

	// 跳转出block块
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));

	// 替换掉break
	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	for (int i = 0; i < (int)bplist.size(); i++)
	{
		cg.set(bplist[i], MAKE_POS(cg.byte_code_size()));
	}
	m_loop_break_pos_stack.pop_back();
	
	m_loop_continue_pos_stack.pop_back();
	
	// 离开作用域
	cg.pop_stack_identifiers();

	FKLOG("[compiler] compile_for_stmt %p OK", fs);

	return true;
}

bool compiler::compile_multi_assign_stmt(codegen & cg, multi_assign_stmt * as)
{
	FKLOG("[compiler] compile_multi_assign_stmt %p", as);

	// 目前多重赋值只支持a,b,c = myfunc1()，需要告诉func1多返回几个值
	m_func_ret_num = as->varlist->varlist.size();

	// 编译value
	if (!compile_node(cg, as->value))
	{
		FKERR("[compiler] compile_multi_assign_stmt value fail");
		return false;
	}

	// 挨个编译var
	std::vector<command> varlist;
	for (int i = 0; i < (int)as->varlist->varlist.size(); i++)
	{
		m_new_var = as->isnew;
		if (!compile_node(cg, as->varlist->varlist[i]))
		{
			FKERR("[compiler] compile_multi_assign_stmt var fail");
			return false;
		}
		m_new_var = false;
		varlist.push_back(m_cur_addr);
	}

	// 挨个赋值
	for (int i = 0; i < (int)as->varlist->varlist.size(); i++)
	{
		command var = 0;
		command value = 0;

		var = varlist[i];
		value = m_cur_addrs[i];

		cg.push(MAKE_OPCODE(OPCODE_ASSIGN), as->lineno());
		cg.push(var, as->lineno());
		cg.push(value, as->lineno());
	}

	FKLOG("[compiler] compile_multi_assign_stmt %p OK", as);

	return true;
}

bool compiler::compile_return_value_list(codegen & cg, return_value_list_node * rn)
{
	FKLOG("[compiler] compile_return_value_list %p", rn);

	if (rn->returnlist.size() > MAX_FAKE_RETURN_NUM)
	{
		FKERR("[compiler] compile_return_value_list return too much");
		compile_seterror(rn, m_fk, efk_compile_variable_not_found, "return too much");
		return false;
	}
	
	command tmp[MAX_FAKE_RETURN_NUM];
	for (int i = 0; i < (int)rn->returnlist.size(); i++)
	{
		if (!compile_node(cg, rn->returnlist[i]))
		{
			FKERR("[compiler] compile_return_value_list value fail");
			return false;
		}
		tmp[i] = m_cur_addr;
	}
	memcpy(m_cur_addrs, tmp, sizeof(command) * MAX_FAKE_RETURN_NUM);
	m_cur_addr = m_cur_addrs[0];

	FKLOG("[compiler] compile_return_value_list %p OK", rn);

	return true;
}

bool compiler::compile_container_get(codegen & cg, container_get_node * cn)
{
	FKLOG("[compiler] compile_container_get %p", cn);

	// 编译con
	command con = 0;
	int pos = cg.getvariable(cn->container);
	if (pos == -1)
	{
		FKERR("[compiler] compile_container_get variable not found %s", cn->container.c_str());
		compile_seterror(cn, m_fk, efk_compile_variable_not_found, "variable %s not found", cn->container.c_str());
		return false;
	}
	con = MAKE_ADDR(ADDR_STACK, pos);

	// 编译key
	command key = 0;
	if (!compile_node(cg, cn->key))
	{
		FKERR("[compiler] compile_container_get key fail");
		return false;
	}
	key = m_cur_addr;

	// 返回
	int addrpos = cg.getcontaineraddr(con, key);
	m_cur_addr = MAKE_ADDR(ADDR_CONTAINER, addrpos);
	
	FKLOG("[compiler] compile_container_get %p OK", cn);

	return true;
}

bool compiler::compile_struct_pointer(codegen & cg, struct_pointer_node * sn)
{
	FKLOG("[compiler] compile_struct_pointer %p", sn);

	fake * fk = m_fk;

	String name = sn->str;
	std::vector<String> tmp;
	do
	{
		int pos = name.find("->");
		if (pos == -1)
		{
			tmp.push_back(name);
			break;
		}
		tmp.push_back(name.substr(0, pos));
		name = name.substr(pos + 2);
	}
	while (1);

	if (tmp.size() < 2)
	{
		FKERR("[compiler] compile_struct_pointer pointer %s fail", sn->str.c_str());
		return false;
	}

	String connname = tmp[0];
	
	// 编译con
	command con = 0;
	int pos = cg.getvariable(connname);
	if (pos == -1)
	{
		FKERR("[compiler] compile_struct_pointer variable not found %s", connname.c_str());
		compile_seterror(sn, m_fk, efk_compile_variable_not_found, "variable %s not found", connname.c_str());
		return false;
	}
	con = MAKE_ADDR(ADDR_STACK, pos);
	
	for (int i = 1; i < (int)tmp.size(); i++)
	{
		String keystr = tmp[i];
		
		// 编译key
		variant v;
		v = fk->sh.allocsysstr(keystr.c_str());
		int pos = cg.getconst(v);
		command key = MAKE_ADDR(ADDR_CONST, pos);

		// 获取容器的位置
		int addrpos = cg.getcontaineraddr(con, key);
		m_cur_addr = MAKE_ADDR(ADDR_CONTAINER, addrpos);
		con = m_cur_addr;
	}

	FKLOG("[compiler] compile_struct_pointer %p OK", sn);

	return true;
}

bool compiler::compile_sleep_stmt(codegen & cg, sleep_stmt * ss)
{
	FKLOG("[compiler] compile_sleep_stmt %p", ss);

	// 编译time
	command time = 0;
	if (!compile_node(cg, ss->time))
	{
		FKERR("[compiler] compile_sleep_stmt time fail");
		return false;
	}
	time = m_cur_addr;

	cg.push(MAKE_OPCODE(OPCODE_SLEEP), ss->lineno());
	cg.push(time, ss->lineno());
	
	FKLOG("[compiler] compile_sleep_stmt %p OK", ss);

	return true;
}

bool compiler::compile_yield_stmt(codegen & cg, yield_stmt * ys)
{
	FKLOG("[compiler] compile_yield_stmt %p", ys);

	// 编译time
	command time = 0;
	if (!compile_node(cg, ys->time))
	{
		FKERR("[compiler] compile_sleep_stmt time fail");
		return false;
	}
	time = m_cur_addr;

	cg.push(MAKE_OPCODE(OPCODE_YIELD), ys->lineno());
	cg.push(time, ys->lineno());
	
	FKLOG("[compiler] compile_yield_stmt %p OK", ys);

	return true;
}

bool compiler::compile_switch_stmt(codegen & cg, switch_stmt * ss)
{
	FKLOG("[compiler] compile_switch_stmt %p", ss);

	command caseleft;
	command caseresult;

	cg.push_stack_identifiers();
	
	// caseleft
	if (!compile_node(cg, ss->cmp))
	{
		FKERR("[compiler] compile_switch_stmt cmp fail");
		return false;
	}
	caseleft = m_cur_addr;

	// caseresult
	int despos = cg.alloc_stack_identifier();
	caseresult = MAKE_ADDR(ADDR_STACK, despos);

	switch_caselist_node * scln = dynamic_cast<switch_caselist_node *>(ss->caselist);

	std::vector<int> jmpswitchposlist;
	
	// 挨个和case的比较
	for (int i = 0; i < (int)scln->list.size(); i++)
	{
		command oper = MAKE_OPCODE(OPCODE_EQUAL);
		command left = caseleft;
		command right = 0;
		command dest = caseresult;

		switch_case_node * scn = dynamic_cast<switch_case_node *>(scln->list[i]);
	
		// right
		if (!compile_node(cg, scn->cmp))
		{
			FKERR("[compiler] compile_switch_stmt case cmp fail");
			return false;
		}
		right = m_cur_addr;

		// push case
		cg.push(oper, scn->lineno());
		cg.push(left, scn->lineno());
		cg.push(right, scn->lineno());
		cg.push(dest, scn->lineno());

		// push jmp
		cg.push(MAKE_OPCODE(OPCODE_JNE), scn->lineno());
		cg.push(dest, scn->lineno());
		cg.push(EMPTY_CMD, scn->lineno()); // 先塞个位置
		int jnepos = cg.byte_code_size() - 1;

		// build block
		if (scn->block)
		{
			cg.push_stack_identifiers();
			if (!compile_node(cg, scn->block))
			{
				FKERR("[compiler] compile_switch_stmt block fail");
				return false;
			}
			cg.pop_stack_identifiers();
		}
		
		// 跳出switch块
		cg.push(MAKE_OPCODE(OPCODE_JMP), scn->lineno());
		cg.push(EMPTY_CMD, scn->lineno()); // 先塞个位置
		int jmpswitchpos = cg.byte_code_size() - 1;
		jmpswitchposlist.push_back(jmpswitchpos);
		
		// 跳转出case块
		cg.set(jnepos, MAKE_POS(cg.byte_code_size()));
		
	}

	// default
	if (ss->def)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, ss->def))
		{
			FKERR("[compiler] compile_switch_stmt default fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}
	
	cg.pop_stack_identifiers();

	// 塞跳出的
	for (int i = 0; i < (int)jmpswitchposlist.size(); i++)
	{
		cg.set(jmpswitchposlist[i], MAKE_POS(cg.byte_code_size()));
	}
	
	FKLOG("[compiler] compile_switch_stmt %p OK", ss);

	return true;
}

bool compiler::compile_for_loop_stmt(codegen & cg, for_loop_stmt * fs)
{
	FKLOG("[compiler] compile_for_loop_stmt %p", fs);

	int startpos = 0;
	int jnepos = 0;
	int continuepos = 0;

	m_loop_break_pos_stack.push_back(beak_pos_list());
	m_continue_end_pos_stack.push_back(continue_end_pos_list());

	// 初始值
	if (!compile_node(cg, fs->begin))
	{
		FKERR("[compiler] compile_for_loop_stmt begin fail");
		return false;
	}
	command begin = m_cur_addr;

	// 循环变量，这个作用域是全for都有效的
	cg.push_stack_identifiers();
	if (!compile_node(cg, fs->var))
	{
		FKERR("[compiler] compile_for_loop_stmt var fail");
		return false;
	}
	command var = m_cur_addr;

	// 最大值
	if (!compile_node(cg, fs->end))
	{
		FKERR("[compiler] compile_for_loop_stmt end fail");
		return false;
	}
	command end = m_cur_addr;

	// 变化值
	if (!compile_node(cg, fs->add))
	{
		FKERR("[compiler] compile_for_loop_stmt add fail");
		return false;
	}
	command add = m_cur_addr;

	// 塞for头
	cg.push(MAKE_OPCODE(OPCODE_FORBEGIN), fs->lineno());
	cg.push(var, fs->lineno());
	cg.push(begin, fs->lineno());
	cg.push(end, fs->lineno());
	cg.push(add, fs->lineno());
	cg.push(EMPTY_CMD, fs->lineno()); // 先塞个位置
	jnepos = cg.byte_code_size() - 1;

	// 需要continue end
	m_loop_continue_pos_stack.push_back(-1);

	startpos = cg.byte_code_size();

	// block块
	if (fs->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, fs->block))
		{
			FKERR("[compiler] compile_for_loop_stmt block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	continuepos = cg.byte_code_size();

	// 最大值
	if (!compile_node(cg, fs->end))
	{
		FKERR("[compiler] compile_for_loop_stmt end fail");
		return false;
	}
	end = m_cur_addr;

	// 变化值
	if (!compile_node(cg, fs->add))
	{
		FKERR("[compiler] compile_for_loop_stmt add fail");
		return false;
	}
	add = m_cur_addr;

	// 塞for loop
	cg.push(MAKE_OPCODE(OPCODE_FORLOOP), fs->lineno());
	cg.push(var, fs->lineno());
	cg.push(end, fs->lineno());
	cg.push(add, fs->lineno());
	cg.push(MAKE_POS(startpos), fs->lineno());

	// 跳转出block块
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));

	// 替换掉break
	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	for (int i = 0; i < (int)bplist.size(); i++)
	{
		cg.set(bplist[i], MAKE_POS(cg.byte_code_size()));
	}
	m_loop_break_pos_stack.pop_back();
	
	// 替换掉continue
	continue_end_pos_list & cplist = m_continue_end_pos_stack[m_continue_end_pos_stack.size() - 1];
	for (int i = 0; i < (int)cplist.size(); i++)
	{
		cg.set(cplist[i], MAKE_POS(continuepos));
	}
	m_continue_end_pos_stack.pop_back();
	
	m_loop_continue_pos_stack.pop_back();
	
	// 离开作用域
	cg.pop_stack_identifiers();

	FKLOG("[compiler] compile_for_loop_stmt %p OK", fs);

	return true;
}

