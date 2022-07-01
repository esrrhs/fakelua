#include "fakelua/fakelua.h"
#include "gperftools/profiler.h"
#include "state.h"

namespace fakelua {

fakelua_state_ptr fakelua_newstate() {
    return std::make_shared<fakelua_state_impl>();
}

void open_profiler(const std::string &fname) {
    ProfilerStart(fname.c_str());
}

void stop_profiler() {
    ProfilerStop();
}

}
