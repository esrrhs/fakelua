#pragma once

#include "types.h"
#include "variant.h"

enum CommandType
{
    COMMAND_OPCODE,
    COMMAND_ADDR,
    COMMAND_POS,
};

enum OpCodeType
{
    OPCODE_ASSIGN,

    OPCODE_PLUS,
    OPCODE_MINUS,
    OPCODE_MULTIPLY,
	OPCODE_DIVIDE,
	OPCODE_DIVIDE_MOD,

    OPCODE_PLUS_ASSIGN,
    OPCODE_MINUS_ASSIGN,
    OPCODE_MULTIPLY_ASSIGN,
	OPCODE_DIVIDE_ASSIGN,
	OPCODE_DIVIDE_MOD_ASSIGN,

	OPCODE_RETURN,
	
	OPCODE_JNE,
	OPCODE_JMP,
	
    OPCODE_AND,
    OPCODE_OR,
    OPCODE_LESS,
	OPCODE_MORE,
	OPCODE_EQUAL,
	OPCODE_MOREEQUAL,
	OPCODE_LESSEQUAL,
	OPCODE_NOTEQUAL,
};

enum AddrType
{
	ADDR_STACK,
	ADDR_CONST,
};

#define MAKE_COMMAND(type, code) MAKEINT64(type, code)
#define MAKE_ADDR(addrtype, pos) MAKE_COMMAND(COMMAND_ADDR, MAKEINT32(addrtype, pos))
#define MAKE_OPCODE(op) MAKE_COMMAND(COMMAND_OPCODE, op)
#define MAKE_POS(pos) MAKE_COMMAND(COMMAND_POS, pos)

#define COMMAND_TYPE(cmd) HIINT32(cmd)
#define COMMAND_CODE(cmd) LOINT32(cmd)

#define ADDR_TYPE(code) HIINT16(code)
#define ADDR_POS(code) LOINT16(code)

const char * OpCodeStr(int opcode);

struct fuck;
class codegen;
typedef uint64_t command;
#define EMPTY_CMD (command(-1))
class func_binary
{
    friend class codegen;
    friend class interpreter;
    friend class binary;
public:
	force_inline func_binary(fuck * fk) : m_fk(fk)
    {
        m_buff = 0;
        m_size = 0;
    }

	force_inline size_t cmdsize() const
    {
        return m_size;
    }
    
	force_inline size_t size() const
    {
        return m_size * sizeof(command);
    }
    
    String dump() const;

	force_inline const String & getname() const
    {
        return m_name;
    }

	force_inline const command & getcmd(int pos) const
    {
        assert(pos >= 0 && pos < (int)m_size);
        return m_buff[pos];
    }

	force_inline const variant * getconst(int pos) const
    {
        assert(pos >= 0 && pos < (int)m_const_list_num);
        return &m_const_list[pos];
    }
    
private:
    fuck * m_fk;
    // 名字
    String m_name;
    // 二进制缓冲区
    command * m_buff;
    size_t m_size;
    // 常量
    variant * m_const_list;
    size_t m_const_list_num;
    // 序列
    int m_pos;
};

struct func_binary_hash_ele
{
    int pos;
    const String * name;
};

typedef std::vector<func_binary> func_binary_list;

class binary
{
public:
    force_inline binary(fuck * fk) : m_fk(fk), m_func_index_map(0), m_func_index_map_size(0)
    {
    }
    force_inline ~binary()
    {
    }

    force_inline fuck * getfuck()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        //m_func_index_map.clear();
        m_func_list.clear();
    }

    force_inline void build_map()
    {
        m_func_index_map_size = m_func_list.size() * 2;
        m_func_index_map = (func_binary_hash_ele*)safe_fkmalloc(m_fk, sizeof(func_binary_hash_ele) * m_func_index_map_size);
        memset(m_func_index_map, 0, sizeof(func_binary_hash_ele) * m_func_index_map_size);
        for (int i = 0; i < (int)m_func_list.size(); i++)
        {
            int pos = m_func_list[i].m_pos;
            const String & name = m_func_list[i].m_name;
            
            uint32_t hash = fkstrhash(name.c_str());
            int index = hash % m_func_index_map_size;
            for (int j = 0; j < (int)m_func_index_map_size; j++)
            {
                if (!m_func_index_map[j].name)
                {
                    m_func_index_map[j].pos = pos;
                    m_func_index_map[j].name = &name;
                    break;
                }
                index = (index + 1) % m_func_index_map_size;
            }
        }
    }

    force_inline bool is_have_func(const char * name) const
    {
        for (int i = 0; i < (int)m_func_list.size(); i++)
        {
            if (m_func_list[i].m_name == name)
            {
                return true;
            }
        }
        return false;
    }
    force_inline bool add_func(func_binary & bin)
    {
        const String & name = bin.getname();
        if (is_have_func(name.c_str()))
        {
            return false;
        }
        
        int index = m_func_list.size();
        bin.m_pos = index;
        m_func_list.push_back(bin);

        return true;
    }
    force_inline const func_binary * get_func(const char * name) const
    {
        uint32_t hash = fkstrhash(name);
        int index = hash % m_func_index_map_size;
        for (int j = 0; j < (int)m_func_index_map_size; j++)
        {
            const func_binary & funcbin = m_func_list[m_func_index_map[index].pos];
            const String & srcname = funcbin.m_name;
            if (&srcname == m_func_index_map[index].name)
            {
                // find it
                return &funcbin;
            }
            index = (index + 1) % m_func_index_map_size;
        }
        return 0;
    }

    String dump() const;
    
private:
    fuck * m_fk;    
    func_binary_list m_func_list;
    func_binary_hash_ele* m_func_index_map;
    size_t m_func_index_map_size;
};

