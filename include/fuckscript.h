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
};

struct fuck;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

// 申请回收
fuck * newfuck(fkmalloc fkm = 0, fkfree fkf = 0);
void delfuck(fuck * fk);

// 错误
efkerror fkerror(fuck * fk);

// 解析文件
bool fkparse(fuck * fk, const char * filename);

