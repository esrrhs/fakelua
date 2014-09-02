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
    
    return 0;
}

