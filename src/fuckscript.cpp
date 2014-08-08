#include "fuckscript.h"
#include "semantic.h"
#include "fuck.h"

fuck * newfuck()
{
    return new fuck;
}

void delfuck(fuck * fk)
{
    delete fk;
}

efkerror fkerror(fuck * fk)
{
    return fk->error();
}

// 解析文件
bool fkparse(fuck * fk, const char * filename)
{
    bool b = fk->m_myflexer.inputfile(filename);
    if (!b)
    {
        return false;
    }

    //进行语法解析
    yyparse((void *)&fk->m_myflexer); 
    
    return true;
}

