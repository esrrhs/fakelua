#include "buildinfuncstring.h"
#include "fake.h"
#include "fakescript.h"

// find
void buildin_string_find(fake * fk, interpreter * inter)
{
	const char * findstr = fkpspopcstrptr(fk);
	int pos = fkpspop<int>(fk);
	const char * str = fkpspopcstrptr(fk);
	int len = str ? strlen(str) : 0;
	if (pos >= 0 && pos < len && str && findstr)
	{
		const char * find = strstr(str + pos, findstr);
		if (find)
		{
			fkpspush<bool>(fk, true);
			fkpspush<int>(fk, (int)(find - str));
		}
		else
		{
			fkpspush<bool>(fk, false);
			fkpspush<int>(fk, -1);
		}
	}
	else
	{
		fkpspush<bool>(fk, false);
		fkpspush<int>(fk, -1);
	}
}

// substr
void buildin_string_substr(fake * fk, interpreter * inter)
{
	int count = fkpspop<int>(fk);
	int pos = fkpspop<int>(fk);
	const char * str = fkpspopcstrptr(fk);
	int len = str ? strlen(str) : 0;
	if (count == -1)
	{
		count = 0x7FFFFFFF;
	}
	if (count < 0)
	{
		fkpspush<const char *>(fk, "");
		return;
	}
	if ((uint32_t)((uint32_t)pos + (uint32_t)count) > (uint32_t)len)
	{
		count = len - pos;
	}
	
	if (pos >= 0 && pos < len && str && count > 0)
	{
		char * buf = (char *)safe_fkmalloc(fk, count + 1);
		buf[count] = 0;
		memcpy(buf, str + pos, count);
		fkpspush<const char *>(fk, buf);
		safe_fkfree(fk, buf);
	}
	else
	{
		fkpspush<const char *>(fk, "");
	}
}

// trim
void buildin_string_trim(fake * fk, interpreter * inter)
{
	const char * trimstr = fkpspopcstrptr(fk);
	const char * str = fkpspopcstrptr(fk);
	if (trimstr && str)
	{
		// left
		int trimlen = strlen(trimstr);
		while (1)
		{
			const char * s = strstr(str, trimstr);
			if (s == str)
			{
				str += trimlen;
			}
			else
			{
				break;
			}
		}

		// right
		const char * srcstr = str;
		int endpos = strlen(str);
		int trimnum = 0;
		while (str < srcstr + endpos)
		{
			const char * s = strstr(str, trimstr);
			if (s == str)
			{
				str += trimlen;
				trimnum++;
			}
			else if (s)
			{
				trimnum = 0;
				str = s;
			}
			else
			{
				trimnum = 0;
				break;
			}
		}
		int afterlen = endpos - trimlen * trimnum;
		assert(afterlen >= 0);
		char * buf = (char *)safe_fkmalloc(fk, afterlen + 1);
		buf[afterlen] = 0;
		memcpy(buf, srcstr, afterlen);
		fkpspush<const char *>(fk, buf);
		safe_fkfree(fk, buf);
	}
	else
	{
		fkpspush<const char *>(fk, str);
	}
}

// trim left
void buildin_string_trim_left(fake * fk, interpreter * inter)
{
	const char * trimstr = fkpspopcstrptr(fk);
	const char * str = fkpspopcstrptr(fk);
	if (trimstr && str)
	{
		int trimlen = strlen(trimstr);
		while (1)
		{
			const char * s = strstr(str, trimstr);
			if (s == str)
			{
				str += trimlen;
			}
			else
			{
				break;
			}
		}
		fkpspush<const char *>(fk, str);
	}
	else
	{
		fkpspush<const char *>(fk, str);
	}
}

// trim
void buildin_string_trim_right(fake * fk, interpreter * inter)
{
	const char * trimstr = fkpspopcstrptr(fk);
	const char * str = fkpspopcstrptr(fk);
	if (trimstr && str)
	{
		int trimlen = strlen(trimstr);
		const char * srcstr = str;
		int endpos = strlen(str);
		int trimnum = 0;
		while (str < srcstr + endpos)
		{
			const char * s = strstr(str, trimstr);
			if (s == str)
			{
				str += trimlen;
				trimnum++;
			}
			else if (s)
			{
				trimnum = 0;
				str = s;
			}
			else
			{
				trimnum = 0;
				break;
			}
		}
		int afterlen = endpos - trimlen * trimnum;
		assert(afterlen >= 0);
		char * buf = (char *)safe_fkmalloc(fk, afterlen + 1);
		buf[afterlen] = 0;
		memcpy(buf, srcstr, afterlen);
		fkpspush<const char *>(fk, buf);
		safe_fkfree(fk, buf);
	}
	else
	{
		fkpspush<const char *>(fk, str);
	}
}

// replace
void buildin_string_replace(fake * fk, interpreter * inter)
{
	String tostr = fkpspopcstrptr(fk);
	String fromstr = fkpspopcstrptr(fk);
	String str = fkpspopcstrptr(fk);

	if (fromstr.empty())
	{
		fkpspush<const char *>(fk, str.c_str());
		return;
	}

	int pos = 0;
	while ((int)(pos = str.find(fromstr, pos)) != (int)str.npos)
	{
		str.replace(pos, fromstr.length(), tostr);
		pos += tostr.length();
	}
	
	fkpspush<const char *>(fk, str.c_str());
}

// cat
void buildin_string_cat(fake * fk, interpreter * inter)
{
	const char * right = fkpspopcstrptr(fk);
	String leftstr = fkpspopcstrptr(fk);
	leftstr += right;
	fkpspush<const char *>(fk, leftstr.c_str());
}

void buildinfuncstring::openstringfunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_find"), buildin_string_find);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_substr"), buildin_string_substr);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_trim"), buildin_string_trim);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_trimleft"), buildin_string_trim_left);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_trimright"), buildin_string_trim_right);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_replace"), buildin_string_replace);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("string_cat"), buildin_string_cat);
}
