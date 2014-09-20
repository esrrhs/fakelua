#include "variant.h"
#include "fuck.h"
#include "interpreter.h"

extern "C" void v_set_real()
{
    variant v1;
    variant v2;
    variant v3;
    USE(v1);
    USE(v2);
    USE(v3);
}

void cheat_complie()
{
#if 0
    v_set_real();
#endif
}

