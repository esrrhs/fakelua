#include "types.h"
#include "bison.h"
#include "fake.h"
#include "variant.h"
#include "paramstack.h"

void fklog(const char * header, const char * file, const char * func, int pos, const char *fmt, ...)
{
	FILE *pLog = NULL;
	time_t clock1;
	struct tm * tptr;
	va_list ap;
	
	pLog = fopen("fakescript.log", "a+");
	if (pLog == NULL)
	{
		return;
	}
	
	clock1 = time(0);
	tptr = localtime(&clock1);

	fprintf(pLog, "===========================[%d.%d.%d, %d.%d.%d]%s:%d,%s:===========================\n%s", 
		tptr->tm_year+1990,tptr->tm_mon+1,
		tptr->tm_mday,tptr->tm_hour,tptr->tm_min,
		tptr->tm_sec,file,pos,func,header);

	va_start(ap, fmt);
	vfprintf(pLog, fmt, ap);
	fprintf(pLog, "\n\n");
	va_end(ap);

	fclose(pLog);
}

String fkget_token_name(int token)
{
#define TOKEN_SWITCH(x) case x: return #x;
    switch (token)
    {
        TOKEN_SWITCH(VAR_BEGIN)
        TOKEN_SWITCH(RETURN)
        TOKEN_SWITCH(BREAK)
        TOKEN_SWITCH(FUNC)
        TOKEN_SWITCH(WHILE)
        TOKEN_SWITCH(TRUE)
        TOKEN_SWITCH(FALSE)
        TOKEN_SWITCH(IF)
        TOKEN_SWITCH(THEN)
        TOKEN_SWITCH(ELSE)
        TOKEN_SWITCH(END)
        TOKEN_SWITCH(STRING_DEFINITION)
        TOKEN_SWITCH(IDENTIFIER)
        TOKEN_SWITCH(NUMBER)
        TOKEN_SWITCH(SINGLE_LINE_COMMENT)
        TOKEN_SWITCH(DIVIDE_MOD)
        TOKEN_SWITCH(ARG_SPLITTER)
        TOKEN_SWITCH(PLUS)
        TOKEN_SWITCH(MINUS)
        TOKEN_SWITCH(DIVIDE)
        TOKEN_SWITCH(MULTIPLY)
        TOKEN_SWITCH(ASSIGN)
        TOKEN_SWITCH(MORE)
        TOKEN_SWITCH(LESS)
        TOKEN_SWITCH(MORE_OR_EQUAL)
        TOKEN_SWITCH(LESS_OR_EQUAL)
        TOKEN_SWITCH(EQUAL)
        TOKEN_SWITCH(NOT_EQUAL)
        TOKEN_SWITCH(OPEN_BRACKET)
        TOKEN_SWITCH(CLOSE_BRACKET)
        TOKEN_SWITCH(FLOAT)
        TOKEN_SWITCH(PLUS_ASSIGN)
        TOKEN_SWITCH(MINUS_ASSIGN)
        TOKEN_SWITCH(DIVIDE_ASSIGN)
		TOKEN_SWITCH(MULTIPLY_ASSIGN)
		TOKEN_SWITCH(DIVIDE_MOD_ASSIGN)
		TOKEN_SWITCH(FOR)
        default:
            return fkitoa(token);
    }
    #undef TOKEN_SWITCH
}

void * safe_fkmalloc(fake * fk, size_t size)
{
    if (fk && size)
    {
        return fk->cfg.fkm(size);
    }
    return 0;
}

void safe_fkfree(fake * fk, const void * p)
{
    if (fk && p)
    {
        fk->cfg.fkf((void *)p);
    }
}

void seterror(fake * fk, efkerror err, const char *fmt, ...)
{
    fk->errorno = err;
	va_list ap;
	va_start(ap, fmt);
    vsnprintf(fk->errorstr, sizeof(fk->errorstr) - 1, fmt, ap);
	va_end(ap);
	fk->errorstr[sizeof(fk->errorstr) - 1] = 0;
}

String vartostring(const variant * v)
{
    String s;
    V_TOSTRING(v, s);
    return s;
}

paramstack * getps(fake * fk)
{
    return &fk->ps;
}

char * stringdump(fake * fk, const char * src, size_t sz)
{
	char * s = (char*)safe_fkmalloc(fk, sz + 1);
	memcpy(s, src, sz);
	s[sz] = 0;
	return s;
}

