#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "fuckscript.h"
#ifndef WIN32
#include "gperftools/profiler.h"
#endif

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    
    fuck * fk = newfuck();
    fkparse(fk, argv[1]);
    if (fkerror(fk))
    {
        printf("parse error %d, %s\n", fkerror(fk), fkerrorstr(fk));
        return 0;
    }

	int ret = 0;

	unsigned int begin = time(0);

#ifndef WIN32
	ProfilerStart("test.prof");
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
			printf("call fail errorno %d, %s\n", fkerror(fk), fkerrorstr(fk));
			return 0;
		}
	}

#ifndef WIN32
	ProfilerStop();
#endif
	
	unsigned int end = time(0);

	printf("call ret %d %d\n", ret, end - begin);

	char c;
	std::cin >> c;

    return 0;
}

