#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/os/native_os.h"
#include "native/utf8/native_utf8.h"
#include "native/io/native_io.h"
#include "native/net/native_net.h"

namespace fakelua {

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
    RegisterNativeObjectApi(this);
    net::RegisterNetLibraryApi(this);
}

}// namespace fakelua
