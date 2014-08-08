#pragma once

#include "types.h"

enum einstruction_type
{
    eit_instruction,
    eit_expression,
    eit_operator,
    eit_block,
    eit_value,
    eit_var,
    eit_binary,
    eit_function_call,
    eit_expression_op,
    eit_function_declaration,
    eit_if_op,
    eit_while_op,
    eit_return_op,
    eit_break_op,
    eit_assign,
};

static inline void printtab(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }
}

class instruction  
{
public:
    instruction() {}
    virtual ~instruction() {}
    virtual einstruction_type gettype()
    {
        return eit_instruction;
    }
    virtual String getname()
    {
        return "instruction";
    }
    virtual void print(int indent)
    {
        printtab(indent);
        printf("instruction\n");
    }
};

//base expression class
class expression : public instruction 
{
public:
    expression() {}
    virtual ~expression() {}
    virtual einstruction_type gettype()
    {
        return eit_expression;
    }
    virtual String getname()
    {
        return "expression";
    }
    virtual void print(int indent)
    {
        printtab(indent);
        printf("expression\n");
    }
};

typedef std::list<expression*> ExpressionsList;

//base operation class
class operat : public instruction 
{
public:
    operat() {}
    virtual ~operat() {}
    virtual einstruction_type gettype()
    {
        return eit_operator;
    }
    virtual String getname()
    {
        return "operat";
    }
    virtual void print(int indent)
    {
        printtab(indent);
        printf("operat\n");
    }
};

typedef std::list<operat*> OperatorsList;

//represent logical commands block
//all source tree will be represented by this node
class block : public operat
{
public:
    block() 
    {
    }
    block(operat* op) 
    {
        m_operators.push_back(op);
    }
    block(operat* op1, operat* op2) 
    {
        m_operators.push_back(op1);
        m_operators.push_back(op2);
    }
    virtual ~block() 
    {
        for(OperatorsList::iterator i = m_operators.begin(); i != m_operators.end(); i++)
            delete (*i);
        m_operators.clear();
    }
    
    virtual einstruction_type gettype()
    {
        return eit_block;
    }
    virtual String getname()
    {
        return "block";
    }
    virtual void print(int indent)
    {
        printtab(indent);
        printf("block begin\n");
        for(OperatorsList::iterator i = m_operators.begin(); i != m_operators.end(); i++)
        {
            (*i)->print(indent + 1);
        }
        printtab(indent);
        printf("block end\n");
    }
    
    int size() 
    {
        return m_operators.size();
    }

    void append(operat* op)
    {
        m_operators.push_back(op);
    }

    OperatorsList * get_operators() { return &m_operators; }
    
private:
    OperatorsList m_operators;
};

/* Expresions */
class value : public expression 
{
public:
    value(const String _text): m_value( _text) 
    {
    }

    value() {}
    
    virtual ~value() {}

    virtual einstruction_type gettype()
    {
        return eit_value;
    }
    virtual String getname()
    {
        return "value";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("value %s\n", m_value.c_str());
    }

    String get_value()
    {
        return m_value;
    }

private:
    String m_value;
};

class var : public expression 
{
public:
    var(const String& _name) : m_name(_name) 
    {
    }
    virtual einstruction_type gettype()
    {
        return eit_var;
    }
    virtual ~var() {}
    
    virtual String getname()
    {
        return "var";
    }
    virtual void print(int indent)
    {
        printtab(indent);
        printf("var %s\n", m_name.c_str());
    }
private:
    String m_name;
};

class binary : public expression 
{
public:
    binary(const char* _op, expression *_arg1, expression *_arg2) : m_op(_op), m_arg1(_arg1), m_arg2(_arg2) 
    {
    }

    binary(const String& str, expression *_arg1, expression *_arg2) : m_op(str), m_arg1(_arg1), m_arg2(_arg2) 
    {
    }
    
    virtual ~binary() 
    {
        delete m_arg1;
        delete m_arg2;
    }

    virtual einstruction_type gettype()
    {
        return eit_binary;
    }
    virtual String getname()
    {
        return "binary";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("binary %s\n", m_op.c_str());
        m_arg1->print(indent + 1);
        m_arg2->print(indent + 1);
    }

    String get_operator() const 
    {
        return m_op;
    }

    expression* get_operand(int i = 0) 
    {
        if(i)
            return m_arg2;
        else
            return m_arg1;
    }

private:
    String m_op;
    expression * m_arg1;
    expression * m_arg2;
};

class function_call : public expression, public operat 
{
public:
    function_call(const String& name, const ExpressionsList & args) : m_name(name), m_args(args) 
    {
    }
    
    virtual ~function_call() {}

    virtual einstruction_type gettype()
    {
        return eit_function_call;
    }
    virtual String getname()
    {
        return "function_call";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("function_call %s\n", m_name.c_str());
        for(ExpressionsList::iterator i = m_args.begin(); i != m_args.end(); i++) 
        {
            (*i)->print(indent + 1);
        }
    }

