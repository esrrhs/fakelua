#pragma once

#include "types.h"
#include "binary.h"

typedef std::map<String, int> stack_identifiers_map;
typedef std::vector<stack_identifiers_map> stack_id_stack;

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
    
private:
    fuck * m_fk;
    stack_id_stack m_stack_id_stack;
	stack_identifiers_map m_stack_identifiers_map;
};

