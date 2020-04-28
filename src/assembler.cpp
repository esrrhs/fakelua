#include "assembler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fake.h"
#include "binary.h"
#include "asmgen.h"

void assembler::clear() {
    m_pos = 0;
    m_posmap.clear();
    m_caremap.clear();
}

void assembler::open() {
    m_isopen = true;
}

void assembler::close() {
    m_isopen = false;
}

void assembler::compile_seterror(const func_binary &fb, command cmd, efkerror err, const char *fmt, ...) {
    char errorstr[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(errorstr, sizeof(errorstr) - 1, fmt, ap);
    va_end(ap);
    errorstr[sizeof(errorstr) - 1] = 0;
    seterror(m_fk, err, FUNC_BINARY_FILENAME(fb), FUNC_BINARY_LINENO(fb, m_pos), FUNC_BINARY_NAME(fb),
             "assembler %llu, %s", cmd, errorstr);
}

bool assembler::compile(binary *bin) {
    FKLOG("[assembler] compile binary %p", bin);

#ifndef FK64
    // 32位不支持
    return true;
#endif

#ifdef __MINGW64__
    // MINGW不支持
    return true;
#endif

    if (!m_isopen) {
        return true;
    }

    for (const fkhashmap<variant, funcunion>::ele *p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next()) {
        const funcunion &f = *p->t;
        if (f.havefb && FUNC_BINARY_FRESH(f.fb)) {
            const func_binary &fb = f.fb;
            if (FUNC_BINARY_BACKUP(fb)) {
                const func_binary &bkfb = *FUNC_BINARY_BACKUP(fb);
                if (!compile_func(bkfb)) {
                    FKERR("[assembler] compile compile_func %s fail", FUNC_BINARY_NAME(bkfb));
                    return false;
                }
                FUNC_BINARY_FRESH(bkfb)--;
            } else {
                if (!compile_func(fb)) {
                    FKERR("[assembler] compile compile_func %s fail", FUNC_BINARY_NAME(fb));
                    return false;
                }
                FUNC_BINARY_FRESH(fb)--;
            }
        }
    }

    FKLOG("[assembler] compile binary %d ok \n%s", bin, m_native->dump().c_str());

    return true;
}

bool assembler::compile_func(const func_binary &fb) {
    FKLOG("[assembler] compile_func func_binary %p", &fb);

    clear();

    asmgen asg(m_fk);

    asg.start_func();

    int stacksize = (fb.m_maxstack + fb.m_const_list_num + MAX_ASSEMBLER_CONTAINER_NUM + 1) * variant_size;
    FKLOG("[assembler] compile_func stack size %d", stacksize);
    asg.alloc_stack(stacksize);

    asg.copy_param(FUNC_BINARY_PARAMNUM(fb));

    asg.copy_const(fb.m_const_list, fb.m_const_list_num, fb.m_maxstack);

    // loop
    m_pos = 0;
    while (m_pos < (int) fb.m_size) {
        m_posmap[m_pos] = asg.size();
        FKLOG("posmap %d %d", m_pos, asg.size());
        if (!compile_next(asg, fb)) {
            FKERR("[assembler] compile_func compile_next %d fail", m_pos);
            return false;
        }
    }
    // end
    m_posmap[m_pos] = asg.size();

    // link jmp pos
    for (caremap::iterator it = m_caremap.begin(); it != m_caremap.end(); it++) {
        int jumpposoff = it->first;
        int bytecodepos = it->second;
        if (m_posmap.find(bytecodepos) != m_posmap.end()) {
            int pos = m_posmap[bytecodepos];
            asg.set_int(jumpposoff, pos - (jumpposoff + sizeof(int)));
            FKLOG("loop set %d -> %d %d", jumpposoff, pos - (jumpposoff + sizeof(int)), pos);
        } else {
            assert(0);
        }
    }

    asg.stop_func();
    func_native nt;
    FUNC_NATIVE_INI(nt);
    asg.output(FUNC_BINARY_FILENAME(fb), FUNC_BINARY_PACKAGENAME(fb), FUNC_BINARY_NAME(fb), &nt);
    variant fv = m_fk->sh.allocsysstr(FUNC_BINARY_NAME(fb));
    m_native->add_func(fv, nt);

    String str = asg.source();

    FKLOG("[assembler] compile_func binary %p ok\n%s", &fb, str.c_str());

    return true;
}

bool assembler::compile_next(asmgen &asg, const func_binary &fb) {
    command cmd = GET_CMD(fb, m_pos);
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    USE(type);
    FKLOG("[assembler] compile_next cmd %d %d %s", type, code, OpCodeStr(code));

    assert (type == COMMAND_OPCODE);

    m_pos++;

    m_conposnum = 0;
    memset(m_conpos, 0, sizeof(m_conpos));

    bool ret = false;
    USE(ret);
    // 执行对应命令，放一起switch效率更高，cpu有缓存
    switch (code) {
        case OPCODE_ASSIGN: {
            ret = compile_assign(asg, fb, cmd);
        }
            break;
        case OPCODE_RETURN: {
            ret = compile_return(asg, fb, cmd);
        }
            break;
        case OPCODE_PLUS:
        case OPCODE_MINUS:
        case OPCODE_MULTIPLY:
        case OPCODE_DIVIDE:
        case OPCODE_DIVIDE_MOD:
        case OPCODE_STRING_CAT: {
            ret = compile_math(asg, fb, cmd);
        }
            break;
        case OPCODE_PLUS_ASSIGN:
        case OPCODE_MINUS_ASSIGN:
        case OPCODE_MULTIPLY_ASSIGN:
        case OPCODE_DIVIDE_ASSIGN:
        case OPCODE_DIVIDE_MOD_ASSIGN: {
            ret = compile_math_assign(asg, fb, cmd);
        }
            break;
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_LESS:
        case OPCODE_MORE:
        case OPCODE_EQUAL:
        case OPCODE_MOREEQUAL:
        case OPCODE_LESSEQUAL:
        case OPCODE_NOTEQUAL: {
            ret = compile_cmp(asg, fb, cmd);
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
            ret = compile_cmp_jne(asg, fb, cmd);
        }
            break;
        case OPCODE_NOT: {
            ret = compile_single(asg, fb, cmd);
        }
            break;
        case OPCODE_NOT_JNE: {
            ret = compile_single_jne(asg, fb, cmd);
        }
            break;
        case OPCODE_JNE: {
            ret = compile_jne(asg, fb, cmd);
        }
            break;
        case OPCODE_JMP: {
            ret = compile_jmp(asg, fb, cmd);
        }
            break;
        case OPCODE_CALL: {
            ret = compile_call(asg, fb, cmd);
        }
            break;
        case OPCODE_FOR: {
            ret = compile_for(asg, fb, cmd);
        }
            break;
        case OPCODE_SLEEP:
        case OPCODE_YIELD: {
            setwarn(m_fk, "assembler only support SLEEP or YIELD, skip code");
            ret = true;
            m_pos++;
        }
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_next err code %d %s", code, OpCodeStr(code));
            compile_seterror(fb, cmd, efk_jit_error, "compile_next err code %d %s", code, OpCodeStr(code));
            return false;
    }
    return ret;
}

#define GET_VARIANT_POS(fb, v, pos) \
    GET_VARIANT_POS_BY_CMD(fb, v, GET_CMD(fb, pos))

#define GET_VARIANT_POS_BY_CMD(fb, v, cmd) \
    command v##_cmd = cmd;\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST || v##_addrtype == ADDR_CONTAINER);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        v = (v##_addrpos);\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
        v = (v##_addrpos) + FUNC_BINARY_MAX_STACK(fb); \
    }\
    else if (v##_addrtype == ADDR_CONTAINER)\
    {\
        assert(m_conposnum >= 0 && m_conposnum < MAX_ASSEMBLER_CONTAINER_NUM); \
        if (!(m_conposnum >= 0 && m_conposnum < MAX_ASSEMBLER_CONTAINER_NUM)) \
        { \
            v = 0;\
            FKERR("container pos recursion too much %d", m_conposnum);\
            compile_seterror(fb, cmd, efk_jit_error, "container pos recursion too much %d", m_conposnum);\
            return false;\
        } \
        m_conpos[m_conposnum] = v##_cmd; \
        v = FUNC_BINARY_MAX_CONST(fb) + FUNC_BINARY_MAX_STACK(fb) + m_conposnum; \
        m_conposnum++; \
        compile_container_to_pos(asg, fb, v##_addrpos, v); \
    }\
    else\
    {\
        v = 0;\
        assert(0);\
        FKERR("addr type cannot be %d %d", v##_addrtype, v##_addrpos);\
        compile_seterror(fb, cmd, efk_jit_error, "addr type cannot be %d %d", v##_addrtype, v##_addrpos);\
        return false;\
    }

#define CHECK_VARIANT_CON_POS(fb, v) \
    if (v >= FUNC_BINARY_MAX_CONST(fb) + FUNC_BINARY_MAX_STACK(fb)) \
    { \
        int v##_off = v - FUNC_BINARY_MAX_CONST(fb) - FUNC_BINARY_MAX_STACK(fb); \
        assert(v##_off >= 0 && v##_off < MAX_ASSEMBLER_CONTAINER_NUM); \
        command v##_cmd = m_conpos[v##_off]; \
        int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd)); \
        USE(v##_addrtype); \
        int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd)); \
        assert (v##_addrtype == ADDR_CONTAINER);\
        compile_pos_to_container(asg, fb, v, v##_addrpos); \
    }

#define GET_TMP_POS(fb, v) v = FUNC_BINARY_MAX_CONST(fb) + FUNC_BINARY_MAX_STACK(fb) + MAX_ASSEMBLER_CONTAINER_NUM

bool assembler::compile_assign(asmgen &asg, const func_binary &fb, command cmd) {
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
            ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int var = 0;
    GET_VARIANT_POS(fb, var, m_pos);
    m_pos++;

    int value = 0;
    GET_VARIANT_POS(fb, value, m_pos);
    m_pos++;

    asg.variant_assign(var, value);

    CHECK_VARIANT_CON_POS(fb, var);

    FKLOG("assign from %d to pos %d", var, value);

    return true;
}

bool assembler::compile_return(asmgen &asg, const func_binary &fb, command cmd) {
    asg.variant_ps_clear();

    if (GET_CMD(fb, m_pos) == EMPTY_CMD) {
        FKLOG("return empty");
        m_pos++;
        asg.variant_jmp(fb.m_size);
        m_caremap[asg.size() - sizeof(int)] = fb.m_size;
        return true;
    }

    int retnum = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    for (int i = 0; i < retnum; i++) {
        int ret = 0;
        GET_VARIANT_POS(fb, ret, m_pos);
        m_pos++;

        asg.variant_push(ret);
    }

    asg.variant_jmp(fb.m_size);
    m_caremap[asg.size() - sizeof(int)] = fb.m_size;

    return true;
}

void assembler_string_cat(fake *fk, variant *left, variant *right, variant *dest) {
    V_STRING_CAT(dest, left, right);
}

bool assembler::compile_math(asmgen &asg, const func_binary &fb, command cmd) {
    int code = COMMAND_CODE(cmd);

    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;

    int right = 0;
    GET_VARIANT_POS(fb, right, m_pos);
    m_pos++;

    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
            ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    switch (code) {
        case OPCODE_PLUS:
            asg.variant_add(dest, left, right);
            break;
        case OPCODE_MINUS:
            asg.variant_sub(dest, left, right);
            break;
        case OPCODE_MULTIPLY:
            asg.variant_mul(dest, left, right);
            break;
        case OPCODE_DIVIDE:
            asg.variant_div(dest, left, right);
            break;
        case OPCODE_DIVIDE_MOD:
            asg.variant_div_mod(dest, left, right);
            break;
        case OPCODE_STRING_CAT:
            asg.call_func_param4((void *) &assembler_string_cat, m_fk, left, right, dest);
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_math err code %d %s", code, OpCodeStr(code));
            break;
    }

    CHECK_VARIANT_CON_POS(fb, dest);

    return true;
}

bool assembler::compile_math_assign(asmgen &asg, const func_binary &fb, command cmd) {
    int code = COMMAND_CODE(cmd);

    assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
           ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;

    int right = 0;
    GET_VARIANT_POS(fb, right, m_pos);
    m_pos++;

    switch (code) {
        case OPCODE_PLUS_ASSIGN:
            asg.variant_add(left, left, right);
            break;
        case OPCODE_MINUS_ASSIGN:
            asg.variant_sub(left, left, right);
            break;
        case OPCODE_MULTIPLY_ASSIGN:
            asg.variant_mul(left, left, right);
            break;
        case OPCODE_DIVIDE_ASSIGN:
            asg.variant_div(left, left, right);
            break;
        case OPCODE_DIVIDE_MOD_ASSIGN:
            asg.variant_div_mod(left, left, right);
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_math_assign err code %d %s", code, OpCodeStr(code));
            break;
    }

    CHECK_VARIANT_CON_POS(fb, left);

    return true;
}

bool assembler::compile_cmp_jne(asmgen &asg, const func_binary &fb, command cmd) {
    int code = COMMAND_CODE(cmd);

    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;

    int right = 0;
    GET_VARIANT_POS(fb, right, m_pos);
    m_pos++;

    assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
           ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    // 1.先计算结果
    switch (code) {
        case OPCODE_AND_JNE:
            asg.variant_and(dest, left, right);
            break;
        case OPCODE_OR_JNE:
            asg.variant_or(dest, left, right);
            break;
        case OPCODE_LESS_JNE:
            asg.variant_less(dest, left, right);
            break;
        case OPCODE_MORE_JNE:
            asg.variant_more(dest, left, right);
            break;
        case OPCODE_EQUAL_JNE:
            asg.variant_equal(dest, left, right);
            break;
        case OPCODE_MOREEQUAL_JNE:
            asg.variant_moreequal(dest, left, right);
            break;
        case OPCODE_LESSEQUAL_JNE:
            asg.variant_lessequal(dest, left, right);
            break;
        case OPCODE_NOTEQUAL_JNE:
            asg.variant_notequal(dest, left, right);
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_cmp_jne err code %d %s", code, OpCodeStr(code));
            break;
    }

    // 2.再jne
    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end()) {
        jumppos = m_posmap[jump_bytecode_pos];
    }

    asg.variant_jne(dest, jumppos);

    int jmpoffset = asg.size() - sizeof(int);
    if (jumppos == -1) {
        // 记录下来
        m_caremap[jmpoffset] = jump_bytecode_pos;
        FKLOG("compile_cmp_jne caremap add %d %d", jmpoffset, jump_bytecode_pos);
    } else {
        asg.set_int(jmpoffset, jumppos - asg.size());
        FKLOG("compile_cmp_jne set jne add %d -> %d", jmpoffset, jumppos - asg.size());
    }

    return true;
}

bool assembler::compile_cmp(asmgen &asg, const func_binary &fb, command cmd) {
    int code = COMMAND_CODE(cmd);

    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;

    int right = 0;
    GET_VARIANT_POS(fb, right, m_pos);
    m_pos++;

    assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
           ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    switch (code) {
        case OPCODE_AND:
            asg.variant_and(dest, left, right);
            break;
        case OPCODE_OR:
            asg.variant_or(dest, left, right);
            break;
        case OPCODE_LESS:
            asg.variant_less(dest, left, right);
            break;
        case OPCODE_MORE:
            asg.variant_more(dest, left, right);
            break;
        case OPCODE_EQUAL:
            asg.variant_equal(dest, left, right);
            break;
        case OPCODE_MOREEQUAL:
            asg.variant_moreequal(dest, left, right);
            break;
        case OPCODE_LESSEQUAL:
            asg.variant_lessequal(dest, left, right);
            break;
        case OPCODE_NOTEQUAL:
            asg.variant_notequal(dest, left, right);
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_cmp err code %d %s", code, OpCodeStr(code));
            break;
    }

    return true;
}

bool assembler::compile_for(asmgen &asg, const func_binary &fb, command cmd) {
    int iter = 0;
    GET_VARIANT_POS(fb, iter, m_pos);
    m_pos++;

    int end = 0;
    GET_VARIANT_POS(fb, end, m_pos);
    m_pos++;

    int step = 0;
    GET_VARIANT_POS(fb, step, m_pos);
    m_pos++;

    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    // 1.计算
    asg.variant_add(iter, iter, step);

    // 2.再je
    asg.variant_less(dest, iter, end);

    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end()) {
        jumppos = m_posmap[jump_bytecode_pos];
    }

    asg.variant_je(dest, jumppos);

    int jmpoffset = asg.size() - sizeof(int);
    if (jumppos == -1) {
        // 记录下来
        m_caremap[jmpoffset] = jump_bytecode_pos;
        FKLOG("compile_for caremap add %d %d", jmpoffset, jump_bytecode_pos);
    } else {
        asg.set_int(jmpoffset, jumppos - asg.size());
        FKLOG("compile_for set jne add %d -> %d", jmpoffset, jumppos - asg.size());
    }

    CHECK_VARIANT_CON_POS(fb, iter);

    return true;
}

bool assembler::compile_single_jne(asmgen &asg, const func_binary &fb, command cmd) {
    int code = COMMAND_CODE(cmd);

    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;

    assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
           ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    // 1.先计算结果
    switch (code) {
        case OPCODE_NOT_JNE:
            asg.variant_not(dest, left);
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_single_jne err code %d %s", code, OpCodeStr(code));
            break;
    }

    // 2.再jne
    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end()) {
        jumppos = m_posmap[jump_bytecode_pos];
    }

    asg.variant_jne(dest, jumppos);

    int jmpoffset = asg.size() - sizeof(int);
    if (jumppos == -1) {
        // 记录下来
        m_caremap[jmpoffset] = jump_bytecode_pos;
        FKLOG("compile_single_jne caremap add %d %d", jmpoffset, jump_bytecode_pos);
    } else {
        asg.set_int(jmpoffset, jumppos - asg.size());
        FKLOG("compile_single_jne set jne add %d -> %d", jmpoffset, jumppos - asg.size());
    }

    return true;
}

bool assembler::compile_single(asmgen &asg, const func_binary &fb, command cmd) {
    int code = COMMAND_CODE(cmd);

    int left = 0;
    GET_VARIANT_POS(fb, left, m_pos);
    m_pos++;

    assert(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK ||
           ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_CONTAINER);
    int dest = 0;
    GET_VARIANT_POS(fb, dest, m_pos);
    m_pos++;

    switch (code) {
        case OPCODE_NOT:
            asg.variant_not(dest, left);
            break;
        default:
            assert(0);
            FKERR("[assembler] compile_single err code %d %s", code, OpCodeStr(code));
            break;
    }

    return true;
}

bool assembler::compile_jne(asmgen &asg, const func_binary &fb, command cmd) {
    int cmp = 0;
    GET_VARIANT_POS(fb, cmp, m_pos);
    m_pos++;

    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end()) {
        jumppos = m_posmap[jump_bytecode_pos];
    }

    asg.variant_jne(cmp, jumppos);

    int jmpoffset = asg.size() - sizeof(int);
    if (jumppos == -1) {
        // 记录下来
        m_caremap[jmpoffset] = jump_bytecode_pos;
        FKLOG("compile_jne caremap add %d %d", jmpoffset, jump_bytecode_pos);
    } else {
        asg.set_int(jmpoffset, jumppos - asg.size());
        FKLOG("compile_jne set jne add %d -> %d", jmpoffset, jumppos - asg.size());
    }

    return true;
}

bool assembler::compile_jmp(asmgen &asg, const func_binary &fb, command cmd) {
    int jump_bytecode_pos = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    int jumppos = -1;
    if (m_posmap.find(jump_bytecode_pos) != m_posmap.end()) {
        jumppos = m_posmap[jump_bytecode_pos];
    }

    asg.variant_jmp(jumppos);

    int jmpoffset = asg.size() - sizeof(int);
    if (jumppos == -1) {
        // 记录下来
        m_caremap[jmpoffset] = jump_bytecode_pos;
        FKLOG("compile_jmp caremap add %d %d", jmpoffset, jump_bytecode_pos);
    } else {
        asg.set_int(jmpoffset, jumppos - asg.size());
        FKLOG("compile_jmp set jne add %d -> %d", jmpoffset, jumppos - asg.size());
    }

    return true;
}

variant *assembler_get_classmem_call(fake *fk, variant *classvar, variant *callpos) {
    bool err = false;
    variant *n = fk->con.newvariant();

    void *classptr = 0;
    const char *classprefix = 0;

    // prefix
    V_GET_POINTER(classvar, classptr, classprefix);

    if (UNLIKE(err)) {
        return 0;
    }

    const char *funcname = 0;
    V_GET_STRING(callpos, funcname);

    if (UNLIKE(err)) {
        return 0;
    }

    if (UNLIKE(!classptr)) {
        err = true;
        seterror(fk, efk_jit_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "jit class mem call error, the class ptr is null, type %s", classprefix);
        return 0;
    }

    // whole name
    char wholename[MAX_FAKE_REG_FUNC_NAME_LEN];
    if (UNLIKE(classvar->data.ponter->typesz + callpos->data.str->sz >= MAX_FAKE_REG_FUNC_NAME_LEN)) {
        err = true;
        seterror(fk, efk_jit_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "jit class mem call error, the name is too long, func %s %s", classprefix, funcname);
        return 0;
    }
    memcpy(wholename, classprefix, classvar->data.ponter->typesz);
    memcpy(wholename + classvar->data.ponter->typesz, funcname, callpos->data.str->sz);
    wholename[classvar->data.ponter->typesz + callpos->data.str->sz] = 0;

    // call it
    V_SET_STRING(n, wholename);

    return n;
}

bool assembler::compile_call(asmgen &asg, const func_binary &fb, command cmd) {
    int calltype = COMMAND_CODE(GET_CMD(fb, m_pos));
    m_pos++;

    if (calltype == CALL_FAKE) {
        setwarn(m_fk, "assembler not supprt fake call, change to normal call");
        calltype = CALL_NORMAL;
    }

    if (calltype == CALL_NORMAL) {
        int callpos = 0;
        GET_VARIANT_POS(fb, callpos, m_pos);
        m_pos++;

        int retnum = COMMAND_CODE(GET_CMD(fb, m_pos));
        m_pos++;

        std::vector<int> retvec;
        for (int i = 0; i < retnum; i++) {
            int ret = 0;
            GET_VARIANT_POS(fb, ret, m_pos);
            m_pos++;
            retvec.push_back(ret);
        }

        int argnum = COMMAND_CODE(GET_CMD(fb, m_pos));
        m_pos++;

        // 1.塞参数
        asg.variant_ps_clear();
        for (int i = 0; i < argnum; i++) {
            int arg = 0;
            GET_VARIANT_POS(fb, arg, m_pos);
            m_pos++;

            asg.variant_push(arg);
        }

        // 2.准备调用函数
        // 3.调用
        asg.call_func_param2((void *) &machine::static_call, m_fk, callpos);

        // 4.处理返回值
        for (int i = (int) retvec.size() - 1; i >= 0; i--) {
            int ret = retvec[i];
            asg.variant_pop(ret);

            CHECK_VARIANT_CON_POS(fb, ret);
        }

        // 5.清理不需要的
        asg.variant_ps_clear();
    } else if (calltype == CALL_CLASSMEM) {
        int callpos = 0;
        GET_VARIANT_POS(fb, callpos, m_pos);
        m_pos++;

        int retnum = COMMAND_CODE(GET_CMD(fb, m_pos));
        m_pos++;

        std::vector<int> retvec;
        for (int i = 0; i < retnum; i++) {
            int ret = 0;
            GET_VARIANT_POS(fb, ret, m_pos);
            m_pos++;
            retvec.push_back(ret);
        }

        int argnum = COMMAND_CODE(GET_CMD(fb, m_pos));
        m_pos++;

        // 0.取出实际的函数名
        int classpos = 0;
        GET_VARIANT_POS(fb, classpos, m_pos + argnum - 1);
        int tmppos = 0;
        GET_TMP_POS(fb, tmppos);

        asg.call_func_param3((void *) &assembler_get_classmem_call, m_fk, classpos, callpos);
        asg.variant_from_rax(tmppos);

        // 1.塞参数
        asg.variant_ps_clear();
        for (int i = 0; i < argnum; i++) {
            int arg = 0;
            GET_VARIANT_POS(fb, arg, m_pos);
            m_pos++;

            asg.variant_push(arg);
        }

        // 2.准备调用函数
        // 3.调用
        asg.call_func_param2((void *) &machine::static_call, m_fk, tmppos);

        // 4.处理返回值
        for (int i = (int) retvec.size() - 1; i >= 0; i--) {
            int ret = retvec[i];
            asg.variant_pop(ret);

            CHECK_VARIANT_CON_POS(fb, ret);
        }

        // 5.清理不需要的
        asg.variant_ps_clear();
    }

    return true;
}

variant *assembler_get_container(fake *fk, variant *conv, variant *keyv) {
    variant *v = 0;
    if (conv->type == variant::MAP) {
        v = con_map_get(fk, conv->data.vm, keyv);
    } else if (conv->type == variant::ARRAY) {
        v = con_array_get(fk, conv->data.va, keyv);
    }
    return v;
}

bool assembler::compile_container_to_pos(asmgen &asg, const func_binary &fb, int conpos, int despos) {
    assert(conpos >= 0 && conpos < (int) fb.m_container_addr_list_num);
    const container_addr &ca = fb.m_container_addr_list[conpos];

    int conv = 0;
    GET_VARIANT_POS_BY_CMD(fb, conv, ca.con);

    int keyv = 0;
    GET_VARIANT_POS_BY_CMD(fb, keyv, ca.key);

    asg.call_func_param3((void *) &assembler_get_container, m_fk, conv, keyv);

    asg.variant_from_rax(despos);

    return true;
}

bool assembler::compile_pos_to_container(asmgen &asg, const func_binary &fb, int srcpos, int conpos) {
    assert(conpos >= 0 && conpos < (int) fb.m_container_addr_list_num);
    const container_addr &ca = fb.m_container_addr_list[conpos];

    int conv = 0;
    GET_VARIANT_POS_BY_CMD(fb, conv, ca.con);

    int keyv = 0;
    GET_VARIANT_POS_BY_CMD(fb, keyv, ca.key);

    asg.call_func_param3((void *) &assembler_get_container, m_fk, conv, keyv);

    asg.variant_to_rax(srcpos);

    return true;
}
