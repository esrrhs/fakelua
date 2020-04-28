#include "optimizer.h"
#include <set>

optimizer::optimizer(fake *fk) {
    m_fk = fk;
    m_isopt = false;
}

optimizer::~optimizer() {
}

void optimizer::clear() {
    m_isopt = false;
}

void optimizer::optimize(func_binary &fb) {
    FKLOG("before %s", fb.dump(-1).c_str());

    while (true) {
        m_isopt = false;

        trans_inslist(fb);

        if (!m_isopt) {
            optimize_write_assign(fb);
        }
        if (!m_isopt) {
            optimize_write_write(fb);
        }
        if (!m_isopt) {
            optimize_no_use_container(fb);
        }
        if (!m_isopt) {
            optimize_no_use_stack(fb);
        }
        if (!m_isopt) {
            optimize_stack_pos(fb);
        }

        if (m_isopt) {
            FKLOG("optimize %s", fb.dump(-1).c_str());
        } else {
            break;
        }
    }
}

void optimizer::optimize_no_use_stack(func_binary &fb) {
    for (int i = 0; i < fb.m_debug_stack_variant_info_num; i++) {
        stack_variant_info &info = fb.m_debug_stack_variant_info[i];
        int pos = info.pos;

        if (pos < fb.m_paramnum) {
            continue;
        }

        bool use = false;
        for (int j = 0; j < fb.m_size; j++) {
            command cmd = fb.m_buff[j];
            if (is_addr_type(cmd, ADDR_STACK, pos)) {
                use = true;
                break;
            }
        }

        for (int j = 0; j < fb.m_container_addr_list_num; j++) {
            container_addr &addr = fb.m_container_addr_list[j];
            if (is_addr_type(addr.con, ADDR_STACK, pos)) {
                use = true;
                break;
            }
            if (is_addr_type(addr.key, ADDR_STACK, pos)) {
                use = true;
                break;
            }
        }

        if (!use) {
            stack_variant_info *newlist = (stack_variant_info *) safe_fkmalloc(m_fk,
                                                                               (fb.m_debug_stack_variant_info_num - 1) *
                                                                               sizeof(stack_variant_info),
                                                                               emt_func_binary);
            for (int z = 0; z < i; z++) {
                newlist[z] = fb.m_debug_stack_variant_info[z];
            }
            for (int z = i; z < fb.m_debug_stack_variant_info_num - 1; z++) {
                newlist[z] = fb.m_debug_stack_variant_info[z + 1];
            }
            safe_fkfree(m_fk, fb.m_debug_stack_variant_info);
            fb.m_debug_stack_variant_info = newlist;
            fb.m_debug_stack_variant_info_num--;

            m_isopt = true;
        }
    }
}

void optimizer::optimize_no_use_container(func_binary &fb) {
    for (int i = 0; i < fb.m_container_addr_list_num; i++) {
        bool use = false;
        for (int j = 0; j < fb.m_size; j++) {
            command cmd = fb.m_buff[j];
            if (is_addr_type(cmd, ADDR_CONTAINER, i)) {
                use = true;
                break;
            }
        }

        for (int j = 0; j < fb.m_container_addr_list_num; j++) {
            container_addr &addr = fb.m_container_addr_list[j];
            if (is_addr_type(addr.con, ADDR_CONTAINER, i)) {
                use = true;
                break;
            }
            if (is_addr_type(addr.key, ADDR_CONTAINER, i)) {
                use = true;
                break;
            }
        }

        if (!use) {
            for (int j = i; j < fb.m_container_addr_list_num - 1; j++) {
                replace_all_addr(fb, ADDR_CONTAINER, j + 1, j);
            }

            container_addr *newlist = (container_addr *) safe_fkmalloc(m_fk, (fb.m_container_addr_list_num - 1) *
                                                                             sizeof(container_addr), emt_func_binary);
            for (int j = 0; j < i; j++) {
                newlist[j] = fb.m_container_addr_list[j];
            }
            for (int j = i; j < fb.m_container_addr_list_num - 1; j++) {
                newlist[j] = fb.m_container_addr_list[j + 1];
            }
            safe_fkfree(m_fk, fb.m_container_addr_list);
            fb.m_container_addr_list = newlist;
            fb.m_container_addr_list_num--;

            m_isopt = true;
            return;
        }
    }
}

bool optimizer::is_addr_type(command cmd, int dsttype, int dstpos) {
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    if (type == COMMAND_ADDR) {
        int addrtype = HIINT16(code);
        int pos = LOINT16(code);

        if (addrtype == dsttype && pos == dstpos) {
            return true;
        }
    }
    return false;
}

