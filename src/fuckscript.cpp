#include "fuckscript.h"
#include "semantic.h"
#include "fuck.h"
#include "compiler.h"
#include "routine.h"
#include "processor.h"

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

const char * fkerrorstr(fuck * fk)
{
    return fk->errorstr();
}

// 解析文件
binary * fkparse(fuck * fk, const char * filename)
{
    // 输入源文件
    myflexer mf(fk);
    
    FKLOG("fkparse %p %s", fk, filename);
    bool b = mf.inputfile(filename);
    if (!b)
    {
        FKLOG("fkparse open %s fail", fk, filename);
        return 0;
    }

    // 进行语法解析
    int ret = yyparse((void *)&mf); 
    if (ret != 0)
    {
        FKLOG("fkparse yyparse %s fail ret %d", fk, filename, ret);
        fk->seterror(efk_parse_file_fail, "parse %s file fail", filename);
        return 0;
    }
    
    FKLOG("fkparse yyparse %p %s OK", fk, filename);

    binary * bin = fknew<binary>(fk, fk);

    // 编译
    compiler mc(fk, bin);
    b = mc.compile(&mf);
    if (!b)
    {
        FKLOG("fkparse compile %s fail", fk, filename);
        return 0;
    }
    
    FKLOG("fkparse %p %s OK", fk, filename);
    
    return bin;
}

// 调用函数
bool fkrun(fuck * fk, binary * bin, const char * func)
{
    FKLOG("fkrun %p %p %s", fk, bin, func);

    routine r(fk, bin, func);
    processor p(fk);

    p.run(&r);
    
    FKLOG("fkrun %p %p %s OK", fk, bin, func);
    
    return true;
}

