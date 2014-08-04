#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "variant.h"

namespace fscript
{

// 错误号
enum fkerror
{
    fk_ok,
};

fkerror fkerror();

// 解析文件
bool fkparse(const char * filename);

// 解析文件
Variant fkcall(const char * func, Variant * arg, int argnum);

}

