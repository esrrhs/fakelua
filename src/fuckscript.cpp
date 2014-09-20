#include "fuckscript.h"
#include "semantic.h"
#include "fuck.h"
#include "compiler.h"
#include "routine.h"
#include "processor.h"
#include "paramstack.h"

FUCK_API fuck * newfuck(fuckconfig * cfg)
{
    fuckconfig _cfg;
    if (!cfg)
    {
        _cfg.fkm = &malloc;
        _cfg.fkf = &free;
        _cfg.per_frame_cmd_num = 100;
        _cfg.delete_routine_scale = 4;
		_cfg.routine_grow_speed = 100;
		_cfg.stack_list_grow_speed = 100;
    }
    else
    {
        _cfg = *cfg;
    }
    
    fuck * ret = (fuck *)_cfg.fkm(sizeof(fuck));
    new (ret) fuck();
    ret->cfg = _cfg;
    cheat_complie();
    FKLOG("newfuck ret %p", ret);
    return ret;
}

FUCK_API void delfuck(fuck * fk)
{
    FKLOG("delfuck %p", fk);
    fk->~fuck();
    safe_fkfree(fk, fk);
}

// 解析文件
FUCK_API bool fkparse(fuck * fk, const char * filename)
{
    // 清空
    fk->clear();

    // 输入源文件
    myflexer & mf = fk->mf;
    
    FKLOG("fkparse %p %s", fk, filename);
    bool b = mf.inputfile(filename);
    if (!b)
    {
        FKERR("fkparse open %s fail", fk, filename);
        return false;
    }

    // 进行语法解析
    int ret = yyparse((void *)&mf); 
    if (ret != 0)
    {
        FKERR("fkparse yyparse %s fail ret %d", fk, filename, ret);
        seterror(fk, efk_parse_file_fail, "parse %s file fail", filename);
        return false;
    }
    
    FKLOG("fkparse yyparse %p %s OK", fk, filename);

    // 编译
    compiler & mc = fk->mc;
    b = mc.compile(&mf);
    if (!b)
    {
        FKERR("fkparse %s compile %s fail", fk, filename);
        return false;
    }

    // jit
    assembler & as = fk->as;
    b = as.compile(&fk->bin);
    if (!b)
    {
        FKERR("fkparse %s jit %s fail", fk, filename);
        return false;
    }
    
    FKLOG("fkparse %p %s OK", fk, filename);
    
    return true;
}

FUCK_API efkerror fkerror(fuck * fk)
{
    return (efkerror)fk->errorno;
}


FUCK_API const char * fkerrorstr(fuck * fk)
{
    return fk->errorstr;
}


FUCK_API bool fkisfunc(fuck * fk, const char * func)
{
    return fk->bin.is_have_func(func);
}

// 调用函数
FUCK_API void fkrun(fuck * fk, const char * func)
{
    FKLOG("fkrun %p %s", fk, func);

    fk->clearerr();
    fk->inter.clear();
    fk->inter.call(&fk->bin, func, &fk->ps);

    while (!fk->inter.isend())
    {
        fk->inter.run(0x7FFFFFFF);
    }
    
    variant * ret = 0;
    PS_PUSH_AND_GET(fk->ps, ret);
    *ret = fk->inter.getret();
    
    FKLOG("fkrun %p %s OK", fk, func);
}

FUCK_API void fkpspushpointer(fuck * fk, void * p)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_POINTER(v, p);
}

FUCK_API void fkpspushchar(fuck * fk, char ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushuchar(fuck * fk, unsigned char ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushshort(fuck * fk, short ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushushort(fuck * fk, unsigned short ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushint(fuck * fk, int ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushuint(fuck * fk, unsigned int ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushfloat(fuck * fk, float ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushdouble(fuck * fk, double ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushcharptr(fuck * fk, char * ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_STRING(v, ret);
}

FUCK_API void fkpspushccharptr(fuck * fk, const char * ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_STRING(v, ret);
}

FUCK_API void fkpspushbool(fuck * fk, bool ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushint64(fuck * fk, int64_t ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspushuint64(fuck * fk, uint64_t ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FUCK_API void fkpspoppointer(fuck * fk, void * & p)
{
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_POINTER(v, p);
}

FUCK_API char fkpspopchar(fuck * fk)
{
    char ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API unsigned char fkpspopuchar(fuck * fk)
{
    char ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API short fkpspopshort(fuck * fk)
{
    char ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API unsigned short fkpspopushort(fuck * fk)
{
    unsigned char ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API int fkpspopint(fuck * fk)
{
    int ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API unsigned int fkpspopuint(fuck * fk)
{
    unsigned int ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API float fkpspopfloat(fuck * fk)
{
    float ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API double fkpspopdouble(fuck * fk)
{
    double ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API const char * fkpspopcstrptr(fuck * fk)
{
    const char * ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_STRING(v, ret);
    return ret;
}

FUCK_API bool fkpspopbool(fuck * fk)
{
    bool ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API int64_t fkpspopint64(fuck * fk)
{
    int64_t ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API uint64_t fkpspopuint64(fuck * fk)
{
    uint64_t ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FUCK_API void fkpsclear(fuck * fk)
{
    fk->ps.clear();
}
