#include "interpreter.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

//////////////////////////////////////////////////////////////////////////

void interpreter::call(const variant &func, int retnum, int *retpos) {
    fake *fk = m_fk;
    paramstack *ps = getps(m_fk);
    const funcunion *f = m_fk->fm.get_func(func);
    bool &err = m_isend;
    USE(err);
    if (UNLIKE(!f)) {
        FKERR("fkrun no func %s fail", vartostring(&func).c_str());
        m_isend = true;
        seterror(m_fk, efk_run_no_func_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "fkrun no func %s fail", vartostring(&func).c_str());
        return;
    }

    // 常规函数
    if (LIKE(f->havefb)) {
        const func_binary *fb = &f->fb;
        variant *v = 0;

        // 准备栈大小
        int needsize = m_sp + BP_SIZE + retnum + FUNC_BINARY_MAX_STACK(*fb);
        if (UNLIKE(needsize > (int) ARRAY_MAX_SIZE(m_stack))) {
            int newsize = needsize + 1 + ARRAY_MAX_SIZE(m_stack) * m_fk->cfg.array_grow_speed / 100;
            ARRAY_GROW(m_stack, newsize, variant);
        }

        // 老的bp
        int oldbp = m_bp;
        m_bp = m_sp;

        // 记录返回位置
        for (int i = 0; i < retnum; i++) {
            v = &ARRAY_GET(m_stack, m_bp);
            v->type = variant::NIL;
            v->data.buf = retpos[i];
            m_bp++;
        }

        // 记录返回值数目
        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = retnum;
        m_bp++;

        // 记录老的ip
        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = m_ip;
        m_bp++;

        // 记录profile
        if (UNLIKE(m_fk->pf.isopen())) {
            v = &ARRAY_GET(m_stack, m_bp);
            v->data.buf = fkgetmstick();
        }
        v->type = variant::NIL;
        m_bp++;

        // 记录老的fb
        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = (uint64_t) m_fb;
        m_bp++;

        // 记录老的bp
        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = oldbp;
        m_bp++;

        // 设置sp
        m_sp = m_bp + FUNC_BINARY_MAX_STACK(*fb);

        if (UNLIKE((int) ps->m_variant_list_num != FUNC_BINARY_PARAMNUM(*fb))) {
            FKERR("call func %s param not match", vartostring(&func).c_str());
            m_isend = true;
            seterror(m_fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "call func %s param not match", vartostring(&func).c_str());
            return;
        }

        assert(FUNC_BINARY_PARAMNUM(*fb) <= REAL_MAX_FAKE_PARAM_NUM);
        assert(m_bp + FUNC_BINARY_PARAMNUM(*fb) <= (int) ARRAY_MAX_SIZE(m_stack));

        // 分配入参
        memcpy(&ARRAY_GET(m_stack, m_bp), ps->m_variant_list, FUNC_BINARY_PARAMNUM(*fb) * sizeof(variant));
        PS_CLEAR(*ps);

        // 清空栈区
        memset(&ARRAY_GET(m_stack, m_bp + FUNC_BINARY_PARAMNUM(*fb)), 0,
               (FUNC_BINARY_MAX_STACK(*fb) - FUNC_BINARY_PARAMNUM(*fb)) * sizeof(variant));

        // 重置ret
        V_SET_NIL(&m_ret[0]);

        // 标记
        FUNC_BINARY_USE(*fb)++;

        // 新函数
        m_fb = fb;
        m_ip = 0;

        return;
    }

    // 记录profile
    uint32_t s = 0;
    if (UNLIKE(m_fk->pf.isopen())) {
        s = fkgetmstick();
    }

    // 绑定函数
    if (f->haveff) {
        // 检查返回值数目对不对
        if (UNLIKE((int) ps->m_variant_list_num != f->ff.argnum)) {
            FKERR("bind func %s param not match, give %d need %d", vartostring(&func).c_str(),
                  (int) ps->m_variant_list_num, f->ff.argnum);
            m_isend = true;
            seterror(m_fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "bind func %s param not match, give %d need %d", vartostring(&func).c_str(),
                     (int) ps->m_variant_list_num, f->ff.argnum);
            return;
        }

        BIND_FUNC_CALL(f, this);
        FKLOG("call C func %s", vartostring(&func).c_str());
    }
        // 内置函数
    else if (f->havebif) {
        BUILDIN_FUNC_CALL(f, this);
        FKLOG("call buildin func %s", vartostring(&func).c_str());
    } else {
        assert(0);
        FKERR("fkrun no inter func %s fail", vartostring(&func).c_str());
        m_isend = true;
        seterror(m_fk, efk_run_no_func_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "fkrun no inter func %s fail", vartostring(&func).c_str());
        return;
    }

    // 返回值
    // 这种情况是直接跳过脚本调用了C函数
    if (UNLIKE(BP_END(m_bp))) {
        variant *cret;
        PS_POP_AND_GET(*ps, cret);
        m_isend = true;
        // 直接塞返回值
        m_ret[0] = *cret;
    }
        // 否则塞到当前堆栈上
    else {
        // 检查返回值数目对不对
        if (UNLIKE((int) ps->m_variant_list_num != retnum)) {
            FKERR("native func %s param not match, give %d need %d", vartostring(&func).c_str(),
                  (int) ps->m_variant_list_num, retnum);
            m_isend = true;
            seterror(m_fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "native func %s param not match, give %d need %d", vartostring(&func).c_str(),
                     (int) ps->m_variant_list_num, retnum);
            return;
        }

        // 塞返回值
        for (int i = 0; i < retnum; i++) {
            variant *ret;
            GET_VARIANT(*m_fb, m_bp, ret, retpos[i]);

            variant *cret;
            PS_GET(*ps, cret, i);

            *ret = *cret;
        }
    }

    if (UNLIKE(m_fk->pf.isopen())) {
        const char *name = 0;
        V_GET_STRING(&func, name);
        m_fk->pf.add_func_sample(name, fkgetmstick() - s);
    }

    return;
}

