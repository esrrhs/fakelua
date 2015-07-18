/************************************************************************/
/*
fakescript是一款轻量级的嵌入式脚本语言，使用c++语言编写，语法吸取自lua、golang、erlang，基于flex、bison生成语法树，编译成字节码解释执行。与lua相比，运行速度不相上下，更易使用，源代码可读性更强

脚本特性：
@.代码风格类似lua
@.全部为函数
@.支持array，map，可以无限嵌套
@.支持fake testfunc(param1)产生routine，实现假多线程效果
@.支持C函数和类成员函数的绑定
@.自带解释器，JIT部分支持
@.支持多返回值
@.自带profile，可获取脚本各个函数运行时间
@.支持热更新
@.支持Int64
@.支持const定义
@.支持包
@.支持struct
@.支持打包bin文件

示例：
-- 当前包名
package mypackage.test
-- 引入的文件
include "common.fk"
-- 结构体定义
struct teststruct
	sample_a
	sample_b
	sample_c
end
-- 常量值
const hellostring = "hello"
const helloint = 1234
-- func1 comment
func myfunc1(arg1, arg2)
	
	-- C函数和类成员函数的调用
	arg3 := cfunc1(helloint) + arg2:memfunc1(arg1)
	
	-- 分支
	if arg1 < arg2 then	
		-- 创建一个协程
		fake myfunc2(arg1, arg2)
	elseif arg1 == arg2 then	
		print("elseif")
	else
		print("else")
	end
	
	-- for循环
	for var i = 0, i < arg2, i++ then
		print("i = ", i)
	end
	
	-- 数组
	var a = array()
	a[1] = 3
	
	-- 集合
	var b = map()
	b[a] = 1
	b[1] = a
	
	-- Int64
	var uid = 1241515236123614u
	log("uid = ", uid)
	-- 子函数调用
	var ret1, var ret2 = myfunc2()
	-- 其他包的函数调用
	ret1 = otherpackage.test.myfunc1(arg1, arg2)
	
	-- 结构体
	var tt = teststruct()
	tt->sample_a = 1
	tt->sample_b = teststruct()
	tt->sample_b->sample_a = 10
	-- 分支
	switch arg1
		case 1 then
			print("1")
		case "a" then
			print("a")
		default
			print("default")
	end
	-- 多返回值
	return arg1, arg3
	
end
使用方法：
fake * fk = newfake();
fkopenbaselib(fk);
fkopenprofile(fk);
fkreg(fk, "cfunc1", cfunc1);
fkreg(fk, "memfunc1", &class1::memfunc1);
fkparse(fk, argv[1]);
ret = fkrun<int>(fk, "myfunc1", 1, 2);
delfake(fk);

BUG反馈：
esrrhs@163.com

*/
/************************************************************************/

#pragma once

#include <stdint.h>
#include <string>
#include <new>
#include <stdlib.h>

#define FAKE_VERSION "1.2"
#define FAKE_VERSION_NUM 100
#define FAKE_AUTHOR "esrrhs@163.com"

// 错误号
enum efkerror
{
	efk_ok = 0,
	efk_strsize = 100,
	
	efk_open_file_fail = 200,
	efk_open_file_empty,
	efk_parse_file_fail,
	
	efk_compile_func_not_define = 300,
	efk_compile_stack_identifier_error,
	efk_compile_stmt_type_error,
	efk_compile_variable_not_found,
	efk_compile_explicit_type_error,
	efk_compile_math_type_error,
	efk_compile_variable_has_define,
	efk_compile_add_stack_identifier,
	efk_compile_cmp_error,
	efk_compile_loop_error,

	efk_reg_memfunc_double_name = 400,
	
	efk_run_no_func_error = 500,
	efk_run_param_error,
	efk_run_data_error,
	efk_run_cal_error,
	efk_run_inter_error,

	efk_jit_error = 600,
};

// 脚本环境
struct fake;

typedef void (*fkerrorcb)(fake * fk, int eno, const char * str);

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

#define FAKE_API extern "C"
#define MAX_FAKE_PARAM_NUM 10	// 最大10个参数
#define MAX_FAKE_RETURN_NUM 10	// 最大10个返回值

