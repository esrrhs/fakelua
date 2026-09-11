#pragma once

struct fake;
struct func_binary;

// SSA optimizer: Cooper-Harvey-Kennedy dominators, Cytron SSA,
// Wegman-Zadeck SCCP, Alpern-Wegman-Zadeck-style GVN, SSA DCE.
class optimizer {
public:
    optimizer(fake *fk);

    ~optimizer();

    void clear();

    void optimize(func_binary &fb);

private:
    fake *m_fk;
};
