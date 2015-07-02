#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "fakescript.h"
#include <string.h>

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
		printf("arg : [-hjp] file [arg]\n");
        return 0;
    }
    
    fake * fk = newfake();
    bool isopenprofile = false;
	bool isopenjit = false;
	if (argv[1][0] == '-')
	{
		if (strstr(argv[1], "h"))
		{
			printf("fakescript\n\narg : [-hjp] file [arg]\n-h help\n-j open jit\n-p open profile\n\nsample:./fake -jp a.fk\n\n");
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
		argc--;
		argv++;
	}

	fksetargv(fk, argc - 2, argv + 2);
	fkopenalllib(fk);
    fkseterrorfunc(fk, error_log);
    fkparse(fk, argv[1]);
    if (fkerror(fk))
    {
        return 0;
    }

	if (!isopenjit)
	{
		fkrun<int>(fk, "main");
	}
	else
	{
		fkrunjit<int>(fk, "main");
	}
	if (fkerror(fk))
	{
		return 0;
	}

	if (isopenprofile)
	{
		printf("call profile:\n%s", fkdumpprofile(fk));
	}

	delfake(fk);

    return 0;
}

