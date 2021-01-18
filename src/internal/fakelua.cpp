#include "fakelua.h"
#include "types.h"
#include "state.h"

extern "C" fakelua_state *fakelua_newstate() {
    return new fakelua_state();
}
