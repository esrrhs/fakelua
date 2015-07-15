#include "binary.h"
#include "fake.h"
#include "buffer.h"

const char * OpCodeStr(int opcode)
{
	switch (opcode)
	{
#define OPCODE_DEF(op)\
	case OPCODE_##op:\
		return #op;
		
	OPCODE_DEF(ASSIGN)

	OPCODE_DEF(PLUS)
	OPCODE_DEF(MINUS)
	OPCODE_DEF(MULTIPLY)
	OPCODE_DEF(DIVIDE)
	OPCODE_DEF(DIVIDE_MOD)

	OPCODE_DEF(PLUS_ASSIGN)
	OPCODE_DEF(MINUS_ASSIGN)
	OPCODE_DEF(MULTIPLY_ASSIGN)
	OPCODE_DEF(DIVIDE_ASSIGN)
	OPCODE_DEF(DIVIDE_MOD_ASSIGN)

	OPCODE_DEF(RETURN)
	
	OPCODE_DEF(JNE)
	OPCODE_DEF(JMP)
	
	OPCODE_DEF(FORBEGIN)
	OPCODE_DEF(FORLOOP)
	
	OPCODE_DEF(AND)
	OPCODE_DEF(OR)
	OPCODE_DEF(LESS)
	OPCODE_DEF(MORE)
	OPCODE_DEF(EQUAL)
	OPCODE_DEF(MOREEQUAL)
	OPCODE_DEF(LESSEQUAL)
	OPCODE_DEF(NOTEQUAL)
	OPCODE_DEF(NOT)
	
	OPCODE_DEF(AND_JNE)
	OPCODE_DEF(OR_JNE)
	OPCODE_DEF(LESS_JNE)
	OPCODE_DEF(MORE_JNE)
	OPCODE_DEF(EQUAL_JNE)
	OPCODE_DEF(MOREEQUAL_JNE)
	OPCODE_DEF(LESSEQUAL_JNE)
	OPCODE_DEF(NOTEQUAL_JNE)
	OPCODE_DEF(NOT_JNE)
	
	OPCODE_DEF(CALL)
	
	OPCODE_DEF(SLEEP)
	OPCODE_DEF(YIELD)
	
#undef OPCODE_DEF
	}
	assert(0);
	return "unknow";
}

String dump_addr(int code)
{
	String ret;
	int addrtype = HIINT16(code);
	int pos = LOINT16(code);
	switch (addrtype)
	{
	case ADDR_STACK:
		ret += "STACK";
		break;
	case ADDR_CONST:
		ret += "CONST";
		break;
	case ADDR_CONTAINER:
		ret += "CONTAINER";
		break;
	default:
		ret += "unknow ";
		ret += fkitoa(addrtype);
	}
	ret += "\t";
	ret += fkitoa(pos);
	return ret;
}

String func_binary::dump() const
{
	String ret;

	// 名字
	ret += "\n[";
	ret += m_name;
	ret += "]\n";

	// 最大栈
	ret += "\tmaxstack:\t";
	ret += fkitoa(m_maxstack);
	ret += "\n\n";
	
	// 常量表
	ret += "\t////// const define ";
	ret += fkitoa(m_const_list_num);
	ret += " //////\n";
	for (int i = 0; i < (int)m_const_list_num; i++)
	{  
		ret += "\t[";
		ret += fkitoa(i);
		ret += "]\t";
		ret += vartypetostring(m_const_list[i].type);
		ret += "\t";
		ret += vartostring(&m_const_list[i]);
		ret += "\n";
	}
	
	// 容器地址表
	ret += "\n\t////// container addr ";
	ret += fkitoa(m_container_addr_list_num);
	ret += " //////\n";
	for (int i = 0; i < (int)m_container_addr_list_num; i++)
	{  
		ret += "\t[";
		ret += fkitoa(i);
		ret += "]\t";
		command concmd = m_container_addr_list[i].con;
		int concode = COMMAND_CODE(concmd);
		ret += "[ CONTAINER ]\t";
		ret += dump_addr(concode);
		command keycmd = m_container_addr_list[i].key;
		int keycode = COMMAND_CODE(keycmd);
		ret += "\t[ KEY ]\t";
		ret += dump_addr(keycode);
		ret += "\n";
	}
	
	ret += "\n\t////// byte code ";
	ret += fkitoa(m_size);
	ret += " //////\n";
	// 字节码
	for (int i = 0; i < (int)m_size; i++)
	{	
		command cmd = m_buff[i];
		int type = COMMAND_TYPE(cmd);
		int code = COMMAND_CODE(cmd);
		ret += "\t[";
		ret += fkitoa(i);
		ret += "]";
		ret += "[LINE ";
		ret += fkitoa(m_lineno_buff[i]);
		ret += "]\t";
		ret += fkxtoa(cmd);
		ret += "\t";
		switch (type)
		{
		case COMMAND_OPCODE:
			{
				ret += "[";
				ret += OpCodeStr(code);
				ret += "]\t";
			}
			break;
		case COMMAND_ADDR:
			{
				ret += "[ ADDR ]\t";
				ret += dump_addr(code);
			}
			break;
		case COMMAND_POS:
			{
				ret += "[ POS  ]\t";
				ret += fkitoa(code);
			}
			break;
		default:
			{
				ret += "[unknow]\t";
			}
			break;
		}
		ret += "\n";
	}
	ret += "\n";
	return ret;
}

String & binary::dump() const
{
	m_dump.clear();
	for (const fkhashmap<variant, funcunion>::ele * p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next())
	{
		const funcunion & f = *p->t;
		if (f.havefb)
		{
			m_dump += f.fb.dump();
		}
	}
	return m_dump;
}
	
