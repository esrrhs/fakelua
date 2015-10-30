#include "fakescript.h"
#include "semantic.h"
#include "fake.h"
#include "compiler.h"
#include "routine.h"
#include "processor.h"
#include "paramstack.h"
#include "buffer.h"
#include "pool.h"

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
	fk->pa.clear();
	return fk->pa.parse(filename);
}

FAKE_API bool fkparsestr(fake * fk, const char * str)
{
	return fk->pa.parsestr(str);
}

FAKE_API void fkclear(fake * fk)
{
	fk->fm.clearfb();
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

FAKE_API const char * fkgetfuncfile(fake * fk, const char * func)
{
	variant funcv;
	V_SET_STRING(&funcv, func);
	const funcunion * f = fk->fm.get_func(funcv);
	if (f && f->havefb)
	{
		return FUNC_BINARY_FILENAME(f->fb);
	}
	return "";
}

FAKE_API int fkgetfuncstartline(fake * fk, const char * func)
{
	variant funcv;
	V_SET_STRING(&funcv, func);
	const funcunion * f = fk->fm.get_func(funcv);
	if (f && f->havefb)
	{
		return FUNC_BINARY_LINENO(f->fb, 0);
	}
	return 0;
}

FAKE_API int fkgetfuncvariantnum(fake * fk, const char * func)
{
	variant funcv;
	V_SET_STRING(&funcv, func);
	const funcunion * f = fk->fm.get_func(funcv);
	if (f && f->havefb)
	{
		return f->fb.m_debug_stack_variant_info_num;
	}
	return 0;
}

FAKE_API const char * fkgetfuncvariantname(fake * fk, const char * func, int index)
{
	variant funcv;
	V_SET_STRING(&funcv, func);
	const funcunion * f = fk->fm.get_func(funcv);
	if (f && f->havefb && index >= 0 && index < f->fb.m_debug_stack_variant_info_num)
	{
		return f->fb.m_debug_stack_variant_info[index].name;
	}
	return "";
}

FAKE_API int fkgetfuncvariantline(fake * fk, const char * func, int index)
{
	variant funcv;
	V_SET_STRING(&funcv, func);
	const funcunion * f = fk->fm.get_func(funcv);
	if (f && f->havefb && index >= 0 && index < f->fb.m_debug_stack_variant_info_num)
	{
		return f->fb.m_debug_stack_variant_info[index].line;
	}
	return 0;
}

// 调用函数
FAKE_API void fkrunps(fake * fk, const char * func)
{
	FKLOG("fkrunps %p %s", fk, func);

	// 预处理，只在完全的退出脚本才执行
	if (LIKE(!fk->rn.rundeps))
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
	if (UNLIKE(POOL_EMPTY(fk->pp)))
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

	PUSH_CUR_PROCESSOR(n, fk->rn);
	// 单独执行，下次再跑run
	if (UNLIKE(fk->rn.stepmod))
	{
		variant * ret = 0;
		bool err = false;
		PS_PUSH_AND_GET(fk->ps, ret);
		*ret = NILV;
		FKLOG("fkrunps %p %s yield", fk, func);
		return;
	}
	
	pro.run();
	POP_CUR_PROCESSOR(fk->rn);
	
	variant * ret = 0;
	bool err = false;
	PS_PUSH_AND_GET(fk->ps, ret);
	*ret = ROUTINE_GETRET(*r);

	POOL_PUSH(fk->pp, n);
	
	fk->rn.rundeps--;
	
	FKLOG("fkrunps %p %s OK", fk, func);
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
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return p->t.m_curroutine->m_interpreter.get_running_func_name();
	}
	return "nil";
}

FAKE_API const char * fkgetcurfile(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return p->t.m_curroutine->m_interpreter.get_running_file_name();
	}
	return "nil";
}

FAKE_API int fkgetcurline(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return p->t.m_curroutine->m_interpreter.get_running_file_line();
	}
	return 0;
}

FAKE_API int fkgetcurbytecodepos(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return p->t.m_curroutine->m_interpreter.get_running_bytecode_pos();
	}
	return -1;
}

FAKE_API const char * fkgetcurcode(fake * fk)
{
	const char * filename = fkgetcurfile(fk);
	int line = fkgetcurline(fk);

	return fkgetfilecode(fk, filename, line);
}

