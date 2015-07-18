#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "fakescript.h"
#include <string.h>
#ifndef WIN32
#include "gperftools/profiler.h"
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

// for test
test_class1 * new_test_class1()
{
	return new test_class1();
}

void error_log(fake * fk, int eno, const char * str)
{
	printf("fake error[%d] in file(%s:%d) func(%s) : ", eno, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk));
	printf("%s\n", str);
	printf("call stack :\n%s\n", fkgetcurcallstack(fk));
}

int main(int argc, const char *argv[])
{
	// 解析参数
	if (argc < 2)
	{
		printf("try -h for help\n");
		return 0;
	}
	
	fake * fk = newfake();

	// 开关选项
	bool isopenprofile = false;
	bool isopenjit = false;
	bool isopengoogleprofile = false;
	int testnum = 1;
	bool issave = false;
	bool isload = false;
	bool isprintoutput = false;
	if (argv[1][0] == '-')
	{
		if (strstr(argv[1], "h"))
		{
			printf("\
fakescript\n\narg : [-hjpgtsl] file [arg]\n\
-h help\n\
-j open jit\n\
-p open profile\n\
-g open google profile\n\
-t open test mode\n\
-s save func to fake.bin\n\
-l load func from file\n\
-o print return value and time\n\
sample:./fake a.fk\n\n");
			return 0;
		}
		
		if (strstr(argv[1], "j"))
		{
			fkopenjit(fk);
			isopenjit = true;
		}
		
		if (strstr(argv[1], "p"))
		{
			fkopenprofile(fk);
			isopenprofile = true;
		}
		
#ifndef WIN32
		if (strstr(argv[1], "g"))
		{
			isopengoogleprofile = true;
		}
#endif

		if (strstr(argv[1], "t"))
		{
			testnum = 9000000;
		}
		
		if (strstr(argv[1], "s"))
		{
			issave = true;
		}
		
		if (strstr(argv[1], "l"))
		{
			isload = true;
		}

		if (strstr(argv[1], "o"))
		{
			isprintoutput = true;
		}
		
		argc--;
		argv++;
	}

	fksetargv(fk, argc - 2, argv + 2);
	fkopenalllib(fk);
	fkseterrorfunc(fk, error_log);

	// for test
	fkreg(fk, "test_cfunc1", test_cfunc1);
	fkreg(fk, "new_test_class1", new_test_class1);
	test_class1 c1;
	fkreg(fk, "test_memfunc1", &test_class1::test_memfunc1); 
	
	// 解析文件
	if (!isload)
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
		FILE * fp = fopen(argv[1], "rb");
		if (!fp)
		{
			printf("open %s for read fail\n", argv[1]);
			return -1;
		}

		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		char * tmpbuf = (char *)malloc(size);
		if (fread(tmpbuf, size, 1, fp) != 1)
		{
			printf("read %s fail\n", argv[1]);
			return -1;
		}

		if (fkloadfunc(fk, tmpbuf, size) == -1)
		{
			printf("load func fail\n");
			return -1;
		}

		fclose(fp);
		free(tmpbuf);
		printf("load from %s ok, size %d\n", argv[1], size);
	}

	// 存文件
	if (issave)
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
		FILE * fp = fopen(filename, "w");
		if (!fp)
		{
			printf("open %s for write fail\n", filename);
			return -1;
		}

		if (fwrite(tmpbuf, size, 1, fp) != 1)
		{
			printf("write %s fail\n", filename);
			return -1;
		}

		fclose(fp);
		free(tmpbuf);
		
		printf("save to %s ok, size %d\n", filename, size);
		
		return 0;
	}

#ifndef WIN32
	if (isopengoogleprofile)
	{
		ProfilerStart("fake.prof");
	}
#endif

	uint32_t begintime = time(0);
	int fkret = 0;

	// run
	for (int i = 0; i < testnum; i++)
	{
		if (!isopenjit)
		{
			fkret = fkrun<int>(fk, "main");
		}
		else
		{
			fkret = fkrunjit<int>(fk, "main");
		}
	}
	
	uint32_t endtime = time(0);

#ifndef WIN32
	if (isopengoogleprofile)
	{
		ProfilerStop();
	}
#endif

	if (fkerror(fk))
	{
		return -1;
	}

	if (isopenprofile)
	{
		printf("\n%s", fkdumpprofile(fk));
	}

	if (isprintoutput)
	{
		printf("main return : %d, use time %d\n", fkret, endtime - begintime);
	}

	delfake(fk);

	return 0;
}

