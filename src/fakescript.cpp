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
	return fk->pa.parse(filename);
}

FAKE_API bool fkparsestr(fake * fk, const char * str)
{
	return fk->pa.parsestr(str);
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
	variant funcv;
	V_SET_STRING(&funcv, func);
	return fk->fm.get_func(funcv) != 0;
}

// 调用函数
FAKE_API void fkrunps(fake * fk, const char * func)
{
	FKLOG("fkrun %p %s", fk, func);

	// 预处理，只在完全的退出脚本才执行
	if (!fk->rn.rundeps)
	{
		fk->sh.checkgc();
		fk->con.clear();
		fk->bif.clear();
	}
	fk->rn.rundeps++;

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
	assert(ARRAY_EMPTY(n->t.m_pl.l));
	assert(n->t.m_routine_num == 0);
	processor & pro = n->t;
	PROCESS_CLEAR(pro);
	variant funcv;
	V_SET_STRING(&funcv, func);
	routine * r = pro.start_routine(funcv, 0, 0);

	pro.run();
	
	variant * ret = 0;
	bool err = false;
	PS_PUSH_AND_GET(fk->ps, ret);
	*ret = ROUTINE_GETRET(*r);

	POOL_PUSH(fk->pp, n);
	
	fk->rn.rundeps--;
	
	FKLOG("fkrun %p %s OK", fk, func);
}

FAKE_API void fkpspushpointer(fake * fk, void * p)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_POINTER(v, p);
}

FAKE_API void fkpspushchar(fake * fk, char ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushuchar(fake * fk, unsigned char ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushshort(fake * fk, short ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushushort(fake * fk, unsigned short ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushint(fake * fk, int ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushuint(fake * fk, unsigned int ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushfloat(fake * fk, float ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushdouble(fake * fk, double ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushcharptr(fake * fk, char * ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_STRING(v, ret);
}

FAKE_API void fkpspushccharptr(fake * fk, const char * ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_STRING(v, ret);
}

FAKE_API void fkpspushbool(fake * fk, bool ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_REAL(v, ret);
}

FAKE_API void fkpspushint64(fake * fk, int64_t ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_UUID(v, ret);
}

FAKE_API void fkpspushuint64(fake * fk, uint64_t ret)
{
	bool err = false;
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_UUID(v, ret);
}

FAKE_API void fkpspoppointer(fake * fk, void * & p)
{
	bool err = false;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_POINTER(v, p);
}

FAKE_API char fkpspopchar(fake * fk)
{
	bool err = false;
	char ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API unsigned char fkpspopuchar(fake * fk)
{
	bool err = false;
	char ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API short fkpspopshort(fake * fk)
{
	bool err = false;
	short ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API unsigned short fkpspopushort(fake * fk)
{
	bool err = false;
	unsigned short ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API int fkpspopint(fake * fk)
{
	bool err = false;
	int ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API unsigned int fkpspopuint(fake * fk)
{
	bool err = false;
	unsigned int ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API float fkpspopfloat(fake * fk)
{
	bool err = false;
	float ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API double fkpspopdouble(fake * fk)
{
	bool err = false;
	double ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API const char * fkpspopcstrptr(fake * fk)
{
	bool err = false;
	const char * ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_STRING(v, ret);
	return ret;
}

FAKE_API bool fkpspopbool(fake * fk)
{
	bool err = false;
	bool ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_REAL(v, ret);
	return ret;
}

FAKE_API int64_t fkpspopint64(fake * fk)
{
	bool err = false;
	int64_t ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_UUID(v, ret);
	return ret;
}

FAKE_API uint64_t fkpspopuint64(fake * fk)
{
	bool err = false;
	uint64_t ret;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	V_GET_UUID(v, ret);
	return ret;
}

FAKE_API void fkpsclear(fake * fk)
{
	PS_CLEAR(fk->ps);
}

FAKE_API void fkrunpsjit(fake * fk, const char * func)
{
	FKLOG("fkrun %p %s", fk, func);

	// 预处理，只在完全的退出脚本才执行
	if (!fk->rn.rundeps)
	{
		fk->sh.checkgc();
		fk->con.clear();
		fk->bif.clear();
	}
	fk->rn.rundeps++;

	fk->clearerr();
	fk->mac.clear();
	variant funcv;
	V_SET_STRING(&funcv, func);
	fk->mac.call(funcv);

	fk->rn.rundeps--;
	
	FKLOG("fkrun %p %s OK", fk, func);
}

FAKE_API void fkpushfunctor(fake * fk, const char * name, fkfunctor ff)
{
	FKLOG("fkpushfunctor %p %s", fk, name);
	fk->bf.addfunc(fk->sh.allocsysstr(name), ff);
}

FAKE_API void fkopenalllib(fake * fk)
{
	fkopenbaselib(fk);
	fkopenfilelib(fk);
	fkopennetlib(fk);
	fkopenoslib(fk);
	fkopenstringlib(fk);
	fkopenmathlib(fk);
}

FAKE_API void fkopenbaselib(fake * fk)
{
	fk->bif.openbasefunc();
}

FAKE_API void fkopenfilelib(fake * fk)
{
	fk->bif.openfilefunc();
}

FAKE_API void fkopennetlib(fake * fk)
{
	fk->bif.opennetfunc();
}

FAKE_API void fkopenoslib(fake * fk)
{
	fk->bif.openosfunc();
}

FAKE_API void fkopenstringlib(fake * fk)
{
	fk->bif.openstringfunc();
}

FAKE_API void fkopenmathlib(fake * fk)
{
	fk->bif.openmathfunc();
}

FAKE_API void fkopenjit(fake * fk)
{
	fk->as.open();
}

FAKE_API void fkclosejit(fake * fk)
{
	fk->as.close();
}

FAKE_API void fkopenprofile(fake * fk)
{
	fk->pf.open();
}

FAKE_API void fkcloseprofile(fake * fk)
{
	fk->pf.close();
}

FAKE_API const char * fkdumpprofile(fake * fk)
{
	return fk->pf.dump();
}

FAKE_API void fkseterrorfunc(fake * fk, fkerrorcb cb)
{
	fk->errorcb = cb;
}

FAKE_API const char * fkgetcurfunc(fake * fk)
{
	if (fk->rn.curroutine)
	{
		return fk->rn.curroutine->m_interpreter.get_running_func_name();
	}
	return "nil";
}

FAKE_API const char * fkgetcurfile(fake * fk)
{
	if (fk->rn.curroutine)
	{
		return fk->rn.curroutine->m_interpreter.get_running_file_name();
	}
	return "nil";
}

FAKE_API int fkgetcurline(fake * fk)
{
	if (fk->rn.curroutine)
	{
		return fk->rn.curroutine->m_interpreter.get_running_file_line();
	}
	return 0;
}

FAKE_API const char * fkgetcurcallstack(fake * fk)
{
	if (fk->rn.curroutine)
	{
		return fk->rn.curroutine->m_interpreter.get_running_call_stack();
	}
	return "nil";
}

FAKE_API void fksetargv(fake * fk, int argc, const char *argv[])
{
	fk->bif.setargv(argc, argv);
}

FAKE_API const char * fkdumpallfunc(fake * fk)
{
	return fk->bin.dump().c_str();
}

FAKE_API const char * fkdumpfunc(fake * fk, const char * func)
{
	return fk->bin.dump(func).c_str();
}

FAKE_API const char * fkdumpfuncmap(fake * fk)
{
	return fk->fm.dump().c_str();
}

