#include "types.h"

namespace fscript
{

int32_t fkatoi(String * p)
{
    if (p)
    {
        return atoi(p->c_str());
    }
    return 0;
}

int64_t fkatol(String * p)
{
    if (p)
    {
        return atoll(p->c_str());
    }
    return 0;
}

float fkatof(String * p)
{
    if (p)
    {
        return (float)atof(p->c_str());
    }
    return 0;
}

double fkatod(String * p)
{
    if (p)
    {
        return atof(p->c_str());
    }
    return 0;
}

String fkitoa(int64_t d)
{
    char buff[100];
    sprintf(buff, "%lld", (long long int)d);
    return buff;
}

String fkdtoa(double d)
{
    char buff[100];
    sprintf(buff, "%f", d);
    return buff;
}

uint32_t fkstrhash(String * p)
{
    uint32_t hashv = 0;
    if (p)
    {
        const char * pstr = p->c_str();
        for (int i = 0; i < (int)p->size(); i++)
        {
            hashv = ((hashv << 5) + hashv) + pstr[i]; /* hash * 33 + c */
        }
    }
    return hashv;
}

}

