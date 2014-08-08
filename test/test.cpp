#include <stdio.h>
#include <stdlib.h>

#include "fuckscript.h"

int main()
{
    fuck * fk = newfuck();
    bool b = fkparse(fk, "test.fk");
    if (!b)
    {
        printf("parse error\n");
    }
    
    return 0;
}

