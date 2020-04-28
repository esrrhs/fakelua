#include "buildinfuncstring.h"
#include "fake.h"
#include "fake-inc.h"

// find
void buildin_string_find(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(3);

    const char *findstr = fkpspopcstrptr(fk);
    int pos = fkpspop<int>(fk);
    const char *srcstr = 0;
    variant *v = 0;
    bool err = false;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_STRING(v, srcstr);
    CHECK_ERR(err);

    int len = srcstr ? v->data.str->sz : 0;
    if (pos >= 0 && pos < len && srcstr && findstr) {
        const char *find = strstr(srcstr + pos, findstr);
        if (find) {
            fkpspush<bool>(fk, true);
            fkpspush<int>(fk, (int) (find - srcstr));
        } else {
            fkpspush<bool>(fk, false);
            fkpspush<int>(fk, -1);
        }
    } else {
        fkpspush<bool>(fk, false);
        fkpspush<int>(fk, -1);
    }
}

// find not
void buildin_string_find_not(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(3);

    const char *findstr = 0;
    variant *findv = 0;
    bool err = false;
    PS_POP_AND_GET(fk->ps, findv);
    V_GET_STRING(findv, findstr);
    CHECK_ERR(err);

    int pos = fkpspop<int>(fk);
    const char *srcstr = 0;
    variant *v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_STRING(v, srcstr);

    int len = srcstr ? v->data.str->sz : 0;
    int findlen = findstr ? findv->data.str->sz : 0;
    if (pos >= 0 && pos < len && srcstr && findstr) {
        for (int i = pos; i < len; i++) {
            if (strncmp(srcstr + i, findstr, findlen)) {
                fkpspush<bool>(fk, true);
                fkpspush<int>(fk, i);
                return;
            }
        }
        fkpspush<bool>(fk, false);
        fkpspush<int>(fk, -1);
    } else {
        fkpspush<bool>(fk, false);
        fkpspush<int>(fk, -1);
    }
}

// substr
void buildin_string_substr(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(3);

    int count = fkpspop<int>(fk);
    int pos = fkpspop<int>(fk);
    const char *srcstr = 0;
    variant *v = 0;
    bool err = false;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_STRING(v, srcstr);
    CHECK_ERR(err);

    int len = srcstr ? v->data.str->sz : 0;
    if (count == -1) {
        count = 0x7FFFFFFF;
    }
    if (count < 0) {
        fkpspush<const char *>(fk, "");
        return;
    }
    if ((uint32_t) ((uint32_t) pos + (uint32_t) count) > (uint32_t) len) {
        count = len - pos;
    }

    if (pos >= 0 && pos < len && srcstr && count > 0) {
        char *buf = (char *) safe_fkmalloc(fk, count + 1, emt_tmp);
        buf[count] = 0;
        memcpy(buf, srcstr + pos, count);
        fkpspush<const char *>(fk, buf);
        safe_fkfree(fk, buf);
    } else {
        fkpspush<const char *>(fk, "");
    }
}

// trim
void buildin_string_trim(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *trimstr = fkpspopcstrptr(fk);
    const char *str = fkpspopcstrptr(fk);
    if (trimstr && str) {
        // left
        int trimlen = strlen(trimstr);
        while (1) {
            const char *s = strstr(str, trimstr);
            if (s == str) {
                str += trimlen;
            } else {
                break;
            }
        }

        // right
        const char *srcstr = str;
        int endpos = strlen(str);
        int trimnum = 0;
        while (str < srcstr + endpos) {
            const char *s = strstr(str, trimstr);
            if (s == str) {
                str += trimlen;
                trimnum++;
            } else if (s) {
                trimnum = 0;
                str = s;
            } else {
                trimnum = 0;
                break;
            }
        }
        int afterlen = endpos - trimlen * trimnum;
        assert(afterlen >= 0);
        char *buf = (char *) safe_fkmalloc(fk, afterlen + 1, emt_tmp);
        buf[afterlen] = 0;
        memcpy(buf, srcstr, afterlen);
        fkpspush<const char *>(fk, buf);
        safe_fkfree(fk, buf);
    } else {
        fkpspush<const char *>(fk, str);
    }
}

// trim left
void buildin_string_trim_left(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *trimstr = fkpspopcstrptr(fk);
    const char *str = fkpspopcstrptr(fk);
    if (trimstr && str) {
        int trimlen = strlen(trimstr);
        while (1) {
            const char *s = strstr(str, trimstr);
            if (s == str) {
                str += trimlen;
            } else {
                break;
            }
        }
        fkpspush<const char *>(fk, str);
    } else {
        fkpspush<const char *>(fk, str);
    }
}

