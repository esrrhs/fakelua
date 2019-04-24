#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "fake-inc.h"
#include <string.h>
#include <vector>
#include <algorithm>
#ifdef WIN32
#else
#ifdef USE_GOOGLE_POFILER
#include "gperftools/profiler.h"
#endif
#endif

// for test
int test_cfunc1(int a, int b)
{
	return a - b;
}

// for test
class test_class1
{
public:
	test_class1()
	{
		b = 10;
	}
	int test_memfunc1(int a)
	{
		return a - b;
	}
private:
	int b;
};
class test_class2
{
public:
	test_class2()
	{
		b = 10;
	}
	int test_memfunc1(int a)
	{
		return a + b;
	}
	int test_memfunc2(int a)
	{
		return a * b;
	}
	test_class2 * self()
	{
		return this;
	}
private:
	int b;
};

// for test
test_class1 * new_test_class1()
{
	return new test_class1();
}
void delete_test_class1(test_class1 * p)
{
	delete p;
}
test_class2 * new_test_class2()
{
	return new test_class2();
}
void delete_test_class2(test_class2 * p)
{
	delete p;
}

void error_log(fake * fk, int eno, const char * file, int lineno, const char * func, const char * str)
{
	printf("fake error[%d] in file(%s:%d) func(%s) : ", eno, file, lineno, func);
	printf("%s\n", str);
	printf("call stack :\n%s\n", fkgetcurcallstack(fk));
	printf("cur routine :\n%s\n", fkgetcurroutine(fk));
}

static const char * g_replacebuff = "start compile";

// 函数
bool readfile(const char * filename, int addsize, char * & buf, int & filesize);
bool writefile(const char * filename, const char * buf, int filesize);
int parsearg(fake * fk, int & argc, const char ** & argv);

// 开关选项
bool g_isopenprofile = false;
bool g_isopenjit = false;
bool g_isopengoogleprofile = false;
int g_testnum = 1;
bool g_issave = false;
bool g_isload = false;
bool g_isprintoutput = false;
bool g_iscompile = false;
bool g_iscompiled = false;
bool g_isstep = false;
const char * g_selfname = 0;

int main(int argc, const char *argv[])
{
	fake * fk = newfake();

	// 解析参数
	int parseret = parsearg(fk, argc, argv);
	if (parseret != 0)
	{
		return parseret;
	}

	if (g_iscompiled)
	{
		fksetargv(fk, argc - 1, argv + 1);
	}
	else
	{
		fksetargv(fk, argc - 2, argv + 2);
	}
	fkopenalllib(fk);
	fkseterrorfunc(fk, error_log);

	// for test
	fkreg(fk, "test_cfunc1", test_cfunc1);
	fkreg(fk, "new_test_class1", new_test_class1);
	fkreg(fk, "new_test_class2", new_test_class2);
	fkreg(fk, "delete_test_class1", delete_test_class1);
	fkreg(fk, "delete_test_class2", delete_test_class2);
	fkreg(fk, "test_memfunc1", &test_class1::test_memfunc1); 
	fkreg(fk, "test_memfunc1", &test_class2::test_memfunc1); 
	fkreg(fk, "test_memfunc2", &test_class2::test_memfunc2); 
	fkreg(fk, "self", &test_class2::self); 

	// 编译后文件
	if (g_iscompiled)
	{
		// 打开自己
		char * selftmpbuf = 0;
		int selfsize = 0;
		if (!readfile(g_selfname, 0, selftmpbuf, selfsize))
		{
			return -1;
		}

		int startpos = 0;
		int size = 0;
		memcpy(&startpos, g_replacebuff, sizeof(startpos));
		memcpy(&size, g_replacebuff + 4, sizeof(size));

		if (fkloadfunc(fk, selftmpbuf + startpos, size) == -1)
		{
			printf("load func fail\n");
			return -1;
		}

		free(selftmpbuf);
	}
	// 解析文件
	else if (!g_isload)
	{
		fkparse(fk, argv[1]);
		if (fkerror(fk))
		{
			return -1;
		}
	}
	// 读文件
	else
	{
		// 读文件
		char * tmpbuf = 0;
		int size = 0;
		if (!readfile(argv[1], 0, tmpbuf, size))
		{
			return -1;
		}

		if (fkloadfunc(fk, tmpbuf, size) == -1)
		{
			printf("load func fail\n");
			return -1;
		}

		free(tmpbuf);
		printf("load from %s ok, size %d\n", argv[1], size);
	}

	// 存文件
	if (g_issave)
	{
		int tmpsize = 1024 * 1024;
		char * tmpbuf = (char *)malloc(tmpsize);
		int size = fksavefunc(fk, tmpbuf, tmpsize);
		if (size == -1)
		{
			printf("save func fail\n");
			return -1;
		}

		const char * filename = "fake.bin";
		if (!writefile(filename, tmpbuf, size))
		{
			return -1;
		}
		free(tmpbuf);
		
		printf("save to %s ok, size %d\n", filename, size);
		
		return 0;
	}

	// 编译到一起
	if (g_iscompile)
	{
		int tmpsize = 1024 * 1024;
		char * tmpbuf = (char *)malloc(tmpsize);
		int size = fksavefunc(fk, tmpbuf, tmpsize);
		if (size == -1)
		{
			printf("compile func fail\n");
			return -1;
		}

		// 打开自己
		char * selftmpbuf = 0;
		int selfsize = 0;
		if (!readfile(g_selfname, size, selftmpbuf, selfsize))
		{
			return -1;
		}

		// 替换内存中的字符串
		bool isfind = false;
		for (int i = 0; i < selfsize; i++)
		{
			if (memcmp(selftmpbuf + i, g_replacebuff, strlen(g_replacebuff)) == 0)
			{
				memcpy(selftmpbuf + i, &selfsize, sizeof(selfsize));
				memcpy(selftmpbuf + i + 4, &size, sizeof(size));
				isfind = true;
				break;
			}
		}
		if (!isfind)
		{
			printf("replace %s pos fail\n", argv[0]);
			return -1;
		}

		// 复制后面的
		memcpy(selftmpbuf + selfsize, tmpbuf, size);
		free(tmpbuf);

		// 输出
		const char * filename = "out.exe";
		if (!writefile(filename, selftmpbuf, selfsize + size))
		{
			return -1;
		}
		free(selftmpbuf);

		printf("compile to %s ok, size %d\n", filename, selfsize + size);

		return 0;
	}

#ifndef WIN32
#ifdef USE_GOOGLE_POFILER
	if (g_isopengoogleprofile)
	{
		ProfilerStart("fake.prof");
	}
#endif
#endif

	uint32_t begintime = time(0);
	int fkret = 0;

	// run
	for (int i = 0; i < g_testnum; i++)
	{
		if (!g_isopenjit)
		{
			if (g_isstep)
			{
				fkret = fkdebugrun<int>(fk, "main");
			}
			else
			{
				fkret = fkrun<int>(fk, "main");
			}
		}
		else
		{
			fkret = fkrunjit<int>(fk, "main");
		}
	}
	
	uint32_t endtime = time(0);

#ifndef WIN32
#ifdef USE_GOOGLE_POFILER
	if (g_isopengoogleprofile)
	{
		ProfilerStop();
	}
#endif
#endif

	if (fkerror(fk))
	{
		return -1;
	}

	if (g_isopenprofile)
	{
		printf("\n%s", fkdumpprofile(fk));
	}

	if (g_isprintoutput)
	{
		printf("main return : %d, use time %d\n", fkret, endtime - begintime);
	}

	delfake(fk);

	return 0;
}

