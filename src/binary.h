#pragma once

#include "types.h"
#include "variant.h"

enum CommandType
{
    COMMAND_OPCODE,
    COMMAND_ADDR,
};

enum OpCodeType
{
    OPCODE_ASSIGN,
};

enum AddrType
{
	ADDR_STACK,
	ADDR_CONST,
};

#define MAKE_COMMAND(type, code) MAKEINT64(type, code)
#define MAKE_ADDR(addrtype, pos) MAKE_COMMAND(COMMAND_ADDR, MAKEINT32(addrtype, pos))
#define MAKE_OPCODE(op) MAKE_COMMAND(COMMAND_OPCODE, op)

#define COMMAND_TYPE(cmd) HIINT32(cmd)
#define COMMAND_CODE(cmd) LOINT32(cmd)

struct fuck;
class codegen;
typedef uint64_t command;
class func_binary
{
    friend class codegen;
public:
    func_binary(fuck * fk) : m_fk(fk)
    {
        m_buff = 0;
        m_size = 0;
    }

    size_t Size() const
    {
        return m_size * sizeof(command);
    }
    
    String dump() const;

    String getname() const
    {
        return m_name;
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

    bool is_have_func(const String & name) const;
    bool add_func(func_binary & bin);

    String dump() const;
    
private:
    fuck * m_fk;
    func_binary_map m_func_index_map;
    func_binary_list m_func_list;
};

