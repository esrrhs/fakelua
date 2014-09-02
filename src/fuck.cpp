#include "fuck.h"

efkerror fuck::error()
{
    return m_efkerror;
}

const char * fuck::errorstr()
{
    return m_errorstr.c_str();
}

void fuck::seterror(efkerror errno, const char *fmt, ...)
{
    m_efkerror = errno;
    char buff[1024];
	va_list ap;
	va_start(ap, fmt);
    vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);
	m_errorstr = buff;
}
