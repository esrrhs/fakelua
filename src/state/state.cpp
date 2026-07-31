#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/native_os.h"

namespace fakelua {

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
    RegisterNativeObjectApi(this);
    RegisterMathLibraryApi(this);
    RegisterTableLibraryApi(this);
    RegisterOsLibraryApi(this);
}

}// namespace fakelua