bool readfile(const char * filename, int addsize, char * & buf, int & filesize)
{
	// 打开
	FILE * fp = fopen(filename, "rb");
	if (!fp)
	{
		printf("open %s for read fail\n", filename);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char * tmpbuf = (char *)malloc(size + addsize);
	if (fread(tmpbuf, size, 1, fp) != 1)
	{
		printf("read %s fail\n", filename);
		free(tmpbuf);
		return false;
	}
	fclose(fp);

	buf = tmpbuf;
	filesize = size;

	return true;
}

bool writefile(const char * filename, const char * buf, int filesize)
{
	FILE * fp = fopen(filename, "wb");
	if (!fp)
	{
		printf("open %s for write fail\n", filename);
		return false;
	}

	if (fwrite(buf, filesize, 1, fp) != 1)
	{
		printf("write %s fail\n", filename);
		return -1;
	}

	fclose(fp);

	return true;
}

int parsearg(fake * fk, int & argc, const char ** & argv)
{
	g_selfname = argv[0];

	// start compile，memcmp无解只能用这种
	if (g_replacebuff[0] != 's' ||
		g_replacebuff[1] != 't' ||
		g_replacebuff[2] != 'a' ||
		g_replacebuff[3] != 'r' ||
		g_replacebuff[4] != 't' ||
		g_replacebuff[5] != ' ' ||
		g_replacebuff[6] != 'c' ||
		g_replacebuff[7] != 'o' ||
		g_replacebuff[8] != 'm' ||
		g_replacebuff[9] != 'p' ||
		g_replacebuff[10] != 'i' ||
		g_replacebuff[11] != 'l' ||
		g_replacebuff[12] != 'e')
	{
		g_iscompiled = true;
	}
	else if (argc < 2)
	{
		printf("try -h for help\n");
		return -1;
	}

	if (argc >= 2 && argv[1][0] == '-')
	{
		if (strstr(argv[1], "h"))
		{
			printf("\
fake\n\narg : [-hjpgtslocd] file [arg]\n\
-h help\n\
-j open jit\n\
-p open profile\n\
-g open google profile\n\
-t open test mode\n\
-s save func to fake.bin\n\
-l load func from file\n\
-o print return value and time\n\
-c compile script to out.exe\n\
-d debug step mod\n\
sample:./fake a.fk\n\n");
			return -1;
		}

		if (strstr(argv[1], "j"))
		{
			fkopenjit(fk);
			g_isopenjit = true;
		}

		if (strstr(argv[1], "p"))
		{
			fkopenprofile(fk);
			g_isopenprofile = true;
		}

#ifndef WIN32
		if (strstr(argv[1], "g"))
		{
			g_isopengoogleprofile = true;
		}
#endif

		if (strstr(argv[1], "t"))
		{
			g_testnum = 9000000;
		}

		if (strstr(argv[1], "s"))
		{
			g_issave = true;
		}

		if (strstr(argv[1], "l"))
		{
			g_isload = true;
		}

		if (strstr(argv[1], "o"))
		{
			g_isprintoutput = true;
		}

		if (strstr(argv[1], "c"))
		{
			printf("%s\n", g_replacebuff);
			g_iscompile = true;
		}

		if (strstr(argv[1], "d"))
		{
			g_isstep = true;
		}

		argc--;
		argv++;
	}

	return 0;
}