int interpreter::run(int cmdnum) {
    fake *fk = m_fk;
    bool &err = m_isend;
    int i = 0;

    // 栈溢出检查
    if (UNLIKE((int) ARRAY_MAX_SIZE(m_stack) > m_fk->cfg.stack_max)) {
        m_isend = true;
        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "stack too big %d",
                 ARRAY_MAX_SIZE(m_stack));
        return 0;
    }

    // 切换检查
    if (UNLIKE(m_sleeping)) {
        if (LIKE(m_yieldtime)) {
            m_yieldtime--;
            return 0;
        } else if (LIKE(fkgetmstick() < m_wakeuptime)) {
            return 0;
        } else {
            m_wakeuptime = 0;
        }
    }

    // gc
    fkcheckgc(fk, false);

    if (UNLIKE(m_isend)) {
        return 0;
    }

    while (1) {
        // 当前函数走完
        if (UNLIKE(m_ip >= (int) FUNC_BINARY_CMDSIZE(*m_fb))) {
            FKLOG("pop stack %s", FUNC_BINARY_NAME(*m_fb));

            // 记录profile
            if (UNLIKE(m_fk->pf.isopen())) {
                uint32_t calltime = 0;
                BP_GET_CALLTIME(m_bp, calltime);
                m_fk->pf.add_func_sample(FUNC_BINARY_NAME(*m_fb), fkgetmstick() - calltime);
            }

            // 标记
            FUNC_BINARY_USE(*m_fb)--;

            // 更新
            if (UNLIKE(!FUNC_BINARY_USE(*m_fb) && FUNC_BINARY_BACKUP(*m_fb))) {
                FUNC_BINARY_BACKUP_MOVE(*m_fb);
            }

            // 出栈
            int oldretnum = 0;
            BP_GET_RETNUM(m_bp, oldretnum);
            int callbp = 0;
            BP_GET_BP(m_bp, callbp);
            BP_GET_FB(m_bp, m_fb);
            BP_GET_IP(m_bp, m_ip);
            int oldbp = m_bp;
            m_sp = m_bp - BP_SIZE - oldretnum;
            m_bp = callbp;

            // 所有都完
            if (UNLIKE(BP_END(m_bp))) {
                FKLOG("stack empty end");
                m_isend = true;
                break;
            }
                // 塞返回值
            else {
                for (int i = 0; i < oldretnum; i++) {
                    int oldretpos = 0;
                    BP_GET_RETPOS(oldbp, oldretnum, oldretpos, i);

                    variant *ret;
                    GET_VARIANT(*m_fb, m_bp, ret, oldretpos);
                    *ret = m_ret[i];
                }
            }
            continue;
        }

        int code = COMMAND_CODE(GET_CMD(*m_fb, m_ip));

        FKLOG("next %d %d %s", COMMAND_TYPE(GET_CMD(*m_fb, m_ip)), code, OpCodeStr(code));

        assert (COMMAND_TYPE(GET_CMD(*m_fb, m_ip)) == COMMAND_OPCODE);

        m_ip++;

        if (UNLIKE(m_fk->pf.isopen())) {
            m_fk->pf.add_code_sample(code);
        }

        // 执行对应命令，放一起switch效率更高，cpu有缓存
        switch (code) {
            case OPCODE_ASSIGN: {
                // 赋值dest，必须为栈上或容器内
                if (UNLIKE(!(CHECK_STACK_POS(*m_fb, m_ip) || CHECK_CONTAINER_POS(*m_fb, m_ip)))) {
                    err = true;
                    seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                             "interpreter assign error, dest is not stack or container, type %s",
                             POS_TYPE_NAME(*m_fb, m_ip));
                    break;
                }

                variant *varv = 0;
                LOG_VARIANT(*m_fb, m_ip, "var");
                GET_VARIANT(*m_fb, m_bp, varv, m_ip);
                if (UNLIKE(CHECK_CONST_MAP_POS(varv) || CHECK_CONST_ARRAY_POS(varv))) {
                    err = true;
                    seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                             "interpreter assign error, dest is const container");
                    break;
                }
                m_ip++;

                // 赋值来源
                const variant *valuev = 0;
                LOG_VARIANT(*m_fb, m_ip, "value");
                GET_VARIANT(*m_fb, m_bp, valuev, m_ip);
                m_ip++;

                // 赋值
                *varv = *valuev;

                FKLOG("assign %s to %s", (vartostring(valuev)).c_str(), (vartostring(varv)).c_str());
            }
                break;
            case OPCODE_PLUS: {
                MATH_OPER(*m_fb, m_bp, m_ip, PLUS);
            }
                break;
            case OPCODE_MINUS: {
                MATH_OPER(*m_fb, m_bp, m_ip, MINUS);
            }
                break;
            case OPCODE_MULTIPLY: {
                MATH_OPER(*m_fb, m_bp, m_ip, MULTIPLY);
            }
                break;
            case OPCODE_DIVIDE: {
                MATH_OPER(*m_fb, m_bp, m_ip, DIVIDE);
            }
                break;
            case OPCODE_DIVIDE_MOD: {
                MATH_OPER(*m_fb, m_bp, m_ip, DIVIDE_MOD);
            }
                break;
            case OPCODE_STRING_CAT: {
                MATH_OPER(*m_fb, m_bp, m_ip, STRING_CAT);
            }
                break;
            case OPCODE_AND: {
                MATH_OPER(*m_fb, m_bp, m_ip, AND);
            }
                break;
            case OPCODE_OR: {
                MATH_OPER(*m_fb, m_bp, m_ip, OR);
            }
                break;
            case OPCODE_LESS: {
                MATH_OPER(*m_fb, m_bp, m_ip, LESS);
            }
                break;
            case OPCODE_MORE: {
                MATH_OPER(*m_fb, m_bp, m_ip, MORE);
            }
                break;
            case OPCODE_EQUAL: {
                MATH_OPER(*m_fb, m_bp, m_ip, EQUAL);
            }
                break;
            case OPCODE_MOREEQUAL: {
                MATH_OPER(*m_fb, m_bp, m_ip, MOREEQUAL);
            }
                break;
            case OPCODE_LESSEQUAL: {
                MATH_OPER(*m_fb, m_bp, m_ip, LESSEQUAL);
            }
                break;
            case OPCODE_NOTEQUAL: {
                MATH_OPER(*m_fb, m_bp, m_ip, NOTEQUAL);
            }
                break;
            case OPCODE_NOT: {
                MATH_SINGLE_OPER(*m_fb, m_bp, m_ip, NOT);
            }
                break;
            case OPCODE_AND_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, AND_JNE);
            }
                break;
            case OPCODE_OR_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, OR_JNE);
            }
                break;
            case OPCODE_LESS_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, LESS_JNE);
            }
                break;
            case OPCODE_MORE_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, MORE_JNE);
            }
                break;
            case OPCODE_EQUAL_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, EQUAL_JNE);
            }
                break;
            case OPCODE_MOREEQUAL_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, MOREEQUAL_JNE);
            }
                break;
            case OPCODE_LESSEQUAL_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, LESSEQUAL_JNE);
            }
                break;
            case OPCODE_NOTEQUAL_JNE: {
                MATH_OPER_JNE(*m_fb, m_bp, m_ip, NOTEQUAL_JNE);
            }
                break;
            case OPCODE_NOT_JNE: {
                MATH_SINGLE_OPER_JNE(*m_fb, m_bp, m_ip, NOT_JNE);
            }
                break;
            case OPCODE_JNE: {
                const variant *cmp = 0;
                LOG_VARIANT(*m_fb, m_ip, "cmp");
                GET_VARIANT(*m_fb, m_bp, cmp, m_ip);
                m_ip++;

                int ip = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                m_ip++;

                if (!(V_ISBOOL(cmp))) {
                    FKLOG("jne %d", ip);
                    m_ip = ip;
                } else {
                    FKLOG("not jne %d", ip);
                }
            }
                break;
            case OPCODE_JMP: {
                int ip = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                m_ip++;

                FKLOG("jmp %d", ip);

                m_ip = ip;
            }
                break;
            case OPCODE_PLUS_ASSIGN: {
                MATH_ASSIGN_OPER(*m_fb, m_bp, m_ip, PLUS);
            }
                break;
            case OPCODE_MINUS_ASSIGN: {
                MATH_ASSIGN_OPER(*m_fb, m_bp, m_ip, MINUS);
            }
                break;
            case OPCODE_MULTIPLY_ASSIGN: {
                MATH_ASSIGN_OPER(*m_fb, m_bp, m_ip, MULTIPLY);
            }
                break;
            case OPCODE_DIVIDE_ASSIGN: {
                MATH_ASSIGN_OPER(*m_fb, m_bp, m_ip, DIVIDE);
            }
                break;
            case OPCODE_DIVIDE_MOD_ASSIGN: {
                MATH_ASSIGN_OPER(*m_fb, m_bp, m_ip, DIVIDE_MOD);
            }
                break;
            case OPCODE_CALL: {
                int calltype = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                m_ip++;

                const variant *callpos = 0;
                LOG_VARIANT(*m_fb, m_ip, "callpos");
                GET_VARIANT(*m_fb, m_bp, callpos, m_ip);
                m_ip++;

                int retnum = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                m_ip++;

                int retpos[MAX_FAKE_RETURN_NUM];

                for (int i = 0; i < retnum; i++) {
                    assert(CHECK_STACK_POS(*m_fb, m_ip) || CHECK_CONTAINER_POS(*m_fb, m_ip));
                    retpos[i] = m_ip;
                    m_ip++;
                }

                int argnum = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                m_ip++;

                paramstack &ps = *getps(m_fk);
                PS_CLEAR(ps);
                for (int i = 0; i < argnum; i++) {
                    variant *arg = 0;
                    LOG_VARIANT(*m_fb, m_ip, "arg");
                    GET_VARIANT(*m_fb, m_bp, arg, m_ip);
                    m_ip++;

                    variant *argdest = 0;
                    PS_PUSH_AND_GET(ps, argdest);
                    *argdest = *arg;
                }

                if (LIKE(calltype == CALL_NORMAL)) {
                    call(*callpos, retnum, retpos);
                } else if (LIKE(calltype == CALL_CLASSMEM)) {
                    void *classptr = 0;
                    const char *classprefix = 0;

                    // prefix
                    variant *classvar;
                    PS_GET(ps, classvar, PS_SIZE(ps) - 1);
                    V_GET_POINTER(classvar, classptr, classprefix);

                    if (UNLIKE(err)) {
                        break;
                    }

                    // mem func name
                    const char *funcname = 0;
                    V_GET_STRING(callpos, funcname);

                    if (UNLIKE(err)) {
                        break;
                    }

                    if (UNLIKE(!classptr)) {
                        err = true;
                        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                                 "interpreter class mem call error, the class ptr is null, type %s", classprefix);
                        break;
                    }

                    // whole name
                    char wholename[MAX_FAKE_REG_FUNC_NAME_LEN];
                    if (UNLIKE(classvar->data.ponter->typesz + callpos->data.str->sz >= MAX_FAKE_REG_FUNC_NAME_LEN)) {
                        err = true;
                        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                                 "interpreter class mem call error, the name is too long, func %s %s", classprefix,
                                 funcname);
                        break;
                    }
                    memcpy(wholename, classprefix, classvar->data.ponter->typesz);
                    memcpy(wholename + classvar->data.ponter->typesz, funcname, callpos->data.str->sz);
                    wholename[classvar->data.ponter->typesz + callpos->data.str->sz] = 0;

                    // call it
                    variant tmp;
                    V_SET_STRING(&tmp, wholename);

                    call(tmp, retnum, retpos);
                } else {
                    m_processor->start_routine(*callpos, retnum, retpos);
                }
            }
                break;
            case OPCODE_FOR: {
                variant *iter = 0;
                LOG_VARIANT(*m_fb, m_ip, "iter");
                GET_VARIANT(*m_fb, m_bp, iter, m_ip);
                m_ip++;

                const variant *end = 0;
                LOG_VARIANT(*m_fb, m_ip, "end");
                GET_VARIANT(*m_fb, m_bp, end, m_ip);
                m_ip++;

                const variant *step = 0;
                LOG_VARIANT(*m_fb, m_ip, "step");
                GET_VARIANT(*m_fb, m_bp, step, m_ip);
                m_ip++;

                // tmp dest
                m_ip++;

                int ip = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                m_ip++;

                V_PLUS(iter, iter, step);

                bool b = false;
                V_FOR_LESS(b, iter, end);

                if (LIKE(b)) {
                    FKLOG("jne %d", ip);
                    m_ip = ip;
                }
            }
                break;
            case OPCODE_RETURN: {
                int returnnum = COMMAND_CODE(GET_CMD(*m_fb, m_ip));
                if (UNLIKE(!returnnum)) {
                    FKLOG("return empty");
                    m_ip = (*m_fb).m_size;
                    break;
                }
                m_ip++;

                // 塞给ret
                for (int i = 0; i < returnnum; i++) {
                    const variant *ret = 0;
                    LOG_VARIANT(*m_fb, m_ip, "ret");
                    GET_VARIANT(*m_fb, m_bp, ret, m_ip);
                    m_ip++;

                    m_ret[i] = *ret;

                    FKLOG("return %s", (vartostring(&m_ret[i])).c_str());
                }

                m_ip = (*m_fb).m_size;
            }
                break;
            case OPCODE_SLEEP: {
                const variant *time = 0;
                LOG_VARIANT(*m_fb, m_ip, "time");
                GET_VARIANT(*m_fb, m_bp, time, m_ip);
                m_ip++;

                uint32_t sleeptime = 0;
                V_GET_REAL(time, sleeptime);

                m_wakeuptime = fkgetmstick() + sleeptime;
                m_sleeping = true;
                return i + 1;
            }
                break;
            case OPCODE_YIELD: {
                const variant *time = 0;
                LOG_VARIANT(*m_fb, m_ip, "time");
                GET_VARIANT(*m_fb, m_bp, time, m_ip);
                m_ip++;

                V_GET_REAL(time, m_yieldtime);
                m_sleeping = true;
                return i + 1;
            }
                break;
            default:
                assert(0);
                FKERR("next err code %d %s", code, OpCodeStr(code));
                break;
        }

        if (UNLIKE(err)) {
            // 发生错误
            m_isend = true;

            // 清除当前栈上函数的使用标记
            {
                int ip = m_ip;
                int bp = m_bp;
                const func_binary *fb = m_fb;

                while (!BP_END(bp)) {
                    // 标记
                    FUNC_BINARY_USE(*fb)--;

                    // 更新
                    if (UNLIKE(!FUNC_BINARY_USE(*fb) && FUNC_BINARY_BACKUP(*fb))) {
                        FUNC_BINARY_BACKUP_MOVE(*fb);
                    }

                    BP_GET_FB(bp, fb);
                    BP_GET_IP(bp, ip);
                    int callbp = 0;
                    BP_GET_BP(bp, callbp);
                    bp = callbp;
                    if (BP_END(bp)) {
                        break;
                    }
                }
            }
        }

        if (UNLIKE(m_isend)) {
            break;
        }

        i++;

        if (UNLIKE(i >= cmdnum)) {
            break;
        }
    }

    return i;
}

