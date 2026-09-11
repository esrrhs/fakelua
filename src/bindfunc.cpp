#include "bindfunc.h"
#include "fake.h"

void bindfunc::addfunc(const variant &name, const fkfunctor &ff) {
    m_fk->fm.add_bind_func(name, ff);
    FKLOG("add bind func %s", vartostring(&name).c_str());
}
