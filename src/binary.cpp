#include "binary.h"
#include "fake.h"

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
	
    OPCODE_DEF(AND)
    OPCODE_DEF(OR)
    OPCODE_DEF(LESS)
	OPCODE_DEF(MORE)
	OPCODE_DEF(EQUAL)
	OPCODE_DEF(MOREEQUAL)
	OPCODE_DEF(LESSEQUAL)
	OPCODE_DEF(NOTEQUAL)
	
	OPCODE_DEF(CALL)
	
#undef OPCODE_DEF
    }
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
        switch (m_const_list[i].type)
        {
        case variant::REAL:
            ret += "REAL";
            break;
        case variant::STRING:
            ret += "STRING";
            break;
        case variant::POINTER:
            ret += "POINTER";
            break;
        case variant::UUID:
            ret += "UUID";
            break;
        default:
            ret += "unknow";
            break;
        }
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

String binary::dump() const
{
    String ret;
	for (const vhashmap<funcunion>::ele * p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next())
    {
		const funcunion & f = p->t;
		if (f.havefb)
		{
			ret += f.fb.dump();
		}
    }
    return ret;
}
    
void binary::move()
{
	if (m_fk->bbin.m_shh.empty())
	{
		return;
	}

    for (const vhashmap<func_binary>::ele * p = m_fk->bbin.m_shh.first(); p != 0; p = m_fk->bbin.m_shh.next())
    {
        const func_binary & bin = p->t;
		add_func(p->k, bin);
    }
    m_fk->bbin.m_shh.clear();
}

bool binary::add_func(const variant & name, const func_binary & bin)
{
	m_fk->fm.add_func(name, bin);

	FKLOG("add func %s", vartostring(&name).c_str());

	return true;
}

String backupbinary::dump() const
{
    String ret;
	for (const vhashmap<func_binary>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
    {
        const func_binary & bin = p->t;
        ret += bin.dump();
    }
    return ret;
}

