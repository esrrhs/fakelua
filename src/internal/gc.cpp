#include "gc.h"
#include "state.h"

gc::gc(fakelua_state *l) : m_l(l) {
}

gc::~gc() {
}

gcobject *gc::add(std::unique_ptr<gcobject> gco) {
    m_gcobject_list.emplace(m_gcobject_list.end(), std::move(gco));
    return gco.get();
}
