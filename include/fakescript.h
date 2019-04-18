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
@.支持打包bin文件或可执行文件
@.自带gdb风格的命令行调试器，以及vs风格的可视化编辑调试ide
@.可在C里直接通过接口调用，开始命令行调试

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
const hellomap = {1 : "a" 2 : "b" 3 : [1 2 3]}
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
ret = fkrun<int>(fk, "myfunc1", 1, 2);	// 或调用fkdebugrun调试
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
#include <typeinfo>

#define FAKE_VERSION "1.4"
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

	efk_reg_func_double_name = 400,
	efk_reg_func_toolong,
	
	efk_run_no_func_error = 500,
	efk_run_param_error,
	efk_run_data_error,
	efk_run_cal_error,
	efk_run_inter_error,

	efk_jit_error = 600,
};

// 脚本环境
struct fake;

typedef void(*fkerrorcb)(fake * fk, int eno, const char * file, int lineno, const char * func, const char * str);

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void * ptr);
typedef void(*fkprint)(fake * fk, const char * str);

#define FAKE_API extern "C"
#define MAX_FAKE_PARAM_NUM 40	// 最大40个参数
#define MAX_FAKE_RETURN_NUM 10	// 最大10个返回值
#define MAX_FAKE_REG_FUNC_NAME_LEN 256	// 最大注册函数名字长度

struct fakeconfig
{
	fakeconfig() : fkm(&malloc), fkf(&free), 
		per_frame_cmd_num(10), 
		array_grow_speed(50), 
		string_heap_num(1000), 
		pointer_heap_num(1000), 
		include_deps(100), 
		stack_max(10000)
		{}
	fkmalloc fkm;
	fkfree fkf;	// 内存管理
	int per_frame_cmd_num;			// 每帧执行命令数目
	int array_grow_speed;			// 增长速度，百分比，10%代表增长10%
	int string_heap_num;			// 字符串集合的最大数目
	int pointer_heap_num;			// 指针集合的最大数目
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

// 清空脚本函数，不会清空c函数和内置函数
FAKE_API void fkclear(fake * fk);

// 是否有函数
FAKE_API bool fkisfunc(fake * fk, const char * func);
// 获取函数所在的文件名
FAKE_API const char * fkgetfuncfile(fake * fk, const char * func);
// 获取函数所在的起始行号
FAKE_API int fkgetfuncstartline(fake * fk, const char * func);
// 获取函数内变量数目
FAKE_API int fkgetfuncvariantnum(fake * fk, const char * func);
// 获取函数内变量名
FAKE_API const char * fkgetfuncvariantname(fake * fk, const char * func, int index);
// 获取函数内变量行号
FAKE_API int fkgetfuncvariantline(fake * fk, const char * func, int index);

// 字节流封装，方便与C交互，使用方法如fakebytes b = fkrun<fakebytes>(fk, "test")或者fakebytes b; fkrun<int>(fk, "test", b);
struct fakebytes
{
	char * data;
	size_t size;
};

// 参数传递
FAKE_API void fkpspushpointer(fake * fk, void * p, const char * type);
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
struct fkremoveconst { typedef T type; };
template<typename T>
struct fkremoveconst<const T> { typedef T type; };

template<typename T>
struct fkbasetype { typedef T type; };
template<typename T>
struct fkbasetype<T*> { typedef T type; };
template<typename T>
struct fkbasetype<T&> { typedef T type; };

template<typename T>
struct fkclasstype { typedef typename fkremoveconst<typename fkbasetype<T>::type>::type type; };

template<typename T>  
inline void fkpspush(fake * fk, T ret)
{ 
	fkpspushpointer(fk, (void *)ret, typeid(typename fkclasstype<T>::type).name()); 
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

FAKE_API void fkpspoppointer(fake * fk, void * & p, const char * type);
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
FAKE_API fakebytes fkpspopbuffer(fake * fk);

template<typename T>  
inline T fkpspop(fake * fk)
{ 
	void * ret = 0; 
	fkpspoppointer(fk, ret, typeid(typename fkclasstype<T>::type).name()); 
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

template<>	inline fakebytes fkpspop(fake * fk)
{
	return fkpspopbuffer(fk);
}

FAKE_API void fkpsclear(fake * fk);

// 此函数内部使用，推荐使用模板
FAKE_API void fkrunps(fake * fk, const char * func);

// 检查回收
FAKE_API void fkcheckgc(fake * fk);

// 调用函数，解释执行
template<typename RVal>
RVal fkrun(fake * fk, const char * func)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrun(fake * fk, const char * func, T1 arg1)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
	fkpsclear(fk);
	fkcheckgc(fk);
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
	fkcheckgc(fk);
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
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkpspush<T6>(fk, arg6);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkpspush<T6>(fk, arg6);
	fkpspush<T7>(fk, arg7);
	fkrunps(fk, func);
	return fkpspop<RVal>(fk);
}

// 单步执行
FAKE_API void fkopenstepmod(fake * fk);
FAKE_API void fkclosestepmod(fake * fk);

// 此函数内部使用，推荐使用模板
FAKE_API void fkrundebugps(fake * fk);

// 调用函数，调试解释执行
template<typename RVal>
RVal fkdebugrun(fake * fk, const char * func)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1, T2 arg2)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1, arg2);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1, arg2, arg3);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1, arg2, arg3, arg4);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1, arg2, arg3, arg4, arg5);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1, arg2, arg3, arg4, arg5, arg6);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
RVal fkdebugrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{
	fkopenstepmod(fk);
	fkrun<RVal>(fk, func, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	fkrundebugps(fk);
	return fkpspop<RVal>(fk);
}

// 此函数内部使用，推荐使用模板
FAKE_API void fkrunpsjit(fake * fk, const char * func);

// 调用函数,native code
template<typename RVal>
RVal fkrunjit(fake * fk, const char * func)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrunjit(fake * fk, const char * func, T1 arg1)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
	fkpsclear(fk);
	fkcheckgc(fk);
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
	fkcheckgc(fk);
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
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkpspush<T6>(fk, arg6);
	fkpspush<T7>(fk, arg7);
	fkrunpsjit(fk, func);
	return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{
	fkpsclear(fk);
	fkcheckgc(fk);
	fkpspush<T1>(fk, arg1);
	fkpspush<T2>(fk, arg2);
	fkpspush<T3>(fk, arg3);
	fkpspush<T4>(fk, arg4);
	fkpspush<T5>(fk, arg5);
	fkpspush<T6>(fk, arg6);
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
	fkfunctor() : ff(0), param1(0), argnum(0){}
	fkfunctor(fkcfunction _ff, const void * _param1, int _argnum) : ff(_ff), param1(_param1), argnum(_argnum) {}
	template <typename F>
	fkfunctor(fkcfunction _ff, const void * _param1, F f, int _argnum) : ff(_ff), param1(_param1), argnum(_argnum)
	{
		new(param2) F(f);
	}
	fkcfunction ff;
	const void * param1;
	char param2[FAKE_MEMFUNC_SIZE];
	int argnum;
};

template<typename RVal, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void, typename T6=void, typename T7=void>
struct fkinvoker
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{ 
		RVal ret = ((RVal(*)(T1,T2,T3,T4,T5,T6,T7))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk),
			fkpspop<T7>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct fkinvoker<RVal, T1, T2, T3, T4, T5, T6> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		RVal ret = ((RVal(*)(T1,T2,T3,T4,T5,T6))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
struct fkinvoker<RVal, T1, T2, T3, T4, T5> 
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

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct fkinvoker<void, T1, T2, T3, T4, T5, T6, T7>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1,T2,T3,T4,T5,T6,T7))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk),
			fkpspop<T7>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct fkinvoker<void, T1, T2, T3, T4, T5, T6>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		((void(*)(T1,T2,T3,T4,T5,T6))ff->param1)(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk));
		fkpspush<int>(fk, 0);
	}
};

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

