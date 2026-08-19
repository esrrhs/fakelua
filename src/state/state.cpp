#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/os/native_os.h"
#include "native/utf8/native_utf8.h"
#include "native/io/native_io.h"
#include "native/net/native_net.h"
#include "native/timer/native_timer.h"
#include "native/serialize/native_serialize.h"
#include "native/protobuf/native_protobuf.h"

namespace fakelua {

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
    RegisterNativeObjectApi(this);
    net::RegisterNetLibraryApi(this);
    timer::RegisterTimerLibraryApi(this);
    serialize::RegisterSerializeLibraryApi(this);
    protobuf::RegisterProtobufLibraryApi(this);
}

}// namespace fakelua
