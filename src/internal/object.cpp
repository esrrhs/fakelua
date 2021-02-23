#include "object.h"

string_object::string_object(fakelua_state *L, const std::string &str) :
        gcobject(L, static_cast<int>(VT_STRING), static_cast<int>(VT_ST_LONG_STRINGS)), m_str(str) {
}

string_object::~string_object() {
}