variant *interpreter::get_container_variant(const func_binary &fb, int conpos) {
    variant *v = 0;
    assert(conpos >= 0 && conpos < (int) fb.m_container_addr_list_num);
    const container_addr &ca = fb.m_container_addr_list[conpos];
    bool &err = m_isend;
    USE(err);
    variant *conv = 0;
    do { GET_VARIANT_BY_CMD(fb, m_bp, conv, ca.con); } while (0);
    const variant *keyv = 0;
    do { GET_VARIANT_BY_CMD(fb, m_bp, keyv, ca.key); } while (0);

    if (UNLIKE(m_isend)) {
        return 0;
    }

    if (UNLIKE(!(conv->type == variant::ARRAY || conv->type == variant::MAP))) {
        m_isend = true;
        seterror(m_fk, efk_run_inter_error, fkgetcurfile(m_fk), fkgetcurline(m_fk), fkgetcurfunc(m_fk),
                 "interpreter get container variant fail, container type error, type %s", vartypetostring(conv->type));
        return 0;
    }

    if (conv->type == variant::MAP) {
        v = con_map_get(m_fk, conv->data.vm, keyv);
    } else if (conv->type == variant::ARRAY) {
        v = con_array_get(m_fk, conv->data.va, keyv);
    }

    return v;
}

