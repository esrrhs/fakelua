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

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    
    fake * fk = newfake();

    fkopenbaselib(fk);
#ifdef _DEBUG
    fkopenprofile(fk);
#endif

    fkreg(fk, "cfunc1", cfunc1); 
    class1 c1;
    fkreg(fk, "memfunc1", &class1::memfunc1); 

    fkparse(fk, argv[1]);
    if (fkerror(fk))
    {
        printf("parse error %d, %s\n", fkerror(fk), fkerrorstr(fk));
        return 0;
    }

	int ret = 0;

	unsigned int begin,end;

    // inter
	begin = time(0);

#ifndef WIN32
	ProfilerStart("testinter.prof");
#endif
		
#ifndef _DEBUG
	for (int i = 0; i < 9000000; i++)
#else
	for (int i = 0; i < 10; i++)
#endif
	{
		ret = fkrun<int>(fk, "myfunc1", 1, 2);
		if (fkerror(fk))
		{
			printf("call fail errorno %d, %s\n", fkerror(fk), fkerrorstr(fk));
			return 0;
		}
	}

#ifndef WIN32
	ProfilerStop();
#endif
	
	end = time(0);

	printf("call ret %d %d\n", ret, end - begin);
	printf("call profile:\n%s", fkdumpprofile(fk));

	delfake(fk);

	char c;
	std::cin >> c;

    return 0;
}

