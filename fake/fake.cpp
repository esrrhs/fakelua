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
	// ½âÎö²ÎÊý
    if (argc < 2)
    {
		printf("arg : [-hjpgt] file [arg]\n");
        return 0;
    }
    
    fake * fk = newfake();
    bool isopenprofile = false;
	bool isopenjit = false;
	bool isopengoogleprofile = false;
	int testnum = 1;
	if (argv[1][0] == '-')
	{
		if (strstr(argv[1], "h"))
		{
			printf("fakescript\n\narg : [-hjpgt] file [arg]\n-h help\n-j open jit\n-p open profile\n-g open google profile\n-t open test mode\nsample:./fake -jp a.fk\n\n");
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
    
    fkparse(fk, argv[1]);
    if (fkerror(fk))
    {
        return 0;
    }
	
#ifndef WIN32
	if (isopengoogleprofile)
	{
		ProfilerStart("fake.prof");
	}
#endif

	for (int i = 0; i < testnum; i++)
	{
		if (!isopenjit)
		{
			fkrun<int>(fk, "main");
		}
		else
		{
			fkrunjit<int>(fk, "main");
		}
	}
	
#ifndef WIN32
	if (isopengoogleprofile)
	{
		ProfilerStop();
	}
#endif

	if (fkerror(fk))
	{
		return 0;
	}

	if (isopenprofile)
	{
		printf("\n%s", fkdumpprofile(fk));
	}

	delfake(fk);

    return 0;
}

