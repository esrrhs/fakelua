#pragma once

#include "types.h"
#include "variant.h"

const uint32_t c_hashsize[] = {
        1ul,
        3ul,
        5ul,
        9ul,
        17ul,
        31ul,
        53ul,
        97ul,
        193ul,
        769ul,
        1543ul,
        3079ul,
        6151ul,
        12289ul,
        24593ul,
        49157ul,
        98317ul,
        196613ul,
        393241ul,
        786433ul,
        1572869ul,
        3145739ul,
        6291469ul,
        12582917ul,
        25165843ul,
        50331653ul,
        100663319ul,
        201326611ul,
        402653189ul,
        805306457ul,
        1610612741ul,
        3221225473ul,
};

#define HASHMAP_INI(hm, fk) (hm).m_set.m_fk = fk
#define HASHMAP_CLEAR(hm) (hm).clear()
#define HASHMAP_DELETE(hm) (hm).~fkhashmap()
#define HASHMAP_RECUR(hm) ((hm).m_set.m_recurflag != 0)
#define HASHMAP_ENTER(hm) ((hm).m_set.m_recurflag++)
#define HASHMAP_LEAVE(hm) ((hm).m_set.m_recurflag--)

template<typename T>
force_inline uint64_t fkkeyhash(const T &k) {
    return k;
}

template<typename T>
force_inline bool fkkeycmp(const T &lk, const T &rk) {
    return lk == rk;
}

template<typename T>
force_inline T fkkeycpy(fake *fk, const T &ok, const T &k) {
    return k;
}

template<typename T>
force_inline void fkkeydel(fake *fk, const T &k) {
}

template<typename T>
force_inline String fkkeytostr(const T &k) {
    return "";
}

// 特化
template<>
force_inline uint64_t fkkeyhash(const char *const &k) {
    return fkstrhash(k);
}

template<>
force_inline bool fkkeycmp(const char *const &lk, const char *const &rk) {
    return !strcmp(lk, rk);
}

template<>
force_inline const char *fkkeycpy(fake *fk, const char *const &ok, const char *const &k) {
    safe_fkfree(fk, ok);
    return stringdump(fk, k, strlen(k), emt_hashstring);
}

template<>
force_inline void fkkeydel(fake *fk, const char *const &k) {
    safe_fkfree(fk, k);
}

template<>
force_inline String fkkeytostr(const char *const &k) {
    return k;
}

// 特化
template<>
force_inline uint64_t fkkeyhash(const variant &k) {
    return k.data.buf;
}

template<>
force_inline bool fkkeycmp(const variant &lk, const variant &rk) {
    bool b = false;
    V_EQUAL_V(b, (&lk), (&rk));
    return b;
}

template<>
force_inline variant fkkeycpy(fake *fk, const variant &ok, const variant &k) {
    return k;
}

template<>
force_inline void fkkeydel(fake *fk, const variant &k) {
}

template<>
force_inline String fkkeytostr(const variant &k) {
    return vartostring(&k);
}

// 特化
template<>
force_inline uint64_t fkkeyhash(void *const &k) {
    MarshallPoiner tmp;
    tmp.p = k;
    return tmp.i;
}

template<>
force_inline bool fkkeycmp(void *const &lk, void *const &rk) {
    return lk == rk;
}

template<>
force_inline void *fkkeycpy(fake *fk, void *const &ok, void *const &k) {
    return k;
}

template<>
force_inline void fkkeydel(fake *fk, void *const &k) {
}

template<>
force_inline String fkkeytostr(void *const &k) {
    return fkptoa(k);
}

// 特化
template<>
force_inline uint64_t fkkeyhash(stringele *const &k) {
    return fkstrhash(k->s);
}

template<>
force_inline bool fkkeycmp(stringele *const &lk, stringele *const &rk) {
    return !strcmp(lk->s, rk->s);
}

template<>
force_inline stringele *fkkeycpy(fake *fk, stringele *const &ok, stringele *const &k) {
    return k;
}

