#include "gcobject.h"
#include "state.h"

gcobject::gcobject(fakelua_state *L, int type, int subtype) : m_type(type), m_subtype(subtype) {
    L->get_gc().add(this);
}

gcobject::~gcobject() {
}
