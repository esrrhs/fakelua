#pragma once

#include "types.h"
#include "variant.h"

const uint32_t c_hashsize[] = {
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

#define HASHMAP_INI(hm, fk) (hm).m_fk = fk
#define HASHMAP_CLEAR(hm) (hm).clear()
#define HASHMAP_DELETE(hm) (hm).~fkhashmap()
#define HASHMAP_RECUR(hm) ((hm).m_recurflag != 0)
#define HASHMAP_ENTER(hm) ((hm).m_recurflag++)
#define HASHMAP_LEAVE(hm) ((hm).m_recurflag--)

template <typename T>
force_inline uint32_t fkkeyhash(const T & k)
{
	return k;
}

template <typename T>
force_inline bool fkkeycmp(const T & lk, const T & rk)
{
	return lk == rk;
}

template <typename T>
force_inline T fkkeycpy(fake * fk, const T & k)
{
	return k;
}

template <typename T>
force_inline void fkkeydel(fake * fk, const T & k)
{
}

template <typename T>
force_inline String fkkeytostr(const T & k)
{
	return "";
}

// 特化
template <>
force_inline uint32_t fkkeyhash(const char * const & k)
{
	return fkstrhash(k);
}

template <>
force_inline bool fkkeycmp(const char * const & lk, const char * const & rk)
{
	return !strcmp(lk, rk);
}

template <>
force_inline const char * fkkeycpy(fake * fk, const char * const & k)
{
	return stringdump(fk, k, strlen(k));
}

template <>
force_inline void fkkeydel(fake * fk, const char * const & k)
{
	safe_fkfree(fk, k);
}

template <>
force_inline String fkkeytostr(const char * const & k)
{
	return k;
}

// 特化
template <>
force_inline uint32_t fkkeyhash(const variant & k)
{
	return k.data.buf;
}

template <>
force_inline bool fkkeycmp(const variant & lk, const variant & rk)
{
	bool b = false;
	V_EQUAL_V(b, (&lk), (&rk));
	return b;
}

template <>
force_inline variant fkkeycpy(fake * fk, const variant & k)
{
	return k;
}

template <>
force_inline void fkkeydel(fake * fk, const variant & k)
{
}

template <>
force_inline String fkkeytostr(const variant & k)
{
	return vartostring(&k);
}


// T需要原始C结构 支持memcpy memset
template <typename K, typename T>
class fkhashmap
{
public:
	struct ele
	{
		uint32_t hv;
		K k;
		T * t;
	};
#define ELE_FAST_BUFFER (8)
#define GET_HASHELE(he, i) (LIKE((i) < ELE_FAST_BUFFER) ? (he).fe[(i)] : (he).overflow[(i) - ELE_FAST_BUFFER])
	struct hashele
	{
		ele fe[ELE_FAST_BUFFER];
		int size;
		int maxsize;
		ele * overflow;
	};
public:
	force_inline fkhashmap(fake * fk) : m_fk(fk), m_hashele(0), m_hashele_size(0), m_ele_size(0), m_grow_times(0),
		m_hashele_iter(0), m_ele_iter(0), m_recurflag(0)
	{

	}
	force_inline ~fkhashmap()
	{
		// 清掉旧的
		for (int i = 0; i < m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			for (int j = 0; j < he.maxsize; j++)
			{
				fkkeydel(m_fk, GET_HASHELE(he, j).k);
				safe_fkfree(m_fk, GET_HASHELE(he, j).t);
			}
			safe_fkfree(m_fk, he.overflow);
		}
		safe_fkfree(m_fk, m_hashele);
	}
	force_inline void clear()
	{
		// 清掉旧的
		for (int i = 0; i < m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			he.size = 0;
		}
		m_ele_size = 0;
		m_hashele_iter = 0;
		m_ele_iter = 0;
	}
	force_inline size_t size() const
	{
		return m_ele_size;
	}
	force_inline bool empty() const
	{
		return m_ele_size == 0;
	}
	force_inline ele * first()
	{
		for (m_hashele_iter = 0; m_hashele_iter < m_hashele_size; m_hashele_iter++)
		{
			hashele & he = m_hashele[m_hashele_iter];
			if (he.size > 0)
			{
				m_ele_iter = 1;
				return &GET_HASHELE(he, 0);
			}
		}
		return 0;
	}
	force_inline const ele * first() const
	{
		for (m_hashele_iter = 0; m_hashele_iter < m_hashele_size; m_hashele_iter++)
		{
			hashele & he = m_hashele[m_hashele_iter];
			if (he.size > 0)
			{
				m_ele_iter = 1;
				return &GET_HASHELE(he, 0);
			}
		}
		return 0;
	}
	force_inline ele * next()
	{
		for (; m_hashele_iter < m_hashele_size; m_hashele_iter++)
		{
			hashele & he = m_hashele[m_hashele_iter];
			if (m_ele_iter < he.size)
			{
				ele * ret = &GET_HASHELE(he, m_ele_iter);
				m_ele_iter++;
				return ret;
			}
			m_ele_iter = 0;
		}
		return 0;
	}
	force_inline const ele * next() const
	{
		for (; m_hashele_iter < m_hashele_size; m_hashele_iter++)
		{
			hashele & he = m_hashele[m_hashele_iter];
			if (m_ele_iter < he.size)
			{
				const ele * ret = &GET_HASHELE(he, m_ele_iter);
				m_ele_iter++;
				return ret;
			}
			m_ele_iter = 0;
		}
		return 0;
	}
	force_inline const ele * at(int pos) const
	{
		for (int i = 0; i < m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			if (he.size)
			{
				if (pos >= he.size)
				{
					pos -= he.size;
				}
				else
				{
					return &GET_HASHELE(he, pos);
				}
			}
		}
		return 0;
	}
	ele * add(const K & v, const T & t)
	{
		ele * e = _add(v);
		if (!e->t)
		{
			e->t = (T*)safe_fkmalloc(m_fk, sizeof(T));
		}
		*e->t = t;
		return e;
	}
	ele * add(const K & v, T * t)
	{
		ele * e = _add(v);
		e->t = t;
		return e;
	}
	force_inline bool del(const K & k)
	{
		if (!m_ele_size)
		{
			return false;
		}

		// hash
		uint32_t hv = fkkeyhash(k);
		uint32_t index = hv % m_hashele_size;
		hashele & he = m_hashele[index];
		for (int i = 0; i < he.size; i++)
		{
			// find
			ele & e = GET_HASHELE(he, i);
			if (LIKE(e.hv == hv && fkkeycmp(e.k, k)))
			{
				// swap
				ele tmp = GET_HASHELE(he, i);
				GET_HASHELE(he, i) = GET_HASHELE(he, he.size - 1);
				GET_HASHELE(he, he.size - 1) = tmp;

				he.size--;
				m_ele_size--;
				return true;
			}
		}

		return false;
	}
	force_inline T * get(const K & k) const
	{
		if (!m_ele_size)
		{
			return 0;
		}

		// hash
		uint32_t hv = fkkeyhash(k);
		uint32_t index = hv % m_hashele_size;
		hashele & he = m_hashele[index];
		for (int i = 0; i < he.size; i++)
		{
			// find
			const ele & e = GET_HASHELE(he, i);
			if (LIKE(e.hv == hv && fkkeycmp(e.k, k)))
			{
				return e.t;
			}
		}

		return 0;
	}
	force_inline void get_conflict(int * buff, int buffsize) const
	{
		memset(buff, 0, sizeof(int) * buffsize);
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			if (he.size < buffsize)
			{
				buff[he.size] += (he.size != 0 ? he.size : 1);
			}
		}
	}
	force_inline void get_conflict_map(int * buff, int & buffsize) const
	{
		buffsize = FKMIN(buffsize, m_hashele_size);
		memset(buff, 0, sizeof(int) * buffsize);
		for (int i = 0; i < (int)buffsize; i++)
		{
			hashele & he = m_hashele[i];
			buff[i] = he.size;
		}
	}
	force_inline void dump(char * buff, int buffsize) const
	{
		int off = 0;
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			for (int j = 0; j < (int)he.size; j++)
			{
				ele & e = GET_HASHELE(he, j);
				off += sprintf(buff + off, "key:%s\n", fkkeytostr(e.k).c_str());
			}
		}
	}
