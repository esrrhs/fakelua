#include "profile.h"
#include "fake.h"

void profile::add_func_sample(const char *func, uint32_t calltime) {
    profilefuncele *p = m_shh.get(func);
    if (!p) {
        profilefuncele tmp;
        tmp.callnum = 1;
        tmp.calltime = calltime;
        m_shh.add(func, tmp);
        return;
    }

    p->callnum++;
    p->calltime += calltime;
}

void profile::add_code_sample(int code) {
    if (LIKE(code >= 0 && code < OPCODE_MAX)) {
        m_codetype[code]++;
    }
}

void profile::add_gc_sample(int type, uint32_t calltime) {
    profilefuncele *p = m_gcshh.get(type);
    if (!p) {
        profilefuncele tmp;
        tmp.callnum = 1;
        tmp.calltime = calltime;
        m_gcshh.add(type, tmp);
        return;
    }

    p->callnum++;
    p->calltime += calltime;
}

typedef std::pair<String, profilefuncele> sortele;

struct profilefuncelesort {
    bool operator()(const sortele &l, const sortele &r) {
        return l.second.calltime > r.second.calltime;
    }
};

const char *profile::dump() {
    std::vector<sortele> sortelevec;
    for (const stringhashmap::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        const profilefuncele &ele = *p->t;
        sortelevec.push_back(std::make_pair(p->k, ele));
    }

    std::sort(sortelevec.begin(), sortelevec.end(), profilefuncelesort());

    std::vector<sortele> gcsortelevec;
    for (const gchashmap::ele *p = m_gcshh.first(); p != 0; p = m_gcshh.next()) {
        const profilefuncele &ele = *p->t;
        gcsortelevec.push_back(std::make_pair(get_gc_type_name(p->k), ele));
    }

    std::sort(gcsortelevec.begin(), gcsortelevec.end(), profilefuncelesort());

    m_dumpstr.clear();

    int wraplen = 30;

    m_dumpstr += "Call Func:\n";
    m_dumpstr += "\t";
    m_dumpstr += fix_string_wrap("Func", wraplen);
    m_dumpstr += fix_string_wrap("Calls", wraplen);
    m_dumpstr += fix_string_wrap("TotalTime(ms)", wraplen);
    m_dumpstr += fix_string_wrap("PerCallTime(ms)", wraplen);
    m_dumpstr += "\n";
    for (int i = 0; i < (int) sortelevec.size(); i++) {
        const sortele &se = sortelevec[i];
        const profilefuncele &ele = se.second;
        m_dumpstr += "\t";
        m_dumpstr += fix_string_wrap(se.first, wraplen);
        m_dumpstr += fix_string_wrap(fkitoa(ele.callnum), wraplen);
        m_dumpstr += fix_string_wrap(fkitoa(ele.calltime), wraplen);
        m_dumpstr += fix_string_wrap(fkitoa(ele.callnum ? ele.calltime / ele.callnum : 0), wraplen);
        m_dumpstr += "\n";
    }

    m_dumpstr += "GC:\n";
    m_dumpstr += "\t";
    m_dumpstr += fix_string_wrap("Type", wraplen);
    m_dumpstr += fix_string_wrap("Calls", wraplen);
    m_dumpstr += fix_string_wrap("TotalTime(ms)", wraplen);
    m_dumpstr += fix_string_wrap("PerCallTime(ms)", wraplen);
    m_dumpstr += "\n";
    for (int i = 0; i < (int) gcsortelevec.size(); i++) {
        const sortele &se = gcsortelevec[i];
        const profilefuncele &ele = se.second;
        m_dumpstr += "\t";
        m_dumpstr += fix_string_wrap(se.first, wraplen);
        m_dumpstr += fix_string_wrap(fkitoa(ele.callnum), wraplen);
        m_dumpstr += fix_string_wrap(fkitoa(ele.calltime), wraplen);
        m_dumpstr += fix_string_wrap(fkitoa(ele.callnum ? ele.calltime / ele.callnum : 0), wraplen);
        m_dumpstr += "\n";
    }

    m_dumpstr += "Code Num:\n";
    for (int i = 0; i < OPCODE_MAX; i++) {
        m_dumpstr += "\t";
        m_dumpstr += OpCodeStr(i);
        for (int j = 0; j < (int) (20 - strlen(OpCodeStr(i))); j++) {
            m_dumpstr += " ";
        }
        m_dumpstr += fkitoa(m_codetype[i]);
        m_dumpstr += "\n";
    }

    return m_dumpstr.c_str();
}

