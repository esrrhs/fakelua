#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "fakescript.h"

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
		printf("arg : file\n");
        return 0;
    }
    
    fake * fk = newfake();

	fksetargv(fk, argc - 2, argv + 2);
    fkopenbaselib(fk);
    fkopenfilelib(fk);
    fkopennetlib(fk);
    fkopenoslib(fk);
    fkopenstringlib(fk);
    fkopenprofile(fk);
    fkseterrorfunc(fk, error_log);
    fkparse(fk, argv[1]);
    if (fkerror(fk))
    {
        return 0;
    }

	fkrun<int>(fk, "main");
	if (fkerror(fk))
	{
		return 0;
	}

	printf("call profile:\n%s", fkdumpprofile(fk));

	delfake(fk);

    return 0;
}