template<>
force_inline void fkkeydel(fake *fk, stringele *const &k) {
}

template<>
force_inline String fkkeytostr(stringele *const &k) {
    return k->s;
}

// 特化
template<>
force_inline uint64_t fkkeyhash(variant *const &k) {
    MarshallPoiner tmp;
    tmp.p = k;
    return tmp.i;
}

template<>
force_inline bool fkkeycmp(variant *const &lk, variant *const &rk) {
    return lk == rk;
}

template<>
force_inline variant *fkkeycpy(fake *fk, variant *const &ok, variant *const &k) {
    return k;
}

template<>
force_inline void fkkeydel(fake *fk, variant *const &k) {
}

template<>
force_inline String fkkeytostr(variant *const &k) {
    return fkptoa(k);
}

// 特化
template<>
force_inline uint64_t fkkeyhash(variant_array *const &k) {
    MarshallPoiner tmp;
    tmp.p = k;
    return tmp.i;
}

template<>
force_inline bool fkkeycmp(variant_array *const &lk, variant_array *const &rk) {
    return lk == rk;
}

template<>
force_inline variant_array *fkkeycpy(fake *fk, variant_array *const &ok, variant_array *const &k) {
    return k;
}

template<>
force_inline void fkkeydel(fake *fk, variant_array *const &k) {
}

template<>
force_inline String fkkeytostr(variant_array *const &k) {
    return fkptoa(k);
}

// 特化
template<>
force_inline uint64_t fkkeyhash(variant_map *const &k) {
    MarshallPoiner tmp;
    tmp.p = k;
    return tmp.i;
}

template<>
force_inline bool fkkeycmp(variant_map *const &lk, variant_map *const &rk) {
    return lk == rk;
}

template<>
force_inline variant_map *fkkeycpy(fake *fk, variant_map *const &ok, variant_map *const &k) {
    return k;
}

template<>
force_inline void fkkeydel(fake *fk, variant_map *const &k) {
}

template<>
force_inline String fkkeytostr(variant_map *const &k) {
    return fkptoa(k);
}

template<typename K, typename T>
struct fkhashmapele {
    K k;
    T *t;
};

// 特化
template<typename K, typename T>
force_inline uint64_t fkkeyhash(const fkhashmapele<K, T> &k) {
    return fkkeyhash(k.k);
}

template<typename K, typename T>
force_inline bool fkkeycmp(const fkhashmapele<K, T> &lk, const fkhashmapele<K, T> &rk) {
    return fkkeycmp(lk.k, rk.k);
}

template<typename K, typename T>
force_inline fkhashmapele<K, T> fkkeycpy(fake *fk, const fkhashmapele<K, T> &ok, const fkhashmapele<K, T> &k) {
    fkhashmapele<K, T> ret;
    ret.k = fkkeycpy(fk, ok.k, k.k);
    ret.t = k.t;
    return ret;
}

template<typename K, typename T>
force_inline void fkkeydel(fake *fk, const fkhashmapele<K, T> &k) {
    fkkeydel(fk, k.k);
}

template<typename K, typename T>
force_inline String fkkeytostr(const fkhashmapele<K, T> &k) {
    return fkkeytostr(k.k);
}

#define ELE_FAST_BUFFER (2)
#define GET_HASHELE(he, i) (LIKE((i) < ELE_FAST_BUFFER) ? (he).fe[(i)] : (he).overflow[(i) - ELE_FAST_BUFFER])

template<typename K>
class fkhashset {
public:
    struct ele {
        uint64_t hv;
        K k;
    };
    struct hashele {
        ele fe[ELE_FAST_BUFFER];
        uint16_t size;
        uint16_t maxsize;
        ele *overflow;
    };
public:
    force_inline fkhashset(fake *fk) : m_fk(fk), m_hashele(0), m_hashele_size(0), m_ele_size(0), m_grow_times(0),
                                       m_hashele_iter(0), m_ele_iter(0), m_recurflag(0) {

    }