struct fakeconfig
{
	fakeconfig() : fkm(&malloc), fkf(&free), 
		per_frame_cmd_num(10), 
		array_grow_speed(100), 
		string_heap_num(100), 
		include_deps(100), 
		stack_max(10000)
		{}
	fkmalloc fkm;
	fkfree fkf;	// 内存管理
	int per_frame_cmd_num;			// 每帧执行命令数目
	int array_grow_speed;			// 增长速度，百分比，10%代表增长10%
	int string_heap_num;			// 字符串集合的最大数目
	int include_deps;				// 解析include最大深度
	int stack_max;					// stack最大尺寸
};

// 申请回收
FAKE_API fake * newfake(fakeconfig * cfg = 0);
FAKE_API void delfake(fake * fk);

// 错误代码
FAKE_API efkerror fkerror(fake * fk);
FAKE_API const char * fkerrorstr(fake * fk);

// 解析文件
// 非运行中的脚本会直接替换，否则会在下次fkrun的时候替换
FAKE_API bool fkparse(fake * fk, const char * filename);
FAKE_API bool fkparsestr(fake * fk, const char * str);

// 是否有函数
FAKE_API bool fkisfunc(fake * fk, const char * func);

// 参数传递
FAKE_API void fkpspushpointer(fake * fk, void * p);
FAKE_API void fkpspushchar(fake * fk, char ret);
FAKE_API void fkpspushuchar(fake * fk, unsigned char ret);
FAKE_API void fkpspushshort(fake * fk, short ret);
FAKE_API void fkpspushushort(fake * fk, unsigned short ret);
FAKE_API void fkpspushint(fake * fk, int ret);
FAKE_API void fkpspushuint(fake * fk, unsigned int ret);
FAKE_API void fkpspushfloat(fake * fk, float ret);
FAKE_API void fkpspushdouble(fake * fk, double ret);
FAKE_API void fkpspushcharptr(fake * fk, char * ret);
FAKE_API void fkpspushccharptr(fake * fk, const char * ret);
FAKE_API void fkpspushbool(fake * fk, bool ret);
FAKE_API void fkpspushint64(fake * fk, int64_t ret);
FAKE_API void fkpspushuint64(fake * fk, uint64_t ret);

template<typename T>  
inline void fkpspush(fake * fk, T ret)
{ 
	fkpspushpointer(fk, (void*)ret); 
}
template<>	inline void fkpspush(fake * fk, char ret)
{
	fkpspushchar(fk, ret);
}

template<>	inline void fkpspush(fake * fk, unsigned char ret)
{
	fkpspushuchar(fk, ret);
}

template<>	inline void fkpspush(fake * fk, signed char ret)
{
	fkpspushuchar(fk, ret);
}

template<>	inline void fkpspush(fake * fk, short ret)
{
	fkpspushshort(fk, ret);
}

template<>	inline void fkpspush(fake * fk, unsigned short ret)
{
	fkpspushushort(fk, ret);
}

template<>	inline void fkpspush(fake * fk, int ret)
{
	fkpspushint(fk, ret);
}

template<>	inline void fkpspush(fake * fk, unsigned int ret)
{
	fkpspushuint(fk, ret);
}

template<>	inline void fkpspush(fake * fk, float ret)
{
	fkpspushfloat(fk, ret);
}

template<>	inline void fkpspush(fake * fk, double ret)
{
	fkpspushdouble(fk, ret);
}

template<>	inline void fkpspush(fake * fk, std::string ret)
{
	fkpspushccharptr(fk, ret.c_str());
}

template<>	inline void fkpspush(fake * fk, char * ret)
{
	fkpspushcharptr(fk, ret);
}

template<>	inline void fkpspush(fake * fk, const char * ret)
{
	fkpspushccharptr(fk, ret);
}

template<>	inline void fkpspush(fake * fk, bool ret)
{
	fkpspushbool(fk, ret);
}

template<>	inline void fkpspush(fake * fk, int64_t ret)
{
	fkpspushint64(fk, ret);
}

template<>	inline void fkpspush(fake * fk, uint64_t ret)
{
	fkpspushuint64(fk, ret);
}

FAKE_API void fkpspoppointer(fake * fk, void * & p);
FAKE_API char fkpspopchar(fake * fk);
FAKE_API unsigned char fkpspopuchar(fake * fk);
FAKE_API short fkpspopshort(fake * fk);
FAKE_API unsigned short fkpspopushort(fake * fk);
FAKE_API int fkpspopint(fake * fk);
FAKE_API unsigned int fkpspopuint(fake * fk);
FAKE_API float fkpspopfloat(fake * fk);
FAKE_API double fkpspopdouble(fake * fk);
FAKE_API const char * fkpspopcstrptr(fake * fk);
FAKE_API bool fkpspopbool(fake * fk);
FAKE_API int64_t fkpspopint64(fake * fk);
FAKE_API uint64_t fkpspopuint64(fake * fk);

