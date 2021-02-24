#pragma once

#include "types.h"
#include "gcobject.h"

class fakelua_state;

class string_object : public gcobject {
public:
    string_object(fakelua_state *L, const std::string &str);

    virtual ~string_object();

private:
    std::string m_str;
};
