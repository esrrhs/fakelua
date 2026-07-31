#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/native_os.h"
#include "native/native_utf8.h"
#include "native/native_io.h"

namespace fakelua {

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
    RegisterNativeObjectApi(this);
    RegisterMathLibraryApi(this);
    RegisterTableLibraryApi(this);
    RegisterOsLibraryApi(this);
    RegisterUtf8LibraryApi(this);
    RegisterIoLibraryApi(this);
}

}// namespace fakelua
