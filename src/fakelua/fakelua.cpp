#include "fakelua.h"
#ifdef __linux__
#include "gperftools/profiler.h"
#endif
#include "glog/logging.h"
#include "state/state.h"

namespace fakelua {

fakelua_state_ptr fakelua_newstate() {
    LOG(INFO) << "fakelua_newstate";
    return std::make_shared<fakelua_state_impl>();
}

void open_profiler(const std::string &fname) {
#ifdef __linux__
    LOG(INFO) << "open_profiler";
    ProfilerStart(fname.c_str());
#endif
}

void stop_profiler() {
#ifdef __linux__
    LOG(INFO) << "stop_profiler";
    ProfilerStop();
#endif
}

}// namespace fakelua
