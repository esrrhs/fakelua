#include "fuckscript.h"
#include "semantic.h"
#include "fuck.h"

fuck * newfuck(fkmalloc fkm, fkfree fkf)
{
    if (!fkm || !fkf)
    {
        fkm = &malloc;
        fkf = &free;
        FKLOG("newfuck use system malloc and free");
    }
    fuck * ret = (fuck *)fkm(sizeof(fuck));
    new (ret) fuck();
    ret->m_fkmalloc = fkm;
    ret->m_fkfree = fkf;
    FKLOG("newfuck ret %p", ret);
    return ret;
}

void delfuck(fuck * fk)
{
    FKLOG("delfuck %p", fk);
    fk->~fuck();
    fk->m_fkfree(fk);
}

efkerror fkerror(fuck * fk)
{
    return fk->error();
}

// 解析文件
bool fkparse(fuck * fk, const char * filename)
{
    FKLOG("fkparse %p %s", fk, filename);
    bool b = fk->m_myflexer.inputfile(filename);
    if (!b)
    {
        FKLOG("fkparse open %s fail", fk, filename);
        return false;
    }

    //进行语法解析
    int ret = yyparse((void *)&fk->m_myflexer); 
    if (ret != 0)
    {
        FKLOG("fkparse yyparse %s fail ret %d", fk, filename, ret);
        return false;
    }
    
    FKLOG("fkparse %p %s OK", fk, filename);
    
    return true;
}

