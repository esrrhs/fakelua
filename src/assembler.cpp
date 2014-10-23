#include "assembler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "binary.h"
#include "asmgen.h"

void assembler::clear()
{
    m_pos = 0;
	m_posmap.clear();
	m_caremap.clear();
}

bool assembler::compile(binary * bin)
{
    FKLOG("[assembler] compile binary %p", bin);

    for (int i = 0; i < (int)bin->m_func_list.size(); i++)
    {
        const func_binary & fb = bin->m_func_list[i];
        if (!compile_func(fb))
        {
            FKERR("[assembler] compile compile_func %s fail", fb.getname().c_str());
            return false;
        }
    }
    
    String str = m_native->dump();
    FKLOG("[assembler] compile binary %d ok \n%s", bin, str.c_str());

    return true;
}

bool assembler::compile_func(const func_binary & fb)
{
    FKLOG("[assembler] compile_func func_binary %p", &fb);

    asmgen asg(m_fk);

    asg.start_func();

    int stacksize = (fb.m_maxstack + fb.m_const_list_num) * variant_size;
    FKLOG("[assembler] compile_func stack size %d", stacksize);
    asg.alloc_stack(stacksize);

    asg.copy_param(fb.paramnum());

    asg.copy_const(fb.m_const_list, fb.m_const_list_num, fb.m_maxstack);

    // loop
    m_pos = 0;
    while (m_pos < (int)fb.m_size)
    {
		m_posmap[m_pos] = asg.size();
		FKLOG("posmap %d %d", m_pos, asg.size());
        if (!compile_next(asg, fb))
        {
            FKERR("[assembler] compile_func compile_next %d fail", m_pos);
            return false;
        }
    }
    // end
	m_posmap[m_pos] = asg.size();

    // link jmp pos
    for (caremap::iterator it = m_caremap.begin(); it != m_caremap.end(); it++)
    {
        int jumpposoff = it->first;
        int bytecodepos = it->second;
        if (m_posmap.find(bytecodepos) != m_posmap.end())
        {
            int pos = m_posmap[bytecodepos];
            asg.set_int(jumpposoff, pos - (jumpposoff + sizeof(int)));
            FKLOG("loop set %d -> %d %d", jumpposoff, pos - (jumpposoff + sizeof(int)), pos);
        }
        else
        {
            assert(0);
        }
    }
    
    asg.stop_func();
    func_native nt(m_fk);
    asg.output(fb.getname(), &nt);
    int pos = m_fk->fm.get_func(fb.getname().c_str());
    assert(pos >= 0);
    m_native->set_func(pos, nt);
    
    String str = asg.source();

    FKLOG("[assembler] compile_func binary %p ok\n%s", &fb, str.c_str());

    return true;
}

bool assembler::compile_next(asmgen & asg, const func_binary & fb)
{
    command cmd = GET_CMD(fb, m_pos);
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    FKLOG("[assembler] compile_next cmd %d %d %s", type, code, OpCodeStr(code));
        
    assert (type == COMMAND_OPCODE);

    m_pos++;

    bool ret = false;

    // 执行对应命令，放一起switch效率更高，cpu有缓存
    switch (code)
    {
    case OPCODE_ASSIGN:
        {
            ret = compile_assign(asg, fb, cmd);
        }
        break;
	case OPCODE_RETURN:
	    {
	        ret = compile_return(asg, fb, cmd);
        }
        break;
    case OPCODE_PLUS:
    case OPCODE_MINUS:
    case OPCODE_MULTIPLY:
	case OPCODE_DIVIDE:
	case OPCODE_DIVIDE_MOD:
	    {
	        ret = compile_math(asg, fb, cmd);
        }
        break;
	case OPCODE_PLUS_ASSIGN:
	case OPCODE_MINUS_ASSIGN:
	case OPCODE_MULTIPLY_ASSIGN:
	case OPCODE_DIVIDE_ASSIGN:
	case OPCODE_DIVIDE_MOD_ASSIGN:
		{
			ret = compile_math_assign(asg, fb, cmd);
		}
		break;
	case OPCODE_AND:
	case OPCODE_OR:
	case OPCODE_LESS:
	case OPCODE_MORE:
	case OPCODE_EQUAL:
	case OPCODE_MOREEQUAL:
	case OPCODE_LESSEQUAL:
	case OPCODE_NOTEQUAL:
		{
			ret = compile_cmp(asg, fb, cmd);
		}
		break;
    case OPCODE_JNE:
		{
			ret = compile_jne(asg, fb, cmd);
		}
		break;
    case OPCODE_JMP:
		{
			ret = compile_jmp(asg, fb, cmd);
		}
		break;
    case OPCODE_CALL:
		{
			ret = compile_call(asg, fb, cmd);
		}
		break;
    default:
        assert(0);
        FKERR("[assembler] compile_next err code %d %s", code, OpCodeStr(code));
        break;
    }
    return true;
}

