#include "fuck.h"

void fuck::seterror(fkerrorinfo * ei, efkerror errno, const char *fmt, ...)
{
    if (ei)
    {
        ei->errorno = errno;
    	va_list ap;
    	va_start(ap, fmt);
        vsnprintf(ei->errorstr, sizeof(ei->errorstr) - 1, fmt, ap);
    	va_end(ap);
    	ei->errorstr[sizeof(ei->errorstr) - 1] = 0;
    }
}

