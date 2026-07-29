#pragma once

#include "fakelua.h"

namespace fakelua {

class TableHelper {
public:
    static bool VarKeyEqualInt(CVar k, int64_t idx);
    static int64_t GetTableLen(CVar tbl);
    static CVar GetTableInt(State *s, CVar tbl, int64_t idx);
    static void SetTableInt(State *s, CVar tbl, int64_t idx, CVar val);
    static void SetTableStrId(State *s, CVar tbl, const char *str_key, CVar val);
};

void RegisterTableLibraryApi(State *s);

}// namespace fakelua
