#pragma once

#include "types.h"
#include "binary.h"

struct opt_ins_addr {
    opt_ins_addr(int _offset, command _addr) {
        offset = _offset;
        addr = _addr;
    }

    int offset;
    command addr;
};

struct opt_ins {
    int offset;
    int size;
    int code;
    std::vector<command> read;
    std::vector<command> write;
    std::vector<opt_ins_addr> src;
    std::vector<opt_ins_addr> dst;
};

class optimizer {
public:
    optimizer(fake *fk);

    ~optimizer();

    void clear();

    void optimize(func_binary &fb);

private:
    void optimize_no_use_stack(func_binary &fb);

    void optimize_no_use_container(func_binary &fb);

    bool is_addr_type(command cmd, int dsttype, int dstpos);

    void replace_all_addr(func_binary &fb, int dsttype, int frompos, int topos);

    command replace_addr_cmd(command cmd, int dsttype, int frompos, int topos);

    void optimize_stack_pos(func_binary &fb);

    void set_ins_write(opt_ins &ins, func_binary &fb, int ip);

    void set_ins_write_by_cmd(opt_ins &ins, func_binary &fb, command v_cmd);

    void set_ins_read(opt_ins &ins, func_binary &fb, int ip);

    void set_ins_read_by_cmd(opt_ins &ins, func_binary &fb, command v_cmd);

    void trans_inslist(func_binary &fb);

    opt_ins *get_assign_src_ins_from(int pos, command cmd);

    opt_ins *get_read_ins_from(int pos, command cmd);

    void replace_ins_addr(func_binary &fb, opt_ins_addr oldaddr, opt_ins_addr newaddr);

    void remove_ins(func_binary &fb, opt_ins &delins);

    void optimize_write_write(func_binary &fb);

    void optimize_write_assign(func_binary &fb);

    int get_assign_src_ins_num(command);

private:
    fake *m_fk;
    bool m_isopt;
    std::vector<opt_ins> m_inslist;
};