FAKE_API void fkpushfunctor(fake * fk, const char * prefix, const char * name, fkfunctor ff);

// 注册C函数
template<typename RVal>
void fkreg(fake * fk, const char * name, RVal (*func)())
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal>::invoke, (void*)func, 0));
}

template<typename RVal, typename T1>
void fkreg(fake * fk, const char * name, RVal (*func)(T1))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1>::invoke, (void*)func, 1));
}

template<typename RVal, typename T1, typename T2>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1, T2>::invoke, (void*)func, 2));
}

template<typename RVal, typename T1, typename T2, typename T3>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1, T2, T3>::invoke, (void*)func, 3));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4>::invoke, (void*)func, 4));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4, T5))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4, T5>::invoke, (void*)func, 5));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4, T5, T6))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4, T5, T6>::invoke, (void*)func, 6));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4, T5, T6, T7))
{
	fkpushfunctor(fk, "", name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4, T5, T6, T7>::invoke, (void*)func, 7));
}

template<typename RVal, typename T, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void, typename T6=void, typename T7=void>
struct fkmeminvoker
{
	static void invoke(fake * fk, const fkfunctor * ff) 
	{ 
		T * p = fkpspop<T*>(fk);	// 不同编译器顺序不一样，提出来安全
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3,T4,T5,T6,T7))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk),
			fkpspop<T7>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct fkmeminvoker<RVal, T, T1, T2, T3, T4, T5, T6> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3,T4,T5,T6))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk));
		fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
struct fkmeminvoker<RVal, T, T1, T2, T3, T4, T5> 
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
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

template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct fkmeminvoker<void, T, T1, T2, T3, T4, T5, T6, T7>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1,T2,T3,T4,T5,T6,T7))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk),
			fkpspop<T7>(fk));
		fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct fkmeminvoker<void, T, T1, T2, T3, T4, T5, T6>
{
	static void invoke(fake * fk, const fkfunctor * ff)
	{
		T * p = fkpspop<T*>(fk);
		if (!p)
			return;
		((p)->*(*(void(T::**)(T1,T2,T3,T4,T5,T6))(ff->param2)))(fkpspop<T1>(fk),
			fkpspop<T2>(fk),
			fkpspop<T3>(fk),
			fkpspop<T4>(fk),
			fkpspop<T5>(fk),
			fkpspop<T6>(fk));
		fkpspush<int>(fk, 0);
	}
};

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
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T>::invoke, 0, func, 1));
}

