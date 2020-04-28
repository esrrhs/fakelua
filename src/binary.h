#pragma once

#include "types.h"
#include "variant.h"
#include "hashmap.h"

typedef uint64_t command;
#define EMPTY_CMD (command(-1))

enum CommandType {
    COMMAND_OPCODE,
    COMMAND_ADDR,
    COMMAND_POS,
};

enum OpCodeType {
    OPCODE_ASSIGN,

    OPCODE_PLUS,
    OPCODE_MINUS,
    OPCODE_MULTIPLY,
    OPCODE_DIVIDE,
    OPCODE_DIVIDE_MOD,
    OPCODE_STRING_CAT,

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
    OPCODE_NOT,

    OPCODE_AND_JNE,
    OPCODE_OR_JNE,
    OPCODE_LESS_JNE,
    OPCODE_MORE_JNE,
    OPCODE_EQUAL_JNE,
    OPCODE_MOREEQUAL_JNE,
    OPCODE_LESSEQUAL_JNE,
    OPCODE_NOTEQUAL_JNE,
    OPCODE_NOT_JNE,

    OPCODE_CALL,

    OPCODE_SLEEP,
    OPCODE_YIELD,

    OPCODE_FOR,

    OPCODE_MAX,
};

enum AddrType {
    ADDR_STACK,
    ADDR_CONST,
    ADDR_CONTAINER,
};

struct container_addr {
    command con;
    command key;
};

enum CallType {
    CALL_NORMAL,
    CALL_FAKE,
    CALL_CLASSMEM,
};

#define MAKE_COMMAND(type, code) MAKEINT64(type, code)
#define MAKE_ADDR(addrtype, pos) MAKE_COMMAND(COMMAND_ADDR, MAKEINT32(addrtype, pos))
#define MAKE_OPCODE(op) MAKE_COMMAND(COMMAND_OPCODE, op)
#define MAKE_POS(pos) MAKE_COMMAND(COMMAND_POS, pos)

#define COMMAND_TYPE(cmd) HIINT32(cmd)
#define COMMAND_CODE(cmd) LOINT32(cmd)

#define ADDR_TYPE(code) HIINT16(code)
#define ADDR_POS(code) LOINT16(code)

const char *OpCodeStr(int opcode);

struct stack_variant_info {
    char name[32];
    int line;
    int pos;
};

struct fake;

class codegen;

struct func_binary {
    String dump(int pos = -1) const;

    bool save(fake *fk, buffer *b) const;

    bool load(fake *fk, buffer *b);

    // 最大栈空间
    int m_maxstack;
    // 参数个数
    int m_paramnum;
    // 名字
    const char *m_name;
    // 文件名
    const char *m_filename;
    // 包名
    const char *m_packagename;
    // 二进制缓冲区
    command *m_buff;
    int m_size;
    // 二进制行号缓冲区
    int *m_lineno_buff;
    int m_lineno_size;
    int m_end_lineno;
    // 常量
    variant *m_const_list;
    int m_const_list_num;
    // container地址
    container_addr *m_container_addr_list;
    int m_container_addr_list_num;
    // 调试信息，栈变量
    stack_variant_info *m_debug_stack_variant_info;
    int m_debug_stack_variant_info_num;
    // 序列
    int m_pos;
    // 占用标记
    mutable int m_use;
    // 备份
    mutable func_binary *m_backup;
    // 新标记
    mutable int m_fresh;
};

#define FUNC_BINARY_INI(fb) \
    memset(&(fb), 0, sizeof(func_binary))

#define FUNC_BINARY_NAME(fb) \
    ((fb).m_name)

#define FUNC_BINARY_FILENAME(fb) \
    ((fb).m_filename)

#define FUNC_BINARY_PACKAGENAME(fb) \
    ((fb).m_packagename)

#define FUNC_BINARY_CMDSIZE(fb) \
    ((fb).m_size)

#define FUNC_BINARY_LINENO_SIZE(fb) \
    ((fb).m_lineno_size)

#define FUNC_BINARY_LINENO(fb, pos) \
    (pos >= 0 && pos < (int)(fb).m_lineno_size) ? (fb).m_lineno_buff[pos] : ((fb).m_lineno_size > 0 ? FUNC_BINARY_END_LINENO(fb) : 0)

#define FUNC_BINARY_END_LINENO(fb) \
    ((fb).m_end_lineno)

#define FUNC_BINARY_SIZE(fb) \
    ((fb).m_size * sizeof(command))

#define FUNC_BINARY_MAX_STACK(fb) \
    ((fb).m_maxstack)

#define FUNC_BINARY_MAX_CONST(fb) \
    ((fb).m_const_list_num)

#define FUNC_BINARY_PARAMNUM(fb) \
    ((fb).m_paramnum)

#define FUNC_BINARY_USE(fb) \
    ((fb).m_use)

#define FUNC_BINARY_BACKUP(fb) \
    ((fb).m_backup)

#define FUNC_BINARY_BACKUP_MOVE(fb) \
    { \
        func_binary * tmp = (fb).m_backup; \
        (fb).m_backup = 0; \
        FUNC_BINARY_DELETE(fb); \
        memcpy((void *)&(fb), tmp, sizeof((fb))); \
        safe_fkfree(m_fk, tmp); \
    }

#define FUNC_BINARY_FRESH(fb) \
    ((fb).m_fresh)

#define FUNC_BINARY_DELETE(fb) \
    safe_fkfree(m_fk, (fb).m_name); \
    safe_fkfree(m_fk, (fb).m_filename); \
    safe_fkfree(m_fk, (fb).m_packagename); \
    safe_fkfree(m_fk, (fb).m_buff); \
    safe_fkfree(m_fk, (fb).m_lineno_buff); \
    safe_fkfree(m_fk, (fb).m_const_list); \
    safe_fkfree(m_fk, (fb).m_container_addr_list); \
    safe_fkfree(m_fk, (fb).m_debug_stack_variant_info); \
    if ((fb).m_backup) \
    { \
        safe_fkfree(m_fk, (fb).m_backup->m_name); \
        safe_fkfree(m_fk, (fb).m_backup->m_filename); \
        safe_fkfree(m_fk, (fb).m_backup->m_packagename); \
        safe_fkfree(m_fk, (fb).m_backup->m_buff); \
        safe_fkfree(m_fk, (fb).m_backup->m_lineno_buff); \
        safe_fkfree(m_fk, (fb).m_backup->m_const_list); \
        safe_fkfree(m_fk, (fb).m_backup->m_container_addr_list); \
        safe_fkfree(m_fk, (fb).m_backup->m_debug_stack_variant_info); \
    } \
    safe_fkfree(m_fk, (fb).m_backup)

class binary {
    friend class assembler;

public:
    force_inline binary(fake *fk) : m_fk(fk) {
    }

    force_inline ~binary() {
    }

    force_inline fake *getfake() {
        return m_fk;
    }

    force_inline void clear() {
        m_dump.clear();
    }

    bool add_func(const variant &name, const func_binary &bin);

    String &dump() const;

    String &dump(const char *func, int pos = -1) const;

    bool save(buffer *b) const;

    bool load(buffer *b);

    void move();

private:
    fake *m_fk;
    mutable String m_dump;
};

