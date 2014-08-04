#include "types.h"

namespace fscript
{

int32_t fkatoi(String * p)
{
    // todo
    if (p)
    {
        return atoi(p->c_str());
    }
    return 0;
}

int64_t fkatol(String * p)
{
    // todo
    if (p)
    {
        return atoi(p->c_str());
    }
    return 0;
}

float fkatof(String * p)
{
    // todo
    if (p)
    {
        return atof(p->c_str());
    }
    return 0;
}

double fkatod(String * p)
{
    // todo
    if (p)
    {
        return atof(p->c_str());
    }
    return 0;
}

String fkitoa(int64_t d)
{
    // todo
    return itoa(d);
}

String fkdtoa(double d)
{
    // todo
    return ftoa(d);
}

uint32_t fkstrhash(String * p)
{
    // todo
    return p;
}


}

