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

// T需要原始C结构 支持memcpy memset
template <typename T>
class stringhashmap
{
public:
/************************************************************************/
/* 
         *       *       * ele    
         |       |       |    
         *   *   *       *   * ele
         |   |   |       |   |		
hashele [1] [2] [3] [4] [5] [6]
*/
/************************************************************************/
	struct ele
	{
		char * s;
		size_t sz;
		T t;
	};
	struct hashele
	{
		ele * e;
		size_t size;
		size_t maxsize;
	};
public:
	force_inline stringhashmap(fake * fk) : m_fk(fk), m_hashele(0), m_hashele_size(0), m_ele_size(0), m_grow_times(0),
		m_hashele_iter(0), m_ele_iter(0)
	{

	}
	force_inline ~stringhashmap()
	{
		// 清掉旧的
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			for (int j = 0; j < (int)he.size; j++)
			{
				safe_fkfree(m_fk, he.e[j].s);
			}
			safe_fkfree(m_fk, he.e);
		}
		safe_fkfree(m_fk, m_hashele);
	}
	force_inline void clear()
	{
		// 清掉旧的
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			for (int j = 0; j < (int)he.size; j++)
			{
				safe_fkfree(m_fk, he.e[j].s);
			}
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
				return &he.e[0];
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
				return &he.e[0];
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
				return &he.e[m_ele_iter++];
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
				return &he.e[m_ele_iter++];
			}
			m_ele_iter = 0;
		}
		return 0;
	}
	// 递归了 不能force_inline
	ele * add(const char * name, const T & t)
	{
		// 检查空间
		size_t newsize = m_ele_size * 2;
		if (newsize >= m_hashele_size)
		{
			m_grow_times++;

			// 获取新大小
			for (int i = 0; i < (int)(sizeof(c_hashsize) / sizeof(uint32_t)); i++)
			{
				if (newsize < c_hashsize[i])
				{
					newsize = c_hashsize[i];
					break;
				}
			}

			// 分配
			hashele* oldhashele = m_hashele;
			size_t old_hashele_size = m_hashele_size;
			size_t old_ele_size = m_ele_size;
			m_hashele = (hashele*)safe_fkmalloc(m_fk, sizeof(hashele) * newsize);
			m_hashele_size = newsize;
			m_ele_size = 0;
			memset(m_hashele, 0, sizeof(hashele) * newsize);
			
			// 加到新的 清掉旧的
			for (int i = 0; i < (int)old_hashele_size; i++)
			{
				hashele & he = oldhashele[i];
				for (int j = 0; j < (int)he.size; j++)
				{
					add(he.e[j].s, he.e[j].t);
					safe_fkfree(m_fk, he.e[j].s);
				}
				safe_fkfree(m_fk, he.e);
			}
			safe_fkfree(m_fk, oldhashele);

			// 回复
			m_ele_size = old_ele_size;
		}

		// hash
		uint32_t index = fkstrhash(name) % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
			if (strcmp(m_hashele[index].e[i].s, name) == 0)
			{
			    // 替换成最新
			    memcpy(&m_hashele[index].e[i].t, &t, sizeof(T));
				return &m_hashele[index].e[i];
			}
		}
		
		// 检查空间
		if (m_hashele[index].size >= m_hashele[index].maxsize)
		{
			size_t newelesize = m_hashele[index].size;
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
			ele * olde = m_hashele[index].e;
			m_hashele[index].e = (ele *)safe_fkmalloc(m_fk, sizeof(ele) * newelesize);
			m_hashele[index].maxsize = newelesize;
			memcpy(m_hashele[index].e, olde, sizeof(ele) * m_hashele[index].size);
		}

		// add
		ele & e = m_hashele[index].e[m_hashele[index].size];
		e.sz = strlen(name);
		e.s = stringdump(m_fk, name, e.sz);
		memcpy(&e.t, &t, sizeof(T));
		m_hashele[index].size++;
		m_ele_size++;

		return &e;
	}
	force_inline bool del(const char * name)
	{
	    if (!m_ele_size)
	    {
	        return false;
	    }
	    
		// hash
		uint32_t index = fkstrhash(name) % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
			if (strcmp(m_hashele[index].e[i].s, name) == 0)
			{
				safe_fkfree(m_fk, m_hashele[index].e[i].s);
				memcpy(&m_hashele[index].e[i], &m_hashele[index].e[m_hashele[index].size - 1], sizeof(ele));
				m_hashele[index].size--;
				m_ele_size--;
				return true;
			}
		}

		return false;
	}
	// 注意，这个指针地址不能存起来，因为会移动
	force_inline T * get(const char * name) const
	{
	    if (!m_ele_size)
	    {
	        return 0;
	    }
	    
		// hash
		uint32_t index = fkstrhash(name) % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
			if (strcmp(m_hashele[index].e[i].s, name) == 0)
			{
				return &m_hashele[index].e[i].t;
			}
		}

		return 0;
	}
	// 注意，这个指针地址不能存起来，因为会移动
	force_inline ele * get_ele(const char * name) const
	{
	    if (!m_ele_size)
	    {
	        return 0;
	    }
	    
		// hash
		uint32_t index = fkstrhash(name) % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
			if (strcmp(m_hashele[index].e[i].s, name) == 0)
			{
				return &m_hashele[index].e[i];
			}
		}

		return 0;
	}
	force_inline void get_conflict(int * buff, size_t buffsize) const
	{
		memset(buff, 0, sizeof(buff) * buffsize);
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			if (he.size < buffsize)
			{
				buff[he.size] += (he.size != 0 ? he.size : 1);
			}
		}
	}
	force_inline void dump(char * buff, size_t buffsize) const
	{
		int off = 0;
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			for (int j = 0; j < (int)he.size; j++)
			{
				off += sprintf(buff + off, "key:%s\n", he.e[j].s);
			}
		}
	}
