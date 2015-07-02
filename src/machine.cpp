#include "machine.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

#ifdef WIN32
extern "C" void __stdcall call_native_func(const void * func, void * param,
	size_t size)
#ifndef FK64
{
	// JIT不全，先临时
}
#else
;
#endif
#endif

void machine::static_call(fake * fk, const variant & func)
{
	fk->mac.call(func);
}

void machine::call(const variant & func)
{
	paramstack * ps = getps(m_fk);
	fake * fk = m_fk;

	const funcunion * f = m_fk->fm.get_func(func);
    if (UNLIKE(!f))
    {
        FKERR("fkrun native no func %s fail", vartostring(&func).c_str());
        seterror(m_fk, efk_run_no_func_error, "fkrun native no func %s fail", vartostring(&func).c_str());
        m_isend = true;
        return;
	}

	// 记录profile
	uint32_t s = 0;
	if (m_fk->pf.isopen())
	{
		s = fkgetmstick();
	}

	// 常规函数
	if (f->havefn)
	{
		const func_native * fn = &f->fn;
		
		// log
		if (UNLIKE(ISLOG))
		{
			String paramstr;
			for (int i = 0; i < (int)ps->m_variant_list_num; i++)
			{
				int64_t type = ps->m_variant_list[i].type;
				int64_t data = ps->m_variant_list[i].data.buf;
				int64_t typeoff = V_TYPE_OFF(i) - 0x10;
				int64_t dataoff = V_DATA_OFF(i) - 0x10;
				paramstr += "movq $" + fkxtoa(type, 8) + ", -" + fkxtoa(-typeoff, 8) + "(%rsp) \n";
				paramstr += "mov $" + fkxtoa(data, 8) + ", %rax\nmov %rax, -" + fkxtoa(-dataoff, 8) + "(%rsp) \n";
			}
			FKLOG("machine call \n%s\n", paramstr.c_str());
		}
		
#ifdef WIN32
		// win64不支持嵌入汇编，只能去.asm文件
		call_native_func(fn->m_buff, ps->m_variant_list, ps->m_variant_list_num);
#else
		typedef void (*macfunc) ();
		macfunc f = (macfunc)fn->m_buff;
		int64_t i = 0;
		int64_t type = 0;
		int64_t data = 0;
		int64_t typeoff = 0;
		int64_t dataoff = 0;

		asm(
			"push %%rax \n"
			"push %%rbx \n"
			"push %%rcx \n"
			"push %%rdx \n"
			"push %%rbp \n"
			"push %%rsp \n"
			"push %%rsi \n"
			"push %%rdi \n"
			:
			:
			:
		);
		
	    // push参数
	    for (i = 0; i < (int)ps->m_variant_list_num; i++)
	    {
	        type = ps->m_variant_list[i].type;
			data = ps->m_variant_list[i].data.buf;

			typeoff = V_TYPE_OFF(i) - 0x10;
			dataoff = V_DATA_OFF(i) - 0x10;
	        asm(
				"mov %%rsp,%%rax \n"
				"add %0,%%rax \n"
				"mov %1,(%%rax) \n"
				:
				:"r"(typeoff),"r"(type)
				:"%rax", "%eax"
			);
			asm(
				"mov %%rsp,%%rax \n"
				"add %0,%%rax \n"
				"movq %1,(%%rax) \n"
				:
				:"r"(dataoff),"r"(data)
				:"%rax", "%eax"
			);
		}
		
	    // call
	    f();
	    
		asm(
			"pop %%rdi \n"
			"pop %%rsi \n"
			"pop %%rsp \n"
			"pop %%rbp \n"
			"pop %%rdx \n"
			"pop %%rcx \n"
			"pop %%rbx \n"
			"pop %%rax \n"
			:
			:
			:
		);
			
#endif
	}
	// 绑定函数
	else if (f->haveff)
	{
		BIND_FUNC_CALL(f, 0);
		FKLOG("call C func %s", vartostring(&func).c_str());
	}
	// 内置函数
	else if (f->havebif)
	{
		BUILDIN_FUNC_CALL(f, 0);
		FKLOG("call buildin func %s", vartostring(&func).c_str());
	}
	else
	{
		FKERR("fkrun no jit func %s fail", vartostring(&func).c_str());
		seterror(m_fk, efk_run_no_func_error, "fkrun no inter func %s fail", vartostring(&func).c_str());
		m_isend = true;
		return;
	}

	if (m_fk->pf.isopen())
	{
	    bool err = false;
		const char * name = 0;
		V_GET_STRING(&func, name);
		m_fk->pf.add_func_sample(name, fkgetmstick() - s);
	}

	return;
}