#define GET_VARIANT_POS(fb, v, pos) \
    command v##_cmd = GET_CMD(fb, pos);\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        v = (v##_addrpos);\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
		v = (v##_addrpos) + fb.maxstack(); \
    }\
    else\
    {\
		v = 0;\
        assert(0);\
        FKERR("next_assign assignaddrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        return false;\
    }


bool assembler::compile_assign(asmgen & asg, const func_binary & fb, command cmd)
{
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK);
    int var = 0;
    GET_VARIANT_POS(fb, var, m_pos);
    m_pos++;
    
    int value = 0;
    GET_VARIANT_POS(fb, value, m_pos);
    m_pos++;

    asg.variant_assign(var, value);

    FKLOG("assign from %d to pos %d", var, value);
    
    return true;
}

bool assembler::compile_return(asmgen & asg, const func_binary & fb, command cmd)
{
	if (GET_CMD(fb, m_pos) == EMPTY_CMD)
	{
		FKLOG("return empty");
        m_pos++;
		return true;
	}
	
    int ret = 0;
    GET_VARIANT_POS(fb, ret, m_pos);
    m_pos++;

    asg.variant_ret(ret);

	return true;
}

bool assembler::compile_math(asmgen & asg, const func_binary & fb, command cmd)
{
    int code = COMMAND_CODE(cmd);

    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;
    
    int right = 0;
    GET_VARIANT_POS(fb, right, m_pos);
    m_pos++;

    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK);
    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    switch (code)
    {
    case OPCODE_PLUS:
	    asg.variant_add(dest, left, right);
        break;
    case OPCODE_MINUS:
	    asg.variant_sub(dest, left, right);
        break;
    case OPCODE_MULTIPLY:
	    asg.variant_mul(dest, left, right);
        break;
	case OPCODE_DIVIDE:
	    asg.variant_div(dest, left, right);
        break;
	case OPCODE_DIVIDE_MOD:
		asg.variant_div_mod(dest, left, right);
        break;
    default:
        assert(0);
        FKERR("[assembler] compile_math err code %d %s", code, OpCodeStr(code));
        break;
    }
    
	return true;
}

bool assembler::compile_math_assign(asmgen & asg, const func_binary & fb, command cmd)
{
	int code = COMMAND_CODE(cmd);

	assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK);
	int left = 0;
	GET_VARIANT_POS(fb, left, m_pos);
	m_pos++;

	int right = 0;
	GET_VARIANT_POS(fb, right, m_pos);
	m_pos++;

	switch (code)
	{
	case OPCODE_PLUS_ASSIGN:
		asg.variant_add(left, left, right);
		break;
	case OPCODE_MINUS_ASSIGN:
		asg.variant_sub(left, left, right);
		break;
	case OPCODE_MULTIPLY_ASSIGN:
		asg.variant_mul(left, left, right);
		break;
	case OPCODE_DIVIDE_ASSIGN:
		asg.variant_div(left, left, right);
		break;
	case OPCODE_DIVIDE_MOD_ASSIGN:
		asg.variant_div_mod(left, left, right);
		break;
	default:
		assert(0);
		FKERR("[assembler] compile_math_assign err code %d %s", code, OpCodeStr(code));
		break;
	}

	return true;
}

