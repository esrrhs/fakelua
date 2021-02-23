#include "gc.h"
#include "state.h"

gc::gc(fakelua_state *l) : m_l(l) {
}

gc::~gc() {
}

void gc::add(gcobject *gco) {
    // TODO
}
