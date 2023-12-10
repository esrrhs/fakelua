#include "fakelua.h"
#ifdef __linux__
#include "gperftools/profiler.h"
#endif
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

fakelua_state_ptr fakelua_newstate() {
    LOG(INFO) << "fakelua_newstate";
    return std::make_shared<state>();
}

void open_profiler(const std::string_view &fname) {
#ifdef __linux__
    LOG(INFO) << "open_profiler";
    ProfilerStart(fname.data());
#endif
}

void stop_profiler() {
#ifdef __linux__
    LOG(INFO) << "stop_profiler";
    ProfilerStop();
#endif
}

}// namespace fakelua
