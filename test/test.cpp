#include <stdio.h>
#include <stdlib.h>

#include "fuckscript.h"

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    
    fuck * fk = newfuck();
    binary * bin = fkparse(fk, argv[1]);
    if (!bin)
    {
        printf("parse error\n");
    }

    int ret = fkrun<int>(bin, "myfunc1", "asf", 2);
    printf("call ret %d\n", ret);
    
    return 0;
}

