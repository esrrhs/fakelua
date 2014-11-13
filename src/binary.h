#pragma once

#include "types.h"
#include "variant.h"
#include "hashmap.h"

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
	
    OPCODE_CALL,
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

struct fake;
class codegen;
typedef uint64_t command;
#define EMPTY_CMD (command(-1))
class func_binary
{
    friend class codegen;
    friend class interpreter;
    friend class binary;
    friend class assembler;
    friend class compiler;
    friend class backupbinary;
public:
	force_inline func_binary(fake * fk) : m_fk(fk)
    {
        m_buff = 0;
        m_size = 0;
        m_maxstack = 0;
        m_paramnum = 0;
        m_const_list = 0;
        m_const_list_num = 0;
        m_pos = 0;
    }

	force_inline size_t cmdsize() const
    {
        return m_size;
    }
    
	force_inline size_t size() const
    {
        return m_size * sizeof(command);
    }

	force_inline size_t maxstack() const
    {
        return m_maxstack;
    }

	force_inline size_t paramnum() const
    {
        return m_paramnum;
    }

    String dump() const;

	force_inline const char * getname() const
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
    fake * m_fk;
    // 最大栈空间
    int m_maxstack;
    // 参数个数
    int m_paramnum;
    // 名字
    const char * m_name;
    // 二进制缓冲区
    command * m_buff;
    size_t m_size;
    // 常量
    variant * m_const_list;
    size_t m_const_list_num;
    // 序列
    int m_pos;
};

class backupbinary
{
    friend class binary;
public:
    force_inline backupbinary(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~backupbinary()
    {
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        for (const stringhashmap<func_binary>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
        {
            const func_binary & bin = p->t;
            safe_fkfree(m_fk, bin.m_name);
            safe_fkfree(m_fk, bin.m_buff);
            safe_fkfree(m_fk, bin.m_const_list);
        }
        m_shh.clear();
    }

    force_inline bool add_func(const char * name, const func_binary & bin)
    {
        func_binary * old = m_shh.get(name);
        if (old)
        {
            safe_fkfree(m_fk, old->m_name);
            safe_fkfree(m_fk, old->m_buff);
            safe_fkfree(m_fk, old->m_const_list);
            FKLOG("backupbinary add_func del old %s", name);
    	}
        m_shh.add(name, bin);
        return true;
    }
    force_inline const func_binary * get_func(const char * name) const
    {
        return m_shh.get(name);
    }
    
    String dump() const;
    
private:
    fake * m_fk; 
    stringhashmap<func_binary> m_shh;   
};

class binary
{
    friend class assembler;
public:
    force_inline binary(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~binary()
    {
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        for (const stringhashmap<func_binary>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
        {
            const func_binary & bin = p->t;
            safe_fkfree(m_fk, bin.m_name);
            safe_fkfree(m_fk, bin.m_buff);
            safe_fkfree(m_fk, bin.m_const_list);
        }
        m_shh.clear();
    }

    force_inline bool add_func(const char * name, const func_binary & bin)
    {
        func_binary * old = m_shh.get(name);
        if (old)
        {
            safe_fkfree(m_fk, old->m_name);
            safe_fkfree(m_fk, old->m_buff);
            safe_fkfree(m_fk, old->m_const_list);
            FKLOG("binary add_func del old %s", name);
    	}
        m_shh.add(name, bin);
        return true;
    }
    
    force_inline const func_binary * get_func(const char * name) const
    {
        return m_shh.get(name);
    }
   
    String dump() const;
    
    void move();
    
private:
    fake * m_fk;    
    stringhashmap<func_binary> m_shh;
};

