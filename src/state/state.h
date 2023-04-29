#pragma once

#include "fakelua.h"

namespace fakelua {

class fakelua_state_impl : public fakelua_state {
public:
    fakelua_state_impl();

    virtual ~fakelua_state_impl();

    virtual void compile_file(const std::string &filename) override;

    virtual void compile_string(const std::string &str) override;
};

}
