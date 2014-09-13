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

// 申请回收
fuck * newfuck(fkmalloc fkm = 0, fkfree fkf = 0,
    int per_frame_cmd_num = 0,
    int delete_routine_scale = 0
    );
void delfuck(fuck * fk);

// 解析文件
binary * fkparse(fuck * fk, fkerrorinfo * ei, const char * filename);

// bin回收
void delbinary(binary * bin);

// 是否bin有函数
bool fkisfunc(binary * bin, const char * func);

// 参数传递
paramstack * fknewparamstack(binary * bin);
void fkdeleteparamstack(paramstack * s);

void fkpspushpointer(paramstack * s, void * p);
template<typename T>  
void fkpspush(paramstack * s, T ret)
{ 
    fkpspushpointer(s, (void*)ret); 
}
template<>	void fkpspush(paramstack * s, char ret);
template<>	void fkpspush(paramstack * s, unsigned char ret);
template<>	void fkpspush(paramstack * s, short ret);
template<>	void fkpspush(paramstack * s, unsigned short ret);
template<>	void fkpspush(paramstack * s, int ret);
template<>	void fkpspush(paramstack * s, unsigned int ret);
template<>	void fkpspush(paramstack * s, float ret);
template<>	void fkpspush(paramstack * s, double ret);
template<>	void fkpspush(paramstack * s, std::string ret);
template<>	void fkpspush(paramstack * s, char * ret);
template<>	void fkpspush(paramstack * s, const char * ret);
template<>	void fkpspush(paramstack * s, bool ret);
template<>	void fkpspush(paramstack * s, int64_t ret);
template<>	void fkpspush(paramstack * s, uint64_t ret);

void fkpspoppointer(paramstack * s, void * & p);
template<typename T>  
T fkpspop(paramstack * s) 
{ 
    void * ret = 0; 
    fkpspoppointer(s, ret); 
    return (T)ret; 
}
template<>	void fkpspop(paramstack * s);
template<>	char fkpspop(paramstack * s);
template<>	unsigned char fkpspop(paramstack * s);
template<>	short fkpspop(paramstack * s);
template<>	unsigned short fkpspop(paramstack * s);
template<>	int fkpspop(paramstack * s);
template<>	unsigned int fkpspop(paramstack * s);
template<>	float fkpspop(paramstack * s);
template<>	double fkpspop(paramstack * s);
template<>	std::string fkpspop(paramstack * s);
template<>	bool fkpspop(paramstack * s);
template<>	int64_t fkpspop(paramstack * s);
template<>	uint64_t fkpspop(paramstack * s);

// 调用函数
void fkrun(binary * bin, fkerrorinfo * ei, const char * func, paramstack * s);

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

