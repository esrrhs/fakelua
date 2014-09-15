#include "fuckscript.h"
#include "semantic.h"
#include "fuck.h"
#include "compiler.h"
#include "routine.h"
#include "processor.h"
#include "paramstack.h"

FUCK_API fuck * newfuck(fkmalloc fkm, fkfree fkf,
    int per_frame_cmd_num,
	int delete_routine_scale,
	int routine_grow_speed,
	int stack_ini_size,
	int stack_grow_speed,
	int stack_list_grow_speed)
{
    if (!fkm || !fkf)
    {
        fkm = &malloc;
        fkf = &free;
        FKLOG("newfuck use system malloc and free");
    }
	if (!per_frame_cmd_num || !delete_routine_scale || !routine_grow_speed)
    {
        per_frame_cmd_num = 100;
        delete_routine_scale = 4;
		routine_grow_speed = 100;
        FKLOG("newfuck use per_frame_cmd_num[%d] delete_routine_scale[%d] stack_list_grow_speed[%d]", 
			per_frame_cmd_num, delete_routine_scale, routine_grow_speed);
	}
	if (!stack_ini_size || !stack_grow_speed || !stack_list_grow_speed)
	{
		stack_ini_size = 10;
		stack_grow_speed = 100;
		stack_list_grow_speed = 100;
		FKLOG("newfuck use stack_ini_size[%d] stack_grow_speed[%d] stack_list_grow_speed[%d]", 
			stack_ini_size, stack_grow_speed, stack_list_grow_speed);
	}
    
    fuck * ret = (fuck *)fkm(sizeof(fuck));
    new (ret) fuck();
    ret->m_fkmalloc = fkm;
	ret->m_fkfree = fkf;
	ret->m_per_frame_cmd_num = per_frame_cmd_num;
	ret->m_delete_routine_scale = delete_routine_scale;
	ret->m_stack_ini_size = stack_ini_size;
	ret->m_stack_grow_speed = stack_grow_speed;
	ret->m_stack_list_grow_speed = stack_list_grow_speed;
    FKLOG("newfuck ret %p", ret);
    return ret;
}

FUCK_API void delfuck(fuck * fk)
{
    FKLOG("delfuck %p", fk);
    fk->~fuck();
    fk->m_fkfree(fk);
}

// 解析文件
FUCK_API binary * fkparse(fuck * fk, fkerrorinfo * ei, const char * filename)
{
    if (ei) ei->clear();

    // 输入源文件
    myflexer mf(fk, ei);
    
    FKLOG("fkparse %p %s", fk, filename);
    bool b = mf.inputfile(filename);
    if (!b)
    {
        FKERR("fkparse open %s fail", fk, filename);
        return 0;
    }

    // 进行语法解析
    int ret = yyparse((void *)&mf); 
    if (ret != 0)
    {
        FKERR("fkparse yyparse %s fail ret %d", fk, filename, ret);
        fk->seterror(ei, efk_parse_file_fail, "parse %s file fail", filename);
        return 0;
    }
    
    FKLOG("fkparse yyparse %p %s OK", fk, filename);

    binary * bin = fknew<binary>(fk, fk);

    // 编译
    compiler mc(fk, ei, bin);
    b = mc.compile(&mf);
    if (!b)
    {
        FKERR("fkparse  %s compile %s fail", fk, filename);
        return 0;
    }
    
    FKLOG("fkparse %p %s OK", fk, filename);
    
    return bin;
}

FUCK_API void delbinary(binary * bin)
{
    fuck *fk = bin->getfuck();
    fkdelete<binary>(fk, bin);
}

FUCK_API bool fkisfunc(binary * bin, const char * func)
{
    return bin->is_have_func(func);
}

// 调用函数
FUCK_API void fkrun(binary * bin, fkerrorinfo * ei, const char * func, paramstack * s)
{
    if (ei) ei->clear();

    fuck *fk = bin->getfuck();
    FKLOG("fkrun %p %p %s", fk, bin, func);
    
    routine r(fk, ei);
    r.entry(bin, func, s);
    
    processor p(fk);

    p.add(&r);

    int cmdnum = 0;
    int framenum = 0;
    int perframecmdnum = fk->m_per_frame_cmd_num;
    while (!p.isend())
    {
        cmdnum += p.run(perframecmdnum);
        framenum++;
    }
    
    FKLOG("fkrun cmdnum[%d] perframecmdnum[%d] framenum[%d]", cmdnum, perframecmdnum, framenum);

    s->push(r.getret());
    
    FKLOG("fkrun %p %p %s OK", fk, bin, func);
}

// 参数传递
FUCK_API paramstack * fknewparamstack(binary * bin)
{
    fuck *fk = bin->getfuck();
    return fknew<paramstack>(fk, fk);
}

FUCK_API void fkdeleteparamstack(paramstack * s)
{
    fuck *fk = s->getfuck();
    fkdelete<paramstack>(fk, s);
}

FUCK_API void fkpspushpointer(paramstack * s, void * p)
{
    fuck *fk = s->getfuck();
    variant v(p, fk);
    s->push(v);
}

FUCK_API void fkpspushchar(paramstack * s, char ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushuchar(paramstack * s, unsigned char ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushshort(paramstack * s, short ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushushort(paramstack * s, unsigned short ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushint(paramstack * s, int ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushuint(paramstack * s, unsigned int ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushfloat(paramstack * s, float ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushdouble(paramstack * s, double ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushcharptr(paramstack * s, char * ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushccharptr(paramstack * s, const char * ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushbool(paramstack * s, bool ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushint64(paramstack * s, int64_t ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspushuint64(paramstack * s, uint64_t ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

FUCK_API void fkpspoppointer(paramstack * s, void * & p)
{
    variant v;
    s->pop(v);
    p = (void*)v;
}

FUCK_API char fkpspopchar(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API unsigned char fkpspopuchar(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API short fkpspopshort(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API unsigned short fkpspopushort(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API int fkpspopint(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API unsigned int fkpspopuint(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API float fkpspopfloat(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API double fkpspopdouble(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API const char * fkpspopcstrptr(paramstack * s)
{
	variant v;
	s->pop(v);
	return ((String)v).c_str();
}

FUCK_API bool fkpspopbool(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API int64_t fkpspopint64(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

FUCK_API uint64_t fkpspopuint64(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}


