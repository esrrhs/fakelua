#pragma once

#include "types.h"

struct func_hash_ele
{
    int pos;
    const String * name;
};

struct func_ele
{
    func_ele(): isok(false), pos(-1) {}
    bool isok;
    int pos;
    String name;
};

typedef std::vector<func_ele> func_ele_list;

class funcmap
{
public:
    force_inline funcmap(fuck * fk) : m_fk(fk), m_func_index_map(0), m_func_index_map_size(0)
    {
    }
    force_inline ~funcmap()
    {
        safe_fkfree(m_fk, m_func_index_map);
    }

    force_inline fuck * getfuck()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        m_func_ele_list.clear();
        safe_fkfree(m_fk, m_func_index_map);
        m_func_index_map = 0;
        m_func_index_map_size = 0;
    }

    force_inline bool checkok()
    {
        for (int i = 0; i < (int)m_func_ele_list.size(); i++)
        {
            if (!m_func_ele_list[i].isok)
            {
                seterror(m_fk, efk_compile_func_not_define, "compile func %s not define", m_func_ele_list[i].name.c_str());
                return false;
            }
        }
        return true;
    }
    
    force_inline void build_map()
    {
        m_func_index_map_size = m_func_ele_list.size() * 2;
        m_func_index_map = (func_hash_ele*)safe_fkmalloc(m_fk, sizeof(func_hash_ele) * m_func_index_map_size);
        memset(m_func_index_map, 0, sizeof(func_hash_ele) * m_func_index_map_size);
        for (int i = 0; i < (int)m_func_ele_list.size(); i++)
        {
            int pos = m_func_ele_list[i].pos;
            const String & name = m_func_ele_list[i].name;
            
            uint32_t hash = fkstrhash(name.c_str());
            int index = hash % m_func_index_map_size;
            for (int j = 0; j < (int)m_func_index_map_size; j++)
            {
                if (!m_func_index_map[index].name)
                {
                    m_func_index_map[index].pos = pos;
                    m_func_index_map[index].name = &name;
                    break;
                }
                index = (index + 1) % m_func_index_map_size;
            }
        }
    }

    force_inline bool is_have_func(const char * name) const
    {
        for (int i = 0; i < (int)m_func_ele_list.size(); i++)
        {
            if (m_func_ele_list[i].name == name)
            {
                return true;
            }
        }
        return false;
    }
    force_inline int reg_func(const String & name)
    {
        if (is_have_func(name.c_str()))
        {
            return -1;
        }
        
        func_ele e;
        e.pos = m_func_ele_list.size();
        e.name = name;
        m_func_ele_list.push_back(e);

        return e.pos;
    }

    force_inline bool reg_func_ok(const String & name)
    {
        for (int i = 0; i < (int)m_func_ele_list.size(); i++)
        {
            if (m_func_ele_list[i].name == name)
            {
                m_func_ele_list[i].isok = true;
                return true;
            }
        }
        return false;
    }
    
    force_inline int get_func(const char * name) const
    {
        uint32_t hash = fkstrhash(name);
        int index = hash % m_func_index_map_size;
        for (int j = 0; j < (int)m_func_index_map_size; j++)
        {
            const func_ele & e = m_func_ele_list[m_func_index_map[index].pos];
            const String & srcname = e.name;
            if (&srcname == m_func_index_map[index].name && strcmp(srcname.c_str(), name) == 0)
            {
                // find it
                return e.pos;
            }
            index = (index + 1) % m_func_index_map_size;
        }
        return -1;
    }

private:
    fuck * m_fk;    
    func_ele_list m_func_ele_list;
    func_hash_ele * m_func_index_map;
    size_t m_func_index_map_size;
};

