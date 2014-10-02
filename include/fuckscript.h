#pragma once

#include <stdint.h>
#include <string>

// 错误号
enum efkerror
{
    efk_ok = 0,
    efk_strsize = 100,
    
    efk_open_file_fail,
    efk_open_file_empty,
    efk_parse_file_fail,
    
    efk_compile_same_func_name,
    efk_compile_func_not_define,
    efk_compile_stack_identifier_error,
    efk_compile_stmt_type_error,
	efk_compile_variable_not_found,
	efk_compile_explicit_type_error,
	efk_compile_math_type_error,
	efk_compile_variable_has_define,
	efk_compile_add_stack_identifier,

	efk_run_no_func_error,
};

// 脚本环境
struct fuck;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

#define FUCK_API extern "C"
#define MAX_FUCK_PARAM_NUM 10	// 最大10个参数
#define MAX_FUCK_RETURN_NUM 1	// 最大1个返回值

struct fuckconfig
{
    fkmalloc fkm;
    fkfree fkf;	// 内存管理
    int per_frame_cmd_num;			// 每帧执行命令数目
	int delete_routine_scale;		// 删除无效协程的比例
	int routine_grow_speed;			// 协程增长速度，百分比，10%代表增长10%
	int stack_list_grow_speed;		// 栈层数增长速度，百分比，10%代表增长10%
};

// 申请回收
FUCK_API fuck * newfuck(fuckconfig * cfg = 0);
FUCK_API void delfuck(fuck * fk);

// 错误代码
FUCK_API efkerror fkerror(fuck * fk);
FUCK_API const char * fkerrorstr(fuck * fk);

// 解析文件
FUCK_API bool fkparse(fuck * fk, const char * filename);

// 是否有函数
FUCK_API bool fkisfunc(fuck * fk, const char * func);

// 参数传递
FUCK_API void fkpspushpointer(fuck * fk, void * p);
FUCK_API void fkpspushchar(fuck * fk, char ret);
FUCK_API void fkpspushuchar(fuck * fk, unsigned char ret);
FUCK_API void fkpspushshort(fuck * fk, short ret);
FUCK_API void fkpspushushort(fuck * fk, unsigned short ret);
FUCK_API void fkpspushint(fuck * fk, int ret);
FUCK_API void fkpspushuint(fuck * fk, unsigned int ret);
FUCK_API void fkpspushfloat(fuck * fk, float ret);
FUCK_API void fkpspushdouble(fuck * fk, double ret);
FUCK_API void fkpspushcharptr(fuck * fk, char * ret);
FUCK_API void fkpspushccharptr(fuck * fk, const char * ret);
FUCK_API void fkpspushbool(fuck * fk, bool ret);
FUCK_API void fkpspushint64(fuck * fk, int64_t ret);
FUCK_API void fkpspushuint64(fuck * fk, uint64_t ret);

template<typename T>  
inline void fkpspush(fuck * fk, T ret)
{ 
    fkpspushpointer(fk, (void*)ret); 
}
template<>	inline void fkpspush(fuck * fk, char ret)
{
	fkpspushchar(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, unsigned char ret)
{
	fkpspushuchar(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, short ret)
{
	fkpspushshort(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, unsigned short ret)
{
	fkpspushushort(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, int ret)
{
	fkpspushint(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, unsigned int ret)
{
	fkpspushuint(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, float ret)
{
	fkpspushfloat(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, double ret)
{
	fkpspushdouble(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, std::string ret)
{
	fkpspushccharptr(fk, ret.c_str());
}

template<>	inline void fkpspush(fuck * fk, char * ret)
{
	fkpspushcharptr(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, const char * ret)
{
	fkpspushccharptr(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, bool ret)
{
	fkpspushbool(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, int64_t ret)
{
	fkpspushint64(fk, ret);
}

template<>	inline void fkpspush(fuck * fk, uint64_t ret)
{
	fkpspushuint64(fk, ret);
}

FUCK_API void fkpspoppointer(fuck * fk, void * & p);
FUCK_API char fkpspopchar(fuck * fk);
FUCK_API unsigned char fkpspopuchar(fuck * fk);
FUCK_API short fkpspopshort(fuck * fk);
FUCK_API unsigned short fkpspopushort(fuck * fk);
FUCK_API int fkpspopint(fuck * fk);
FUCK_API unsigned int fkpspopuint(fuck * fk);
FUCK_API float fkpspopfloat(fuck * fk);
FUCK_API double fkpspopdouble(fuck * fk);
FUCK_API const char * fkpspopcstrptr(fuck * fk);
FUCK_API bool fkpspopbool(fuck * fk);
FUCK_API int64_t fkpspopint64(fuck * fk);
FUCK_API uint64_t fkpspopuint64(fuck * fk);

template<typename T>  
inline T fkpspop(fuck * fk)
{ 
    void * ret = 0; 
    fkpspoppointer(fk, ret); 
    return (T)ret; 
}

template<>	inline void fkpspop(fuck * fk)
{
	// nothing
}

template<>	inline char fkpspop(fuck * fk)
{
	return fkpspopchar(fk);
}

template<>	inline unsigned char fkpspop(fuck * fk)
{
	return fkpspopuchar(fk);
}

template<>	inline short fkpspop(fuck * fk)
{
	return fkpspopshort(fk);
}

template<>	inline unsigned short fkpspop(fuck * fk)
{
	return fkpspopushort(fk);
}

template<>	inline int fkpspop(fuck * fk)
{
	return fkpspopint(fk);
}

template<>	inline unsigned int fkpspop(fuck * fk)
{
	return fkpspopuint(fk);
}

template<>	inline float fkpspop(fuck * fk)
{
	return fkpspopfloat(fk);
}

template<>	inline double fkpspop(fuck * fk)
{
	return fkpspopdouble(fk);
}

template<>	inline std::string fkpspop(fuck * fk)
{
	return fkpspopcstrptr(fk);
}

template<>	inline bool fkpspop(fuck * fk)
{
	return fkpspopbool(fk);
}

template<>	inline int64_t fkpspop(fuck * fk)
{
	return fkpspopint64(fk);
}

template<>	inline uint64_t fkpspop(fuck * fk)
{
	return fkpspopuint64(fk);
}

FUCK_API void fkpsclear(fuck * fk);

// 此函数内部使用，推荐使用模板
FUCK_API void fkrun(fuck * fk, const char * func);

// 调用函数，解释执行
template<typename RVal>
RVal fkrun(fuck * fk, const char * func)
{
    fkrun(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrun(fuck * fk, const char * func, T1 arg1)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkrun(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrun(fuck * fk, const char * func, T1 arg1, T2 arg2)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkrun(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrun(fuck * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkrun(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrun(fuck * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkrun(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrun(fuck * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkpspush<T5>(fk, arg5);
    fkrun(fk, func);
    return fkpspop<RVal>(fk);
}


// 此函数内部使用，推荐使用模板
FUCK_API void fkrunjit(fuck * fk, const char * func);

// 调用函数,native code
template<typename RVal>
RVal fkrunjit(fuck * fk, const char * func)
{
    fkrunjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrunjit(fuck * fk, const char * func, T1 arg1)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkrunjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrunjit(fuck * fk, const char * func, T1 arg1, T2 arg2)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkrunjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrunjit(fuck * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkrunjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrunjit(fuck * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkrunjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrunjit(fuck * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkpspush<T5>(fk, arg5);
    fkrunjit(fk, func);
    return fkpspop<RVal>(fk);
}