    force_inline ~fkhashset() {
        clear();
    }

    force_inline void clear() {
        // 清掉旧的
        for (int i = 0; i < m_hashele_size; i++) {
            hashele &he = m_hashele[i];
            for (int j = 0; j < he.size; j++) {
                fkkeydel(m_fk, GET_HASHELE(he, j).k);
            }
            safe_fkfree(m_fk, he.overflow);
            he.size = 0;
            he.maxsize = 0;
            he.overflow = 0;
        }
        safe_fkfree(m_fk, m_hashele);
        m_hashele = 0;
        m_hashele_size = 0;
        m_ele_size = 0;
        m_grow_times = 0;
        m_hashele_iter = 0;
        m_ele_iter = 0;
        m_recurflag = 0;
    }

    force_inline size_t size() const {
        return m_ele_size;
    }

    force_inline bool empty() const {
        return m_ele_size == 0;
    }
    // 小心循环内重入遍历的bug
    force_inline ele *first() {
        for (m_hashele_iter = 0; m_hashele_iter < m_hashele_size; m_hashele_iter++) {
            hashele &he = m_hashele[m_hashele_iter];
            if (he.size > 0) {
                m_ele_iter = 1;
                return &GET_HASHELE(he, 0);
            }
        }
        return 0;
    }
    // 小心循环内重入遍历的bug
    force_inline const ele *first() const {
        for (m_hashele_iter = 0; m_hashele_iter < m_hashele_size; m_hashele_iter++) {
            hashele &he = m_hashele[m_hashele_iter];
            if (he.size > 0) {
                m_ele_iter = 1;
                return &GET_HASHELE(he, 0);
            }
        }
        return 0;
    }

    force_inline ele *next() {
        for (; m_hashele_iter < m_hashele_size; m_hashele_iter++) {
            hashele &he = m_hashele[m_hashele_iter];
            if (m_ele_iter < he.size) {
                ele *ret = &GET_HASHELE(he, m_ele_iter);
                m_ele_iter++;
                return ret;
            }
            m_ele_iter = 0;
        }
        return 0;
    }

    force_inline const ele *next() const {
        for (; m_hashele_iter < m_hashele_size; m_hashele_iter++) {
            hashele &he = m_hashele[m_hashele_iter];
            if (m_ele_iter < he.size) {
                const ele *ret = &GET_HASHELE(he, m_ele_iter);
                m_ele_iter++;
                return ret;
            }
            m_ele_iter = 0;
        }
        return 0;
    }

    force_inline const ele *at(int pos) const {
        for (int i = 0; i < m_hashele_size; i++) {
            hashele &he = m_hashele[i];
            if (he.size) {
                if (pos >= he.size) {
                    pos -= he.size;
                } else {
                    return &GET_HASHELE(he, pos);
                }
            }
        }
        return 0;
    }

    ele *add(const K &k) {
        return _add(k);
    }

    force_inline ele *del(const K &k) {
        if (!m_ele_size) {
            return 0;
        }

        // hash
        uint64_t hv = fkkeyhash(k);
        uint32_t index = hv % m_hashele_size;
        hashele &he = m_hashele[index];
        for (int i = 0; i < he.size; i++) {
            // find
            ele &e = GET_HASHELE(he, i);
            if (LIKE(e.hv == hv && fkkeycmp(e.k, k))) {
                fkkeydel(m_fk, GET_HASHELE(he, i).k);

                // swap
                ele tmp = GET_HASHELE(he, i);
                GET_HASHELE(he, i) = GET_HASHELE(he, he.size - 1);
                GET_HASHELE(he, he.size - 1) = tmp;

                ele *ret = &(GET_HASHELE(he, he.size - 1));

                he.size--;
                m_ele_size--;
                return ret;
            }
        }

        return 0;
    }

