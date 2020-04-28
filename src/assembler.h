#pragma once

#include "types.h"
#include "codegen.h"
#include "syntree.h"
#include "asmgen.h"

// 每个bytecode pos -> machinecode pos，全量数据，用来替换跳转指令的机器指令地址
typedef std::map<int, int> posmap;
// 如果先解析jump，还没有jump目标的实际地址，就先存起来，结束后再一起替换
// key为jump地址在机器码中的位置，value为bytecode pos
typedef std::map<int, int> caremap;

#define MAX_ASSEMBLER_CONTAINER_NUM 20

struct fake;

class native;

class binary;

struct func_binary;

class assembler {
public:
    assembler(fake *fk, native *nt) : m_fk(fk), m_native(nt), m_isopen(false) {
    }

    ~assembler() {
    }

    void clear();

    bool compile(binary *bin);

    void open();

    void close();

private:
    bool compile_func(const func_binary &fb);

    bool compile_next(asmgen &asg, const func_binary &fb);

    bool compile_assign(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_return(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_math(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_math_assign(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_cmp(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_single(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_jne(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_jmp(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_call(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_cmp_jne(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_single_jne(asmgen &asg, const func_binary &fb, command cmd);

    bool compile_for(asmgen &asg, const func_binary &fb, command cmd);

private:
    void compile_seterror(const func_binary &fb, command cmd, efkerror err, const char *fmt, ...);

    bool compile_container_to_pos(asmgen &asg, const func_binary &fb, int conpos, int despos);

    bool compile_pos_to_container(asmgen &asg, const func_binary &fb, int srcpos, int conpos);

private:
    fake *m_fk;
    native *m_native;
    int m_pos;
    posmap m_posmap;
    caremap m_caremap;
    bool m_isopen;
    command m_conpos[MAX_ASSEMBLER_CONTAINER_NUM];
    int m_conposnum;
};

