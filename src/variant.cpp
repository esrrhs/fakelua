#include "variant.h"
#include "fuck.h"
#include "interpreter.h"

extern "C" void v_set_real()
{
    variant v1;
    v1.type = variant::REAL;
    v1.data.real = (double)1.154151511251235f;
}

void cheat_complie()
{
#if 0
    v_set_real();
#endif
}

