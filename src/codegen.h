#pragma once

#include "types.h"
#include "binary.h"
#include "variant.h"

typedef std::map<String, int> stack_identifiers_map;
typedef std::vector<stack_identifiers_map> stack_id_stack;
typedef std::vector<int> byte_code_list;
typedef std::vector<variant> const_list;

class fuck;
class codegen
{
public:
    codegen(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    ~codegen() {}

    void clear()
    {
        m_stack_id_stack.clear();
        m_stack_identifiers_map.clear();
        m_byte_code_list.clear();
    }
    
    bool add_stack_identifier(const String & name, int stackpos)
    {
        if (m_stack_identifiers_map.find(name) != m_stack_identifiers_map.end())
        {
            return false;
        }
        m_stack_identifiers_map[name] = stackpos;
        return true;
    }
    
    void push_stack_identifiers()
    {
        m_stack_id_stack.push_back(m_stack_identifiers_map);
    }
    
    void pop_stack_identifiers()
    {
        m_stack_identifiers_map = m_stack_id_stack[m_stack_id_stack.size() - 1];
        m_stack_id_stack.pop_back();
    }

    int getvariable(const String & name)
    {
        stack_identifiers_map::iterator it = m_stack_identifiers_map.find(name);
        if (it != m_stack_identifiers_map.end())
        {
            return it->second;
        }
        
        // ´ÓÏÂÍùÉÏÕÒ
        for (int i = (int)m_stack_id_stack.size() - 1; i >= 0; i++)
        {
            stack_identifiers_map & map = m_stack_id_stack[i];
            stack_identifiers_map::iterator it = map.find(name);
            if (it != map.end())
            {
                return it->second;
            }
        }
        return -1;
    }

	int getconst(const String & name, bool isstr);

    void push(int code)
    {
        m_byte_code_list.push_back(code);
    }
    
private:
    fuck * m_fk;
    stack_id_stack m_stack_id_stack;
	stack_identifiers_map m_stack_identifiers_map;
	byte_code_list m_byte_code_list;
	const_list m_const_list;
};

