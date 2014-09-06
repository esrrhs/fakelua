#pragma once

#include "types.h"
#include "variant.h"

struct fuck;
struct fkerrorinfo;
struct binary;
struct paramstack;
struct func_binary;

typedef std::vector<variant> stack_variant_list;

struct stack
{
    stack() : m_fk(0), m_ei(0), m_fb(0), m_cmdpos(0)
    {
    }
    stack(fuck * fk, fkerrorinfo * ei, const  func_binary * fb) : m_fk(fk), m_ei(ei), m_fb(fb), m_cmdpos(0)
    {
    }
    void set_stack_variant(const variant & v, int pos)
    {
        m_stack_variant_list.resize(pos + 1, variant(m_fk));
        m_stack_variant_list[pos] = v;
    }
    
    fuck * m_fk;
    fkerrorinfo * m_ei;
    // 函数二进制
    const func_binary * m_fb;
    // 当前执行位置
    int m_cmdpos;
    // 当前栈上的变量
    stack_variant_list m_stack_variant_list;
};

typedef std::vector<stack> stack_list;

class interpreter
{
public:
    interpreter(fuck * fk, fkerrorinfo * ei) : m_fk(fk), m_ei(ei)
    {
        clear();
    }
    ~interpreter()
    {
    }

    void clear();
    
    bool isend() const;
    
    void call(binary * bin, const String & func, paramstack * ps);
    const variant & getret() const;

    int run(int cmdnum);
    
private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    bool m_isend;
    variant m_ret;
    stack_list m_stack_list;
};