const char *interpreter::get_running_call_stack() const {
    if (!m_fb) {
        return "";
    }

    m_fk->rn.cur_runinginfo.clear();
    int deps = 0;

    int ip = m_ip;
    int bp = m_bp;
    const func_binary *fb = m_fb;

    while (!BP_END(bp)) {
        m_fk->rn.cur_runinginfo += "#";
        m_fk->rn.cur_runinginfo += fkitoa(deps);
        m_fk->rn.cur_runinginfo += "	";
        m_fk->rn.cur_runinginfo += fb ? FUNC_BINARY_NAME(*fb) : "";
        m_fk->rn.cur_runinginfo += " at ";
        m_fk->rn.cur_runinginfo += fb ? FUNC_BINARY_FILENAME(*fb) : "";
        m_fk->rn.cur_runinginfo += ":";
        m_fk->rn.cur_runinginfo += fb ? fkitoa(GET_CMD_LINENO(*fb, ip)) : 0;
        m_fk->rn.cur_runinginfo += "\n";
        for (int j = 0; fb && j < FUNC_BINARY_MAX_STACK(*fb); j++) {
            m_fk->rn.cur_runinginfo += "		";

            String variant_name;
            for (int i = 0; i < fb->m_debug_stack_variant_info_num; i++) {
                const stack_variant_info &info = fb->m_debug_stack_variant_info[i];
                if (info.pos == j) {
                    variant_name += info.name;
                    variant_name += "(line:";
                    variant_name += fkitoa(info.line);
                    variant_name += ") ";
                }
            }
            if (variant_name.empty()) {
                variant_name = "(anonymous)";
            }

            m_fk->rn.cur_runinginfo += variant_name;
            m_fk->rn.cur_runinginfo += "\t[";
            m_fk->rn.cur_runinginfo += fkitoa(j);
            m_fk->rn.cur_runinginfo += "]\t";
            variant *v = 0;
            GET_STACK(bp, v, j);
            m_fk->rn.cur_runinginfo += vartostring(v);
            m_fk->rn.cur_runinginfo += "\n";
        }

        BP_GET_FB(bp, fb);
        BP_GET_IP(bp, ip);
        int callbp = 0;
        BP_GET_BP(bp, callbp);
        bp = callbp;
        if (BP_END(bp)) {
            break;
        }

        deps++;
    }

    return m_fk->rn.cur_runinginfo.c_str();
}

