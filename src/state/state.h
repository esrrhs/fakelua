#pragma once

#include "fakelua.h"
#include "jit/vm.h"
#include "var_pool.h"
#include "var_string_heap.h"

namespace fakelua {

// the state contains the running environment we need.
class state : public fakelua_state {
public:
    state();

    virtual ~state();

    virtual void compile_file(const std::string &filename, compile_config cfg) override;

    virtual void compile_string(const std::string &str, compile_config cfg) override;

    // call before running. this will reset the state. just for speed.
    void reset() {
        var_string_heap_.reset();
        var_pool_.reset();
    }

    var_string_heap &get_var_string_heap() {
        return var_string_heap_;
    }

    var_pool &get_var_pool() {
        return var_pool_;
    }

    vm &get_vm() {
        return vm_;
    }

private:
    var_string_heap var_string_heap_;
    var_pool var_pool_;
    vm vm_;
};

}// namespace fakelua
