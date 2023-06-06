#pragma once

#include "fakelua.h"
#include "var_string_heap.h"
#include "var_pool.h"

namespace fakelua {

// the state contains the running environment we need.
class state : public fakelua_state, public no_copy<state> {
public:
    state();

    virtual ~state();

    virtual void compile_file(const std::string &filename) override;

    virtual void compile_string(const std::string &str) override;

    var_string_heap &get_var_string_heap() {
        return var_string_heap_;
    }

    var_pool &get_var_pool() {
        return var_pool_;
    }

private:
    var_string_heap var_string_heap_;
    var_pool var_pool_;
};

}// namespace fakelua
