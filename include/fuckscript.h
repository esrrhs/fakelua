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
    efk_openfilefail,
    efk_openfileempty,
};

struct fuck;

// 申请回收
fuck * newfuck();
void delfuck(fuck * fk);

// 错误
efkerror fkerror(fuck * fk);

// 解析文件
bool fkparse(fuck * fk, const char * filename);

