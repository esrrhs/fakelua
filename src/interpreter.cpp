#include "interpreter.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

//////////////////////////////////////////////////////////////////////////

void interpreter::call(const variant &func, int retnum, int *retpos) {
    call_func(m_fk->fm.get_func(func), retnum, retpos, &func);
}

void interpreter::call_func(const funcunion *f, int retnum, int *retpos, const variant *func,
                            const variant *args, int argn) {
    fake *fk = m_fk;
    paramstack *ps = &fk->ps;
    bool &err = m_isend;
    USE(err);
    if (UNLIKE(!f)) {
        FKERR("fkrun no func %s fail", vartostring(func).c_str());
        m_isend = true;
        seterror(m_fk, efk_run_no_func_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "fkrun no func %s fail", vartostring(func).c_str());
        return;
    }

    if (LIKE(f->havefb)) {
        const func_binary *fb = &f->fb;
        variant *v = 0;

        int needsize = m_sp + BP_SIZE + retnum + FUNC_BINARY_MAX_STACK(*fb);
        if (UNLIKE(needsize > (int) ARRAY_MAX_SIZE(m_stack))) {
            int newsize = needsize + 1;
            int grown = (int) ARRAY_MAX_SIZE(m_stack) * 2;
            if (grown > newsize) {
                newsize = grown;
            }
            if (newsize < 32) {
                newsize = 32;
            }
            ARRAY_GROW(m_stack, newsize, variant);
        }

        int oldbp = m_bp;
        m_bp = m_sp;

        for (int i = 0; i < retnum; i++) {
            v = &ARRAY_GET(m_stack, m_bp);
            v->type = variant::NIL;
            v->data.buf = retpos[i];
            m_bp++;
        }

        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = retnum;
        m_bp++;

        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = m_ip;
        m_bp++;

        v = &ARRAY_GET(m_stack, m_bp);
        if (UNLIKE(m_fk->pf.isopen())) {
            v->data.buf = fkgetmstick();
        } else {
            v->data.buf = 0;
        }
        v->type = variant::NIL;
        m_bp++;

        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = (uint64_t) m_fb;
        m_bp++;

        v = &ARRAY_GET(m_stack, m_bp);
        v->type = variant::NIL;
        v->data.buf = oldbp;
        m_bp++;

        m_sp = m_bp + FUNC_BINARY_MAX_STACK(*fb);

        int nparam = args ? argn : (int) ps->m_variant_list_num;
        if (UNLIKE(nparam != FUNC_BINARY_PARAMNUM(*fb))) {
            FKERR("call func %s param not match", vartostring(func).c_str());
            m_isend = true;
            seterror(m_fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "call func %s param not match", vartostring(func).c_str());
            return;
        }

        assert(FUNC_BINARY_PARAMNUM(*fb) <= REAL_MAX_FAKE_PARAM_NUM);
        assert(m_bp + FUNC_BINARY_PARAMNUM(*fb) <= (int) ARRAY_MAX_SIZE(m_stack));

        if (nparam > 0) {
            memcpy(&ARRAY_GET(m_stack, m_bp), args ? args : ps->m_variant_list,
                   nparam * sizeof(variant));
        }
        if (!args) {
            PS_CLEAR(*ps);
        }

        // ??????
        int locals = FUNC_BINARY_MAX_STACK(*fb) - FUNC_BINARY_PARAMNUM(*fb);
        if (locals > 0) {
            memset(&ARRAY_GET(m_stack, m_bp + FUNC_BINARY_PARAMNUM(*fb)), 0,
                   locals * sizeof(variant));
        }

        // ????ret
        V_SET_NIL(&m_ret[0]);

        m_fb = fb;
        m_ip = 0;

        return;
    }

    // ???profile
    uint32_t s = 0;
    if (UNLIKE(m_fk->pf.isopen())) {
        s = fkgetmstick();
    }

    // ?????
    if (f->haveff) {
        // ???????????????
        if (UNLIKE((int) ps->m_variant_list_num != f->ff.argnum)) {
            FKERR("bind func %s param not match, give %d need %d", vartostring(func).c_str(),
                  (int) ps->m_variant_list_num, f->ff.argnum);
            m_isend = true;
            seterror(m_fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "bind func %s param not match, give %d need %d", vartostring(func).c_str(),
                     (int) ps->m_variant_list_num, f->ff.argnum);
            return;
        }

        BIND_FUNC_CALL(f, this);
        FKLOG("call C func %s", vartostring(func).c_str());
    }
        // ????????
    else if (f->havebif) {
        BUILDIN_FUNC_CALL(f, this);
        FKLOG("call buildin func %s", vartostring(func).c_str());
    } else {
        assert(0);
        FKERR("fkrun no inter func %s fail", vartostring(func).c_str());
        m_isend = true;
        seterror(m_fk, efk_run_no_func_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "fkrun no inter func %s fail", vartostring(func).c_str());
        return;
    }

    // ?????
    // ?????????????????????????C????
    if (UNLIKE(BP_END(m_bp))) {
        variant *cret;
        PS_POP_AND_GET(*ps, cret);
        m_isend = true;
        // ??????????
        m_ret[0] = *cret;
    }
        // ????????????????
    else {
        // ???????????????
        if (UNLIKE((int) ps->m_variant_list_num != retnum)) {
            FKERR("native func %s param not match, give %d need %d", vartostring(func).c_str(),
                  (int) ps->m_variant_list_num, retnum);
            m_isend = true;
            seterror(m_fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "native func %s param not match, give %d need %d", vartostring(func).c_str(),
                     (int) ps->m_variant_list_num, retnum);
            return;
        }

        // ???????
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
        V_GET_STRING(func, name);
        m_fk->pf.add_func_sample(name, fkgetmstick() - s);
    }

    return;
}

