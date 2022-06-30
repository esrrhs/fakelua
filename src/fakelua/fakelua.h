#pragma once

namespace fakelua {

// This file define interface, try to be consistent with the lua interface
class fakelua_state;

// create fake lua state
fakelua_state *fakelua_newstate();

// close fake lua state
void fakelua_close(fakelua_state *L);

}
