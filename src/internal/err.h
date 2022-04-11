#pragma once

#include "types.h"

class err {
public:
    err();

    err(int code, const std::string &code_str);

    virtual ~err();

public:
    bool empty()const;

    int code()const;

    const std::string &str()const;

private:
    int m_code = 0;
    std::string m_code_str;
};