int interpreter::get_running_call_stack_length() const {
    if (!m_fb) {
        return 0;
    }

    int deps = 0;

    int bp = m_bp;

    while (!BP_END(bp)) {
        deps++;
        int callbp = 0;
        BP_GET_BP(bp, callbp);
        bp = callbp;
        if (BP_END(bp)) {
            break;
        }
    }

    return deps;
}

void interpreter::get_running_call_stack_frame_info(int frame,
                                                    const char *&stackinfo,
                                                    const char *&func,
                                                    const char *&file,
                                                    int &line) const {
    stackinfo = "";
    func = "";
    file = "";
    line = 0;

    if (!m_fb) {
        return;
    }

    m_fk->rn.cur_runinginfo.clear();
    int deps = 0;

    int ip = m_ip;
    int bp = m_bp;
    const func_binary *fb = m_fb;

    while (!BP_END(bp)) {
        if (deps >= frame) {
            func = fb ? FUNC_BINARY_NAME(*fb) : "";
            file = fb ? FUNC_BINARY_FILENAME(*fb) : "";
            line = fb ? GET_CMD_LINENO(*fb, ip) : 0;

            m_fk->rn.cur_runinginfo += "#";
            m_fk->rn.cur_runinginfo += fkitoa(deps);
            m_fk->rn.cur_runinginfo += "	";
            m_fk->rn.cur_runinginfo += func;
            m_fk->rn.cur_runinginfo += " at ";
            m_fk->rn.cur_runinginfo += file;
            m_fk->rn.cur_runinginfo += ":";
            m_fk->rn.cur_runinginfo += fkitoa(line);
            m_fk->rn.cur_runinginfo += "\n";

            stackinfo = m_fk->rn.cur_runinginfo.c_str();

            return;
        }

        BP_GET_FB(bp, fb);
        BP_GET_IP(bp, ip);
        int callbp = 0;
        BP_GET_BP(bp, callbp);
        bp = callbp;
        if (BP_END(bp)) {
            break;
        }

        deps++;
    }

}

