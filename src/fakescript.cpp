#include "fakescript.h"
#include "semantic.h"
#include "fake.h"
#include "compiler.h"
#include "routine.h"
#include "processor.h"
#include "paramstack.h"

FAKE_API fake * newfake(fakeconfig * cfg)
{
    fakeconfig _cfg;
    if (cfg)
    {
        _cfg = *cfg;
    }
    
    fake * ret = (fake *)_cfg.fkm(sizeof(fake));
	memset(ret, 0, sizeof(fake));
    new (ret) fake();
    ret->cfg = _cfg;
    FKLOG("newfake ret %p", ret);
    return ret;
}

FAKE_API void delfake(fake * fk)
{
    FKLOG("delfake %p", fk);
    fk->~fake();
    safe_fkfree(fk, fk);
}

// 解析文件
FAKE_API bool fkparse(fake * fk, const char * filename)
{
    // 清空错误
    fk->clearerr();

    // 输入源文件
    fk->mf.clear();
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
        seterror(fk, efk_parse_file_fail, "parse %s file fail for reason : %s", filename, mf.get_error());
        return false;
    }
    
    FKLOG("fkparse yyparse %p %s OK", fk, filename);

    // 编译
    fk->mc.clear();
    compiler & mc = fk->mc;
    b = mc.compile(&mf);
    if (!b)
    {
        FKERR("fkparse %s compile %s fail", fk, filename);
        return false;
    }

    // jit
    fk->as.clear();
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

FAKE_API efkerror fkerror(fake * fk)
{
    return (efkerror)fk->errorno;
}


FAKE_API const char * fkerrorstr(fake * fk)
{
    return fk->errorstr;
}


FAKE_API bool fkisfunc(fake * fk, const char * func)
{
    return fk->bin.get_func(func) != 0;
}

// 调用函数
FAKE_API void fkrunps(fake * fk, const char * func)
{
    FKLOG("fkrun %p %s", fk, func);

    // 预处理，只在完全的退出脚本才执行
    if (!fk->rundeps)
    {
        fk->bin.move();
        fk->sh.checkgc();
    }
    fk->rundeps++;

    // 清空运行环境    
    fk->clearerr();

    // 分配个
    pool<processor>::node * n = 0;
    if (POOL_EMPTY(fk->pp))
	{
        POOL_GROW(fk->pp, pool<processor>::node, n);
        PROCESS_INI(n->t, fk);
    }
    POOL_POP(fk->pp, n);
    assert(ARRAY_EMPTY(n->t.m_routine_list));
    assert(n->t.m_routine_num == 0);
    PROCESS_CLEAR(n->t);
    routine * r = n->t.start_routine(&fk->bin, func, &fk->ps);

    n->t.run();
    
    variant * ret = 0;
    PS_PUSH_AND_GET(fk->ps, ret);
    *ret = ROUTINE_GETRET(*r);

    POOL_PUSH(fk->pp, n);
    
    fk->rundeps--;
    
    FKLOG("fkrun %p %s OK", fk, func);
}

FAKE_API void fkpspushpointer(fake * fk, void * p)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_POINTER(v, p);
}

FAKE_API void fkpspushchar(fake * fk, char ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushuchar(fake * fk, unsigned char ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushshort(fake * fk, short ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushushort(fake * fk, unsigned short ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushint(fake * fk, int ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushuint(fake * fk, unsigned int ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushfloat(fake * fk, float ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushdouble(fake * fk, double ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushcharptr(fake * fk, char * ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_STRING(v, ret);
}

FAKE_API void fkpspushccharptr(fake * fk, const char * ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_STRING(v, ret);
}

FAKE_API void fkpspushbool(fake * fk, bool ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_REAL(v, ret);
}

FAKE_API void fkpspushint64(fake * fk, int64_t ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_UUID(v, ret);
}

FAKE_API void fkpspushuint64(fake * fk, uint64_t ret)
{
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_UUID(v, ret);
}

FAKE_API void fkpspoppointer(fake * fk, void * & p)
{
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_POINTER(v, p);
}

FAKE_API char fkpspopchar(fake * fk)
{
    char ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API unsigned char fkpspopuchar(fake * fk)
{
    char ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API short fkpspopshort(fake * fk)
{
    short ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API unsigned short fkpspopushort(fake * fk)
{
    unsigned short ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API int fkpspopint(fake * fk)
{
    int ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API unsigned int fkpspopuint(fake * fk)
{
    unsigned int ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API float fkpspopfloat(fake * fk)
{
    float ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API double fkpspopdouble(fake * fk)
{
    double ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API const char * fkpspopcstrptr(fake * fk)
{
	const char * ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_STRING(v, ret);
    return ret;
}

FAKE_API bool fkpspopbool(fake * fk)
{
    bool ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_REAL(v, ret);
    return ret;
}

FAKE_API int64_t fkpspopint64(fake * fk)
{
    int64_t ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_UUID(v, ret);
    return ret;
}

FAKE_API uint64_t fkpspopuint64(fake * fk)
{
    uint64_t ret;
    variant * v = 0;
    PS_POP_AND_GET(fk->ps, v);
    V_GET_UUID(v, ret);
    return ret;
}

FAKE_API void fkpsclear(fake * fk)
{
    fk->ps.clear();
}

FAKE_API void fkrunpsjit(fake * fk, const char * func)
{
    FKLOG("fkrun %p %s", fk, func);

    fk->clearerr();
    fk->mac.clear();
    fk->mac.call(&fk->nt, func, &fk->ps);

    variant * ret = 0;
    PS_PUSH_AND_GET(fk->ps, ret);
    *ret = fk->mac.getret();
    
    FKLOG("fkrun %p %s OK", fk, func);
}

void fkpushfunctor(fake * fk, const char * name, fkfunctor ff)
{
    FKLOG("fkpushfunctor %p %s", fk, name);

    fk->bf.addfunc(name, ff);
}

void fkopenbaselib(fake * fk)
{
    fk->bif.openbasefunc();
}

void fkopenprofile(fake * fk)
{
    fk->pf.open();
}

void fkcloseprofile(fake * fk)
{
    fk->pf.close();
}

const char * fkdumpprofile(fake * fk)
{
    return fk->pf.dump();
}

