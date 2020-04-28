#include "binary.h"
#include "fake.h"
#include "buffer.h"

const char *OpCodeStr(int opcode) {
    switch (opcode) {
#define OPCODE_DEF(op)\
    case OPCODE_##op:\
        return #op;

        OPCODE_DEF(ASSIGN)

        OPCODE_DEF(PLUS)
        OPCODE_DEF(MINUS)
        OPCODE_DEF(MULTIPLY)
        OPCODE_DEF(DIVIDE)
        OPCODE_DEF(DIVIDE_MOD)
        OPCODE_DEF(STRING_CAT)

        OPCODE_DEF(PLUS_ASSIGN)
        OPCODE_DEF(MINUS_ASSIGN)
        OPCODE_DEF(MULTIPLY_ASSIGN)
        OPCODE_DEF(DIVIDE_ASSIGN)
        OPCODE_DEF(DIVIDE_MOD_ASSIGN)

        OPCODE_DEF(RETURN)

        OPCODE_DEF(JNE)
        OPCODE_DEF(JMP)

        OPCODE_DEF(AND)
        OPCODE_DEF(OR)
        OPCODE_DEF(LESS)
        OPCODE_DEF(MORE)
        OPCODE_DEF(EQUAL)
        OPCODE_DEF(MOREEQUAL)
        OPCODE_DEF(LESSEQUAL)
        OPCODE_DEF(NOTEQUAL)
        OPCODE_DEF(NOT)

        OPCODE_DEF(AND_JNE)
        OPCODE_DEF(OR_JNE)
        OPCODE_DEF(LESS_JNE)
        OPCODE_DEF(MORE_JNE)
        OPCODE_DEF(EQUAL_JNE)
        OPCODE_DEF(MOREEQUAL_JNE)
        OPCODE_DEF(LESSEQUAL_JNE)
        OPCODE_DEF(NOTEQUAL_JNE)
        OPCODE_DEF(NOT_JNE)

        OPCODE_DEF(CALL)

        OPCODE_DEF(SLEEP)
        OPCODE_DEF(YIELD)

        OPCODE_DEF(FOR)
#undef OPCODE_DEF
    }
    assert(0);
    return "unknow";
}

String dump_addr(int code) {
    String ret;
    int addrtype = HIINT16(code);
    int pos = LOINT16(code);
    switch (addrtype) {
        case ADDR_STACK:
            ret += "STACK";
            break;
        case ADDR_CONST:
            ret += "CONST";
            break;
        case ADDR_CONTAINER:
            ret += "CONTAINER";
            break;
        default:
            ret += "unknow ";
            ret += fkitoa(addrtype);
    }
    ret += "\t";
    ret += fkitoa(pos);
    return ret;
}

String func_binary::dump(int pos) const {
    String ret;

    // 名字
    ret += "\n[";
    ret += m_name;
    ret += "]\n";

    // 最大栈
    ret += "\tmaxstack:\t";
    ret += fkitoa(m_maxstack);
    ret += "\n\n";

    // 常量表
    ret += "\t////// const define ";
    ret += fkitoa(m_const_list_num);
    ret += " //////\n";
    for (int i = 0; i < (int) m_const_list_num; i++) {
        ret += "\t[";
        ret += fkitoa(i);
        ret += "]\t";
        ret += vartypetostring(m_const_list[i].type);
        ret += "\t";
        ret += vartostring(&m_const_list[i]);
        ret += "\n";
    }

    // 容器地址表
    ret += "\n\t////// container addr ";
    ret += fkitoa(m_container_addr_list_num);
    ret += " //////\n";
    for (int i = 0; i < (int) m_container_addr_list_num; i++) {
        ret += "\t[";
        ret += fkitoa(i);
        ret += "]\t";
        command concmd = m_container_addr_list[i].con;
        int concode = COMMAND_CODE(concmd);
        ret += "[ CONTAINER ]\t";
        ret += dump_addr(concode);
        command keycmd = m_container_addr_list[i].key;
        int keycode = COMMAND_CODE(keycmd);
        ret += "\t[ KEY ]\t";
        ret += dump_addr(keycode);
        ret += "\n";
    }

    // 变量地址
    ret += "\n\t////// stack variant addr ";
    ret += fkitoa(m_debug_stack_variant_info_num);
    ret += " //////\n";
    for (int i = 0; i < (int) m_debug_stack_variant_info_num; i++) {
        stack_variant_info info = m_debug_stack_variant_info[i];
        ret += "\t[";
        ret += fkitoa(info.pos);
        ret += "]\t";
        ret += info.name;
        ret += "\t\tLINE\t";
        ret += fkitoa(info.line);
        ret += "\n";
    }

    ret += "\n\t////// byte code ";
    ret += fkitoa(m_size);
    ret += " //////\n";
    // 字节码
    for (int i = 0; i < (int) m_size; i++) {
        command cmd = m_buff[i];
        int type = COMMAND_TYPE(cmd);
        int code = COMMAND_CODE(cmd);
        if (i == pos) {
            ret += "->";
        }
        ret += "\t[";
        ret += fkitoa(i);
        ret += "]";
        ret += "[LINE ";
        ret += fkitoa(FUNC_BINARY_LINENO(*this, i));
        ret += "]\t";
        ret += fkxtoa(cmd);
        ret += "\t";
        switch (type) {
            case COMMAND_OPCODE: {
                ret += "[";
                ret += OpCodeStr(code);
                ret += "]\t";
            }
                break;
            case COMMAND_ADDR: {
                ret += "[ ADDR ]\t";
                ret += dump_addr(code);
            }
                break;
            case COMMAND_POS: {
                ret += "[ POS  ]\t";
                ret += fkitoa(code);
            }
                break;
            default: {
                ret += "[unknow]\t";
            }
                break;
        }
        ret += "\n";
    }
    ret += "\n";
    return ret;
}