void interpreter::get_running_vaiant(int frame, const char *name, int line, const char *&value, int &outline) {
    value = "";
    outline = 0;

    if (!m_fb) {
        return;
    }

    // const define
    variant *gcv = m_fk->pa.get_const_define(name);
    if (gcv) {
        if (gcv->type == variant::STRING) {
            m_fk->rn.cur_runinginfo += "\"";
            m_fk->rn.cur_runinginfo += vartostring(gcv);
            m_fk->rn.cur_runinginfo += "\"";
        } else {
            m_fk->rn.cur_runinginfo += vartostring(gcv);
        }
        value = m_fk->rn.cur_runinginfo.c_str();
        outline = m_fk->pa.get_const_define_lineno(name);
        return;
    }

    m_fk->rn.cur_runinginfo.clear();
    int deps = 0;

    int bp = m_bp;
    const func_binary *fb = m_fb;

    while (!BP_END(bp)) {
        if (deps >= frame) {
            for (int i = 0; i < fb->m_debug_stack_variant_info_num; i++) {
                const stack_variant_info &info = fb->m_debug_stack_variant_info[i];
                if ((line != -1 && !strcmp(info.name, name) && info.line == line) ||
                    (line == -1 && !strcmp(info.name, name))) {
                    variant *v = 0;
                    GET_STACK(bp, v, info.pos);
                    if (v->type == variant::STRING) {
                        m_fk->rn.cur_runinginfo += "\"";
                        m_fk->rn.cur_runinginfo += vartostring(v);
                        m_fk->rn.cur_runinginfo += "\"";
                    } else {
                        m_fk->rn.cur_runinginfo += vartostring(v);
                    }

                    value = m_fk->rn.cur_runinginfo.c_str();
                    outline = info.line;
                    return;
                }
            }
            break;
        }

        BP_GET_FB(bp, fb);
        int callbp = 0;
        BP_GET_BP(bp, callbp);
        bp = callbp;
        if (BP_END(bp)) {
            break;
        }

        deps++;
    }

    return;
}