String & binary::dump(const char * func) const
{
	fake * fk = m_fk;
	m_dump.clear();
	variant funcv;
	V_SET_STRING(&funcv, func);
	const funcunion * f = m_fk->fm.get_func(funcv);
	if (f && f->havefb)
	{
		m_dump += f->fb.dump();
	}
	else
	{
		m_dump = "not find ";
		m_dump += func;
	}
	return m_dump;
}

bool binary::add_func(const variant & name, const func_binary & bin)
{
	m_fk->fm.add_func(name, bin);

	FKLOG("add func %s", vartostring(&name).c_str());

	return true;
}

bool binary::save(buffer * b) const
{
	int n = 0;
	for (const fkhashmap<variant, funcunion>::ele * p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next())
	{
		const funcunion & f = *p->t;
		if (f.havefb)
		{
			n++;
		}
	}
	if (!b->write((const char *)&n, sizeof(n)))
	{
		return false;
	}
	FKLOG("start save func_binary %d", n);

	for (const fkhashmap<variant, funcunion>::ele * p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next())
	{
		const variant & fv = p->k;
		const funcunion & f = *p->t;
		if (f.havefb)
		{
			if (!save_variant(m_fk, &fv, b))
			{
				return false;
			}
			if (!f.fb.save(m_fk, b))
			{
				return false;
			}
			FKLOG("save func_binary %s", vartostring(&fv).c_str());
		}
	}
	return true;
}

bool binary::load(buffer * b)
{
	int n = 0;
	if (!b->read((char *)&n, sizeof(n)))
	{
		return false;
	}
	FKLOG("start load func_binary %d", n);
	
	for (int i = 0; i < n; i++)
	{
		variant fv;
		if (!load_variant(m_fk, &fv, b))
		{
			return false;
		}
		func_binary fb;
		FUNC_BINARY_INI(fb);
		if (!fb.load(m_fk, b))
		{
			return false;
		}
		if (!add_func(fv, fb))
		{
			return false;
		}
		FKLOG("load func_binary %s", vartostring(&fv).c_str());
	}
	return true;
}

bool func_binary::save(fake * fk, buffer * b) const
{
	if (!b->write((const char *)&m_maxstack, sizeof(m_maxstack)))
	{
		return false;
	}
	if (!b->write((const char *)&m_paramnum, sizeof(m_paramnum)))
	{
		return false;
	}
	if (!save_string(fk, m_name, b))
	{
		return false;
	}
	if (!save_string(fk, m_filename, b))
	{
		return false;
	}
	if (!save_string(fk, m_packagename, b))
	{
		return false;
	}
	if (!b->write((const char *)&m_size, sizeof(m_size)))
	{
		return false;
	}
	if (!b->write((const char *)m_buff, m_size * sizeof(command)))
	{
		return false;
	}
	if (!b->write((const char *)&m_lineno_size, sizeof(m_lineno_size)))
	{
		return false;
	}
	if (!b->write((const char *)m_lineno_buff, m_lineno_size * sizeof(int)))
	{
		return false;
	}
	if (!b->write((const char *)&m_const_list_num, sizeof(m_const_list_num)))
	{
		return false;
	}
	for (int i = 0 ; i < m_const_list_num; i++)
	{
		if (!save_variant(fk, &m_const_list[i], b))
		{
			return false;
		}
	}
	if (!b->write((const char *)&m_container_addr_list_num, sizeof(m_container_addr_list_num)))
	{
		return false;
	}
	if (!b->write((const char *)m_container_addr_list, m_container_addr_list_num * sizeof(container_addr)))
	{
		return false;
	}
	return true;
}

bool func_binary::load(fake * fk, buffer * b)
{
	if (!b->read((char *)&m_maxstack, sizeof(m_maxstack)))
	{
		return false;
	}
	if (!b->read((char *)&m_paramnum, sizeof(m_paramnum)))
	{
		return false;
	}
	String name;
	if (!load_string(fk, name, b))
	{
		return false;
	}
	m_name = stringdump(fk, name.c_str(), name.size());
	String filename;
	if (!load_string(fk, filename, b))
	{
		return false;
	}
	m_filename = stringdump(fk, filename.c_str(), filename.size());
	String packagename;
	if (!load_string(fk, packagename, b))
	{
		return false;
	}
	m_packagename = stringdump(fk, packagename.c_str(), packagename.size());
	if (!b->read((char *)&m_size, sizeof(m_size)))
	{
		return false;
	}
	m_buff = (command *)safe_fkmalloc(fk, (m_size * sizeof(command)));
	if (!b->read((char *)m_buff, m_size * sizeof(command)))
	{
		return false;
	}
	if (!b->read((char *)&m_lineno_size, sizeof(m_lineno_size)))
	{
		return false;
	}
	m_lineno_buff = (int *)safe_fkmalloc(fk, (m_lineno_size * sizeof(int)));
	if (!b->read((char *)m_lineno_buff, m_lineno_size * sizeof(int)))
	{
		return false;
	}
	if (!b->read((char *)&m_const_list_num, sizeof(m_const_list_num)))
	{
		return false;
	}
	m_const_list = (variant *)safe_fkmalloc(fk, (m_const_list_num * sizeof(variant)));
	for (int i = 0 ; i < m_const_list_num; i++)
	{
		if (!load_variant(fk, &m_const_list[i], b))
		{
			return false;
		}
	}
	if (!b->write((const char *)&m_container_addr_list_num, sizeof(m_container_addr_list_num)))
	{
		return false;
	}
	m_container_addr_list = (container_addr *)safe_fkmalloc(fk, (m_container_addr_list_num * sizeof(container_addr)));
	if (!b->write((const char *)m_container_addr_list, m_container_addr_list_num * sizeof(container_addr)))
	{
		return false;
	}
	m_fresh++;
	return true;
}


