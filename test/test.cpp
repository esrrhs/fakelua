#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "fakescript.h"
#ifndef WIN32
#include "gperftools/profiler.h"
#endif

int cfunc1(int a, int b)
{
    return a - b;
}

class class1
{
public:
    class1()
    {
        b = 10;
    }
    int memfunc1(int a)
    {
        return a - b;
    }
private:
    int b;
};

void error_log(fake * fk, int eno, const char * str)
{
    printf("fake error[%d] in file(%s:%d) func(%s) : ", eno, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk));
    printf("%s\n", str);
    printf("call stack :\n%s\n", fkgetcurcallstack(fk));
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    
    fake * fk = newfake();

	fksetargv(fk, argc - 2, argv + 2);
	fkopenalllib(fk);
#ifdef _DEBUG
    fkopenprofile(fk);
#endif
    fkseterrorfunc(fk, error_log);

    fkreg(fk, "cfunc1", cfunc1); 
    class1 c1;
    fkreg(fk, "memfunc1", &class1::memfunc1); 

    fkparse(fk, argv[1]);
    if (fkerror(fk))
    {
        return 0;
    }

	int ret = 0;

	unsigned int begin,end;

    // inter
	begin = time(0);

#ifndef WIN32
#ifndef _DEBUG
	ProfilerStart("testinter.prof");
#endif
#endif
		
#ifndef _DEBUG
	for (int i = 0; i < 9000000; i++)
#else
	for (int i = 0; i < 1; i++)
#endif
	{
		ret = fkrun<int>(fk, "myfunc1", 1, 2);
		if (fkerror(fk))
		{
			return 0;
		}
	}

#ifndef WIN32
#ifndef _DEBUG
	ProfilerStop();
#endif
#endif
	
	end = time(0);

	printf("call ret %d %d\n", ret, end - begin);
#ifdef _DEBUG
	printf("call profile:\n%s", fkdumpprofile(fk));
#endif

	delfake(fk);

	char c;
	std::cin >> c;

    return 0;
}

