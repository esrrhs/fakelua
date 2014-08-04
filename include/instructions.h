#pragma once

#include "types.h"

namespace fscript
{

enum einstruction_type
{
    eit_instruction,
    eit_expression,
    eit_operator,
    eit_block,
};

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
    void printtab(int indent)
    {
        for (int i = 0; i < indent; i++)
        {
            printf("\t");
        }
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
        printf("operat");
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
        operators.push_back(op);
    }
    block(operat* op1, operat* op2) 
    {
        operators.push_back(op1);
        operators.push_back(op2);
    }
    virtual ~block() 
    {
        for(OperatorsList::iterator i = operators.begin(); i != operators.end(); i++)
            delete (*i);
        operators.clear();
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
        printf("block begin");
        for(OperatorsList::iterator i = operators.begin(); i != operators.end(); i++)
        {
            (*i)->print(indent);
        }
        printtab(indent);
        printf("block end");
    }
    
    int size() 
    {
        return operators.size();
    }

    OperatorsList * get_operators() { return &operators; }
    
private:
    OperatorsList operators;
};

}

