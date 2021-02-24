#pragma once

#include "types.h"
#include "gcobject.h"

class fakelua_state;

class string_object : public gcobject {
public:
    string_object(const std::string &str);

    virtual ~string_object();

public:
    static string_object *create(fakelua_state *L, const std::string &str);

private:
    std::string m_str;
};
