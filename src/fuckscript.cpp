#include "fuckscript.h"

namespace fscript
{

efkerror g_fkerror;

efkerror fkerror()
{
    return g_fkerror;
}

// 解析文件
bool fkparse(const char * filename)
{
    return true;
}

// 解析文件
Variant fkcall(const char * func, Variant * arg, int argnum)
{
    return Variant();
}

}

