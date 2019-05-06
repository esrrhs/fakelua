#include "gc.h"
#include "fake.h"

gc::gc(fake * fk) : m_fk(fk)
{
}

gc::~gc()
{
	clear();
}

void gc::clear()
{
}

void gc::check(bool forcegc)
{
    m_fk->sh.checkgc(forcegc);

    // TODO
}

const char * gc::dump()
{
	m_dumpstr.clear();
	return m_dumpstr.c_str();
}

#define PUSH_STRING_RET(ret, ss) \
    \
    if (UNLIKE(ARRAY_SIZE(ret) >= ARRAY_MAX_SIZE(ret))) \
    { \
        size_t newsize = ARRAY_SIZE(ret) + 1 + ARRAY_MAX_SIZE(ret) * (m_fk->cfg.array_grow_speed) / 100; \
        ARRAY_GROW(ret, newsize, stringele *); \
    } \
    ARRAY_PUSH_BACK(ret); \
    ARRAY_BACK(ret) = ss; \
    \
    FKLOG("PUSH_STRING_RET %p add string %s", m_fk, v->data.str->s);


#define PUSH_ENTRY(ret, v, find) \
    if (UNLIKE(!find.get((void*)v))) \
    { \
        if (UNLIKE(ARRAY_SIZE(entry) >= ARRAY_MAX_SIZE(entry))) \
        { \
            size_t newsize = ARRAY_SIZE(entry) + 1 + ARRAY_MAX_SIZE(entry) * (m_fk->cfg.array_grow_speed) / 100; \
            ARRAY_GROW(entry, newsize, variant *); \
        } \
        ARRAY_PUSH_BACK(entry); \
        ARRAY_BACK(entry) = (variant*)v; \
        find.add((void*)v); \
        FKLOG("PUSH_ENTRY %p add %s %s", m_fk, vartypetostring(v->type), vartostring(v).c_str()); \
    } \
    else \
    { \
        FKLOG("PUSH_ENTRY %p looped %s %s", m_fk, vartypetostring(v->type), vartostring(v).c_str()); \
    }

array<stringele *> gc::get_used_stringele()
{
    FKLOG("start get_used_stringele %p", m_fk);

    array<stringele *> ret;

    array<processor *> & curprocessor = m_fk->rn.curprocessor;

    array<variant *> entry;

    fkhashset<void *> find(m_fk);

    for (int i = 0; LIKE(i < (int)ARRAY_SIZE(curprocessor)); i++)
    {
        processor * pro = ARRAY_GET(curprocessor, i);

        for (int j = 0; LIKE(j < (int)ARRAY_SIZE(pro->m_pl.l)); j++)
        {
            routine * n = ARRAY_GET(pro->m_pl.l, j);

            int bp = n->m_interpreter.m_bp;
            const func_binary * fb = n->m_interpreter.m_fb;

            while (LIKE((!BP_END(bp))))
            {
                for (int z = 0; LIKE(z < fb->m_maxstack); z++)
                {
                    variant * v = 0;
                    GET_STACK_BY_INTER(bp, v, z, n->m_interpreter);
                    if (UNLIKE(v->type == variant::STRING))
                    {
                        PUSH_STRING_RET(ret, v->data.str);
                    }
                    else if (UNLIKE(v->type == variant::ARRAY ||
                        v->type == variant::MAP))
                    {
                        PUSH_ENTRY(ret, v, find);
                    }
                }

                BP_GET_FB_BY_INTER(bp, fb, n->m_interpreter);
                int callbp = 0;
                BP_GET_BP_BY_INTER(bp, callbp, n->m_interpreter);
                bp = callbp;
                if (UNLIKE(BP_END(bp)))
                {
                    break;
                }
            }

        }
    }

    while (UNLIKE(!ARRAY_EMPTY(entry)))
    {
        variant * v = ARRAY_BACK(entry);
        ARRAY_POP_BACK(entry);
        if (LIKE(v->type == variant::STRING))
        {
            PUSH_STRING_RET(ret, v->data.str);
        }
        else if (UNLIKE(v->type == variant::ARRAY))
        {
            variant_array * va = v->data.va;

            for (int i = 0; LIKE(i < va->va.m_max_size); ++i)
            {
                variant * n = ARRAY_GET(va->va, i);
                if (LIKE(n != 0))
                {
                    if (UNLIKE(n->type == variant::STRING))
                    {
                        PUSH_STRING_RET(ret, n->data.str);
                    }
                    else if (UNLIKE(n->type == variant::ARRAY ||
                                    n->type == variant::MAP))
                    {
                        PUSH_ENTRY(ret, n, find);
                    }
                }
            }
        }
        else if (UNLIKE(v->type == variant::MAP))
        {
            variant_map * vm = v->data.vm;

            for (const fkhashmap<variant, variant *>::ele * p = vm->vm.first(); p != 0; p = vm->vm.next())
            {
                const variant *key = &(p->k);
                const variant *value = *p->t;

                if (UNLIKE(key->type == variant::STRING))
                {
                    PUSH_STRING_RET(ret, key->data.str);
                }
                else if (UNLIKE(key->type == variant::ARRAY ||
                                         key->type == variant::MAP))
                {
                    PUSH_ENTRY(ret, key, find);
                }

                if (UNLIKE(value->type == variant::STRING))
                {
                    PUSH_STRING_RET(ret, value->data.str);
                }
                else if (UNLIKE(value->type == variant::ARRAY ||
                                        value->type == variant::MAP))
                {
                    PUSH_ENTRY(ret, value, find);
                }
            }
        }
    }

    ARRAY_DELETE(entry);

    FKLOG("end get_used_stringele %p %u", m_fk, ARRAY_SIZE(ret));

    return ret;
}


