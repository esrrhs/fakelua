#include "compile/c_gen.h"

namespace fakelua {

CGen::CGen(State *s) : s_(s), pp_(s), jitter_(s) {
}

}// namespace fakelua
