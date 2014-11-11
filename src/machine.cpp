#include "machine.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

#ifdef WIN32
extern "C" void __stdcall CallNativeFunc(const void * func, void * param,
	size_t size, void * ret);
#endif

void machine::call(native * nt, const char * func, paramstack * ps)
{
	// log
	if (ISLOG)
	{
		String paramstr;
		for (int i = 0; i < (int)ps->m_variant_list_num; i++)
		{
			int64_t type = ps->m_variant_list[i].type;
			int64_t data = ps->m_variant_list[i].data.buf;
			int64_t typeoff = V_TYPE_OFF(i) - 0x10;
			int64_t dataoff = V_DATA_OFF(i) - 0x10;
			paramstr += "movq $" + fkxtoa(type, 8) + ",-" + fkxtoa(-typeoff, 8) + "(%rsp) \n";
			paramstr += "movq $" + fkxtoa(data, 8) + ",-" + fkxtoa(-dataoff, 8) + "(%rsp) \n";
		}
		FKLOG("machine call \n%s\n", paramstr.c_str());
	}

    const func_native * fn = nt->get_func(func);
    if (!fn)
    {
        FKERR("fkrun native %p no func %s fail", nt, func);
        seterror(m_fk, efk_run_no_func_error, "fkrun native %p no func %s fail", nt, func);
        m_isend = true;
        return;
	}

#ifdef WIN32
	// win64不支持嵌入汇编，只能去.asm文件
	CallNativeFunc(fn->m_buff, ps->m_variant_list, ps->m_variant_list_num, &m_ret);
#else
	typedef void(*macfunc) ();
	macfunc f = (macfunc)fn->m_buff;
	int64_t i = 0;
	int64_t type = 0;
	int64_t data = 0;
	int64_t typeoff = 0;
	int64_t dataoff = 0;

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

    // 获得返回值
    asm(
        "mov    %%eax,%0    \n\t"      
        "movq   %%rdx,%1    \n\t"
        :"=r"(m_ret.type),"=r"(m_ret.data.real)
        :
		:"%eax","%rdx"
        );
#endif
}

