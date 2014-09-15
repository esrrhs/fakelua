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
    efk_compile_stack_identifier_error,
    efk_compile_stmt_type_error,
	efk_compile_variable_not_found,
	efk_compile_explicit_type_error,
	efk_compile_math_type_error,
	efk_compile_variable_has_define,
	efk_compile_add_stack_identifier,

	efk_run_no_func_error,
};

// 错误代码
#pragma pack(1)
struct fkerrorinfo
{
    efkerror fkerror()
    {
        return (efkerror)errorno;
    }
    const char * fkerrorstr()
    {
        return errorstr;
    }
    void clear()
    {
        errorno = 0;
        errorstr[0] = 0;
    }
    int errorno;
    char errorstr[efk_strsize];
};
#pragma pack()

// 脚本环境
struct fuck;

// 二进制
struct binary;

// 参数传递
struct paramstack;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

#define FUCK_API extern "C"

// 申请回收
FUCK_API fuck * newfuck(fkmalloc fkm = 0, fkfree fkf = 0,
    int per_frame_cmd_num = 0,
    int delete_routine_scale = 0
    );
FUCK_API void delfuck(fuck * fk);

// 解析文件
FUCK_API binary * fkparse(fuck * fk, fkerrorinfo * ei, const char * filename);

// bin回收
FUCK_API void delbinary(binary * bin);

// 是否bin有函数
FUCK_API bool fkisfunc(binary * bin, const char * func);

// 参数传递
FUCK_API paramstack * fknewparamstack(binary * bin);
FUCK_API void fkdeleteparamstack(paramstack * s);

FUCK_API void fkpspushpointer(paramstack * s, void * p);
FUCK_API void fkpspushchar(paramstack * s, char ret);
FUCK_API void fkpspushuchar(paramstack * s, unsigned char ret);
FUCK_API void fkpspushshort(paramstack * s, short ret);
FUCK_API void fkpspushushort(paramstack * s, unsigned short ret);
FUCK_API void fkpspushint(paramstack * s, int ret);
FUCK_API void fkpspushuint(paramstack * s, unsigned int ret);
FUCK_API void fkpspushfloat(paramstack * s, float ret);
FUCK_API void fkpspushdouble(paramstack * s, double ret);
FUCK_API void fkpspushcharptr(paramstack * s, char * ret);
FUCK_API void fkpspushccharptr(paramstack * s, const char * ret);
FUCK_API void fkpspushbool(paramstack * s, bool ret);
FUCK_API void fkpspushint64(paramstack * s, int64_t ret);
FUCK_API void fkpspushuint64(paramstack * s, uint64_t ret);

template<typename T>  
inline void fkpspush(paramstack * s, T ret)
{ 
    fkpspushpointer(s, (void*)ret); 
}
template<>	inline void fkpspush(paramstack * s, char ret)
{
	fkpspushchar(s, ret);
}

template<>	inline void fkpspush(paramstack * s, unsigned char ret)
{
	fkpspushuchar(s, ret);
}

template<>	inline void fkpspush(paramstack * s, short ret)
{
	fkpspushshort(s, ret);
}

template<>	inline void fkpspush(paramstack * s, unsigned short ret)
{
	fkpspushushort(s, ret);
}

template<>	inline void fkpspush(paramstack * s, int ret)
{
	fkpspushint(s, ret);
}

template<>	inline void fkpspush(paramstack * s, unsigned int ret)
{
	fkpspushuint(s, ret);
}

template<>	inline void fkpspush(paramstack * s, float ret)
{
	fkpspushfloat(s, ret);
}

template<>	inline void fkpspush(paramstack * s, double ret)
{
	fkpspushdouble(s, ret);
}

template<>	inline void fkpspush(paramstack * s, std::string ret)
{
	fkpspushccharptr(s, ret.c_str());
}

template<>	inline void fkpspush(paramstack * s, char * ret)
{
	fkpspushcharptr(s, ret);
}

