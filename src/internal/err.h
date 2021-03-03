#pragma once

#include "types.h"

class err {
public:
    err();

    err(int code, const std::string &code_str);

    virtual ~err();

public:
    bool empty();

    int code();

    const std::string &str();

private:
    int m_code = 0;
    std::string m_code_str;
};
