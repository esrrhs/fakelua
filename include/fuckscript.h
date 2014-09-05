#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>

// 错误号
enum efkerror
{
    efk_ok,
    
    efk_open_file_fail,
    efk_open_file_empty,
    efk_parse_file_fail,
    
    efk_compile_same_func_name,
    efk_compile_func_arg_error,
    efk_compile_stmt_type_error,
	efk_compile_variable_not_found,
	efk_compile_explicit_type_error,

	efk_run_no_func_error,
};

// 脚本环境
struct fuck;

// 二进制
struct binary;

// 参数传递
struct paramstack;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

// 申请回收
fuck * newfuck(fkmalloc fkm = 0, fkfree fkf = 0);
void delfuck(fuck * fk);

// 错误
efkerror fkerror(fuck * fk);
const char * fkerrorstr(fuck * fk);

// 解析文件
binary * fkparse(fuck * fk, const char * filename);

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
void fkrun(binary * bin, const char * func, paramstack * s);

template<typename RVal>
RVal fkrun(binary * bin, const char * func)
{
    paramstack * s = fknewparamstack(bin);
    fkrun(bin, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1>
RVal fkrun(binary * bin, const char * func, T1 arg1)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkrun(bin, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2>
RVal fkrun(binary * bin, const char * func, T1 arg1, T2 arg2)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkrun(bin, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrun(binary * bin, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkpspush<T3>(s, arg3);
    fkrun(bin, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrun(binary * bin, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkpspush<T3>(s, arg3);
    fkpspush<T4>(s, arg4);
    fkrun(bin, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrun(binary * bin, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    paramstack * s = fknewparamstack(bin);
    fkpspush<T1>(s, arg1);
    fkpspush<T2>(s, arg2);
    fkpspush<T3>(s, arg3);
    fkpspush<T4>(s, arg4);
    fkpspush<T5>(s, arg5);
    fkrun(bin, func, s);
    RVal ret = fkpspop<RVal>(s);
    fkdeleteparamstack(s);
    return ret;
}

