#pragma once

#include "fakelua.h"

namespace fakelua {

class TableHelper {
public:
    static bool VarKeyEqualInt(CVar k, int64_t idx);
    static int64_t GetTableLen(CVar tbl);
    static CVar GetTableInt(State *s, CVar tbl, int64_t idx);
    static CVar GetTableStrId(State *s, CVar tbl, const char *str_key);
    static void SetTableInt(State *s, CVar tbl, int64_t idx, CVar val);
    static void SetTableStrId(State *s, CVar tbl, const char *str_key, CVar val);
    // 任意类型键的写入。键会先做浮点归一化，再按与 JIT 侧一致的哈希布局落槽。
    static void SetTable(State *s, CVar tbl, CVar key, CVar val);
    // 分配一张空表（temp arena，帧内有效）。
    static CVar CreateTable(State *s);
};

void RegisterTableLibraryApi(State *s);

}// namespace fakelua
