#pragma once

#include "fakelua.h"
#include "jit/vm.h"
#include "var_string_heap.h"
#include "var_table_heap.h"

namespace fakelua {

// the state contains the running environment we need.
class state final : public fakelua_state {
public:
    state() = default;

    ~state() override = default;

    void compile_file(const std::string &filename, const compile_config &cfg) override;

    void compile_string(const std::string &str, const compile_config &cfg) override;

    // call before running. this will reset the state. just for speed.
    void reset() {
        var_string_heap_.reset();
        var_table_heap_.reset();
    }

    var_string_heap &get_var_string_heap() {
        return var_string_heap_;
    }

    var_table_heap &get_var_table_heap() {
        return var_table_heap_;
    }

    vm &get_vm() {
        return vm_;
    }

private:
    var_string_heap var_string_heap_;
    var_table_heap var_table_heap_;
    vm vm_;
};

}// namespace fakelua