    virtual ExpressionsList & get_args() 
    {
        return m_args;
    }

private:
    String m_name;
    ExpressionsList m_args;
};

//represent expression operator
// expresion ';' will be represented by this node
class expression_op : public operat 
{
public:
    expression_op(expression* expr) : m_expr(expr) 
    {
    }
    virtual ~expression_op() 
    {
        delete m_expr;
    }
    virtual einstruction_type gettype()
    {
        return eit_expression_op;
    }
    virtual String getname()
    {
        return "expression_op";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("expression_op\n");
        m_expr->print(indent + 1);
    }
    
private:
    expression* m_expr;
};

//function declaration node
class function_declaration: public operat 
{
public:
    function_declaration(const String &_name, ExpressionsList _args, operat* _impl):
        m_implementation(_impl), m_args(_args), m_name(_name) {
    }
    
    virtual ~function_declaration() {}
    
    virtual einstruction_type gettype()
    {
        return eit_function_declaration;
    }
    virtual String getname()
    {
        return "function_declaration";
    }
    virtual void print(int indent)
    {
        printtab(indent);
        printf("function_declaration %s \n", m_name.c_str());
        for(ExpressionsList::iterator i = m_args.begin(); i != m_args.end(); i++) {
            (*i)->print(indent + 1);
        }

        m_implementation.print(indent + 1);
    }
    
    block* get_instructions() { return &m_implementation; }
    
    ExpressionsList & get_args() { return m_args; }
        
private:
    block m_implementation;
    ExpressionsList m_args;
    String m_name;
};

//if statement node
class if_op : public operat 
{
public:
    if_op(expression* _cond, operat* _thenops, operat* _elseops) : m_cond(_cond), m_thenops(_thenops)
    {
    	if( _elseops != 0x0)
    	{
    	    m_elseops.append(_elseops);
    	}
    }
   
    virtual ~if_op() {
        delete m_cond;
    }
    
    virtual einstruction_type gettype()
    {
        return eit_if_op;
    }
    virtual String getname()
    {
        return "if_op";
    }
    void print(int indent) 
    {
        printtab(indent);
        printf("IF statement\n");
        m_cond->print(indent + 1);
        printtab(indent);
        printf("then\n");
        m_thenops.print(indent + 1);
        if(m_elseops.size())
        {
            printtab(indent);
            printf("else\n");
            m_elseops.print(indent+1);
        }
    }
    
    expression* get_condition() { return m_cond; }
    block* get_then_instr() { return &m_thenops; }
    block* get_else_instr() { return (m_elseops.size()) ? (&m_elseops) : (0x0); }

private:
    expression* m_cond;
    block m_thenops; 
    block m_elseops;
};

//while loop node
class while_op : public operat 
{
public:
    while_op(expression* cond, operat* ops) : m_cond(cond), m_ops(ops) 
    {
    }

    virtual ~while_op() 
    {
        delete m_cond;
    }
    
    virtual einstruction_type gettype()
    {
        return eit_while_op;
    }
    virtual String getname()
    {
        return "while_op";
    }
    void print(int indent) 
    {
        printtab(indent);
        printf("while statement\n");
	    m_cond->print(indent + 1);
        printtab(indent);
        printf("do\n");
	    m_ops.print(indent + 1);
    }

    expression* get_condition() { return m_cond; }
    block* get_block() { return &m_ops; }
    
private:
    expression* m_cond;
    block m_ops;
};

class return_op : public operat 
{
public:
    return_op(expression* value) : m_retvalue(value) 
    {
    }
    
    virtual ~return_op() 
    {
        delete m_retvalue;
    }

    virtual einstruction_type gettype()
    {
        return eit_return_op;
    }
    virtual String getname()
    {
        return "return_op";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("RETURN\n");
        m_retvalue->print(indent + 1);
    }
    
    expression* get_value() { return m_retvalue; }

private:
    expression * m_retvalue;
};

class break_op : public operat 
{
public:
    break_op() 
    {
    }
    virtual ~break_op() 
    {
    }
    
    virtual einstruction_type gettype()
    {
        return eit_break_op;
    }
    virtual String getname()
    {
        return "break_op";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("BREAK\n");
    }
    
};

/* Assign value - operator implementation */
class assign : public operat 
{
public:
    assign(expression* _var, expression* value) : m_value(value), m_var(dynamic_cast<var*>(_var))
    {
    }
    virtual ~assign() 
    {
        delete m_var;
    }

    virtual einstruction_type gettype()
    {
        return eit_assign;
    }
    virtual String getname()
    {
        return "assign";
    }
    virtual void print(int indent) 
    {
        printtab(indent);
        printf("ASSIGN\n");
        m_var->print(indent + 1); 
        printtab(indent);
        printf("=\n");
        m_value->print(indent + 1); 
    }
    
    var * get_var() { return m_var; }
    expression* get_value() { return m_value; }
        
private:
    expression* m_value;
    var * m_var;
};

