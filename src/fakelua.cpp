#include "fakelua.h"

namespace fakelua {

class fakelua_state {

};

fakelua_state *fakelua_newstate() {
    auto L = new fakelua_state();
    return L;
}

void fakelua_close(fakelua_state *L) {
    delete L;
}

}
