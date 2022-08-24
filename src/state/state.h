#pragma once

#include "fakelua/fakelua.h"

namespace fakelua {

class fakelua_state_impl : public fakelua_state {
public:
    fakelua_state_impl();

    virtual ~fakelua_state_impl();
};

}
