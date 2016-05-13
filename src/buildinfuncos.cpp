#include "buildinfuncos.h"
#include "fake.h"
#include "fakescript.h"
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#else 
#include <unistd.h>
#endif

// sleep
void buildin_os_sleep(fake * fk, interpreter * inter)
{
	BIF_CHECK_ARG_NUM(1);

	int millionseconds = fkpspop<int>(fk);
#if defined(WIN32)
	::Sleep(millionseconds);
#else
	usleep(millionseconds * 1000);
#endif
	fkpspush<int>(fk, 0);
}

// argc
void buildin_os_argc(fake * fk, interpreter * inter)
{
	fkpspush<int>(fk, fk->bif.get_argc());
}

// argv
void buildin_os_argv(fake * fk, interpreter * inter)
{
	BIF_CHECK_ARG_NUM(1);

	int pos = fkpspop<int>(fk);
	const char * arg = fk->bif.get_argv(pos);
	if (arg)
	{
		fkpspush<const char *>(fk, arg);
	}
	else
	{
		fkpspush<const char *>(fk, "");
	}
}

void buildinfuncos::openosfunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("os_sleep"), buildin_os_sleep);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("os_argc"), buildin_os_argc);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("os_argv"), buildin_os_argv);
}

void buildinfuncos::setargv(int argc, const char *argv[])
{
	m_argc = argc;
	m_argv = argv;
}

int buildinfuncos::get_argc() const
{
	return m_argc;
}

const char * buildinfuncos::get_argv(int pos) const
{
	if (pos >= 0 && pos < m_argc)
	{
		return m_argv[pos];
	}
	return 0;
}
