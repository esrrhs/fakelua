#include "funcmap.h"
#include "fake.h"

String &funcmap::dump() {
    String &ret = m_dump;
    ret.clear();

    ret += "conflict\n";
    const int conflict_len = 10;
    int conflict[conflict_len];
    m_shh.get_conflict(conflict, conflict_len);
    for (int i = 0; i < conflict_len; i++) {
        ret += "[";
        ret += fkitoa(i);
        ret += "]\t";
        ret += fkitoa(conflict[i]);
        ret += "\n";
    }

    ret += "\nconflict map\n";
    const int conflict_map_len = 1024;
    int len = conflict_map_len;
    int conflict_map[conflict_map_len];
    m_shh.get_conflict_map(conflict_map, len);
    for (int i = 0; i < (int) len; i++) {
        ret += "[";
        ret += fkitoa(i);
        ret += "]\t";
        ret += fkitoa(conflict_map[i]);
        ret += "\n";
    }

    return ret;
}