bool assembler::compile_cmp(asmgen & asg, const func_binary & fb, command cmd)
{
	int code = COMMAND_CODE(cmd);

	int left = 0;
	GET_VARIANT_POS(fb, left, m_pos);
	m_pos++;

	int right = 0;
	GET_VARIANT_POS(fb, right, m_pos);
	m_pos++;

	assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK);
	int dest = 0;
	GET_VARIANT_POS(fb, dest, m_pos);
	m_pos++;

	switch (code)
	{
	case OPCODE_AND:
		asg.variant_and(dest, left, right);
		break;
	case OPCODE_OR:
		asg.variant_or(dest, left, right);
		break;
	case OPCODE_LESS:
		asg.variant_less(dest, left, right);
		break;
	case OPCODE_MORE:
		asg.variant_more(dest, left, right);
		break;
	case OPCODE_EQUAL:
		asg.variant_equal(dest, left, right);
		break;
	case OPCODE_MOREEQUAL:
		asg.variant_moreequal(dest, left, right);
		break;
	case OPCODE_LESSEQUAL:
		asg.variant_lessequal(dest, left, right);
		break;
	case OPCODE_NOTEQUAL:
		asg.variant_notequal(dest, left, right);
		break;
	default:
		assert(0);
		FKERR("[assembler] compile_cmp err code %d %s", code, OpCodeStr(code));
		break;
	}

	return true;
}

bool assembler::compile_jne(asmgen & asg, const func_binary & fb, command cmd)
{
	int cmp = 0;
	GET_VARIANT_POS(fb, cmp, m_pos);
	m_pos++;

    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
	m_pos++;

    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end())
    {
        jumppos = m_posmap[jump_bytecode_pos];
    }

	asg.variant_jne(cmp, jumppos);

    int jmpoffset = asg.size() - sizeof(int);
	if (jumppos == -1)
	{
	    // 记录下来
	    m_caremap[jmpoffset] = jump_bytecode_pos;
	    FKLOG("compile_jne caremap add %d %d", jmpoffset, jump_bytecode_pos);
	}
	else
	{
	    asg.set_int(jmpoffset, jumppos - asg.size());
	    FKLOG("compile_jne set jne add %d -> %d", jmpoffset, jumppos - asg.size());
	}
	
	return true;
}

bool assembler::compile_jmp(asmgen & asg, const func_binary & fb, command cmd)
{
    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
	m_pos++;

    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end())
    {
        jumppos = m_posmap[jump_bytecode_pos];
    }

    asg.variant_jmp(jumppos);

    int jmpoffset = asg.size() - sizeof(int);
	if (jumppos == -1)
	{
	    // 记录下来
	    m_caremap[jmpoffset] = jump_bytecode_pos;
	    FKLOG("compile_jmp caremap add %d %d", jmpoffset, jump_bytecode_pos);
	}
	else
	{
	    asg.set_int(jmpoffset, jumppos - asg.size());
	    FKLOG("compile_jmp set jne add %d -> %d", jmpoffset, jumppos - asg.size());
	}

	return true;
}

bool assembler::compile_call(asmgen & asg, const func_binary & fb, command cmd)
{
	int callpos = 0;
	GET_VARIANT_POS(fb, callpos, m_pos);
	m_pos++;

	int ret = 0;
	GET_VARIANT_POS(fb, ret, m_pos);
	m_pos++;

    int argnum = COMMAND_CODE(GET_CMD(fb, m_pos));
	m_pos++;
    
    m_pos += argnum;
    
    // TODO
    USE(callpos);
    
	return true;
}