template<typename T>  
inline T fkpspop(fake * fk)
{ 
	void * ret = 0; 
	fkpspoppointer(fk, ret); 
	return (T)ret; 
}

template<>	inline void fkpspop(fake * fk)
{
	// nothing
}

template<>	inline char fkpspop(fake * fk)
{
	return fkpspopchar(fk);
}

template<>	inline unsigned char fkpspop(fake * fk)
{
	return fkpspopuchar(fk);
}

template<>	inline signed char fkpspop(fake * fk)
{
	return fkpspopuchar(fk);
}

template<>	inline short fkpspop(fake * fk)
{
	return fkpspopshort(fk);
}

template<>	inline unsigned short fkpspop(fake * fk)
{
	return fkpspopushort(fk);
}

template<>	inline int fkpspop(fake * fk)
{
	return fkpspopint(fk);
}

template<>	inline unsigned int fkpspop(fake * fk)
{
	return fkpspopuint(fk);
}

template<>	inline float fkpspop(fake * fk)
{
	return fkpspopfloat(fk);
}

template<>	inline double fkpspop(fake * fk)
{
	return fkpspopdouble(fk);
}

template<>	inline std::string fkpspop(fake * fk)
{
	const char * ret = fkpspopcstrptr(fk);
	return ret ? ret : "";
}

template<>	inline bool fkpspop(fake * fk)
{
	return fkpspopbool(fk);
}

template<>	inline int64_t fkpspop(fake * fk)
{
	return fkpspopint64(fk);
}

template<>	inline uint64_t fkpspop(fake * fk)
{
	return fkpspopuint64(fk);
}

template<>	inline const char * fkpspop(fake * fk)
{
	return fkpspopcstrptr(fk);
}

FAKE_API void fkpsclear(fake * fk);

// 此函数内部使用，推荐使用模板
FAKE_API void fkrunps(fake * fk, const char * func);

// 调用函数，解释执行
template<typename RVal>
RVal fkrun(fake * fk, const char * func)
{
	fkpsclear(fk);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrun(fake * fk, const char * func, T1 arg1)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}


// 此函数内部使用，推荐使用模板
FAKE_API void fkrunpsjit(fake * fk, const char * func);

// 调用函数,native code
template<typename RVal>
RVal fkrunjit(fake * fk, const char * func)
{
	fkpsclear(fk);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrunjit(fake * fk, const char * func, T1 arg1)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	fkpsclear(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

// 一个成员函数指针存放的空间，一般16个字节
#define FAKE_MEMFUNC_SIZE 32

// functor
struct fkfunctor;
typedef void (*fkcfunction) (fake * fk, const fkfunctor * ff);
struct fkfunctor
{
	fkfunctor() : ff(0), param1(0) {}
	fkfunctor(fkcfunction _ff, const void * _param1) : ff(_ff), param1(_param1) {}
	template <typename F>
	fkfunctor(fkcfunction _ff, const void * _param1, F f) : ff(_ff), param1(_param1)
	{
		new(param2) F(f);
	}
	fkcfunction ff;
	const void * param1;
	char param2[FAKE_MEMFUNC_SIZE];
};

template<typename RVal, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
struct fkinvoker
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{ 
		RVal ret = ((RVal(*)(T1,T2,T3,T4,T5))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
struct fkinvoker<RVal, T1, T2, T3, T4> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		RVal ret = ((RVal(*)(T1,T2,T3,T4))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2, typename T3>
struct fkinvoker<RVal, T1, T2, T3> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		RVal ret = ((RVal(*)(T1,T2,T3))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2>
struct fkinvoker<RVal, T1, T2> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		RVal ret = ((RVal(*)(T1,T2))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1>
struct fkinvoker<RVal, T1> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		RVal ret = ((RVal(*)(T1))ff->param1)(fkpspop<T1>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal>
struct fkinvoker<RVal>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		RVal ret = ((RVal(*)(void))ff->param1)();
		fkpspush<RVal>(fk, ret);
	}
};

//
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct fkinvoker<void, T1, T2, T3, T4, T5>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1,T2,T3,T4,T5))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2, typename T3, typename T4>
struct fkinvoker<void, T1, T2, T3, T4>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1,T2,T3,T4))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2, typename T3>
struct fkinvoker<void, T1, T2, T3>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1,T2,T3))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2>
struct fkinvoker<void, T1, T2>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1,T2))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T1>
struct fkinvoker<void, T1>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1))ff->param1)(fkpspop<T1>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<>
struct fkinvoker<void>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(void))ff->param1)();
		fkpspush<int>(fk, 0);
	}
};