void interpreter::run() {
    interpret(false);
}

void interpreter::step() {
    interpret(true);
}

static force_inline variant *vm_addr(uint32_t w, variant *stack, int bp, variant *consts) {
    int t = (int) (w >> 16);
    int p = (int) (int16_t) w;
    if (t == ADDR_STACK) {
        return stack + bp + p;
    }
    if (t == ADDR_CONST) {
        return consts + p;
    }
    return 0;
}

static force_inline bool vm_for_tight(bool step, const command *code, int destip, int for_ip,
                                      variant *iter, const variant *endv, const variant *stepv,
                                      variant *stack, int bp, variant *consts, const func_binary *fb) {
    if (step || destip < 0) {
        return false;
    }
    if (UNLIKE(iter->type != variant::REAL || endv->type != variant::REAL || stepv->type != variant::REAL)) {
        return false;
    }
    double *iv = &iter->data.real;
    double lim = endv->data.real;
    double st = stepv->data.real;
    int op = (int) COMMAND_CODE(code[destip]);

    if (op == OPCODE_PLUS_ASSIGN && destip + 3 == for_ip) {
        variant *var = vm_addr((uint32_t) code[destip + 1], stack, bp, consts);
        variant *addv = vm_addr((uint32_t) code[destip + 2], stack, bp, consts);
        if (UNLIKE(!var || !addv || var->type != variant::REAL || addv->type != variant::REAL)) {
            return false;
        }
        uint32_t aw = (uint32_t) code[destip + 2];
        if ((aw >> 16) == ADDR_CONST || addv != iter) {
            double add = addv->data.real;
            for (;;) {
                *iv += st;
                if (!(*iv < lim)) {
                    break;
                }
                var->data.real += add;
            }
        } else {
            for (;;) {
                *iv += st;
                if (!(*iv < lim)) {
                    break;
                }
                var->data.real += addv->data.real;
            }
        }
        return true;
    }

    if (op == OPCODE_PLUS && destip + 4 == for_ip) {
        variant *left = vm_addr((uint32_t) code[destip + 1], stack, bp, consts);
        variant *right = vm_addr((uint32_t) code[destip + 2], stack, bp, consts);
        variant *dest = vm_addr((uint32_t) code[destip + 3], stack, bp, consts);
        if (UNLIKE(!left || !right || !dest || left->type != variant::REAL || right->type != variant::REAL)) {
            return false;
        }
        dest->type = variant::REAL;
        for (;;) {
            *iv += st;
            if (!(*iv < lim)) {
                break;
            }
            dest->data.real = left->data.real + right->data.real;
        }
        return true;
    }

    if (op == OPCODE_PLUS && destip + 7 == for_ip &&
        (int) COMMAND_CODE(code[destip + 4]) == OPCODE_ASSIGN) {
        variant *left = vm_addr((uint32_t) code[destip + 1], stack, bp, consts);
        variant *right = vm_addr((uint32_t) code[destip + 2], stack, bp, consts);
        variant *adest = vm_addr((uint32_t) code[destip + 5], stack, bp, consts);
        if (UNLIKE(!left || !right || !adest || left->type != variant::REAL || right->type != variant::REAL)) {
            return false;
        }
        adest->type = variant::REAL;
        for (;;) {
            *iv += st;
            if (!(*iv < lim)) {
                break;
            }
            adest->data.real = left->data.real + right->data.real;
        }
        return true;
    }

    if (op == OPCODE_ASSIGN && destip + 3 == for_ip) {
        uint32_t dw = (uint32_t) code[destip + 1];
        uint32_t sw = (uint32_t) code[destip + 2];
        if ((dw >> 16) != ADDR_CONTAINER) {
            return false;
        }
        variant *src = vm_addr(sw, stack, bp, consts);
        if (UNLIKE(!src)) {
            return false;
        }
        int conpos = (int) (int16_t) dw;
        if (UNLIKE(conpos < 0 || conpos >= fb->m_container_addr_list_num)) {
            return false;
        }
        const container_addr &ca = fb->m_container_addr_list[conpos];
        variant *conv = vm_addr((uint32_t) ca.con, stack, bp, consts);
        variant *keyv = vm_addr((uint32_t) ca.key, stack, bp, consts);
        if (UNLIKE(!conv || !keyv || conv->type != variant::ARRAY)) {
            return false;
        }
        variant_array *va = conv->data.va;
        if (st > 0 && lim > *iv) {
            int need = (int) lim;
            if (need > (int) ARRAY_MAX_SIZE(va->va)) {
                size_t newsize = ARRAY_MAX_SIZE(va->va) ? (size_t) ARRAY_MAX_SIZE(va->va) * 2 : 16;
                while ((int) newsize < need) {
                    newsize *= 2;
                }
                ARRAY_GROW(va->va, newsize, variant);
            }
        }
        variant *data = va->va.m_data;
        uint32_t *psz = &va->va.m_size;
        uint32_t cap = va->va.m_max_size;
        if (keyv == iter && src == iter) {
            int last = -1;
            for (;;) {
                *iv += st;
                if (!(*iv < lim)) {
                    break;
                }
                int index = (int) *iv;
                if (UNLIKE((uint32_t) index >= cap)) {
                    break;
                }
                data[index].type = variant::REAL;
                data[index].data.real = *iv;
                last = index;
            }
            if (last >= 0 && *psz < (uint32_t) (last + 1)) {
                *psz = (uint32_t) (last + 1);
            }
        } else {
            for (;;) {
                *iv += st;
                if (!(*iv < lim)) {
                    break;
                }
                int index = (int) keyv->data.real;
                if (UNLIKE(index < 0 || (uint32_t) index >= cap)) {
                    break;
                }
                data[index] = *src;
                if (*psz < (uint32_t) (index + 1)) {
                    *psz = (uint32_t) (index + 1);
                }
            }
        }
        return true;
    }

    return false;
}

