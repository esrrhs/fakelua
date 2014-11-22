#pragma once

#include "types.h"
#include "variant.h"
#include "hashmap.h"

typedef uint64_t command;
#define EMPTY_CMD (command(-1))

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
	ADDR_CONTAINER,
};

struct container_addr
{
    command con;
    command key;
};

enum CallType
{
	CALL_NORMAL,
	CALL_FAKE,
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
struct func_binary
{
    String dump() const;
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
    // container地址
    container_addr * m_container_addr_list;
    size_t m_container_addr_list_num;
    // 序列
    int m_pos;
};

#define FUNC_BINARY_INI(fb) \
	memset(&(fb), 0, sizeof(func_binary))

#define FUNC_BINARY_NAME(fb) \
	((fb).m_name)

#define FUNC_BINARY_CMDSIZE(fb) \
	((fb).m_size)

#define FUNC_BINARY_SIZE(fb) \
	((fb).m_size * sizeof(command))

#define FUNC_BINARY_MAX_STACK(fb) \
	((fb).m_maxstack)

#define FUNC_BINARY_PARAMNUM(fb) \
	((fb).m_paramnum)

#define FUNC_BINARY_DELETE(fb) \
	safe_fkfree(m_fk, (fb).m_name); \
	safe_fkfree(m_fk, (fb).m_buff); \
	safe_fkfree(m_fk, (fb).m_const_list); \
	safe_fkfree(m_fk, (fb).m_container_addr_list)

class backupbinary
{
    friend class binary;
public:
    force_inline backupbinary(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~backupbinary()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        for (const vhashmap<func_binary>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
        {
            const func_binary & bin = p->t;
			FUNC_BINARY_DELETE(bin);
        }
        m_shh.clear();
    }

	force_inline bool add_func(const variant & name, const func_binary & bin)
    {
		func_binary * old = m_shh.get(name);
        if (old)
		{
			FUNC_BINARY_DELETE(*old);
            FKLOG("backupbinary add_func del old %s", vartostring(&name).c_str());
    	}
		m_shh.add(name, bin);
        return true;
    }
    
    String dump() const;
    
private:
    fake * m_fk; 
    vhashmap<func_binary> m_shh;   
};

class binary
{
    friend class assembler;
public:
    force_inline binary(fake * fk) : m_fk(fk)
    {
    }

    force_inline ~binary()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
    }

	bool add_func(const variant & name, const func_binary & bin);
    
    String dump() const;
    
    void move();
    
private:
    fake * m_fk;    
};