FAKE_API const char * fkgetfilecode(fake * fk, const char * filename, int line)
{
	if (!filename || line <= 0)
	{
		return 0;
	}
	
	char buff[512];
	
	FILE * file = fopen(filename, "rb");
	if (!file)
	{
		tsnprintf(buff, sizeof(buff), "open file(%s:%d) fail", filename, line);
		fk->rn.curcode = buff;
		return fk->rn.curcode.c_str();
	}

	int i = 0;
	while (!feof(file))
	{
		if (!fgets(buff, sizeof(buff), file))
		{
			return 0;
		}
		i++;
		if (i >= line)
		{
			fclose(file);
			fk->rn.curcode = buff;
			return fk->rn.curcode.c_str();
		}
	}
	
	fclose(file);
	
	return 0;
}

FAKE_API const char * fkgetcurcallstack(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return p->t.m_curroutine->m_interpreter.get_running_call_stack();
	}
	return "nil";
}

FAKE_API int fkgetcurcallstacklength(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return p->t.m_curroutine->m_interpreter.get_running_call_stack_length();
	}
	return 0;
}

FAKE_API const char * fkgetcurcallstackbyframe(fake * fk, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.m_curroutine->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return stackinfo;
	}
	return "nil";
}

FAKE_API const char * fkgetcurfuncbyframe(fake * fk, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.m_curroutine->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return func;
	}
	return "nil";
}

FAKE_API const char * fkgetcurfilebyframe(fake * fk, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.m_curroutine->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return file;
	}
	return "nil";
}

FAKE_API int fkgetcurlinebyframe(fake * fk, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.m_curroutine->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return line;
	}
	return 0;
}

FAKE_API const char * fkgetcurvaiantbyframe(fake * fk, int frame, const char * name, int line)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		const char * ret = "";
		int retline = 0;
		p->t.m_curroutine->m_interpreter.get_running_vaiant(frame, name, line, ret, retline);
		return ret;
	}
	return "";
}

FAKE_API int fkgetcurvaiantlinebyframe(fake * fk, int frame, const char * name, int line)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		const char * ret = "";
		int retline = 0;
		p->t.m_curroutine->m_interpreter.get_running_vaiant(frame, name, line, ret, retline);
		return retline;
	}
	return 0;
}

FAKE_API void fksetcurvaiantbyframe(fake * fk, int frame, const char * name, const char * value, int line)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		p->t.m_curroutine->m_interpreter.set_running_vaiant(frame, name, line, value);
	}
}

FAKE_API const char * fkgetcurroutine(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p)
	{
		return p->t.get_routine_info();
	}
	return "";
}

FAKE_API int fkgetcurroutinenum(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p)
	{
		return p->t.get_routine_num();
	}
	return 0;
}

FAKE_API const char * fkgetcurroutinebyindex(fake * fk, int index)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p)
	{
		return p->t.get_routine_info_by_index(index);
	}
	return "";
}

FAKE_API const char * fkgetcurroutinebyid(fake * fk, int rid)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p)
	{
		return p->t.get_routine_info_by_id(rid);
	}
	return "";
}

FAKE_API int fkgetroutineidbyindex(fake * fk, int index)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_index(index))
	{
		return ROUTINE_ID(*(p->t.get_routine_by_index(index)));
	}
	return 0;
}

FAKE_API bool fkishaveroutine(fake * fk, int rid)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p)
	{
		return p->t.get_routine_by_id(rid) != 0;
	}
	return false;
}

FAKE_API int fkgetcurroutineid(fake * fk)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.m_curroutine)
	{
		return ROUTINE_ID(*(p->t.m_curroutine));
	}
	return 0;
}

FAKE_API const char * fkgetcurvaiantbyroutinebyframe(fake * fk, int rid, int frame, const char * name, int line)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		const char * ret = "";
		int retline = 0;
		p->t.get_routine_by_id(rid)->m_interpreter.get_running_vaiant(frame, name, line, ret, retline);
		return ret;
	}
	return "";
}

FAKE_API int fkgetcurvaiantlinebyroutinebyframe(fake * fk, int rid, int frame, const char * name, int line)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		const char * ret = "";
		int retline = 0;
		p->t.get_routine_by_id(rid)->m_interpreter.get_running_vaiant(frame, name, line, ret, retline);
		return retline;
	}
	return 0;
}

FAKE_API void fksetcurvaiantbyroutinebyframe(fake * fk, int rid, int frame, const char * name, const char * value, int line)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		p->t.get_routine_by_id(rid)->m_interpreter.set_running_vaiant(frame, name, line, value);
	}
}

FAKE_API int fkgetcurlinebyroutinebyframe(fake * fk, int rid, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.get_routine_by_id(rid)->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return line;
	}
	return 0;
}

FAKE_API const char * fkgetcurfuncbyroutinebyframe(fake * fk, int rid, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.get_routine_by_id(rid)->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return func;
	}
	return "nil";
}

