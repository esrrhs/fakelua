#include "types.h"
#include "bison.h"

void fklog(const char * header, const char * file, const char * func, int pos, const char *fmt, ...)
{
	FILE *pLog = NULL;
	time_t clock1;
	struct tm * tptr;
	va_list ap;
	
	pLog = fopen("fuckscript.log", "a+");
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

int32_t fkatoi(const String * p)
{
    if (p)
    {
        return atoi(p->c_str());
    }
    return 0;
}

int64_t fkatol(const String * p)
{
    if (p)
    {
        return atoll(p->c_str());
    }
    return 0;
}

float fkatof(const String * p)
{
    if (p)
    {
        return (float)atof(p->c_str());
    }
    return 0;
}

double fkatod(const String * p)
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

String fkxtoa(int64_t d)
{
    String ret;
    
	char tmpbuf[32]={0};
	uint32_t idx = 31;

	// special case '0'

	if (!d)
	{
		tmpbuf[30] = '0';
		ret = &tmpbuf[30];
	}
    else
    {
    	// add numbers
    	const uint8_t chars[]="0123456789ABCDEF";
    	while(d && idx)
    	{
    		--idx;
    		tmpbuf[idx] = chars[(d % 16)];
    		d /= 16;
    	}

    	ret = &tmpbuf[idx];
    }

    if (ret.size() < 16)
    {
        String tmp;
        tmp.insert(0, 16 - ret.size(), '0');
        ret = tmp + ret;
    }

    String tmp = "0x";
    ret = tmp + ret;
	return ret;
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
        default:
            return fkitoa(token);
    }
    #undef TOKEN_SWITCH
}