const char *profile::dumpstat() {
    m_dumpstr.clear();

    int wraplen = 40;

    m_dumpstr += fix_string_wrap("Func Map size:", wraplen);
    m_dumpstr += fkitoa(m_fk->fm.size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("String Heap size:", wraplen);
    m_dumpstr += fkitoa(m_fk->sh.size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("System String Heap size:", wraplen);
    m_dumpstr += fkitoa(m_fk->sh.sys_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Stack String Heap size:", wraplen);
    m_dumpstr += fkitoa(m_fk->sh.size() - m_fk->sh.sys_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("String Heap Byte size:", wraplen);
    m_dumpstr += fkitoa(m_fk->sh.bytesize());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("System String Heap Byte size:", wraplen);
    m_dumpstr += fkitoa(m_fk->sh.sys_bytesize());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Stack String Heap Byte size:", wraplen);
    m_dumpstr += fkitoa(m_fk->sh.bytesize() - m_fk->sh.sys_bytesize());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Pointer Heap size:", wraplen);
    m_dumpstr += fkitoa(m_fk->ph.size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Processor Pool size:", wraplen);
    m_dumpstr += fkitoa(POOL_GROW_SIZE(m_fk->pp));
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Map Container Pool size:", wraplen);
    m_dumpstr += fkitoa(m_fk->con.get_map_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Array Container Pool size:", wraplen);
    m_dumpstr += fkitoa(m_fk->con.get_array_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Variant Container Pool size:", wraplen);
    m_dumpstr += fkitoa(m_fk->con.get_variant_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Const Map Container Pool size:", wraplen);
    m_dumpstr += fkitoa(m_fk->con.get_cmap_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Const Array Container Pool size:", wraplen);
    m_dumpstr += fkitoa(m_fk->con.get_carray_size());
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Const Variant Container Pool size:", wraplen);
    m_dumpstr += fkitoa(m_fk->con.get_cvariant_size());
    m_dumpstr += "\n";

    return m_dumpstr.c_str();
}

const char *profile::dumpmem() {
    if (!m_isopen) {
        return "not open profile\n";
    }

    m_dumpstr.clear();

    int wraplen = 20;

    m_dumpstr += fix_string_wrap("Malloc Num:", wraplen);
    m_dumpstr += fkitoa(m_memmalloc_num);
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Free Num:", wraplen);
    m_dumpstr += fkitoa(m_memfree_num);
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Malloc Size:", wraplen);
    m_dumpstr += fkitoa(m_memmalloc_size);
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Free Size:", wraplen);
    m_dumpstr += fkitoa(m_memfree_size);
    m_dumpstr += "\n";

    m_dumpstr += fix_string_wrap("Mem Pointer Num:", wraplen);
    m_dumpstr += fkitoa(m_memuse.size());
    m_dumpstr += "\n";

    size_t memnumtotal[emt_max];
    size_t memsizetotal[emt_max];
    memset(memnumtotal, 0, sizeof(memnumtotal));
    memset(memsizetotal, 0, sizeof(memsizetotal));
    for (memhashmap::iterator it = m_memuse.begin(); it != m_memuse.end(); it++) {
        const profilememele &ele = it->second;
        assert(ele.type >= 0 && ele.type < emt_max);
        memsizetotal[ele.type] += ele.size;
        memnumtotal[ele.type]++;
    }

    m_dumpstr += "[Mem Type Num]:\n";
    for (int i = 0; i < emt_max; i++) {
        e_mem_type type = (e_mem_type) i;
        m_dumpstr += fix_string_wrap(get_mem_type_name(type), wraplen);
        m_dumpstr += ":";
        m_dumpstr += fkitoa(memnumtotal[type]);
        m_dumpstr += "\n";
    }

    m_dumpstr += "[Mem Type Size]:\n";
    for (int i = 0; i < emt_max; i++) {
        e_mem_type type = (e_mem_type) i;
        m_dumpstr += fix_string_wrap(get_mem_type_name(type), wraplen);
        m_dumpstr += ":";
        m_dumpstr += fkitoa(memsizetotal[type]);
        m_dumpstr += "\n";
    }

    return m_dumpstr.c_str();
}

void profile::add_mem(void *p, size_t size, e_mem_type type) {
    memhashmap::iterator it = m_memuse.find(p);
    if (it == m_memuse.end()) {
        profilememele tmp;
        tmp.type = type;
        tmp.size = size;
        m_memuse[p] = tmp;
        m_memmalloc_num++;
        m_memmalloc_size += size;
    } else {
        assert(0);
    }
}

void profile::dec_mem(void *p) {
    memhashmap::iterator it = m_memuse.find(p);
    if (it == m_memuse.end()) {
        assert(0);
    } else {
        m_memfree_num++;
        m_memfree_size += it->second.size;
        m_memuse.erase(p);
    }
}