String &binary::dump() const {
    m_dump.clear();
    for (const fkhashmap<variant, funcunion>::ele *p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next()) {
        const funcunion &f = *p->t;
        if (f.havefb) {
            m_dump += f.fb.dump();
        }
    }
    return m_dump;
}

String &binary::dump(const char *func, int pos) const {
    fake *fk = m_fk;
    m_dump.clear();
    variant funcv;
    V_SET_STRING(&funcv, func);
    const funcunion *f = m_fk->fm.get_func(funcv);
    if (f && f->havefb) {
        m_dump += f->fb.dump(pos);
    } else {
        m_dump = "not find ";
        m_dump += func;
    }
    return m_dump;
}

bool binary::add_func(const variant &name, const func_binary &bin) {
    const funcunion *f = m_fk->fm.get_func(name);
    if (f && f->havefb && FUNC_BINARY_USE(f->fb)) {
        FKLOG("[binary] add_func func %s add back bin", vartostring(&name).c_str());
        if (FUNC_BINARY_BACKUP(f->fb)) {
            func_binary &fb = *FUNC_BINARY_BACKUP(f->fb);
            FUNC_BINARY_DELETE(fb);
            safe_fkfree(m_fk, FUNC_BINARY_BACKUP(f->fb));
        }
        FUNC_BINARY_BACKUP(f->fb) = (func_binary *) safe_fkmalloc(m_fk, sizeof(func_binary), emt_func_binary);
        *FUNC_BINARY_BACKUP(f->fb) = bin;
    } else {
        FKLOG("[binary] add_func func %s add bin", vartostring(&name).c_str());
        m_fk->fm.add_func(name, bin);
    }

    FKLOG("add func %s", vartostring(&name).c_str());

    return true;
}

bool binary::save(buffer *b) const {
    int n = 0;
    for (const fkhashmap<variant, funcunion>::ele *p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next()) {
        const funcunion &f = *p->t;
        if (f.havefb) {
            n++;
        }
    }
    if (!b->write((const char *) &n, sizeof(n))) {
        return false;
    }
    FKLOG("start save func_binary %d", n);

    for (const fkhashmap<variant, funcunion>::ele *p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next()) {
        const variant &fv = p->k;
        const funcunion &f = *p->t;
        if (f.havefb) {
            if (!save_variant(m_fk, &fv, b)) {
                return false;
            }
            if (!f.fb.save(m_fk, b)) {
                return false;
            }
            FKLOG("save func_binary %s %d", vartostring(&fv).c_str(), (int) b->size());
        }
    }
    return true;
}

bool binary::load(buffer *b) {
    int n = 0;
    if (!b->read((char *) &n, sizeof(n))) {
        return false;
    }
    FKLOG("start load func_binary %d", n);

    for (int i = 0; i < n; i++) {
        variant fv;
        if (!load_variant(m_fk, &fv, b)) {
            return false;
        }
        func_binary fb;
        FUNC_BINARY_INI(fb);
        if (!fb.load(m_fk, b)) {
            return false;
        }
        if (!add_func(fv, fb)) {
            return false;
        }

        FKLOG("load func_binary %s %d", vartostring(&fv).c_str(), (int) b->capacity() - (int) b->size());
    }
    return true;
}

bool func_binary::save(fake *fk, buffer *b) const {
    SAVE_NORMAL(m_maxstack);
    SAVE_NORMAL(m_paramnum);
    SAVE_STRING(m_name);
    SAVE_STRING(m_filename);
    SAVE_STRING(m_packagename);
    SAVE_ARRAY(m_buff, m_size);
    SAVE_VARRAY(m_const_list, m_const_list_num);
    SAVE_ARRAY(m_container_addr_list, m_container_addr_list_num);
    return true;
}

bool func_binary::load(fake *fk, buffer *b) {
    LOAD_NORMAL(m_maxstack);
    LOAD_NORMAL(m_paramnum);
    LOAD_STRING(m_name);
    LOAD_STRING(m_filename);
    LOAD_STRING(m_packagename);
    LOAD_ARRAY(m_buff, m_size, command);
    LOAD_VARRAY(m_const_list, m_const_list_num);
    LOAD_ARRAY(m_container_addr_list, m_container_addr_list_num, container_addr);
    m_fresh++;
    return true;
}