void optimizer::replace_all_addr(func_binary &fb, int dsttype, int frompos, int topos) {
    for (int j = 0; j < fb.m_size; j++) {
        command cmd = fb.m_buff[j];
        fb.m_buff[j] = replace_addr_cmd(cmd, dsttype, frompos, topos);
    }

    for (int j = 0; j < fb.m_container_addr_list_num; j++) {
        container_addr &addr = fb.m_container_addr_list[j];
        addr.con = replace_addr_cmd(addr.con, dsttype, frompos, topos);
        addr.key = replace_addr_cmd(addr.key, dsttype, frompos, topos);
    }
}

command optimizer::replace_addr_cmd(command cmd, int dsttype, int frompos, int topos) {
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    if (type == COMMAND_ADDR) {
        int addrtype = HIINT16(code);
        int pos = LOINT16(code);
        if (addrtype == dsttype && pos == frompos) {
            pos = topos;
            cmd = MAKE_ADDR(addrtype, pos);
        }
    }

    return cmd;
}

void optimizer::optimize_stack_pos(func_binary &fb) {
    // 整理下栈，可能有空洞
    std::set<int> posset;

    int max = -1;
    for (int j = 0; j < fb.m_paramnum; j++) {
        posset.insert(j);
        if (j > max) {
            max = j;
        }
    }

    for (int j = 0; j < fb.m_size; j++) {
        command cmd = fb.m_buff[j];
        int type = COMMAND_TYPE(cmd);
        int code = COMMAND_CODE(cmd);

        if (type == COMMAND_ADDR) {
            int addrtype = HIINT16(code);
            int pos = LOINT16(code);
            if (addrtype == ADDR_STACK) {
                posset.insert(pos);
                if (pos > max) {
                    max = pos;
                }
            }
        }
    }
    for (int j = 0; j < fb.m_container_addr_list_num; j++) {
        container_addr &addr = fb.m_container_addr_list[j];

        command cmd = addr.con;
        int type = COMMAND_TYPE(cmd);
        int code = COMMAND_CODE(cmd);

        if (type == COMMAND_ADDR) {
            int addrtype = HIINT16(code);
            int pos = LOINT16(code);
            if (addrtype == ADDR_STACK) {
                posset.insert(pos);
                if (pos > max) {
                    max = pos;
                }
            }
        }

        cmd = addr.key;
        type = COMMAND_TYPE(cmd);
        code = COMMAND_CODE(cmd);

        if (type == COMMAND_ADDR) {
            int addrtype = HIINT16(code);
            int pos = LOINT16(code);
            if (addrtype == ADDR_STACK) {
                posset.insert(pos);
                if (pos > max) {
                    max = pos;
                }
            }
        }
    }

    if ((int) posset.size() == max + 1) {
        return;
    }

    int empty = -1;
    for (int j = 0; j < max + 1; j++) {
        if (posset.find(j) == posset.end()) {
            empty = j;
            break;
        }
    }

    for (int i = empty; i < max; i++) {
        replace_all_addr(fb, ADDR_STACK, i + 1, i);
    }

    for (int i = empty; i < max; i++) {
        for (int j = 0; j < fb.m_debug_stack_variant_info_num; j++) {
            stack_variant_info &info = fb.m_debug_stack_variant_info[j];
            if (info.pos == i) {
                stack_variant_info *newlist = (stack_variant_info *) safe_fkmalloc(m_fk,
                                                                                   (fb.m_debug_stack_variant_info_num -
                                                                                    1) * sizeof(stack_variant_info),
                                                                                   emt_func_binary);
                for (int z = 0; z < j; z++) {
                    newlist[z] = fb.m_debug_stack_variant_info[z];
                }
                for (int z = j; z < fb.m_debug_stack_variant_info_num - 1; z++) {
                    newlist[z] = fb.m_debug_stack_variant_info[z + 1];
                }
                safe_fkfree(m_fk, fb.m_debug_stack_variant_info);
                fb.m_debug_stack_variant_info = newlist;
                fb.m_debug_stack_variant_info_num--;
                break;
            }
        }
        for (int j = 0; j < fb.m_debug_stack_variant_info_num; j++) {
            stack_variant_info &info = fb.m_debug_stack_variant_info[j];
            if (info.pos == i + 1) {
                info.pos = i;
            }
        }
    }

    m_isopt = true;
}

