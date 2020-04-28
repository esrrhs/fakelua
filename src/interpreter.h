#pragma once

#include "types.h"
#include "variant.h"
#include "binary.h"
#include "paramstack.h"
#include "array.h"
#include "container.h"

/**************
[ret pos] .. [ret pos] [ret num] [old ip] [call time] [old fb] [old bp] 
**************/
#define BP_SIZE (5)

#define BP_END(bp) (!bp)

#define BP_GET_RETPOS(bp, retnum, retpos, i) \
    assert(bp - 5 - retnum + i >= 0);\
    assert(ARRAY_GET(m_stack, bp - 5 - retnum + i).type == variant::NIL);\
    retpos = ((int)(ARRAY_GET(m_stack, bp - 5 - retnum + i).data.buf))

#define BP_GET_RETNUM(bp, retnum) \
    assert(bp - 5 >= 0);\
    assert(ARRAY_GET(m_stack, bp - 5).type == variant::NIL);\
    retnum = ((int)(ARRAY_GET(m_stack, bp - 5).data.buf))

#define BP_GET_IP(bp, oldip) \
    assert(bp - 4 >= 0);\
    assert(ARRAY_GET(m_stack, bp - 4).type == variant::NIL);\
    oldip = ((int)(ARRAY_GET(m_stack, bp - 4).data.buf));\
    USE(oldip);

#define BP_GET_CALLTIME(bp, calltime) \
    assert(bp - 3 >= 0);\
    assert(ARRAY_GET(m_stack, bp - 3).type == variant::NIL);\
    calltime = ((uint32_t)(ARRAY_GET(m_stack, bp - 3).data.buf))

#define BP_GET_FB(bp, fb) \
    BP_GET_FB_BY_INTER(bp, fb, *this)

#define BP_GET_FB_BY_INTER(bp, fb, inter) \
    assert(bp - 2 >= 0);\
    assert(ARRAY_GET((inter).m_stack, bp - 2).type == variant::NIL);\
    fb = ((const func_binary *)(ARRAY_GET((inter).m_stack, bp - 2).data.buf))

#define BP_GET_BP(bp, fb) \
    BP_GET_BP_BY_INTER(bp, fb, *this)

#define BP_GET_BP_BY_INTER(bp, callbp, inter) \
    assert(bp - 1 >= 0);\
    assert(ARRAY_GET((inter).m_stack, bp - 1).type == variant::NIL);\
    callbp = ((int)(ARRAY_GET((inter).m_stack, bp - 1).data.buf))

#define GET_CMD(fb, ip) (fb).m_buff[ip]

#define GET_CMD_LINENO(fb, pos) FUNC_BINARY_LINENO(fb, pos)

#define GET_CONST(v, fb, pos) \
    assert(pos >= 0 && pos < (int)(fb).m_const_list_num);\
    v = &((fb).m_const_list[pos]);

#define GET_CONTAINER(v, fb, pos) v = get_container_variant(fb, pos)

#define GET_STACK(bp, v, pos) \
    GET_STACK_BY_INTER(bp, v, pos, *this);

#define GET_STACK_BY_INTER(bp, v, pos, inter) \
    assert(pos >= 0 && pos < (int)ARRAY_MAX_SIZE((inter).m_stack));\
    v = &ARRAY_GET((inter).m_stack, bp + pos);

#define CHECK_STACK_POS(fb, ip) (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, ip))) == ADDR_STACK)
#define CHECK_CONTAINER_POS(fb, ip) (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, ip))) == ADDR_CONTAINER)
#define CHECK_CONST_MAP_POS(v) ((v)->type == variant::MAP && (v)->data.vm->isconst)
#define CHECK_CONST_ARRAY_POS(v) ((v)->type == variant::ARRAY && (v)->data.va->isconst)
#define POS_TYPE_NAME(fb, ip) vartypetostring((int)ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, ip))))

#define GET_VARIANT(fb, bp, v, pos) \
    GET_VARIANT_BY_CMD(fb, bp, v, GET_CMD(fb, pos))