// trim
void buildin_string_trim_right(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *trimstr = fkpspopcstrptr(fk);
    const char *str = fkpspopcstrptr(fk);
    if (trimstr && str) {
        int trimlen = strlen(trimstr);
        const char *srcstr = str;
        int endpos = strlen(str);
        int trimnum = 0;
        while (str < srcstr + endpos) {
            const char *s = strstr(str, trimstr);
            if (s == str) {
                str += trimlen;
                trimnum++;
            } else if (s) {
                trimnum = 0;
                str = s;
            } else {
                trimnum = 0;
                break;
            }
        }
        int afterlen = endpos - trimlen * trimnum;
        assert(afterlen >= 0);
        char *buf = (char *) safe_fkmalloc(fk, afterlen + 1, emt_tmp);
        buf[afterlen] = 0;
        memcpy(buf, srcstr, afterlen);
        fkpspush<const char *>(fk, buf);
        safe_fkfree(fk, buf);
    } else {
        fkpspush<const char *>(fk, str);
    }
}

// replace
void buildin_string_replace(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(3);

    const char *tostr = fkpspopcstrptr(fk);
    const char *fromstr = fkpspopcstrptr(fk);
    const char *str = fkpspopcstrptr(fk);

    if (!tostr || !fromstr || !str) {
        fkpspush<const char *>(fk, "");
        return;
    }

    int str_size = strlen(str);
    int from_size = strlen(fromstr);
    int to_size = strlen(tostr);

    if (from_size == 0) {
        fkpspush<const char *>(fk, str);
        return;
    }

    char *tmpbuff = 0;

    int delta = to_size - from_size;
    if (delta == 0) {
        tmpbuff = (char *) safe_fkmalloc(fk, str_size + 1, emt_tmp);
        memcpy(tmpbuff, str, str_size);
        tmpbuff[str_size] = 0;

        int pos = 0;
        const char *find = strstr(tmpbuff + pos, fromstr);
        while (find) {
            memcpy((void *) find, tostr, to_size);
            pos = find - tmpbuff + to_size;
            find = strstr(tmpbuff + pos, fromstr);
        }
    } else {
        int num = 0;

        int pos = 0;
        const char *find = strstr(str + pos, fromstr);
        while (find) {
            num++;
            pos = find - str + from_size;
            find = strstr(str + pos, fromstr);
        }

        if (num >= 0) {
            int new_str_size = str_size + num * delta + 1;
            tmpbuff = (char *) safe_fkmalloc(fk, new_str_size, emt_tmp);
            tmpbuff[str_size + num * delta] = 0;

            char *pbuff = tmpbuff;

            int pos = 0;
            const char *find = strstr(str + pos, fromstr);
            while (find) {
                int skip_size = find - str - pos;
                if (skip_size > 0) {
                    memcpy(pbuff, str + pos, skip_size);
                    pbuff += skip_size;
                }

                memcpy(pbuff, tostr, to_size);
                pbuff += to_size;

                pos = find - str + from_size;
                find = strstr(str + pos, fromstr);
            }

            if (pos < str_size) {
                memcpy(pbuff, str + pos, str_size - pos);
            }
        } else {
            fkpspush<const char *>(fk, str);
            return;
        }
    }

    fkpspush<const char *>(fk, tmpbuff);
    safe_fkfree(fk, tmpbuff);
}

// cat
void buildin_string_cat(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *right = fkpspopcstrptr(fk);
    String leftstr = fkpspopcstrptr(fk);
    leftstr += right;
    fkpspush<const char *>(fk, leftstr.c_str());
}

// cmp
void buildin_string_cmp(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *right = fkpspopcstrptr(fk);
    const char *left = fkpspopcstrptr(fk);
    int ret = strcmp(right, left);
    fkpspush<int>(fk, ret);
}

// tolower
void buildin_string_tolower(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(1);

    String str = fkpspopcstrptr(fk);
    for (int i = 0; i < (int) str.size(); i++) {
        str[i] = tolower(str[i]);
    }
    fkpspush<const char *>(fk, str.c_str());
}

// toupper
void buildin_string_toupper(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(1);

    String str = fkpspopcstrptr(fk);
    for (int i = 0; i < (int) str.size(); i++) {
        str[i] = toupper(str[i]);
    }
    fkpspush<const char *>(fk, str.c_str());
}

void buildinfuncstring::openstringfunc() {
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_find"), buildin_string_find);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_find_not"), buildin_string_find_not);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_substr"), buildin_string_substr);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_trim"), buildin_string_trim);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_trimleft"), buildin_string_trim_left);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_trimright"), buildin_string_trim_right);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_replace"), buildin_string_replace);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_cat"), buildin_string_cat);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_cmp"), buildin_string_cmp);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_tolower"), buildin_string_tolower);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_toupper"), buildin_string_toupper);
}
