#include "object.h"
#include "state.h"

string_object::string_object(const std::string &str) :
        gcobject(static_cast<int>(VT_STRING), static_cast<int>(VT_ST_LONG_STRINGS)), m_str(str) {
}

string_object::~string_object() {
}

string_object *string_object::create(fakelua_state *L, const std::string &str) {
    auto pso = std::make_unique<string_object>(str);
    return dynamic_cast<string_object *>(L->get_gc().add(std::move(pso)));
}