FAKE_API const char * fkgetcurfilebyroutinebyframe(fake * fk, int rid, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.get_routine_by_id(rid)->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return file;
	}
	return "nil";
}

FAKE_API int fkgetcurcallstacklengthbyroutine(fake * fk, int rid)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		return p->t.get_routine_by_id(rid)->m_interpreter.get_running_call_stack_length();
	}
	return 0;
}

FAKE_API const char * fkgetcurcallstackbyroutinebyframe(fake * fk, int rid, int frame)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		const char * stackinfo = 0;
		const char * func = 0;
		const char * file = 0;
		int line = 0;
		p->t.get_routine_by_id(rid)->m_interpreter.get_running_call_stack_frame_info(frame, stackinfo, func, file, line);
		return stackinfo;
	}
	return "nil";
}

FAKE_API int fkgetcurbytecodeposbyroutine(fake * fk, int rid)
{
	pool<processor>::node * p = 0;
	GET_CUR_PROCESSOR(p, fk->rn);
	if (p && p->t.get_routine_by_id(rid))
	{
		return p->t.get_routine_by_id(rid)->m_interpreter.get_running_bytecode_pos();
	}
	return -1;	
}

FAKE_API void fksetargv(fake * fk, int argc, const char *argv[])
{
	fk->bif.setargv(argc, argv);
}

FAKE_API const char * fkdumpallfunc(fake * fk)
{
	return fk->bin.dump().c_str();
}

FAKE_API const char * fkdumpfunc(fake * fk, const char * func, int pos)
{
	return fk->bin.dump(func, pos).c_str();
}

FAKE_API const char * fkdumpfuncmap(fake * fk)
{
	return fk->fm.dump().c_str();
}

FAKE_API int fksavefunc(fake * fk, char * buff, int size)
{
	buffer b;
	b.ini(fk, buff, size);
	if (!fk->bin.save(&b))
	{
		return -1;
	}
	return b.size();
}

FAKE_API int fkloadfunc(fake * fk, char * buff, int size)
{
	buffer b;
	b.ini(fk, buff, size);
	b.skip_write(size);
	if (!fk->bin.load(&b))
	{
		return -1;
	}
	return b.size();
}

FAKE_API const char ** fkgetkeyword()
{
	static const char * keyword[] = {
		"var",
		"return",
		"break",
		"func",
		"fake",
		"while",
		"for",
		"true",
		"false",
		"if",
		"then",
		"else",
		"elseif",
		"end",
		"const",
		"package",
		"include",
		"struct",
		"and",
		"or",
		"is",
		"not",
		"continue",
		"yield",
		"sleep",
		"switch",
		"case",
		"default",
		"",
	};
	return keyword;
}

FAKE_API void fksetprintfunc(fake * fk, fkprint func)
{
	fk->bif.set_print_func(func);
}

FAKE_API void fkopenstepmod(fake * fk)
{
	fk->rn.stepmod = true;
}

FAKE_API void fkclosestepmod(fake * fk)
{
	fk->rn.stepmod = false;
}

FAKE_API void fkresumeps(fake * fk, bool & isend)
{
	FKLOG("fkresumeps %p", fk);

	isend = false;

	// 上次的processor
	pool<processor>::node * n = 0;
	GET_CUR_PROCESSOR(n, fk->rn);
	if (UNLIKE(!n))
	{
		variant * ret = 0;
		bool err = false;
		PS_PUSH_AND_GET(fk->ps, ret);
		*ret = NILV;
		FKLOG("fkresumeps %p no processor", fk);
		return;
	}

	// 先pop防止重入
	POP_CUR_PROCESSOR(fk->rn);
	
	processor & pro = n->t;
	pro.run();
	if (LIKE(!PROCESS_END(pro)))
	{
		// 重新塞入
		PUSH_CUR_PROCESSOR(n, fk->rn);
	
		variant * ret = 0;
		bool err = false;
		PS_PUSH_AND_GET(fk->ps, ret);
		*ret = NILV;
		FKLOG("fkresumeps %p not end", fk);
		return;
	}

	// 结束了
	variant * ret = 0;
	bool err = false;
	PS_PUSH_AND_GET(fk->ps, ret);
	*ret = ROUTINE_GETRET(*pro.m_entryroutine);

	POOL_PUSH(fk->pp, n);
	
	fk->rn.rundeps--;
	
	isend = true;
	
	FKLOG("fkresumeps %p OK", fk);
}

FAKE_API void fkrundebugps(fake * fk)
{
	FKLOG("fkrundebugps %p", fk);
	fk->dbg.debug();
}