private:
	ele * _add(const K & k)
	{
		// 检查空间
		int newsize = m_ele_size * 2;
		if (UNLIKE(newsize >= m_hashele_size))
		{
			m_grow_times++;

			// 获取新大小
			for (int i = 0; i < (int)(sizeof(c_hashsize) / sizeof(uint32_t)); i++)
			{
				if (newsize < (int)c_hashsize[i])
				{
					newsize = c_hashsize[i];
					break;
				}
			}

			// 分配
			hashele* oldhashele = m_hashele;
			int old_hashele_size = m_hashele_size;
			int old_ele_size = m_ele_size;
			m_hashele = (hashele*)safe_fkmalloc(m_fk, sizeof(hashele) * newsize);
			m_hashele_size = newsize;
			m_ele_size = 0;
			memset(m_hashele, 0, sizeof(hashele) * newsize);

			// 加到新的 清掉旧的
			for (int i = 0; i < old_hashele_size; i++)
			{
				hashele & he = oldhashele[i];
				for (int j = 0; j < he.size; j++)
				{
					ele * e = &GET_HASHELE(he, j);
					add(e->k, e->t);
				}
				for (int j = ELE_FAST_BUFFER; j < (int)he.maxsize; j++)
				{
					fkkeydel(m_fk, GET_HASHELE(he, j).k);
					safe_fkfree(m_fk, GET_HASHELE(he, j).t);
				}
				safe_fkfree(m_fk, he.overflow);
			}
			safe_fkfree(m_fk, oldhashele);

			// 回复
			m_ele_size = old_ele_size;
		}

		// hash
		uint32_t hv = fkkeyhash(k);
		uint32_t index = hv % m_hashele_size;
		hashele & he = m_hashele[index];
		for (int i = 0; i < he.size; i++)
		{
			// find
			ele & e = GET_HASHELE(he, i);
			if (UNLIKE(e.hv == hv && fkkeycmp(e.k, k)))
			{
				return &e;
			}
		}

		// 检查空间
		if (UNLIKE(he.size >= he.maxsize))
		{
			if (UNLIKE(he.size >= ELE_FAST_BUFFER))
			{
				size_t newelesize = he.size;
				// 获取新大小
				for (int i = 0; i < (int)(sizeof(c_hashsize) / sizeof(uint32_t)); i++)
				{
					if (newelesize < c_hashsize[i])
					{
						newelesize = c_hashsize[i];
						break;
					}
				}

				// 分配
				ele * oldoverflow = he.overflow;
				he.overflow = (ele *)safe_fkmalloc(m_fk, sizeof(ele) * (newelesize - ELE_FAST_BUFFER));
				he.maxsize = newelesize;
				memcpy(he.overflow, oldoverflow, sizeof(ele) * (he.size - ELE_FAST_BUFFER));
				safe_fkfree(m_fk, oldoverflow);
			}
			else
			{
				he.maxsize = ELE_FAST_BUFFER;
			}
		}

		// add
		ele & e = GET_HASHELE(he, he.size);
		e.hv = hv;
		e.k = fkkeycpy(m_fk, k);
		he.size++;
		m_ele_size++;
		return &e;
	}
public:
	fake * m_fk;
	hashele* m_hashele;
	int m_hashele_size;
	int m_ele_size;
	int m_grow_times;
	mutable int m_hashele_iter;
	mutable int m_ele_iter;
	char m_recurflag;
};

