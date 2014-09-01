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

    for (int i = 0; i < (int)blocknode->stmtlist.size(); i++)
    {
        syntree_node * stmt = blocknode->stmtlist[i];
        compile_node(cg, stmt, stack_level);
    }
    
    FKLOG("[compiler] compile_block block_node %p OK", blocknode);
    
    return true;
}

bool compiler::compile_node(codegen & cg, syntree_node * node, int stack_level)
{
    FKLOG("[compiler] compile_node %p %s", node, node->gettypename());

    esyntreetype type = node->gettype();
    switch (type)
    {
    case est_block:
        {
            block_node * bn = dynamic_cast<block_node *>(node);
            if (!compile_block(cg, bn, stack_level + 1))
            {
                FKLOG("[compiler] compile_node block_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_while_stmt:
        {
            while_stmt * ws = dynamic_cast<while_stmt *>(node);
            if (!compile_while_stmt(cg, ws, stack_level))
            {
                FKLOG("[compiler] compile_node while_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_cmp_stmt:
        {
            cmp_stmt * cs = dynamic_cast<cmp_stmt *>(node);
            if (!compile_cmp_stmt(cg, cs, stack_level))
            {
                FKLOG("[compiler] compile_node cmp_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_if_stmt:
        {
            if_stmt * is = dynamic_cast<if_stmt *>(node);
            if (!compile_if_stmt(cg, is, stack_level))
            {
                FKLOG("[compiler] compile_node compile_if_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_explicit_value:
        {
            explicit_value_node * ev = dynamic_cast<explicit_value_node *>(node);
            if (!compile_explicit_value(cg, ev, stack_level))
            {
                FKLOG("[compiler] compile_node compile_if_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_return_stmt:
        {
            return_stmt * rs = dynamic_cast<return_stmt *>(node);
            if (!compile_return_stmt(cg, rs, stack_level))
            {
                FKLOG("[compiler] compile_node compile_return_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_assign_stmt:
        {
            assign_stmt * as = dynamic_cast<assign_stmt *>(node);
            if (!compile_assign_stmt(cg, as, stack_level))
            {
                FKLOG("[compiler] compile_node compile_assign_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_variable:
        {
            variable_node * vn = dynamic_cast<variable_node *>(node);
            if (!compile_variable_node(cg, vn, stack_level))
            {
                FKLOG("[compiler] compile_node compile_variable_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_var:
        {
            var_node * vn = dynamic_cast<var_node *>(node);
            if (!compile_var_node(cg, vn, stack_level))
            {
                FKLOG("[compiler] compile_node compile_var_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_function_call:
        {
            function_call_node * fn = dynamic_cast<function_call_node *>(node);
            if (!compile_function_call_node(cg, fn, stack_level))
            {
                FKLOG("[compiler] compile_node function_call_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_break:
        {
            break_stmt * bs = dynamic_cast<break_stmt *>(node);
            if (!compile_break_stmt(cg, bs, stack_level))
            {
                FKLOG("[compiler] compile_node compile_break_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_math_expr:
        {
            math_expr_node * mn = dynamic_cast<math_expr_node *>(node);
            if (!compile_math_expr_node(cg, mn, stack_level))
            {
                FKLOG("[compiler] compile_node math_expr_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_identifier:
        {
            identifier_node * in = dynamic_cast<identifier_node *>(node);
            if (!compile_identifier_node(cg, in, stack_level))
            {
                FKLOG("[compiler] compile_node compile_identifier_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    default:
        {
            FKLOG("[compiler] compile_node type error %d %s", type, node->gettypename());
            m_fk->m_efkerror = efk_compile_stmt_type_error;
            return false;
        }
        break;
    }

    FKLOG("[compiler] compile_node %p %s OK", node, node->gettypename());
    
    return true;
}

bool compiler::compile_while_stmt(codegen & cg, while_stmt * ws, int stack_level)
{
    FKLOG("[compiler] compile_while_stmt %p", ws);

    FKLOG("[compiler] compile_while_stmt %p OK", ws);
    
    return true;
}

bool compiler::compile_if_stmt(codegen & cg, if_stmt * is, int stack_level)
{
    FKLOG("[compiler] compile_if_stmt %p", is);

    FKLOG("[compiler] compile_if_stmt %p OK", is);
    
    return true;
}

bool compiler::compile_return_stmt(codegen & cg, return_stmt * rs, int stack_level)
{
    FKLOG("[compiler] compile_return_stmt %p", rs);

    FKLOG("[compiler] compile_return_stmt %p OK", rs);
    
    return true;
}

bool compiler::compile_assign_stmt(codegen & cg, assign_stmt * as, int stack_level)
{
    FKLOG("[compiler] compile_assign_stmt %p", as);

    FKLOG("[compiler] compile_assign_stmt %p OK", as);
    
    return true;
}

bool compiler::compile_break_stmt(codegen & cg, break_stmt * bs, int stack_level)
{
    FKLOG("[compiler] compile_break_stmt %p", bs);

    FKLOG("[compiler] compile_break_stmt %p OK", bs);
    
    return true;
}

bool compiler::compile_cmp_stmt(codegen & cg, cmp_stmt * cs, int stack_level)
{
    FKLOG("[compiler] compile_cmp_stmt %p", cs);

    FKLOG("[compiler] compile_cmp_stmt %p OK", cs);
    
    return true;
}

bool compiler::compile_explicit_value(codegen & cg, explicit_value_node * ev, int stack_level)
{
    FKLOG("[compiler] compile_explicit_value %p", ev);

    FKLOG("[compiler] compile_explicit_value %p OK", ev);
    
    return true;
}

bool compiler::compile_variable_node(codegen & cg, variable_node * vn, int stack_level)
{
    FKLOG("[compiler] compile_variable_node %p", vn);

    FKLOG("[compiler] compile_variable_node %p OK", vn);
    
    return true;
}

bool compiler::compile_var_node(codegen & cg, var_node * vn, int stack_level)
{
    FKLOG("[compiler] compile_var_node %p", vn);

    FKLOG("[compiler] compile_var_node %p OK", vn);
    
    return true;
}

bool compiler::compile_function_call_node(codegen & cg, function_call_node * fn, int stack_level)
{
    FKLOG("[compiler] compile_function_call_node %p", fn);

    FKLOG("[compiler] compile_function_call_node %p OK", fn);
    
    return true;
}

bool compiler::compile_math_expr_node(codegen & cg, math_expr_node * mn, int stack_level)
{
    FKLOG("[compiler] compile_math_expr_node %p", mn);

    FKLOG("[compiler] compile_math_expr_node %p OK", mn);
    
    return true;
}

bool compiler::compile_identifier_node(codegen & cg, identifier_node * in, int stack_level)
{
    FKLOG("[compiler] compile_identifier_node %p", in);

    FKLOG("[compiler] compile_identifier_node %p OK", in);
    
    return true;
}