void interpreter::set_running_vaiant(int frame, const char *name, int line, const char *value) {
    fake *fk = m_fk;

    if (!m_fb) {
        return;
    }

    // const define
    variant *gcv = m_fk->pa.get_const_define(name);
    if (gcv) {
        // can not change
        return;
    }

    int deps = 0;

    int bp = m_bp;
    const func_binary *fb = m_fb;

    while (!BP_END(bp)) {
        if (deps >= frame) {
            for (int i = 0; i < fb->m_debug_stack_variant_info_num; i++) {
                const stack_variant_info &info = fb->m_debug_stack_variant_info[i];
                if ((line != -1 && !strcmp(info.name, name) && info.line == line) ||
                    (line == -1 && !strcmp(info.name, name))) {
                    variant *v = 0;
                    GET_STACK(bp, v, info.pos);

                    std::string valuestr = value;
                    if (valuestr.empty()) {
                        return;
                    }

                    if (valuestr[0] == '\"') {
                        valuestr[valuestr.size() - 1] = 0;
                        V_SET_STRING(v, &valuestr[1]);
                    } else {
                        V_SET_REAL(v, atoi(value));
                    }

                    return;
                }
            }
            break;
        }

        BP_GET_FB(bp, fb);
        int callbp = 0;
        BP_GET_BP(bp, callbp);
        bp = callbp;
        if (BP_END(bp)) {
            break;
        }

        deps++;
    }

}



