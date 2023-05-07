#pragma once

#include "fakelua.h"
#include "var_string_heap.h"

namespace fakelua {

class state : public fakelua_state {
public:
    state();

    virtual ~state();

    virtual void compile_file(const std::string &filename) override;

    virtual void compile_string(const std::string &str) override;

    var_string_heap &get_var_string_heap() {
        return var_string_heap_;
    }

private:
    var_string_heap var_string_heap_;
};

}// namespace fakelua
