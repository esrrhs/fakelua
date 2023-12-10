#pragma once

#include <memory>
#include <string>

namespace fakelua {

// fake_lua state interface, every state can only run in one thread, just like Lua.
// every state has its own running environment. there could be many states in one process.
class fakelua_state : public std::enable_shared_from_this<fakelua_state> {
public:
    fakelua_state() {
    }

    virtual ~fakelua_state() {
    }

    // compile file, the file is a lua file.
    virtual void compile_file(const std::string_view &filename) = 0;

    // compile string, the string is the content of a file.
    virtual void compile_string(const std::string_view &str) = 0;

protected:
    virtual void *get_func_addr(const std::string_view &name) = 0;
};

using fakelua_state_ptr = std::shared_ptr<fakelua_state>;

// create fake_lua state
fakelua_state_ptr fakelua_newstate();

// open global profiler by gperftools
void open_profiler(const std::string_view &fname);

// stop global profiler by gperftools
void stop_profiler();

}// namespace fakelua
