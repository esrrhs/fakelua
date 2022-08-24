#pragma once

#include <string>
#include <memory>

namespace fakelua {

// fake lua state interface
class fakelua_state {
public:
    fakelua_state() {}

    virtual ~fakelua_state() {}

    virtual void compile_file(const std::string &filename) = 0;

    virtual void compile_string(const std::string &str) = 0;
};

using fakelua_state_ptr = std::shared_ptr<fakelua_state>;

// create fake lua state
fakelua_state_ptr fakelua_newstate();

// open global profiler by gperftools
void open_profiler(const std::string &fname);

// stop global profiler by gperftools
void stop_profiler();

}