template<>	inline void fkpspush(paramstack * s, const char * ret)
{
	fkpspushccharptr(s, ret);
}

template<>	inline void fkpspush(paramstack * s, bool ret)
{
	fkpspushbool(s, ret);
}

template<>	inline void fkpspush(paramstack * s, int64_t ret)
{
	fkpspushint64(s, ret);
}

template<>	inline void fkpspush(paramstack * s, uint64_t ret)
{
	fkpspushuint64(s, ret);
}


FUCK_API void fkpspoppointer(paramstack * s, void * & p);
FUCK_API char fkpspopchar(paramstack * s);
FUCK_API unsigned char fkpspopuchar(paramstack * s);
FUCK_API short fkpspopshort(paramstack * s);
FUCK_API unsigned short fkpspopushort(paramstack * s);
FUCK_API int fkpspopint(paramstack * s);
FUCK_API unsigned int fkpspopuint(paramstack * s);
FUCK_API float fkpspopfloat(paramstack * s);
FUCK_API double fkpspopdouble(paramstack * s);
FUCK_API const char * fkpspopcstrptr(paramstack * s);
FUCK_API bool fkpspopbool(paramstack * s);
FUCK_API int64_t fkpspopint64(paramstack * s);
FUCK_API uint64_t fkpspopuint64(paramstack * s);

template<typename T>  
inline T fkpspop(paramstack * s)
{ 
    void * ret = 0; 
    fkpspoppointer(s, ret); 
    return (T)ret; 
}

template<>	inline void fkpspop(paramstack * s)
{
	// nothing
}

template<>	inline char fkpspop(paramstack * s)
{
	return fkpspopchar(s);
}

template<>	inline unsigned char fkpspop(paramstack * s)
{
	return fkpspopuchar(s);
}

template<>	inline short fkpspop(paramstack * s)
{
	return fkpspopshort(s);
}

template<>	inline unsigned short fkpspop(paramstack * s)
{
	return fkpspopushort(s);
}

template<>	inline int fkpspop(paramstack * s)
{
	return fkpspopint(s);
}

template<>	inline unsigned int fkpspop(paramstack * s)
{
	return fkpspopuint(s);
}

template<>	inline float fkpspop(paramstack * s)
{
	return fkpspopfloat(s);
}

template<>	inline double fkpspop(paramstack * s)
{
	return fkpspopdouble(s);
}

template<>	inline std::string fkpspop(paramstack * s)
{
	return fkpspopcstrptr(s);
}

template<>	inline bool fkpspop(paramstack * s)
{
	return fkpspopbool(s);
}

template<>	inline int64_t fkpspop(paramstack * s)
{
	return fkpspopint64(s);
}

template<>	inline uint64_t fkpspop(paramstack * s)
{
	return fkpspopuint64(s);
}


// 调用函数
FUCK_API void fkrun(binary * bin, fkerrorinfo * ei, const char * func, paramstack * s);

template<typename RVal>
RVal fkrun(binary * bin, fkerrorinfo * ei, const char * func)
{
    paramstack * s = fknewparamstack(bin);
    fkrun(bin, ei, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1>
RVal fkrun(binary * bin, fkerrorinfo * ei, const char * func, T1 arg1)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkrun(bin, ei, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2>
RVal fkrun(binary * bin, fkerrorinfo * ei, const char * func, T1 arg1, T2 arg2)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkrun(bin, ei, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrun(binary * bin, fkerrorinfo * ei, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkpspush<T3>(s, arg3);
    fkrun(bin, ei, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrun(binary * bin, fkerrorinfo * ei, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkpspush<T3>(s, arg3);
    fkpspush<T4>(s, arg4);
    fkrun(bin, ei, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrun(binary * bin, fkerrorinfo * ei, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkpspush<T3>(s, arg3);
    fkpspush<T4>(s, arg4);
    fkpspush<T5>(s, arg5);
    fkrun(bin, ei, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