    force_inline ele *get(const K &k) const {
        if (UNLIKE(!m_ele_size)) {
            return 0;
        }

        // hash
        uint64_t hv = fkkeyhash(k);
        uint32_t index = hv % m_hashele_size;
        hashele &he = m_hashele[index];
        for (int i = 0; i < he.size; i++) {
            // find
            ele &e = GET_HASHELE(he, i);
            if (LIKE(e.hv == hv && fkkeycmp(e.k, k))) {
                return &e;
            }
        }

        return 0;
    }

    force_inline void get_conflict(int *buff, int buffsize) const {
        memset(buff, 0, sizeof(int) * buffsize);
        for (int i = 0; i < (int) m_hashele_size; i++) {
            hashele &he = m_hashele[i];
            if (he.size < buffsize) {
                buff[he.size] += (he.size != 0 ? he.size : 1);
            }
        }
    }

    force_inline void get_conflict_map(int *buff, int &buffsize) const {
        buffsize = FKMIN(buffsize, m_hashele_size);
        memset(buff, 0, sizeof(int) * buffsize);
        for (int i = 0; i < (int) buffsize; i++) {
            hashele &he = m_hashele[i];
            buff[i] = he.size;
        }
    }

    force_inline void dump(char *buff, int buffsize) const {
        int off = 0;
        for (int i = 0; i < (int) m_hashele_size; i++) {
            hashele &he = m_hashele[i];
            for (int j = 0; j < (int) he.size && off < buffsize; j++) {
                ele &e = GET_HASHELE(he, j);
                off += tsnprintf(buff + off, buffsize - off, "key:%s\n", fkkeytostr(e.k).c_str());
            }
        }
    }

private:
    ele *_add(const K &k) {
        // 检查空间
        int newsize = m_ele_size;
        if (UNLIKE(newsize >= m_hashele_size)) {
            m_grow_times++;

            // 获取新大小
            for (int i = 0; i < (int) (sizeof(c_hashsize) / sizeof(uint32_t)); i++) {
                if (newsize < (int) c_hashsize[i]) {
                    newsize = c_hashsize[i];
                    break;
                }
            }

            // 分配
            hashele *oldhashele = m_hashele;
            int old_hashele_size = m_hashele_size;
            int old_ele_size = m_ele_size;
            m_hashele = (hashele *) safe_fkmalloc(m_fk, sizeof(hashele) * newsize, emt_hashset);
            m_hashele_size = newsize;
            m_ele_size = 0;
            memset(m_hashele, 0, sizeof(hashele) * newsize);

            // 加到新的 清掉旧的
            for (int i = 0; i < old_hashele_size; i++) {
                hashele &he = oldhashele[i];
                for (int j = 0; j < he.size; j++) {
                    ele *e = &GET_HASHELE(he, j);
                    add(e->k);
                }
                for (int j = 0; j < (int) he.maxsize; j++) {
                    fkkeydel(m_fk, GET_HASHELE(he, j).k);
                }
                safe_fkfree(m_fk, he.overflow);
            }
            safe_fkfree(m_fk, oldhashele);

            // 回复
            m_ele_size = old_ele_size;
        }

        // hash
        uint64_t hv = fkkeyhash(k);
        uint32_t index = hv % m_hashele_size;
        hashele &he = m_hashele[index];
        for (int i = 0; i < he.size; i++) {
            // find
            ele &e = GET_HASHELE(he, i);
            if (UNLIKE(e.hv == hv && fkkeycmp(e.k, k))) {
                return &e;
            }
        }

        // 检查空间
        if (UNLIKE(he.size >= he.maxsize)) {
            if (UNLIKE(he.size >= ELE_FAST_BUFFER)) {
                size_t newelesize = he.size;
                // 获取新大小
                for (int i = 0; i < (int) (sizeof(c_hashsize) / sizeof(uint32_t)); i++) {
                    if (newelesize < c_hashsize[i]) {
                        newelesize = c_hashsize[i];
                        break;
                    }
                }

                // 分配
                ele *oldoverflow = he.overflow;
                he.overflow = (ele *) safe_fkmalloc(m_fk, sizeof(ele) * (newelesize - ELE_FAST_BUFFER), emt_hashlist);
                memset(he.overflow, 0, sizeof(ele) * (newelesize - ELE_FAST_BUFFER));
                he.maxsize = newelesize;
                memcpy(he.overflow, oldoverflow, sizeof(ele) * (he.size - ELE_FAST_BUFFER));
                safe_fkfree(m_fk, oldoverflow);
            } else {
                he.maxsize = ELE_FAST_BUFFER;
            }
        }

        // add
        ele &e = GET_HASHELE(he, he.size);
        e.hv = hv;
        e.k = fkkeycpy(m_fk, e.k, k);
        he.size++;
        m_ele_size++;
        return &e;
    }

public:
    fake *m_fk;
    hashele *m_hashele;
    int m_hashele_size;
    int m_ele_size;
    int m_grow_times;
    mutable int m_hashele_iter;
    mutable int m_ele_iter;
    char m_recurflag;
};

