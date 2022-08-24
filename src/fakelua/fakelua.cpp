#include "fakelua/fakelua.h"
#include "gperftools/profiler.h"
#include "state/state.h"
#include "glog/logging.h"

namespace fakelua {

fakelua_state_ptr fakelua_newstate() {
    LOG(INFO) << "fakelua_newstate";
    return std::make_shared<fakelua_state_impl>();
}

void open_profiler(const std::string &fname) {
    LOG(INFO) << "open_profiler";
    ProfilerStart(fname.c_str());
}

void stop_profiler() {
    LOG(INFO) << "stop_profiler";
    ProfilerStop();
}

}
