#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "variant.h"

namespace fscript
{

// 错误号
enum efkerror
{
    efk_ok,
};

efkerror fkerror();

// 解析文件
bool fkparse(const char * filename);

// 解析文件
Variant fkcall(const char * func, Variant * arg, int argnum);

}