// T需要原始C结构 支持memcpy memset
template<typename K, typename T>
class fkhashmap {
public:
    typedef fkhashmapele<K, T> ele;
public:
    force_inline fkhashmap(fake *fk) : m_set(fk) {

    }

    force_inline ~fkhashmap() {
        clear();
    }

    force_inline void clear() {
        for (int i = 0; i < m_set.m_hashele_size; i++) {
            typename fkhashset<ele>::hashele &he = m_set.m_hashele[i];
            for (int j = 0; j < he.size; j++) {
                safe_fkfree(m_set.m_fk, GET_HASHELE(he, j).k.t);
            }
        }
        m_set.clear();
    }

    force_inline size_t size() const {
        return m_set.size();
    }

    force_inline bool empty() const {
        return m_set.empty();
    }
    // 小心循环内重入遍历的bug
    force_inline ele *first() {
        typename fkhashset<ele>::ele *e = m_set.first();
        return e ? &e->k : 0;
    }
    // 小心循环内重入遍历的bug
    force_inline const ele *first() const {
        const typename fkhashset<ele>::ele *e = m_set.first();
        return e ? &e->k : 0;
    }

    force_inline ele *next() {
        typename fkhashset<ele>::ele *e = m_set.next();
        return e ? &e->k : 0;
    }

    force_inline const ele *next() const {
        const typename fkhashset<ele>::ele *e = m_set.next();
        return e ? &e->k : 0;
    }

    force_inline const ele *at(int pos) const {
        const typename fkhashset<ele>::ele *e = m_set.at(pos);
        return e ? &e->k : 0;
    }

    ele *add(const K &k, const T &t) {
        ele tmp;
        tmp.k = k;
        typename fkhashset<ele>::ele *e = m_set.get(tmp);
        if (LIKE(!e)) {
            tmp.t = (T *) safe_fkmalloc(m_set.m_fk, sizeof(T), emt_hashmap);
            *(tmp.t) = t;
            return &(m_set.add(tmp)->k);
        } else {
            *(e->k.t) = t;
            return &(e->k);
        }
    }

    force_inline bool del(const K &k) {
        ele tmp;
        tmp.k = k;
        typename fkhashset<ele>::ele *e = m_set.del(tmp);
        if (LIKE(e != 0)) {
            safe_fkfree(m_set.m_fk, e->k.t);
            return true;
        }
        return false;
    }

    force_inline T *get(const K &k) const {
        ele tmp;
        tmp.k = k;
        typename fkhashset<ele>::ele *e = m_set.get(tmp);
        if (UNLIKE(e == 0)) {
            return 0;
        }
        return e->k.t;
    }

    force_inline void get_conflict(int *buff, int buffsize) const {
        m_set.get_conflict(buff, buffsize);
    }

    force_inline void get_conflict_map(int *buff, int &buffsize) const {
        m_set.get_conflict_map(buff, buffsize);
    }

    force_inline void dump(char *buff, int buffsize) const {
        m_set.dump(buff, buffsize);
    }

public:
    fkhashset<ele> m_set;
};

