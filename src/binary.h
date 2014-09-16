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
public:
    func_binary(fuck * fk) : m_fk(fk)
    {
        m_buff = 0;
        m_size = 0;
    }

    size_t cmdsize() const
    {
        return m_size;
    }
    
    size_t size() const
    {
        return m_size * sizeof(command);
    }
    
    String dump() const;

    String getname() const
    {
        return m_name;
    }

    command getcmd(int pos) const
    {
        assert(pos >= 0 && pos < (int)m_size);
        return m_buff[pos];
    }

	const variant * getconst(int pos) const
    {
        assert(pos >= 0 && pos < (int)m_const_list.size());
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
    const_list m_const_list;
};

typedef std::map<String, int> func_binary_map;
typedef std::vector<func_binary> func_binary_list;

class binary
{
public:
    binary(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    ~binary()
    {
    }

    void clear();
    fuck * getfuck();

    bool is_have_func(const String & name) const;
    bool add_func(func_binary & bin);
    const func_binary * get_func(const String & name) const;

    String dump() const;
    
private:
    fuck * m_fk;
    func_binary_map m_func_index_map;
    func_binary_list m_func_list;
};

