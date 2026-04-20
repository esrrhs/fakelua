#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"

namespace fakelua {

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
}

}// namespace fakelua