void optimizer::set_ins_write(opt_ins &ins, func_binary &fb, int ip) {
    command v_cmd = fb.m_buff[ip];
    ins.dst.push_back(opt_ins_addr(ip, v_cmd));
    set_ins_write_by_cmd(ins, fb, v_cmd);
}

void optimizer::set_ins_write_by_cmd(opt_ins &ins, func_binary &fb, command v_cmd) {
    int v_addrtype = ADDR_TYPE(COMMAND_CODE(v_cmd));
    int v_addrpos = ADDR_POS(COMMAND_CODE(v_cmd));
    if (v_addrtype == ADDR_STACK) {
        ins.write.push_back(v_cmd);
    } else if (v_addrtype == ADDR_CONST) {
    } else if (v_addrtype == ADDR_CONTAINER) {
        container_addr ca = fb.m_container_addr_list[v_addrpos];

        set_ins_read_by_cmd(ins, fb, ca.con);
        set_ins_write_by_cmd(ins, fb, ca.con);
        set_ins_read_by_cmd(ins, fb, ca.key);
    }
}

void optimizer::set_ins_read(opt_ins &ins, func_binary &fb, int ip) {
    command v_cmd = fb.m_buff[ip];
    ins.src.push_back(opt_ins_addr(ip, v_cmd));
    set_ins_read_by_cmd(ins, fb, v_cmd);
}

void optimizer::set_ins_read_by_cmd(opt_ins &ins, func_binary &fb, command v_cmd) {
    int v_addrtype = ADDR_TYPE(COMMAND_CODE(v_cmd));
    int v_addrpos = ADDR_POS(COMMAND_CODE(v_cmd));
    if (v_addrtype == ADDR_STACK) {
        ins.read.push_back(v_cmd);
    } else if (v_addrtype == ADDR_CONST) {
    } else if (v_addrtype == ADDR_CONTAINER) {
        container_addr ca = fb.m_container_addr_list[v_addrpos];

        set_ins_read_by_cmd(ins, fb, ca.con);
        set_ins_write_by_cmd(ins, fb, ca.con);
        set_ins_read_by_cmd(ins, fb, ca.key);
    }
}

