#include "compiler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"

void compiler::clear()
{
    m_binary.clear();
}

bool compiler::compile(myflexer * mf)
{
    func_desc_list & funclist = mf->get_func_list();
    FKLOG("[compiler] compile funclist %d", funclist.size());

    for (int i = 0; i < (int)funclist.size(); i++)
    {
        func_desc_node * funcnode = funclist[i];
        if (!compile_func(funcnode))
        {   
            return false;
        }
    }

    return true;
}

bool compiler::compile_func(func_desc_node * funcnode)
{
    FKLOG("[compiler] compile_func func %s", funcnode->funcname.c_str());
    
    func_binary bin(m_fk);
    codegen cg(m_fk);
    
    // 检测重名
    if (m_binary.is_have_func(funcnode->funcname))
    {
        m_fk->m_efkerror = efk_compile_same_func_name;
        return false;
    }

	int stack_level = 0;
	
    // 参数入栈
    func_desc_arglist & arglist = funcnode->arglist->arglist;
    for (int i = 0; i < (int)arglist.size(); i++)
    {
        const String & arg = arglist[i];
        if (!cg.add_stack_identifier(arg, i))
        {
            m_fk->m_efkerror = efk_compile_func_arg_error;
            return false;
        }
    }
    stack_level = arglist.size();

    // 编译函数体
    if (!compile_block(cg, funcnode->block, stack_level))
    {
        return false;
    }

    // 编译成功
    m_binary.add_func(funcnode->funcname, bin);
    
    FKLOG("[compiler] compile_func func %s OK", funcnode->funcname.c_str());
    
    return true;
}

bool compiler::compile_block(codegen & cg, block_node * blocknode, int stack_level)
{
    FKLOG("[compiler] compile_block block_node %p", blocknode);

    
    
    FKLOG("[compiler] compile_block block_node %p OK", blocknode);
    
    return true;
}

