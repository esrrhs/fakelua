#include "binary.h"
#include "fuck.h"

void func_binary::grow()
{
    int * oldbuff = m_buff;
    size_t newsize = m_size * 2;
    m_buff = (int*)m_fk->m_fkmalloc(newsize * sizeof(int));
    memcpy(m_buff, oldbuff, m_size * sizeof(int));
    if (oldbuff)
    {
        m_fk->m_fkfree(m_buff);
    }
    m_size = newsize;
}

void binary::clear()
{
    m_func_index_map.clear();
    m_func_list.clear();
}

bool binary::is_have_func(const String & name)
{
    return m_func_index_map.find(name) != m_func_index_map.end();
}

bool binary::add_func(const String & name, func_binary bin)
{
    if (is_have_func(name))
    {
        return false;
    }
    
    int index = m_func_list.size();
    m_func_list.push_back(bin);
    m_func_index_map[name] = index;

    return true;
}

