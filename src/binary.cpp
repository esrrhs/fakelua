#include "binary.h"
#include "fuck.h"

String func_binary::dump() const
{
    String ret;

    // 名字
    ret += "\n[";
    ret += m_name;
    ret += "]\n";
    
    // 常量表
    ret += "\t//////const define//////\n";
    for (int i = 0; i < (int)m_const_list.size(); i++)
    {  
        ret += "\t[";
        ret += fkitoa(i);
        ret += "]\t";
        switch (m_const_list[i].get_type())
        {
        case variant::BOOL:
            ret += "BOOL";
            break;
        case variant::INT:
            ret += "INT";
            break;
        case variant::REAL:
            ret += "REAL";
            break;
        case variant::STRING:
            ret += "STRING";
            break;
        case variant::POINTER:
            ret += "POINTER";
            break;
        default:
            ret += "unknow";
            break;
        }
        ret += "\t";
        String str = m_const_list[i];
        ret += str;
        ret += "\n";
    }
    
    ret += "\n\t//////byte code//////\n";
    // 字节码
    for (int i = 0; i < (int)m_size; i++)
    {    
        command cmd = m_buff[i];
        int type = COMMAND_TYPE(cmd);
        int code = COMMAND_CODE(cmd);
        ret += "\t[";
        ret += fkitoa(i);
        ret += "]\t";
        switch (type)
        {
        case COMMAND_OPCODE:
            {
                ret += "[OPCODE]\t";
                switch (code)
                {
                case OPCODE_ASSIGN:
                    ret += "ASSIGN";
                    break;
                default:
                    ret += "unknow ";
                    ret += fkitoa(code);
                }
            }
            break;
        case COMMAND_ADDR:
            {
                ret += "[ ADDR ]\t";
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
                default:
                    ret += "unknow ";
                    ret += fkitoa(addrtype);
                }
                ret += "\t";
                ret += fkitoa(pos);
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
    
void binary::clear()
{
    m_func_index_map.clear();
    m_func_list.clear();
}

bool binary::is_have_func(const String & name) const
{
    return m_func_index_map.find(name) != m_func_index_map.end();
}

bool binary::add_func(func_binary & bin)
{
    const String & name = bin.getname();
    if (is_have_func(name))
    {
        return false;
    }
    
    int index = m_func_list.size();
    m_func_list.push_back(bin);
    m_func_index_map[name] = index;

    return true;
}

String binary::dump() const
{
    String ret;
    for (int i = 0; i < (int)m_func_list.size(); i++)
    {
        const func_binary & bin = m_func_list[i];
        ret += bin.dump();
    }
    return ret;
}
    