FAKE_API void fkpushfunctor(fake * fk, const char * name, fkfunctor ff);

// 注册C函数
template<typename RVal>
void fkreg(fake * fk, const char * name, RVal (*func)())
{
	fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal>::invoke, (void*)func));
}

template<typename RVal, typename T1>
void fkreg(fake * fk, const char * name, RVal (*func)(T1))
{
	fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2))
{
	fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2, typename T3>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3))
{
	fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2, T3>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4))
{
	fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4, T5))
{
	fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4, T5>::invoke, (void*)func));
}

template<typename RVal, typename T, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
struct fkmeminvoker
{
	static void invoke(fake * fk, const fkfunctor * ff) 
	{ 
		T * p = fkpspop<T*>(fk);	// 不同编译器顺序不一样，提出来安全
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3,T4,T5))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
struct fkmeminvoker<RVal, T, T1, T2, T3, T4> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3,T4))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2, typename T3>
struct fkmeminvoker<RVal, T, T1, T2, T3> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2>
struct fkmeminvoker<RVal, T, T1 ,T2> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(T1,T2))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1>
struct fkmeminvoker<RVal, T, T1> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		RVal(T::*func)(T1) = *(RVal(T::**)(T1))(ff->param2);
		RVal ret = ((p)->*(func))(fkpspop<T1>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T>
struct fkmeminvoker<RVal, T>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(void))(ff->param2)))();
		fkpspush<RVal>(fk, ret);
	}
};

//
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
struct fkmeminvoker<void, T, T1, T2, T3, T4, T5>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1,T2,T3,T4,T5))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2, typename T3, typename T4>
struct fkmeminvoker<void, T, T1, T2, T3, T4>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1,T2,T3,T4))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2, typename T3>
struct fkmeminvoker<void, T, T1, T2, T3>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1,T2,T3))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2>
struct fkmeminvoker<void, T, T1, T2>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1,T2))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1>
struct fkmeminvoker<void, T, T1>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1))(ff->param2)))(fkpspop<T1>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T>
struct fkmeminvoker<void, T>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(void))(ff->param2)))();
		fkpspush<int>(fk, 0);
	}
};

// 注册类函数，不支持类函数重名
template<typename RVal, typename T>
void fkreg(fake * fk, const char * name, RVal (T::*func)())
{
	fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1))
{
	fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2))
{
	fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3))
{
	fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4))
{
	fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4, T5))
{
	fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4, T5>::invoke, 0, func));
}

// 开启常用内置函数
FAKE_API void fkopenalllib(fake * fk);
FAKE_API void fkopenbaselib(fake * fk);
FAKE_API void fkopenfilelib(fake * fk);
FAKE_API void fkopennetlib(fake * fk);
FAKE_API void fkopenoslib(fake * fk);
FAKE_API void fkopenstringlib(fake * fk);
FAKE_API void fkopenmathlib(fake * fk);

// profile 
FAKE_API void fkopenprofile(fake * fk);
FAKE_API void fkcloseprofile(fake * fk);
FAKE_API const char * fkdumpprofile(fake * fk);

// jit
FAKE_API void fkopenjit(fake * fk);
FAKE_API void fkclosejit(fake * fk);

// 设置错误回调
FAKE_API void fkseterrorfunc(fake * fk, fkerrorcb cb);

// 获取当前运行状态
FAKE_API const char * fkgetcurfunc(fake * fk);
FAKE_API const char * fkgetcurfile(fake * fk);
FAKE_API int fkgetcurline(fake * fk);
FAKE_API const char * fkgetcurcallstack(fake * fk);

// 设置系统命令行
FAKE_API void fksetargv(fake * fk, int argc, const char *argv[]);

// dump函数
FAKE_API const char * fkdumpallfunc(fake * fk);
FAKE_API const char * fkdumpfunc(fake * fk, const char * func);
FAKE_API const char * fkdumpfuncmap(fake * fk);

// save load函数
FAKE_API int fksavefunc(fake * fk, char * buff, int size);
FAKE_API int fkloadfunc(fake * fk, char * buff, int size);