public:
    fake * m_fk;
	hashele* m_hashele;
	size_t m_hashele_size;
	size_t m_ele_size;
	size_t m_grow_times;
	mutable size_t m_hashele_iter;
	mutable size_t m_ele_iter;
};

// T需要原始C结构 支持memcpy memset
template <typename T>
class vhashmap
{
public:
/************************************************************************/
/* 
         *       *       * ele    
         |       |       |    
         *   *   *       *   * ele
         |   |   |       |   |		
hashele [1] [2] [3] [4] [5] [6]
*/
/************************************************************************/
	struct ele
	{
		variant k;
		T t;
	};
	struct hashele
	{
		ele * e;
		size_t size;
		size_t maxsize;
	};
public:
	force_inline vhashmap(fake * fk) : m_fk(fk), m_hashele(0), m_hashele_size(0), m_ele_size(0), m_grow_times(0),
		m_hashele_iter(0), m_ele_iter(0)
	{

	}
	force_inline ~vhashmap()
	{
		// 清掉旧的
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			safe_fkfree(m_fk, he.e);
		}
		safe_fkfree(m_fk, m_hashele);
	}
	force_inline void clear()
	{
		// 清掉旧的
		for (int i = 0; i < (int)m_hashele_size; i++)
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
				return &he.e[0];
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
				return &he.e[0];
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
				return &he.e[m_ele_iter++];
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
				return &he.e[m_ele_iter++];
			}
			m_ele_iter = 0;
		}
		return 0;
	}
	// 递归了 不能force_inline
	ele * add(const variant & v, const T & t)
	{
		// 检查空间
		size_t newsize = m_ele_size * 2;
		if (newsize >= m_hashele_size)
		{
			m_grow_times++;

			// 获取新大小
			for (int i = 0; i < (int)(sizeof(c_hashsize) / sizeof(uint32_t)); i++)
			{
				if (newsize < c_hashsize[i])
				{
					newsize = c_hashsize[i];
					break;
				}
			}

			// 分配
			hashele* oldhashele = m_hashele;
			size_t old_hashele_size = m_hashele_size;
			size_t old_ele_size = m_ele_size;
			m_hashele = (hashele*)safe_fkmalloc(m_fk, sizeof(hashele) * newsize);
			m_hashele_size = newsize;
			m_ele_size = 0;
			memset(m_hashele, 0, sizeof(hashele) * newsize);
			
			// 加到新的 清掉旧的
			for (int i = 0; i < (int)old_hashele_size; i++)
			{
				hashele & he = oldhashele[i];
				for (int j = 0; j < (int)he.size; j++)
				{
					add(he.e[j].k, he.e[j].t);
				}
				safe_fkfree(m_fk, he.e);
			}
			safe_fkfree(m_fk, oldhashele);

			// 回复
			m_ele_size = old_ele_size;
		}

		// hash
		uint32_t index = v.data.buf % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
    		bool b = false;
    		V_EQUAL_V(b, (&m_hashele[index].e[i].k), (&v));
			if (b)
			{
			    // 替换成最新
			    memcpy(&m_hashele[index].e[i].t, &t, sizeof(T));
				return &m_hashele[index].e[i];
			}
		}
		
		// 检查空间
		if (m_hashele[index].size >= m_hashele[index].maxsize)
		{
			size_t newelesize = m_hashele[index].size;
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
			ele * olde = m_hashele[index].e;
			m_hashele[index].e = (ele *)safe_fkmalloc(m_fk, sizeof(ele) * newelesize);
			m_hashele[index].maxsize = newelesize;
			memcpy(m_hashele[index].e, olde, sizeof(ele) * m_hashele[index].size);
		}

		// add
		ele & e = m_hashele[index].e[m_hashele[index].size];
		e.k = v;
		memcpy(&e.t, &t, sizeof(T));
		m_hashele[index].size++;
		m_ele_size++;

		return &e;
	}
	force_inline bool del(const variant & v)
	{
	    if (!m_ele_size)
	    {
	        return false;
	    }
	    
		// hash
		uint32_t index = v.data.buf % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
    		bool b = false;
    		V_EQUAL_V(b, (&m_hashele[index].e[i].k), (&v));
			if (b)
			{
				safe_fkfree(m_fk, m_hashele[index].e[i].s);
				memcpy(&m_hashele[index].e[i], &m_hashele[index].e[m_hashele[index].size - 1], sizeof(ele));
				m_hashele[index].size--;
				m_ele_size--;
				return true;
			}
		}

		return false;
	}
	// 注意，这个指针地址不能存起来，因为会移动
	force_inline T * get(const variant & v) const
	{
	    if (!m_ele_size)
	    {
	        return 0;
	    }
	    
		// hash
		uint32_t index = v.data.buf % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
    		bool b = false;
    		V_EQUAL_V(b, (&m_hashele[index].e[i].k), (&v));
			if (b)
			{
				return &m_hashele[index].e[i].t;
			}
		}

		return 0;
	}
	// 注意，这个指针地址不能存起来，因为会移动
	force_inline ele * get_ele(const variant & v) const
	{
	    if (!m_ele_size)
	    {
	        return 0;
	    }
	    
		// hash
		uint32_t index = v.data.buf % m_hashele_size;
		for (int i = 0; i < (int)m_hashele[index].size; i++)
		{
			// find
    		bool b = false;
    		V_EQUAL_V(b, (&m_hashele[index].e[i].k), (&v));
			if (b)
			{
				return &m_hashele[index].e[i];
			}
		}

		return 0;
	}
	force_inline void get_conflict(int * buff, size_t buffsize) const
	{
		memset(buff, 0, sizeof(buff) * buffsize);
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			if (he.size < buffsize)
			{
				buff[he.size] += (he.size != 0 ? he.size : 1);
			}
		}
	}
	force_inline void dump(char * buff, size_t buffsize) const
	{
		int off = 0;
		for (int i = 0; i < (int)m_hashele_size; i++)
		{
			hashele & he = m_hashele[i];
			for (int j = 0; j < (int)he.size; j++)
			{
				off += sprintf(buff + off, "key:%s\n", vartostring(he.e[j].k).c_str());
			}
		}
	}
public:
    fake * m_fk;
	hashele* m_hashele;
	size_t m_hashele_size;
	size_t m_ele_size;
	size_t m_grow_times;
	mutable size_t m_hashele_iter;
	mutable size_t m_ele_iter;
};

#define HASHMAP_INI(hm, fk) (hm).m_fk = fk
#define HASHMAP_CLEAR(hm) (hm).clear()