void optimizer::trans_inslist(func_binary &fb) {
    m_inslist.clear();

    int ip = 0;
    while (ip < fb.m_size) {
        int code = COMMAND_CODE(fb.m_buff[ip]);

        opt_ins ins;
        ins.code = code;
        ins.offset = ip;

        ip++;

        // 执行对应命令，放一起switch效率更高，cpu有缓存
        switch (code) {
            case OPCODE_ASSIGN: {
                set_ins_write(ins, fb, ip);
                ip++;

                // 赋值来源
                set_ins_read(ins, fb, ip);
                ip++;
            }
                break;

            case OPCODE_PLUS:
            case OPCODE_MINUS:
            case OPCODE_MULTIPLY:
            case OPCODE_DIVIDE:
            case OPCODE_DIVIDE_MOD:
            case OPCODE_STRING_CAT:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_LESS:
            case OPCODE_MORE:
            case OPCODE_EQUAL:
            case OPCODE_MOREEQUAL:
            case OPCODE_LESSEQUAL:
            case OPCODE_NOTEQUAL: {
                set_ins_read(ins, fb, ip);
                ip++;

                set_ins_read(ins, fb, ip);
                ip++;

                set_ins_write(ins, fb, ip);
                ip++;
            }
                break;

            case OPCODE_NOT: {
                set_ins_read(ins, fb, ip);
                ip++;

                set_ins_write(ins, fb, ip);
                ip++;
            }
                break;

            case OPCODE_AND_JNE:
            case OPCODE_OR_JNE:
            case OPCODE_LESS_JNE:
            case OPCODE_MORE_JNE:
            case OPCODE_EQUAL_JNE:
            case OPCODE_MOREEQUAL_JNE:
            case OPCODE_LESSEQUAL_JNE:
            case OPCODE_NOTEQUAL_JNE: {
                set_ins_read(ins, fb, ip);
                ip++;

                set_ins_read(ins, fb, ip);
                ip++;

                /* dest */
                ip++;
                ip++;
            }
                break;

            case OPCODE_NOT_JNE: {
                set_ins_read(ins, fb, ip);
                ip++;

                /* dest */
                ip++;
                ip++;
            }
                break;

            case OPCODE_JNE: {
                set_ins_read(ins, fb, ip);
                ip++;

                ip++;
            }
                break;

            case OPCODE_JMP: {
                ip++;
            }
                break;

            case OPCODE_PLUS_ASSIGN:
            case OPCODE_MINUS_ASSIGN:
            case OPCODE_MULTIPLY_ASSIGN:
            case OPCODE_DIVIDE_ASSIGN:
            case OPCODE_DIVIDE_MOD_ASSIGN: {
                set_ins_read(ins, fb, ip);
                set_ins_write(ins, fb, ip);
                ip++;

                set_ins_read(ins, fb, ip);
                ip++;
            }
                break;

            case OPCODE_CALL: {
                ip++;

                set_ins_read(ins, fb, ip);
                ip++;

                int retnum = COMMAND_CODE(fb.m_buff[ip]);
                ip++;

                for (int i = 0; i < retnum; i++) {
                    set_ins_write(ins, fb, ip);
                    ip++;
                }

                int argnum = COMMAND_CODE(fb.m_buff[ip]);
                ip++;

                for (int i = 0; i < argnum; i++) {
                    set_ins_write(ins, fb, ip);
                    set_ins_read(ins, fb, ip);
                    ip++;
                }
            }
                break;

            case OPCODE_FOR: {
                set_ins_read(ins, fb, ip);
                set_ins_write(ins, fb, ip);
                ip++;

                set_ins_read(ins, fb, ip);
                ip++;

                set_ins_read(ins, fb, ip);
                ip++;

                set_ins_read(ins, fb, ip);
                set_ins_write(ins, fb, ip);
                ip++;

                ip++;
            }
                break;

            case OPCODE_RETURN: {
                int returnnum = COMMAND_CODE(fb.m_buff[ip]);
                ip++;

                // 塞给ret
                for (int i = 0; i < returnnum; i++) {
                    set_ins_read(ins, fb, ip);
                    ip++;
                }
            }
                break;

            case OPCODE_SLEEP: {
                set_ins_read(ins, fb, ip);
                ip++;
            }
                break;

            case OPCODE_YIELD: {
                set_ins_read(ins, fb, ip);
                ip++;
            }
                break;

            default: {
                FKERR("next err code %d %s ", code, OpCodeStr(code));
                assert(0);
            }
        }
        ins.size = ip - ins.offset;
        m_inslist.push_back(ins);
    }
}

int optimizer::get_assign_src_ins_num(command cmd) {
    int num = 0;
    for (int i = 0; i < (int) m_inslist.size(); i++) {
        opt_ins &ins = m_inslist[i];
        if (ins.code == OPCODE_ASSIGN) {
            for (int j = 0; j < (int) ins.src.size(); j++) {
                if (ins.src[j].addr == cmd) {
                    num++;
                }
            }
        }
    }
    return num;
}

opt_ins *optimizer::get_assign_src_ins_from(int pos, command cmd) {
    for (int i = pos; i < (int) m_inslist.size(); i++) {
        opt_ins &ins = m_inslist[i];
        if (ins.code == OPCODE_ASSIGN) {
            for (int j = 0; j < (int) ins.src.size(); j++) {
                if (ins.src[j].addr == cmd) {
                    return &ins;
                }
            }
        }
    }
    return 0;
}

opt_ins *optimizer::get_read_ins_from(int pos, command cmd) {
    for (int i = pos; i < (int) m_inslist.size(); i++) {
        opt_ins &ins = m_inslist[i];
        for (int j = 0; j < (int) ins.read.size(); j++) {
            if (ins.read[j] == cmd) {
                return &ins;
            }
        }
    }
    return 0;
}

void optimizer::replace_ins_addr(func_binary &fb, opt_ins_addr oldaddr, opt_ins_addr newaddr) {
    int offset = oldaddr.offset;
    fb.m_buff[offset] = newaddr.addr;
}

