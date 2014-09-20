#pragma once

#include "types.h"
#include "binary.h"
#include "variant.h"

struct block_identifiers
{
    block_identifiers() : name(""), pos(0) {}
    block_identifiers(String _name, int _pos) : name(_name), pos(_pos) {}
    String name;
    int pos;
};

typedef std::vector<block_identifiers> block_identifiers_list;
typedef std::vector<block_identifiers_list> block_identifiers_stack;
typedef std::vector<command> byte_code_list;

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
        m_stackpos = 0;
        m_maxstackpos = 0;
        m_block_identifiers_stack.clear();
        m_byte_code_list.clear();
    }
    
    int add_stack_identifier(const String & name);

    int alloc_stack_identifier()
    {
        assert(m_block_identifiers_stack.size() > 0);
        int ret = m_stackpos;
        block_identifiers_list & list = m_block_identifiers_stack.back();
        list.push_back(block_identifiers("", m_stackpos));
        m_stackpos++;
        if (m_stackpos > m_maxstackpos)
        {
            m_maxstackpos = m_stackpos;
        }
        return ret;
    }

    void push_stack_identifiers()
    {
        m_block_identifiers_stack.push_back(block_identifiers_list());
    }
    
    void pop_stack_identifiers()
    {
        assert(m_block_identifiers_stack.size() > 0);
        block_identifiers_list & list = m_block_identifiers_stack.back();
        int stacksize = list.size();
        m_block_identifiers_stack.pop_back();
        m_stackpos -= stacksize;
        assert(m_stackpos >= 0);
    }

    int getvariable(const String & name)
    {
        assert(m_block_identifiers_stack.size() > 0);
        // ´ÓÏÂÍùÉÏÕÒ
        for (int i = (int)m_block_identifiers_stack.size() - 1; i >= 0; i--)
        {
            block_identifiers_list & list = m_block_identifiers_stack[i];
            for (int j = 0; j < (int)list.size(); j++)
            {
                if (name == list[j].name)
                {
                    return list[j].pos;
                }
            }
        }
        return -1;
    }

	int getconst(const variant & v);

    void push(uint64_t code)
    {
        m_byte_code_list.push_back(code);
    }
    void set(int pos, uint64_t code)
    {
        assert(pos >= 0 && pos < (int)m_byte_code_list.size());
        m_byte_code_list[pos] = code;
    }
    size_t byte_code_size() const
    {
        return m_byte_code_list.size();
    }

	void output(const String & name, func_binary * bin);
    
    int get_cur_variable_pos(const String & name)
    {
        assert(m_block_identifiers_stack.size() > 0);
        block_identifiers_list & list = m_block_identifiers_stack.back();
        for (int i = 0; i < (int)list.size(); i++)
        {
            if (name == list[i].name)
            {
                return list[i].pos;
            }
        }
        return -1;
    }
    
private:
    fuck * m_fk;
    int m_stackpos;
    int m_maxstackpos;
    block_identifiers_stack m_block_identifiers_stack;
	byte_code_list m_byte_code_list;
	const_list m_const_list;
};