template<typename RVal, typename T, typename T1>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1>::invoke, 0, func, 2));
}

template<typename RVal, typename T, typename T1, typename T2>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1, T2>::invoke, 0, func, 3));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3>::invoke, 0, func, 4));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4>::invoke, 0, func, 5));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4, T5))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4, T5>::invoke, 0, func, 6));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4, T5, T6))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4, T5, T6>::invoke, 0, func, 7));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4, T5, T6, T7))
{
	fkpushfunctor(fk, typeid(typename fkclasstype<T>::type).name(), name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4, T5, T6, T7>::invoke, 0, func, 8));
}

// 开启常用内置函数
FAKE_API void fkopenalllib(fake * fk);
FAKE_API void fkopenbaselib(fake * fk);
FAKE_API void fkopenfilelib(fake * fk);
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

// optimize
FAKE_API void fkopenoptimize(fake * fk);
FAKE_API void fkcloseoptimize(fake * fk);

// 设置错误回调
FAKE_API void fkseterrorfunc(fake * fk, fkerrorcb cb);

// 获取当前运行状态
FAKE_API const char * fkgetcurfunc(fake * fk);
FAKE_API const char * fkgetcurfile(fake * fk);
FAKE_API int fkgetcurline(fake * fk);
FAKE_API int fkgetcurbytecodepos(fake * fk);
FAKE_API const char * fkgetcurcode(fake * fk);
FAKE_API const char * fkgetcurcallstack(fake * fk);
FAKE_API int fkgetcurcallstacklength(fake * fk);
FAKE_API const char * fkgetcurcallstackbyframe(fake * fk, int frame);
FAKE_API const char * fkgetfilecode(fake * fk, const char * filename, int line);

// 获取当前运行状态
FAKE_API const char * fkgetcurfuncbyframe(fake * fk, int frame);
FAKE_API const char * fkgetcurfilebyframe(fake * fk, int frame);
FAKE_API int fkgetcurlinebyframe(fake * fk, int frame);
FAKE_API const char * fkgetcurvaiantbyframe(fake * fk, int frame, const char * name, int line = -1);
FAKE_API int fkgetcurvaiantlinebyframe(fake * fk, int frame, const char * name, int line = -1);
FAKE_API void fksetcurvaiantbyframe(fake * fk, int frame, const char * name, const char * value, int line = -1);

// 获取当前运行状态
FAKE_API const char * fkgetcurroutine(fake * fk);
FAKE_API int fkgetcurroutinenum(fake * fk);
FAKE_API const char * fkgetcurroutinebyindex(fake * fk, int index);
FAKE_API const char * fkgetcurroutinebyid(fake * fk, int rid);
FAKE_API int fkgetroutineidbyindex(fake * fk, int index);
FAKE_API bool fkishaveroutine(fake * fk, int rid);
FAKE_API int fkgetcurroutineid(fake * fk);
FAKE_API const char * fkgetcurvaiantbyroutinebyframe(fake * fk, int rid, int frame, const char * name, int line = -1);
FAKE_API int fkgetcurvaiantlinebyroutinebyframe(fake * fk, int rid, int frame, const char * name, int line = -1);
FAKE_API void fksetcurvaiantbyroutinebyframe(fake * fk, int rid, int frame, const char * name, const char * value, int line = -1);
FAKE_API int fkgetcurlinebyroutinebyframe(fake * fk, int rid, int frame);
FAKE_API const char * fkgetcurfuncbyroutinebyframe(fake * fk, int rid, int frame);
FAKE_API const char * fkgetcurfilebyroutinebyframe(fake * fk, int rid, int frame);
FAKE_API int fkgetcurcallstacklengthbyroutine(fake * fk, int rid);
FAKE_API const char * fkgetcurcallstackbyroutinebyframe(fake * fk, int rid, int frame);
FAKE_API int fkgetcurbytecodeposbyroutine(fake * fk, int rid);


// 设置系统命令行
FAKE_API void fksetargv(fake * fk, int argc, const char *argv[]);

// dump函数
FAKE_API const char * fkdumpallfunc(fake * fk);
FAKE_API const char * fkdumpfunc(fake * fk, const char * func, int pos = -1);
FAKE_API const char * fkdumpfuncmap(fake * fk);

// save load函数
FAKE_API int fksavefunc(fake * fk, char * buff, int size);
FAKE_API int fkloadfunc(fake * fk, char * buff, int size);

// 获取关键词
FAKE_API const char ** fkgetkeyword();

// 设置print位置
FAKE_API void fksetprintfunc(fake * fk, fkprint func);

// 继续上次的执行
FAKE_API void fkresumeps(fake * fk, bool & isend);

// 继续上次的执行
template<typename RVal>
RVal fkresume(fake * fk, bool & isend)
{
	fkpsclear(fk);
	fkresumeps(fk, isend);
	return fkpspop<RVal>(fk);
}