#define IGET(v, idx) \
    do { \
        uint32_t _w = (uint32_t) code[(idx)]; \
        int _t = (int) (_w >> 16); \
        int _p = (int) (int16_t) _w; \
        if (LIKE(_t == ADDR_STACK)) { \
            (v) = stack + bp + _p; \
        } else if (_t == ADDR_CONST) { \
            (v) = consts + _p; \
        } else { \
            m_ip = ip; \
            m_bp = bp; \
            m_fb = fb; \
            (v) = get_container_variant(*fb, _p); \
            stack = m_stack.m_data; \
            if (UNLIKE(!(v))) { \
                err = true; \
                m_isend = true; \
                return; \
            } \
        } \
    } while (0)

template<bool STEP>
void interpreter::interpret_t() {
    fake *fk = m_fk;
    bool &err = m_isend;

    if (UNLIKE(m_isend)) {
        return;
    }

    int ip = m_ip;
    int bp = m_bp;
    const func_binary *fb = m_fb;
    variant *stack = m_stack.m_data;
    const command *code = fb->m_buff;
    int codesize = fb->m_size;
    variant *consts = fb->m_const_list;

    static const void *const dt[OPCODE_MAX] = {
            &&L_ASSIGN,
            &&L_PLUS, &&L_MINUS, &&L_MULTIPLY, &&L_DIVIDE, &&L_DIVIDE_MOD, &&L_STRING_CAT,
            &&L_PLUS_ASSIGN, &&L_MINUS_ASSIGN, &&L_MULTIPLY_ASSIGN, &&L_DIVIDE_ASSIGN, &&L_DIVIDE_MOD_ASSIGN,
            &&L_RETURN,
            &&L_JNE, &&L_JMP,
            &&L_AND, &&L_OR, &&L_LESS, &&L_MORE, &&L_EQUAL, &&L_MOREEQUAL, &&L_LESSEQUAL, &&L_NOTEQUAL, &&L_NOT,
            &&L_AND_JNE, &&L_OR_JNE, &&L_LESS_JNE, &&L_MORE_JNE, &&L_EQUAL_JNE, &&L_MOREEQUAL_JNE, &&L_LESSEQUAL_JNE, &&L_NOTEQUAL_JNE, &&L_NOT_JNE,
            &&L_CALL,
            &&L_FOR,
    };

#define VM_SAVE() do { m_ip = ip; m_bp = bp; m_fb = fb; } while (0)
#define VM_RELOAD() do { \
        ip = m_ip; \
        bp = m_bp; \
        fb = m_fb; \
        stack = m_stack.m_data; \
        code = fb->m_buff; \
        codesize = fb->m_size; \
        consts = fb->m_const_list; \
    } while (0)