void optimizer::remove_ins(func_binary &fb, opt_ins &delins) {
    // 刷一遍jmp，然后删除
    for (int i = 0; i < (int) m_inslist.size(); i++) {
        opt_ins &ins = m_inslist[i];
        switch (ins.code) {
            case OPCODE_AND_JNE:
            case OPCODE_OR_JNE:
            case OPCODE_LESS_JNE:
            case OPCODE_MORE_JNE:
            case OPCODE_EQUAL_JNE:
            case OPCODE_MOREEQUAL_JNE:
            case OPCODE_LESSEQUAL_JNE:
            case OPCODE_NOTEQUAL_JNE: {
                int destip = COMMAND_CODE(fb.m_buff[ins.offset + 4]);
                if (destip > delins.offset) {
                    fb.m_buff[ins.offset + 4] = MAKE_POS(destip - delins.size);
                }
            }
                break;

            case OPCODE_NOT_JNE: {
                int destip = COMMAND_CODE(fb.m_buff[ins.offset + 3]);
                if (destip > delins.offset) {
                    fb.m_buff[ins.offset + 3] = MAKE_POS(destip - delins.size);
                }
            }
                break;

            case OPCODE_JNE: {
                int destip = COMMAND_CODE(fb.m_buff[ins.offset + 2]);
                if (destip > delins.offset) {
                    fb.m_buff[ins.offset + 2] = MAKE_POS(destip - delins.size);
                }
            }
                break;

            case OPCODE_JMP: {
                int destip = COMMAND_CODE(fb.m_buff[ins.offset + 1]);
                if (destip > delins.offset) {
                    fb.m_buff[ins.offset + 1] = MAKE_POS(destip - delins.size);
                }
            }
                break;

            case OPCODE_FOR: {
                int destip = COMMAND_CODE(fb.m_buff[ins.offset + 5]);
                if (destip > delins.offset) {
                    fb.m_buff[ins.offset + 5] = MAKE_POS(destip - delins.size);
                }
            }
                break;

        }
    }

    command *newbuff = (command *) safe_fkmalloc(m_fk, (fb.m_size - delins.size) * sizeof(command), emt_func_binary);
    int *newlinenobuff = (int *) safe_fkmalloc(m_fk, (fb.m_lineno_size - delins.size) * sizeof(int), emt_func_binary);

    for (int i = 0; i < delins.offset; i++) {
        newbuff[i] = fb.m_buff[i];
        newlinenobuff[i] = fb.m_lineno_buff[i];
    }

    for (int i = 0; i < fb.m_size - delins.offset - delins.size; i++) {
        newbuff[delins.offset + i] = fb.m_buff[delins.offset + delins.size + i];
        newlinenobuff[delins.offset + i] = fb.m_lineno_buff[delins.offset + delins.size + i];
    }

    safe_fkfree(m_fk, fb.m_buff);
    fb.m_buff = newbuff;
    fb.m_size -= delins.size;
    safe_fkfree(m_fk, fb.m_lineno_buff);
    fb.m_lineno_buff = newlinenobuff;
    fb.m_lineno_size -= delins.size;
}

void optimizer::optimize_write_write(func_binary &fb) {
    // eg.a=1 return 1
    // write，并没有read，删除这个write
    for (int i = 0; i < (int) m_inslist.size(); i++) {
        opt_ins &ins = m_inslist[i];
        if (ins.write.size() > 0) {
            int num = 0;
            for (int j = 0; j < (int) ins.write.size(); j++) {
                command addr = ins.write[j];

                if (ADDR_TYPE(COMMAND_CODE(addr)) == ADDR_CONTAINER) {
                    continue;
                }

                if (ins.code == OPCODE_CALL) {
                    continue;
                }

                opt_ins *readins = get_read_ins_from(0, addr);
                if (readins == 0) {
                    num++;
                    continue;
                }
            }

            if (num == (int) ins.write.size()) {
                remove_ins(fb, ins);
                m_isopt = true;
                return;
            }
        }
    }
}

void optimizer::optimize_write_assign(func_binary &fb) {
    // eg.a=b+c
    // write一个后，只assign了一次，再无read，替换掉write的dst，删除assign
    for (int i = 0; i < (int) m_inslist.size(); i++) {
        opt_ins &ins = m_inslist[i];
        for (int j = 0; j < (int) ins.dst.size(); j++) {
            opt_ins_addr &addr = ins.dst[j];
            if (ADDR_TYPE(COMMAND_CODE(addr.addr)) == ADDR_CONTAINER) {
                continue;
            }

            if (get_assign_src_ins_num(addr.addr) == 1) {
                opt_ins *assign_ins = get_assign_src_ins_from(i + 1, addr.addr);
                if (assign_ins != 0 && get_read_ins_from(i + 1, addr.addr) == assign_ins) {
                    if (get_read_ins_from(i + 2, addr.addr) == 0) {
                        opt_ins_addr &assign_ins_addr = assign_ins->dst[0];
                        replace_ins_addr(fb, addr, assign_ins_addr);
                        remove_ins(fb, *assign_ins);
                        m_isopt = true;
                        return;
                    }
                }
            }
        }
    }
}

