#pragma once

#include "types.h"

class string_object {
public:
    string_object();

    ~string_object();

private:
    std::string_view m_short_str;
    std::string m_long_str;
};