#define VM_DISPATCH() do { \
        if (STEP) { VM_SAVE(); return; } \
        goto vm_dispatch; \
    } while (0)

    goto vm_dispatch;

    vm_return: {
        if (UNLIKE(m_fk->pf.isopen())) {
            uint32_t calltime = 0;
            BP_GET_CALLTIME(bp, calltime);
            m_fk->pf.add_func_sample(FUNC_BINARY_NAME(*fb), fkgetmstick() - calltime);
        }
        int oldretnum = 0;
        BP_GET_RETNUM(bp, oldretnum);
        int callbp = 0;
        BP_GET_BP(bp, callbp);
        BP_GET_FB(bp, fb);
        BP_GET_IP(bp, ip);
        int oldbp = bp;
        m_sp = bp - BP_SIZE - oldretnum;
        bp = callbp;
        if (UNLIKE(BP_END(bp))) {
            VM_SAVE();
            m_isend = true;
            return;
        }
        stack = m_stack.m_data;
        code = fb->m_buff;
        codesize = fb->m_size;
        consts = fb->m_const_list;
        for (int i = 0; i < oldretnum; i++) {
            int oldretpos = 0;
            BP_GET_RETPOS(oldbp, oldretnum, oldretpos, i);
            variant *ret = 0;
            IGET(ret, oldretpos);
            *ret = m_ret[i];
        }
        goto vm_dispatch;
    }

    vm_dispatch: {
        if (UNLIKE(ip >= codesize)) {
            goto vm_return;
        }
        int opcode = (int) (uint32_t) code[ip];
        ip++;
        if (UNLIKE(m_fk->pf.isopen())) {
            m_fk->pf.add_code_sample(opcode);
        }
        goto *dt[opcode];
    }

    L_ASSIGN: {
        variant *varv = 0;
        IGET(varv, ip);
        if (UNLIKE(CHECK_CONST_MAP_POS(varv) || CHECK_CONST_ARRAY_POS(varv))) {
            err = true;
            seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "interpreter assign error, dest is const container");
            VM_SAVE();
            m_isend = true;
            return;
        }
        ip++;
        const variant *valuev = 0;
        IGET(valuev, ip);
        ip++;
        *varv = *valuev;
        VM_DISPATCH();
    }
    L_PLUS: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        if (LIKE(left->type == variant::REAL && right->type == variant::REAL)) {
            dest->type = variant::REAL;
            dest->data.real = left->data.real + right->data.real;
        } else {
            V_PLUS(dest, left, right);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        }
        VM_DISPATCH();
    }
    L_MINUS: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        if (LIKE(left->type == variant::REAL && right->type == variant::REAL)) {
            dest->type = variant::REAL;
            dest->data.real = left->data.real - right->data.real;
        } else {
            V_MINUS(dest, left, right);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        }
        VM_DISPATCH();
    }
    L_MULTIPLY: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        if (LIKE(left->type == variant::REAL && right->type == variant::REAL)) {
            dest->type = variant::REAL;
            dest->data.real = left->data.real * right->data.real;
        } else {
            V_MULTIPLY(dest, left, right);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        }
        VM_DISPATCH();
    }
    L_DIVIDE: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_DIVIDE(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_DIVIDE_MOD: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_DIVIDE_MOD(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_STRING_CAT: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_STRING_CAT(dest, left, right);
        VM_DISPATCH();
    }
    L_AND: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_AND(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_OR: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_OR(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_LESS: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        if (LIKE(left->type == variant::REAL && right->type == variant::REAL)) {
            dest->type = variant::REAL;
            dest->data.real = left->data.real < right->data.real;
        } else {
            V_LESS(dest, left, right);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        }
        VM_DISPATCH();
    }
    L_MORE: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_MORE(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_EQUAL: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_EQUAL(dest, left, right);
        VM_DISPATCH();
    }
    L_MOREEQUAL: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_MOREEQUAL(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_LESSEQUAL: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_LESSEQUAL(dest, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_NOTEQUAL: {
        const variant *left = 0;
        const variant *right = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        IGET(dest, ip); ip++;
        V_NOTEQUAL(dest, left, right);
        VM_DISPATCH();
    }
    L_NOT: {
        const variant *left = 0;
        variant *dest = 0;
        IGET(left, ip); ip++;
        IGET(dest, ip); ip++;
        V_NOT(dest, left);
        VM_DISPATCH();
    }
    L_AND_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_AND_JNE(b, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_OR_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_OR_JNE(b, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_LESS_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        if (LIKE(left->type == variant::REAL && right->type == variant::REAL)) {
            if (!(left->data.real < right->data.real)) {
                ip = destip;
            }
        } else {
            bool b = false;
            V_LESS_JNE(b, left, right);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
            if (!b) {
                ip = destip;
            }
        }
        VM_DISPATCH();
    }
    L_MORE_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_MORE_JNE(b, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_EQUAL_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_EQUAL_JNE(b, left, right);
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_MOREEQUAL_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_MOREEQUAL_JNE(b, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_LESSEQUAL_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_LESSEQUAL_JNE(b, left, right);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_NOTEQUAL_JNE: {
        const variant *left = 0;
        const variant *right = 0;
        IGET(left, ip); ip++;
        IGET(right, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_NOTEQUAL_JNE(b, left, right);
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_NOT_JNE: {
        const variant *left = 0;
        IGET(left, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        bool b = false;
        V_NOT_JNE(b, left);
        if (!b) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_JNE: {
        const variant *cmp = 0;
        IGET(cmp, ip); ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        if (!(V_ISBOOL(cmp))) {
            ip = destip;
        }
        VM_DISPATCH();
    }
    L_JMP: {
        ip = (int) COMMAND_CODE(code[ip]);
        VM_DISPATCH();
    }
    L_PLUS_ASSIGN: {
        variant *var = 0;
        const variant *value = 0;
        IGET(var, ip); ip++;
        IGET(value, ip); ip++;
        if (LIKE(var->type == variant::REAL && value->type == variant::REAL)) {
            var->data.real += value->data.real;
        } else {
            V_PLUS(var, var, value);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        }
        VM_DISPATCH();
    }
    L_MINUS_ASSIGN: {
        variant *var = 0;
        const variant *value = 0;
        IGET(var, ip); ip++;
        IGET(value, ip); ip++;
        V_MINUS(var, var, value);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_MULTIPLY_ASSIGN: {
        variant *var = 0;
        const variant *value = 0;
        IGET(var, ip); ip++;
        IGET(value, ip); ip++;
        V_MULTIPLY(var, var, value);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_DIVIDE_ASSIGN: {
        variant *var = 0;
        const variant *value = 0;
        IGET(var, ip); ip++;
        IGET(value, ip); ip++;
        V_DIVIDE(var, var, value);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_DIVIDE_MOD_ASSIGN: {
        variant *var = 0;
        const variant *value = 0;
        IGET(var, ip); ip++;
        IGET(value, ip); ip++;
        V_DIVIDE_MOD(var, var, value);
        if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
        VM_DISPATCH();
    }
    L_CALL: {
        int call_ip = ip - 1;
        int calltype = (int) COMMAND_CODE(code[ip]);
        ip++;
        const variant *callpos = 0;
        IGET(callpos, ip); ip++;
        int retnum = (int) COMMAND_CODE(code[ip]);
        ip++;
        int retpos[MAX_FAKE_RETURN_NUM];
        for (int i = 0; i < retnum; i++) {
            retpos[i] = ip;
            ip++;
        }
        int argnum = (int) COMMAND_CODE(code[ip]);
        ip++;
        variant callargs[REAL_MAX_FAKE_PARAM_NUM];
        for (int i = 0; i < argnum; i++) {
            variant *arg = 0;
            IGET(arg, ip); ip++;
            callargs[i] = *arg;
        }
        VM_SAVE();
        if (LIKE(calltype == CALL_NORMAL)) {
            const funcunion *f = 0;
            if (LIKE(fb->m_call_cache != 0)) {
                f = (const funcunion *) fb->m_call_cache[call_ip];
            }
            if (UNLIKE(!f)) {
                f = m_fk->fm.get_func(*callpos);
                if (fb->m_call_cache) {
                    fb->m_call_cache[call_ip] = f;
                }
            }
            if (LIKE(f && f->havefb)) {
                call_func(f, retnum, retpos, callpos, callargs, argnum);
            } else {
                paramstack &ps = fk->ps;
                PS_CLEAR(ps);
                for (int i = 0; i < argnum; i++) {
                    variant *argdest = 0;
                    PS_PUSH_AND_GET(ps, argdest);
                    *argdest = callargs[i];
                }
                call_func(f, retnum, retpos, callpos);
            }
        } else if (LIKE(calltype == CALL_CLASSMEM)) {
            paramstack &ps = fk->ps;
            PS_CLEAR(ps);
            for (int i = 0; i < argnum; i++) {
                variant *argdest = 0;
                PS_PUSH_AND_GET(ps, argdest);
                *argdest = callargs[i];
            }
            void *classptr = 0;
            const char *classprefix = 0;
            variant *classvar;
            PS_GET(ps, classvar, PS_SIZE(ps) - 1);
            V_GET_POINTER(classvar, classptr, classprefix);
            if (UNLIKE(err)) {
                m_isend = true;
                return;
            }
            const char *funcname = 0;
            V_GET_STRING(callpos, funcname);
            if (UNLIKE(err)) {
                m_isend = true;
                return;
            }
            if (UNLIKE(!classptr)) {
                err = true;
                seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                         "interpreter class mem call error, the class ptr is null, type %s", classprefix);
                m_isend = true;
                return;
            }
            char wholename[MAX_FAKE_REG_FUNC_NAME_LEN];
            if (UNLIKE(classvar->data.ponter->typesz + callpos->data.str->sz >= MAX_FAKE_REG_FUNC_NAME_LEN)) {
                err = true;
                seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                         "interpreter class mem call error, the name is too long, func %s %s", classprefix, funcname);
                m_isend = true;
                return;
            }
            memcpy(wholename, classprefix, classvar->data.ponter->typesz);
            memcpy(wholename + classvar->data.ponter->typesz, funcname, callpos->data.str->sz);
            wholename[classvar->data.ponter->typesz + callpos->data.str->sz] = 0;
            variant tmp;
            V_SET_STRING(&tmp, wholename);
            call(tmp, retnum, retpos);
        } else {
            err = true;
            seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                     "interpreter call error, unknown call type %d", calltype);
            m_isend = true;
            return;
        }
        if (UNLIKE(m_isend)) {
            return;
        }
        VM_RELOAD();
        VM_DISPATCH();
    }
    L_FOR: {
        int for_ip = ip - 1;
        variant *iter = 0;
        const variant *endv = 0;
        const variant *step = 0;
        IGET(iter, ip); ip++;
        IGET(endv, ip); ip++;
        IGET(step, ip); ip++;
        ip++;
        int destip = (int) COMMAND_CODE(code[ip]);
        ip++;
        if (LIKE(vm_for_tight(STEP, code, destip, for_ip, iter, endv, step, stack, bp, consts, fb))) {
            VM_DISPATCH();
        }
        if (LIKE(iter->type == variant::REAL && endv->type == variant::REAL && step->type == variant::REAL)) {
            iter->data.real += step->data.real;
            if (iter->data.real < endv->data.real) {
                ip = destip;
            }
        } else {
            V_PLUS(iter, iter, step);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
            bool b = false;
            V_FOR_LESS(b, iter, endv);
            if (UNLIKE(err)) { VM_SAVE(); m_isend = true; return; }
            if (LIKE(b)) {
                ip = destip;
            }
        }
        VM_DISPATCH();
    }
    L_RETURN: {
        int returnnum = (int) COMMAND_CODE(code[ip]);
        if (UNLIKE(!returnnum)) {
            ip = codesize;
            goto vm_dispatch;
        }
        ip++;
        for (int i = 0; i < returnnum; i++) {
            const variant *ret = 0;
            IGET(ret, ip); ip++;
            m_ret[i] = *ret;
        }
        ip = codesize;
        goto vm_dispatch;
    }

#undef VM_SAVE
#undef VM_RELOAD
#undef VM_DISPATCH
}

#undef IGET

void interpreter::interpret(bool onestep) {
    if (onestep) {
        interpret_t<true>();
    } else {
        interpret_t<false>();
    }
}

template void interpreter::interpret_t<true>();
template void interpreter::interpret_t<false>();

variant *interpreter::get_container_variant(const func_binary &fb, int conpos) {
    assert(conpos >= 0 && conpos < (int) fb.m_container_addr_list_num);
    const container_addr &ca = fb.m_container_addr_list[conpos];
    uint32_t cw = (uint32_t) ca.con;
    uint32_t kw = (uint32_t) ca.key;
    int ct = (int) (cw >> 16);
    int cp = (int) (int16_t) cw;
    int kt = (int) (kw >> 16);
    int kp = (int) (int16_t) kw;
    variant *conv = (ct == ADDR_STACK) ? &ARRAY_GET(m_stack, m_bp + cp) :
                    (ct == ADDR_CONST) ? &fb.m_const_list[cp] : 0;
    const variant *keyv = (kt == ADDR_STACK) ? &ARRAY_GET(m_stack, m_bp + kp) :
                          (kt == ADDR_CONST) ? &fb.m_const_list[kp] : 0;
    if (UNLIKE(!conv || !keyv)) {
        bool &err = m_isend;
        USE(err);
        variant *tmp = 0;
        do { GET_VARIANT_BY_CMD(fb, m_bp, tmp, ca.con); } while (0);
        conv = tmp;
        tmp = 0;
        do { GET_VARIANT_BY_CMD(fb, m_bp, tmp, ca.key); } while (0);
        keyv = tmp;
        if (UNLIKE(m_isend || !conv || !keyv)) {
            return 0;
        }
    }

    if (LIKE(conv->type == variant::ARRAY && keyv->type == variant::REAL)) {
        int index = (int) keyv->data.real;
        variant_array *va = conv->data.va;
        if (UNLIKE(index < 0)) {
            m_isend = true;
            seterror(m_fk, efk_run_inter_error, fkgetcurfile(m_fk), fkgetcurline(m_fk), fkgetcurfunc(m_fk),
                     "interpreter get array fail, index %d", index);
            return 0;
        }
        if (UNLIKE(index >= (int) ARRAY_MAX_SIZE(va->va))) {
            size_t need = (size_t) index + 1;
            size_t newsize = ARRAY_MAX_SIZE(va->va) ? (size_t) ARRAY_MAX_SIZE(va->va) * 2 : 16;
            while (newsize < need) {
                newsize *= 2;
            }
            ARRAY_GROW(va->va, newsize, variant);
        }
        ARRAY_SIZE(va->va) = FKMAX((int) ARRAY_SIZE(va->va), index + 1);
        return &ARRAY_GET(va->va, index);
    }

    if (UNLIKE(!(conv->type == variant::ARRAY || conv->type == variant::MAP))) {
        m_isend = true;
        seterror(m_fk, efk_run_inter_error, fkgetcurfile(m_fk), fkgetcurline(m_fk), fkgetcurfunc(m_fk),
                 "interpreter get container variant fail, container type error, type %s", vartypetostring(conv->type));
        return 0;
    }

    if (conv->type == variant::MAP) {
        return con_map_get(m_fk, conv->data.vm, keyv);
    }
    return con_array_get(m_fk, conv->data.va, keyv);
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