#define GET_VARIANT_BY_CMD(fb, bp, v, cmd) \
    command v##_cmd = cmd;\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST || v##_addrtype == ADDR_CONTAINER);\
    if (LIKE(v##_addrtype == ADDR_STACK))\
    {\
        GET_STACK(bp, v, (v##_addrpos));\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
        GET_CONST(v, fb, (v##_addrpos)); \
    }\
    else if (v##_addrtype == ADDR_CONTAINER)\
    {\
        GET_CONTAINER(v, fb, (v##_addrpos)); \
        if (!v) \
        { \
            err = true; \
            break;\
        } \
    }\
    else\
    {\
        v = 0;\
        assert(0);\
        FKERR("addrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        err = true;\
        break;\
    }

#define LOG_VARIANT(fb, ip, prefix) \
    FKLOG(prefix " variant %d %d", \
        ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, ip))),\
        ADDR_POS(COMMAND_CODE(GET_CMD(fb, ip))));

#define MATH_OPER(fb, bp, ip, oper) \
    const variant * left = 0;\
    LOG_VARIANT(fb, ip, "left");\
    GET_VARIANT(fb, bp, left, ip);\
    ip++;\
    \
    const variant * right = 0;\
    LOG_VARIANT(fb, ip, "right");\
    GET_VARIANT(fb, bp, right, ip);\
    ip++;\
    \
    variant * dest;\
    if (UNLIKE(!(CHECK_STACK_POS(fb, ip) || CHECK_CONTAINER_POS(fb, ip)))) \
    { \
        err = true; \
        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "interpreter math oper error, dest is not stack, type %s", POS_TYPE_NAME(fb, ip)); \
        break; \
    } \
    LOG_VARIANT(fb, ip, "dest");\
    GET_VARIANT(fb, bp, dest, ip);\
    ip++;\
    \
    FKLOG("math left %s right %s", (vartostring(left)).c_str(), (vartostring(right)).c_str());\
    \
    V_##oper(dest, left, right);\
    \
    FKLOG("math %s %s", OpCodeStr(code), (vartostring(dest)).c_str());

#define MATH_ASSIGN_OPER(fb, bp, ip, oper) \
    variant * var = 0;\
    if (UNLIKE(!(CHECK_STACK_POS(fb, ip) || CHECK_CONTAINER_POS(fb, ip))))\
    { \
        err = true; \
        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "interpreter math assign oper error, dest is not stack or container, type %s", POS_TYPE_NAME(fb, ip)); \
        break; \
    } \
    LOG_VARIANT(fb, ip, "var");\
    GET_VARIANT(fb, bp, var, ip);\
    if (UNLIKE(CHECK_CONST_MAP_POS(var) || CHECK_CONST_ARRAY_POS(var)))\
    {\
        err = true;\
        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "interpreter assign error, dest is const container"); \
        break;\
    }\
    ip++;\
    \
    const variant * value = 0;\
    LOG_VARIANT(fb, ip, "value");\
    GET_VARIANT(fb, bp, value, ip);\
    ip++;\
    \
    FKLOG("math var %s value %s", (vartostring(var)).c_str(), (vartostring(value)).c_str());\
    \
    V_##oper(var, var, value);\
    \
    FKLOG("math %s %s", OpCodeStr(code), (vartostring(var)).c_str());

#define MATH_SINGLE_OPER(fb, bp, ip, oper) \
    const variant * left = 0;\
    LOG_VARIANT(fb, ip, "left");\
    GET_VARIANT(fb, bp, left, ip);\
    ip++;\
    \
    variant * dest;\
    if (UNLIKE(!(CHECK_STACK_POS(fb, ip) || CHECK_CONTAINER_POS(fb, ip)))) \
    { \
        err = true; \
        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "interpreter math oper error, dest is not stack, type %s", POS_TYPE_NAME(fb, ip)); \
        break; \
    } \
    LOG_VARIANT(fb, ip, "dest");\
    GET_VARIANT(fb, bp, dest, ip);\
    ip++;\
    \
    FKLOG("single math left %s", (vartostring(left)).c_str());\
    \
    V_##oper(dest, left);\
    \
    FKLOG("single math %s %s", OpCodeStr(code), (vartostring(dest)).c_str());

#define MATH_OPER_JNE(fb, bp, ip, oper) \
    const variant * left = 0;\
    LOG_VARIANT(fb, ip, "left");\
    GET_VARIANT(fb, bp, left, ip);\
    ip++;\
    \
    const variant * right = 0;\
    LOG_VARIANT(fb, ip, "right");\
    GET_VARIANT(fb, bp, right, ip);\
    ip++;\
    \
    /*dest*/\
    ip++;\
    int destip = COMMAND_CODE(GET_CMD(fb, ip));\
    ip++;\
    \
    FKLOG("math left %s right %s", (vartostring(left)).c_str(), (vartostring(right)).c_str());\
    \
    bool b = false;\
    V_##oper(b, left, right);\
    \
    if (!b)\
    {\
        FKLOG("jne %d", destip);\
        ip = destip;\
    }\
    FKLOG("math %s %d", OpCodeStr(code), ip);

#define MATH_SINGLE_OPER_JNE(fb, bp, ip, oper) \
    const variant * left = 0;\
    LOG_VARIANT(fb, ip, "left");\
    GET_VARIANT(fb, bp, left, ip);\
    ip++;\
    \
    /*dest*/\
    ip++;\
    int destip = COMMAND_CODE(GET_CMD(fb, ip));\
    ip++;\
    \
    FKLOG("single math left %s", (vartostring(left)).c_str());\
    \
    bool b = false;\
    V_##oper(b, left);\
    \
    if (!b)\
    {\
        FKLOG("jne %d", destip);\
        ip = destip;\
    }\
    FKLOG("single math %s %d", OpCodeStr(code), ip);

#define CHECK_ERR(err) \
    if (UNLIKE(err)) \
    { \
        processor * p = 0; \
        GET_CUR_PROCESSOR(p, fk->rn); \
        if (p && p->m_curroutine) \
        { \
            p->m_curroutine->m_interpreter.end(); \
        } \
    }

struct fake;
struct processor;

struct interpreter {
public:
    force_inline bool isend() const {
        return m_isend;
    }

    force_inline void end() {
        m_isend = true;
    }

    void call(const variant &func, int retnum, int *retpos);

    variant *get_container_variant(const func_binary &fb, int conpos);

    force_inline const variant &getret() const {
        return m_ret[0];
    }

    force_inline const char *get_running_func_name() const {
        return m_fb ? FUNC_BINARY_NAME(*m_fb) : "";
    }

    force_inline const char *get_running_file_name() const {
        return m_fb ? FUNC_BINARY_FILENAME(*m_fb) : "";
    }

    force_inline int get_running_bytecode_pos() const {
        return m_fb ? m_ip : -1;
    }

    force_inline int get_running_file_line() const {
        return m_fb ? GET_CMD_LINENO(*m_fb, m_ip) : 0;
    }

    const char *get_running_call_stack() const;

    int get_running_call_stack_length() const;

    void get_running_call_stack_frame_info(int frame,
                                           const char *&stackinfo,
                                           const char *&func,
                                           const char *&file,
                                           int &line) const;

    void get_running_vaiant(int frame, const char *name, int line, const char *&value, int &outline);

    void set_running_vaiant(int frame, const char *name, int line, const char *value);

    int run(int cmdnum);

public:
    fake *m_fk;
    bool m_isend;
    variant m_ret[MAX_FAKE_RETURN_NUM];
    array<variant> m_stack;
    const func_binary *m_fb;
    int m_ip;
    int m_bp;
    int m_sp;
    uint32_t m_wakeuptime;
    uint32_t m_yieldtime;
    bool m_sleeping;
    processor *m_processor;
};

#define INTER_DELETE(inter) ARRAY_DELETE((inter).m_stack)

#define INTER_INI(inter, fk) (inter).m_fk = fk;\
    (inter).m_isend = false;\
    ARRAY_INI((inter).m_stack, fk);\
    (inter).m_fb = 0;\
    (inter).m_ip = 0;\
    (inter).m_bp = 0;\
    (inter).m_sp = 0;\
    (inter).m_wakeuptime = 0;\
    (inter).m_yieldtime = 0;\
    (inter).m_sleeping = false


#define INTER_CLEAR(inter) (inter).m_isend = false;\
    ARRAY_CLEAR((inter).m_stack);\
    (inter).m_fb = 0;\
    (inter).m_ip = 0;\
    (inter).m_bp = 0;\
    (inter).m_sp = 0;\
    (inter).m_wakeuptime = 0;\
    (inter).m_yieldtime = 0;\
    (inter).m_sleeping = false;

#define INTER_SET_PRO(inter, pro) (inter).m_processor = pro

