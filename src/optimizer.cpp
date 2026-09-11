#include "optimizer.h"
#include "fake.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <queue>
#include <set>
#include <vector>

namespace {

bool cmd_is_addr(command c) {
    return COMMAND_TYPE(c) == COMMAND_ADDR;
}

int cmd_addr_type(command c) {
    return ADDR_TYPE(COMMAND_CODE(c));
}

int cmd_addr_pos(command c) {
    return ADDR_POS(COMMAND_CODE(c));
}

bool is_binop(int op) {
    return op == OPCODE_PLUS || op == OPCODE_MINUS || op == OPCODE_MULTIPLY ||
           op == OPCODE_DIVIDE || op == OPCODE_DIVIDE_MOD || op == OPCODE_STRING_CAT ||
           op == OPCODE_AND || op == OPCODE_OR || op == OPCODE_LESS || op == OPCODE_MORE ||
           op == OPCODE_EQUAL || op == OPCODE_MOREEQUAL || op == OPCODE_LESSEQUAL ||
           op == OPCODE_NOTEQUAL;
}

bool is_math_assign(int op) {
    return op == OPCODE_PLUS_ASSIGN || op == OPCODE_MINUS_ASSIGN ||
           op == OPCODE_MULTIPLY_ASSIGN || op == OPCODE_DIVIDE_ASSIGN ||
           op == OPCODE_DIVIDE_MOD_ASSIGN;
}

bool is_cmp_jne(int op) {
    return op == OPCODE_AND_JNE || op == OPCODE_OR_JNE || op == OPCODE_LESS_JNE ||
           op == OPCODE_MORE_JNE || op == OPCODE_EQUAL_JNE || op == OPCODE_MOREEQUAL_JNE ||
           op == OPCODE_LESSEQUAL_JNE || op == OPCODE_NOTEQUAL_JNE;
}

int math_assign_to_binop(int op) {
    switch (op) {
        case OPCODE_PLUS_ASSIGN:
            return OPCODE_PLUS;
        case OPCODE_MINUS_ASSIGN:
            return OPCODE_MINUS;
        case OPCODE_MULTIPLY_ASSIGN:
            return OPCODE_MULTIPLY;
        case OPCODE_DIVIDE_ASSIGN:
            return OPCODE_DIVIDE;
        case OPCODE_DIVIDE_MOD_ASSIGN:
            return OPCODE_DIVIDE_MOD;
        default:
            return -1;
    }
}

bool binop_may_trap(int op) {
    return op == OPCODE_PLUS || op == OPCODE_MINUS || op == OPCODE_MULTIPLY ||
           op == OPCODE_DIVIDE || op == OPCODE_DIVIDE_MOD ||
           op == OPCODE_AND || op == OPCODE_OR || op == OPCODE_LESS || op == OPCODE_MORE ||
           op == OPCODE_MOREEQUAL || op == OPCODE_LESSEQUAL;
}

struct Insn {
    int op;
    int lineno;
    std::vector<command> ops;
    int jump_to;
    int jump_op;
    bool deleted;
};

struct Block {
    int begin;
    int end;
    std::vector<int> succ;
    std::vector<int> pred;
};

enum {
    LAT_UNDEF = 0,
    LAT_CONST = 1,
    LAT_NAC = 2
};

struct Lat {
    int kind;
    command c;

    Lat() : kind(LAT_UNDEF), c(0) {
    }
};

Lat lat_meet(const Lat &a, const Lat &b) {
    if (a.kind == LAT_UNDEF) {
        return b;
    }
    if (b.kind == LAT_UNDEF) {
        return a;
    }
    if (a.kind == LAT_NAC || b.kind == LAT_NAC) {
        Lat n;
        n.kind = LAT_NAC;
        return n;
    }
    if (a.c == b.c) {
        return a;
    }
    Lat n;
    n.kind = LAT_NAC;
    return n;
}

bool lat_eq(const Lat &a, const Lat &b) {
    return a.kind == b.kind && (a.kind != LAT_CONST || a.c == b.c);
}

struct Phi {
    int slot;
    int dest;
    int block;
    std::vector<int> opnds;
};

struct GvnKey {
    int op;
    int a;
    int b;

    bool operator<(const GvnKey &o) const {
        if (op != o.op) {
            return op < o.op;
        }
        if (a != o.a) {
            return a < o.a;
        }
        return b < o.b;
    }
};

int max_slot(const func_binary *fb, const std::vector<Insn> &insns) {
    int m = fb->m_paramnum;
    if (fb->m_maxstack > m) {
        m = fb->m_maxstack;
    }
    for (size_t i = 0; i < insns.size(); i++) {
        for (size_t j = 0; j < insns[i].ops.size(); j++) {
            command c = insns[i].ops[j];
            if (cmd_is_addr(c) && cmd_addr_type(c) == ADDR_STACK) {
                int p = cmd_addr_pos(c) + 1;
                if (p > m) {
                    m = p;
                }
            }
        }
    }
    for (int i = 0; i < fb->m_container_addr_list_num; i++) {
        command cs[2] = {fb->m_container_addr_list[i].con, fb->m_container_addr_list[i].key};
        for (int k = 0; k < 2; k++) {
            if (cmd_is_addr(cs[k]) && cmd_addr_type(cs[k]) == ADDR_STACK) {
                int p = cmd_addr_pos(cs[k]) + 1;
                if (p > m) {
                    m = p;
                }
            }
        }
    }
    return m;
}

struct Opt {
    fake *fk;
    func_binary *fb;
    std::vector<Insn> insns;

    std::vector<Block> blocks;
    std::vector<int> insn_block;
    std::vector<int> idom;
    std::vector<std::vector<int> > dom_child;
    int entry;

    int nslots;
    std::vector<Phi> phis;
    std::vector<std::vector<int> > block_phis;
    std::vector<std::vector<int> > use_ssa;
    std::vector<int> def_ssa;
    std::vector<std::vector<int> > extra_defs;
    std::vector<int> val_slot;
    std::vector<int> val_def_insn;
    std::vector<Lat> lattice;
    std::vector<char> exec_block;
    std::vector<std::vector<char> > exec_edge;
    std::vector<std::vector<int> > ssa_insn_users;
    std::vector<std::vector<int> > ssa_phi_users;

    explicit Opt(fake *fk_, func_binary *fb_) : fk(fk_), fb(fb_), entry(0), nslots(0) {
    }

    int n() const {
        return (int) insns.size();
    }

    bool decode();
    void encode();
    void compact();
    void run();
    void build_cfg();
    bool elim_unreachable();
    bool fold_constants();
    bool copy_propagate();
    bool dest_forward();
    bool dce();
    bool thread_jumps();
    bool ssa_sccp();
    void rewrite_dummies();
    void pack_stack();
    void pack_containers();

    const variant *as_const(command c) const;
    int intern_variant(const variant &v);
    int intern_real(double r);
    void replace_with_assign(Insn &in, command dest, command src);
    bool fold_binop(Insn &in);
    bool fold_not(Insn &in);
    bool eval_cmp_jne(int op, const variant *l, const variant *r, bool &take_jump);
    void collect_rw(const Insn &in, std::vector<int> &reads, std::vector<int> &writes,
                    bool &side, bool &trap, bool &ctrl) const;
    void touch_addr(command c, bool is_write, std::vector<int> &reads, std::vector<int> &writes,
                    bool &side, bool &trap) const;
    void remap_stack_cmd(command &c, const std::vector<int> &map);
    void remap_container_cmd(command &c, const std::vector<int> &map);
    void subst_use_ops(Insn &in, int insn_i);

    void compute_dominators();
    bool dominates(int a, int b) const;
    void build_ssa();
    int new_ssa(int slot, int def_insn);
    void rename_block(int b, std::vector<std::vector<int> > &stk, std::vector<int> &saved);
    void visit_insn_sccp(int i, std::queue<int> &ssa_wl, std::queue<int> &cfg_wl);
    void visit_phi_sccp(int pi, std::queue<int> &ssa_wl);
    void visit_term_sccp(int b, std::queue<int> &cfg_wl);
    void mark_edge(int p, int s, std::queue<int> &cfg_wl);
    void set_lat(int id, const Lat &nv, std::queue<int> &ssa_wl);
    Lat lat_of_op(const Insn &in, int insn_i, int opidx) const;
    Lat eval_binop_lat(int op, const Lat &l, const Lat &r);
    bool fold_to_const_assign(Insn &in, const Lat &res);
    bool gvn();
    bool slot_still_holds(int def_i, int slot, int use_i) const;
};

bool Opt::decode() {
    insns.clear();
    std::vector<int> off_of_insn;
    int ip = 0;
    while (ip < fb->m_size) {
        Insn in;
        in.op = COMMAND_CODE(fb->m_buff[ip]);
        in.lineno = (fb->m_lineno_buff && ip < fb->m_lineno_size) ? fb->m_lineno_buff[ip] : 0;
        in.jump_to = -1;
        in.jump_op = -1;
        in.deleted = false;
        off_of_insn.push_back(ip);
        ip++;

        switch (in.op) {
            case OPCODE_ASSIGN:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                break;
            case OPCODE_PLUS:
            case OPCODE_MINUS:
            case OPCODE_MULTIPLY:
            case OPCODE_DIVIDE:
            case OPCODE_DIVIDE_MOD:
            case OPCODE_STRING_CAT:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_LESS:
            case OPCODE_MORE:
            case OPCODE_EQUAL:
            case OPCODE_MOREEQUAL:
            case OPCODE_LESSEQUAL:
            case OPCODE_NOTEQUAL:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                break;
            case OPCODE_NOT:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                break;
            case OPCODE_AND_JNE:
            case OPCODE_OR_JNE:
            case OPCODE_LESS_JNE:
            case OPCODE_MORE_JNE:
            case OPCODE_EQUAL_JNE:
            case OPCODE_MOREEQUAL_JNE:
            case OPCODE_LESSEQUAL_JNE:
            case OPCODE_NOTEQUAL_JNE:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.jump_op = (int) in.ops.size();
                in.ops.push_back(fb->m_buff[ip]);
                in.jump_to = (int) COMMAND_CODE(fb->m_buff[ip]);
                ip++;
                break;
            case OPCODE_NOT_JNE:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.jump_op = (int) in.ops.size();
                in.ops.push_back(fb->m_buff[ip]);
                in.jump_to = (int) COMMAND_CODE(fb->m_buff[ip]);
                ip++;
                break;
            case OPCODE_JNE:
                in.ops.push_back(fb->m_buff[ip++]);
                in.jump_op = (int) in.ops.size();
                in.ops.push_back(fb->m_buff[ip]);
                in.jump_to = (int) COMMAND_CODE(fb->m_buff[ip]);
                ip++;
                break;
            case OPCODE_JMP:
                in.jump_op = (int) in.ops.size();
                in.ops.push_back(fb->m_buff[ip]);
                in.jump_to = (int) COMMAND_CODE(fb->m_buff[ip]);
                ip++;
                break;
            case OPCODE_PLUS_ASSIGN:
            case OPCODE_MINUS_ASSIGN:
            case OPCODE_MULTIPLY_ASSIGN:
            case OPCODE_DIVIDE_ASSIGN:
            case OPCODE_DIVIDE_MOD_ASSIGN:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                break;
            case OPCODE_CALL: {
                if (ip + 3 > fb->m_size) {
                    return false;
                }
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                int retnum = (int) COMMAND_CODE(fb->m_buff[ip]);
                if (retnum < 0 || ip + 1 + retnum >= fb->m_size) {
                    return false;
                }
                in.ops.push_back(fb->m_buff[ip++]);
                for (int i = 0; i < retnum; i++) {
                    in.ops.push_back(fb->m_buff[ip++]);
                }
                int argnum = (int) COMMAND_CODE(fb->m_buff[ip]);
                if (argnum < 0 || ip + 1 + argnum > fb->m_size) {
                    return false;
                }
                in.ops.push_back(fb->m_buff[ip++]);
                for (int i = 0; i < argnum; i++) {
                    in.ops.push_back(fb->m_buff[ip++]);
                }
            }
                break;
            case OPCODE_FOR:
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.ops.push_back(fb->m_buff[ip++]);
                in.jump_op = (int) in.ops.size();
                in.ops.push_back(fb->m_buff[ip]);
                in.jump_to = (int) COMMAND_CODE(fb->m_buff[ip]);
                ip++;
                break;
            case OPCODE_RETURN: {
                if (ip >= fb->m_size) {
                    return false;
                }
                int retnum = (int) COMMAND_CODE(fb->m_buff[ip]);
                if (retnum < 0 || ip + 1 + retnum > fb->m_size) {
                    return false;
                }
                in.ops.push_back(fb->m_buff[ip++]);
                for (int i = 0; i < retnum; i++) {
                    in.ops.push_back(fb->m_buff[ip++]);
                }
            }
                break;
            default:
                FKERR("optimizer decode unknown opcode %d %s", in.op, OpCodeStr(in.op));
                assert(0);
                return false;
        }
        if (ip > fb->m_size) {
            return false;
        }
        insns.push_back(in);
    }

    std::vector<int> insn_at(fb->m_size + 1, -1);
    for (int i = 0; i < n(); i++) {
        insn_at[off_of_insn[i]] = i;
    }
    insn_at[fb->m_size] = n();
    for (int i = 0; i < n(); i++) {
        if (insns[i].jump_to < 0) {
            continue;
        }
        int dest = insns[i].jump_to;
        if (dest < 0) {
            dest = 0;
        }
        if (dest > fb->m_size) {
            dest = fb->m_size;
        }
        int ti = insn_at[dest];
        if (ti < 0) {
            while (dest > 0 && insn_at[dest] < 0) {
                dest--;
            }
            ti = insn_at[dest];
        }
        insns[i].jump_to = (ti < 0) ? n() : ti;
    }
    return true;
}

void Opt::encode() {
    std::vector<int> new_off(n() + 1, 0);
    int pos = 0;
    for (int i = 0; i < n(); i++) {
        new_off[i] = pos;
        if (insns[i].deleted) {
            continue;
        }
        pos += 1 + (int) insns[i].ops.size();
    }
    new_off[n()] = pos;

    auto resolve = [&](int t) {
        while (t < n() && insns[t].deleted) {
            t++;
        }
        return t;
    };

    int newsize = pos;
    command *nb = 0;
    int *nl = 0;
    if (newsize > 0) {
        nb = (command *) safe_fkmalloc(fk, newsize * sizeof(command), emt_func_binary);
        nl = (int *) safe_fkmalloc(fk, newsize * sizeof(int), emt_func_binary);
    }

    pos = 0;
    for (int i = 0; i < n(); i++) {
        Insn &in = insns[i];
        if (in.deleted) {
            continue;
        }
        if (in.jump_op >= 0 && in.jump_op < (int) in.ops.size()) {
            int t = resolve(in.jump_to);
            if (t < 0 || t > n()) {
                t = n();
            }
            in.ops[in.jump_op] = MAKE_POS(new_off[t]);
        }
        nb[pos] = MAKE_OPCODE(in.op);
        nl[pos] = in.lineno;
        pos++;
        for (int j = 0; j < (int) in.ops.size(); j++) {
            nb[pos] = in.ops[j];
            nl[pos] = in.lineno;
            pos++;
        }
    }

    safe_fkfree(fk, fb->m_buff);
    safe_fkfree(fk, fb->m_lineno_buff);
    fb->m_buff = nb;
    fb->m_size = newsize;
    fb->m_lineno_buff = nl;
    fb->m_lineno_size = newsize;
}

void Opt::compact() {
    std::vector<int> map(n(), -1);
    int k = 0;
    for (int i = 0; i < n(); i++) {
        if (!insns[i].deleted) {
            map[i] = k++;
        }
    }
    int newn = k;
    for (int i = 0; i < n(); i++) {
        if (insns[i].deleted || insns[i].jump_to < 0) {
            continue;
        }
        int t = insns[i].jump_to;
        while (t < n() && insns[t].deleted) {
            t++;
        }
        insns[i].jump_to = (t >= n()) ? newn : map[t];
    }
    std::vector<Insn> kept;
    kept.reserve(newn);
    for (int i = 0; i < n(); i++) {
        if (!insns[i].deleted) {
            kept.push_back(insns[i]);
        }
    }
    insns.swap(kept);
}

void Opt::build_cfg() {
    blocks.clear();
    insn_block.assign(n(), -1);
    if (n() == 0) {
        return;
    }
    std::vector<char> leader(n(), 0);
    leader[0] = 1;
    for (int i = 0; i < n(); i++) {
        const Insn &in = insns[i];
        if (in.jump_to >= 0 && in.jump_to < n()) {
            leader[in.jump_to] = 1;
        }
        bool term = (in.op == OPCODE_JMP || in.op == OPCODE_RETURN);
        if (!term && i + 1 < n() && in.jump_to >= 0) {
            leader[i + 1] = 1;
        }
        if (term && i + 1 < n()) {
            leader[i + 1] = 1;
        }
    }
    int b = -1;
    for (int i = 0; i < n(); i++) {
        if (leader[i]) {
            if (b >= 0) {
                blocks[b].end = i;
            }
            Block blk;
            blk.begin = i;
            blk.end = n();
            blocks.push_back(blk);
            b = (int) blocks.size() - 1;
        }
        insn_block[i] = b;
    }
    if (b >= 0) {
        blocks[b].end = n();
    }

    for (int bi = 0; bi < (int) blocks.size(); bi++) {
        Block &blk = blocks[bi];
        if (blk.begin >= blk.end) {
            continue;
        }
        int last = blk.end - 1;
        const Insn &in = insns[last];
        auto add_succ = [&](int target_insn) {
            if (target_insn < 0 || target_insn >= n()) {
                return;
            }
            int tb = insn_block[target_insn];
            if (tb < 0) {
                return;
            }
            if (std::find(blk.succ.begin(), blk.succ.end(), tb) == blk.succ.end()) {
                blk.succ.push_back(tb);
            }
        };
        if (in.op != OPCODE_RETURN && in.op != OPCODE_JMP && blk.end < n()) {
            add_succ(blk.end);
        }
        if (in.jump_to >= 0) {
            add_succ(in.jump_to);
        }
    }
    for (int bi = 0; bi < (int) blocks.size(); bi++) {
        for (int s : blocks[bi].succ) {
            blocks[s].pred.push_back(bi);
        }
    }
}

bool Opt::elim_unreachable() {
    build_cfg();
    if (blocks.empty()) {
        return false;
    }
    std::vector<char> reach((int) blocks.size(), 0);
    std::queue<int> q;
    q.push(0);
    reach[0] = 1;
    while (!q.empty()) {
        int b = q.front();
        q.pop();
        for (int s : blocks[b].succ) {
            if (!reach[s]) {
                reach[s] = 1;
                q.push(s);
            }
        }
    }
    bool ch = false;
    for (int i = 0; i < n(); i++) {
        int b = insn_block[i];
        if (b >= 0 && !reach[b] && !insns[i].deleted) {
            insns[i].deleted = true;
            ch = true;
        }
    }
    if (ch) {
        compact();
    }
    return ch;
}

const variant *Opt::as_const(command c) const {
    if (!cmd_is_addr(c) || cmd_addr_type(c) != ADDR_CONST) {
        return 0;
    }
    int p = cmd_addr_pos(c);
    if (p < 0 || p >= fb->m_const_list_num) {
        return 0;
    }
    return &fb->m_const_list[p];
}

int Opt::intern_variant(const variant &v) {
    for (int i = 0; i < fb->m_const_list_num; i++) {
        bool eq = false;
        V_EQUAL_V(eq, &fb->m_const_list[i], &v);
        if (eq) {
            return i;
        }
    }
    int nconst = fb->m_const_list_num + 1;
    variant *nl = (variant *) safe_fkmalloc(fk, nconst * sizeof(variant), emt_func_binary);
    if (fb->m_const_list_num > 0) {
        memcpy(nl, fb->m_const_list, fb->m_const_list_num * sizeof(variant));
    }
    nl[fb->m_const_list_num] = v;
    safe_fkfree(fk, fb->m_const_list);
    fb->m_const_list = nl;
    int idx = fb->m_const_list_num;
    fb->m_const_list_num = nconst;
    return idx;
}

int Opt::intern_real(double r) {
    variant v;
    V_SET_REAL(&v, r);
    return intern_variant(v);
}

void Opt::replace_with_assign(Insn &in, command dest, command src) {
    in.op = OPCODE_ASSIGN;
    in.ops.clear();
    in.ops.push_back(dest);
    in.ops.push_back(src);
    in.jump_to = -1;
    in.jump_op = -1;
}

bool Opt::fold_binop(Insn &in) {
    const variant *l = as_const(in.ops[0]);
    const variant *r = as_const(in.ops[1]);
    if (!l || !r) {
        return false;
    }
    command dest = in.ops[2];
    if (in.op == OPCODE_STRING_CAT) {
        if (l->type != variant::STRING || r->type != variant::STRING || !l->data.str || !r->data.str) {
            return false;
        }
        String s;
        s.append(l->data.str->s, l->data.str->sz);
        s.append(r->data.str->s, r->data.str->sz);
        variant v = fk->sh.allocsysstr(s.c_str());
        replace_with_assign(in, dest, MAKE_ADDR(ADDR_CONST, intern_variant(v)));
        return true;
    }
    if (in.op == OPCODE_EQUAL || in.op == OPCODE_NOTEQUAL) {
        bool eq = false;
        V_EQUAL_V(eq, l, r);
        double rv = (in.op == OPCODE_EQUAL) ? (eq ? 1.0 : 0.0) : (eq ? 0.0 : 1.0);
        replace_with_assign(in, dest, MAKE_ADDR(ADDR_CONST, intern_real(rv)));
        return true;
    }
    if (l->type != variant::REAL || r->type != variant::REAL) {
        return false;
    }
    double a = l->data.real;
    double b = r->data.real;
    double rv = 0;
    switch (in.op) {
        case OPCODE_PLUS:
            rv = a + b;
            break;
        case OPCODE_MINUS:
            rv = a - b;
            break;
        case OPCODE_MULTIPLY:
            rv = a * b;
            break;
        case OPCODE_DIVIDE:
            if (b == 0) {
                return false;
            }
            rv = a / b;
            break;
        case OPCODE_DIVIDE_MOD:
            if (b == 0) {
                return false;
            }
            rv = (double) ((int64_t) a % (int64_t) b);
            break;
        case OPCODE_AND:
            rv = (a != 0 && b != 0) ? 1 : 0;
            break;
        case OPCODE_OR:
            rv = (a != 0 || b != 0) ? 1 : 0;
            break;
        case OPCODE_LESS:
            rv = a < b;
            break;
        case OPCODE_MORE:
            rv = a > b;
            break;
        case OPCODE_MOREEQUAL:
            rv = a >= b;
            break;
        case OPCODE_LESSEQUAL:
            rv = a <= b;
            break;
        default:
            return false;
    }
    replace_with_assign(in, dest, MAKE_ADDR(ADDR_CONST, intern_real(rv)));
    return true;
}

bool Opt::fold_not(Insn &in) {
    const variant *l = as_const(in.ops[0]);
    if (!l || l->type != variant::REAL) {
        return false;
    }
    replace_with_assign(in, in.ops[1], MAKE_ADDR(ADDR_CONST, intern_real(l->data.real == 0 ? 1.0 : 0.0)));
    return true;
}

bool Opt::eval_cmp_jne(int op, const variant *l, const variant *r, bool &ok_jump_if_false) {
    if (!l || !r) {
        return false;
    }
    if (op == OPCODE_EQUAL_JNE || op == OPCODE_NOTEQUAL_JNE) {
        bool eq = false;
        V_EQUAL_V(eq, l, r);
        bool cond = (op == OPCODE_EQUAL_JNE) ? eq : !eq;
        ok_jump_if_false = !cond;
        return true;
    }
    if (l->type != variant::REAL || r->type != variant::REAL) {
        return false;
    }
    bool cond = false;
    switch (op) {
        case OPCODE_AND_JNE:
            cond = (l->data.real != 0) && (r->data.real != 0);
            break;
        case OPCODE_OR_JNE:
            cond = (l->data.real != 0) || (r->data.real != 0);
            break;
        case OPCODE_LESS_JNE:
            cond = l->data.real < r->data.real;
            break;
        case OPCODE_MORE_JNE:
            cond = l->data.real > r->data.real;
            break;
        case OPCODE_MOREEQUAL_JNE:
            cond = l->data.real >= r->data.real;
            break;
        case OPCODE_LESSEQUAL_JNE:
            cond = l->data.real <= r->data.real;
            break;
        default:
            return false;
    }
    ok_jump_if_false = !cond;
    return true;
}

bool Opt::fold_constants() {
    bool ch = false;
    for (int i = 0; i < n(); i++) {
        Insn &in = insns[i];
        if (in.deleted) {
            continue;
        }
        if (is_binop(in.op) && in.ops.size() >= 3) {
            ch |= fold_binop(in);
            continue;
        }
        if (in.op == OPCODE_NOT && in.ops.size() >= 2) {
            ch |= fold_not(in);
            continue;
        }
        if (in.op == OPCODE_JNE && in.ops.size() >= 2) {
            const variant *c = as_const(in.ops[0]);
            if (c && c->type == variant::REAL) {
                if (c->data.real != 0) {
                    in.deleted = true;
                } else {
                    command dest = in.ops[in.jump_op];
                    int jt = in.jump_to;
                    in.op = OPCODE_JMP;
                    in.ops.clear();
                    in.ops.push_back(dest);
                    in.jump_op = 0;
                    in.jump_to = jt;
                }
                ch = true;
            }
            continue;
        }
        if (is_cmp_jne(in.op) && in.ops.size() >= 4) {
            bool take = false;
            if (eval_cmp_jne(in.op, as_const(in.ops[0]), as_const(in.ops[1]), take)) {
                if (!take) {
                    in.deleted = true;
                } else {
                    command dest = in.ops[in.jump_op];
                    int jt = in.jump_to;
                    in.op = OPCODE_JMP;
                    in.ops.clear();
                    in.ops.push_back(dest);
                    in.jump_op = 0;
                    in.jump_to = jt;
                }
                ch = true;
            }
            continue;
        }
        if (in.op == OPCODE_NOT_JNE && in.ops.size() >= 3) {
            const variant *c = as_const(in.ops[0]);
            if (c && c->type == variant::REAL) {
                bool cond = !(c->data.real);
                if (cond) {
                    in.deleted = true;
                } else {
                    command dest = in.ops[in.jump_op];
                    int jt = in.jump_to;
                    in.op = OPCODE_JMP;
                    in.ops.clear();
                    in.ops.push_back(dest);
                    in.jump_op = 0;
                    in.jump_to = jt;
                }
                ch = true;
            }
        }
    }
    if (ch) {
        compact();
    }
    return ch;
}

void Opt::touch_addr(command c, bool is_write, std::vector<int> &reads, std::vector<int> &writes,
                     bool &side, bool &trap) const {
    if (!cmd_is_addr(c)) {
        return;
    }
    int at = cmd_addr_type(c);
    int p = cmd_addr_pos(c);
    if (at == ADDR_STACK) {
        if (is_write) {
            writes.push_back(p);
        } else {
            reads.push_back(p);
        }
    } else if (at == ADDR_CONTAINER) {
        side = true;
        trap = true;
        if (p >= 0 && p < fb->m_container_addr_list_num) {
            container_addr ca = fb->m_container_addr_list[p];
            touch_addr(ca.con, false, reads, writes, side, trap);
            touch_addr(ca.key, false, reads, writes, side, trap);
        }
    }
}

void Opt::collect_rw(const Insn &in, std::vector<int> &reads, std::vector<int> &writes,
                     bool &side, bool &trap, bool &ctrl) const {
    reads.clear();
    writes.clear();
    side = false;
    trap = false;
    ctrl = false;
    switch (in.op) {
        case OPCODE_ASSIGN:
            touch_addr(in.ops[1], false, reads, writes, side, trap);
            if (cmd_is_addr(in.ops[0]) && cmd_addr_type(in.ops[0]) == ADDR_CONTAINER) {
                touch_addr(in.ops[0], false, reads, writes, side, trap);
                side = true;
                trap = true;
            } else {
                touch_addr(in.ops[0], true, reads, writes, side, trap);
            }
            break;
        case OPCODE_PLUS:
        case OPCODE_MINUS:
        case OPCODE_MULTIPLY:
        case OPCODE_DIVIDE:
        case OPCODE_DIVIDE_MOD:
        case OPCODE_STRING_CAT:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_LESS:
        case OPCODE_MORE:
        case OPCODE_EQUAL:
        case OPCODE_MOREEQUAL:
        case OPCODE_LESSEQUAL:
        case OPCODE_NOTEQUAL:
            touch_addr(in.ops[0], false, reads, writes, side, trap);
            touch_addr(in.ops[1], false, reads, writes, side, trap);
            if (cmd_is_addr(in.ops[2]) && cmd_addr_type(in.ops[2]) == ADDR_CONTAINER) {
                touch_addr(in.ops[2], false, reads, writes, side, trap);
                side = true;
            } else {
                touch_addr(in.ops[2], true, reads, writes, side, trap);
            }
            if (binop_may_trap(in.op)) {
                trap = true;
            }
            break;
        case OPCODE_NOT:
            touch_addr(in.ops[0], false, reads, writes, side, trap);
            touch_addr(in.ops[1], true, reads, writes, side, trap);
            break;
        case OPCODE_PLUS_ASSIGN:
        case OPCODE_MINUS_ASSIGN:
        case OPCODE_MULTIPLY_ASSIGN:
        case OPCODE_DIVIDE_ASSIGN:
        case OPCODE_DIVIDE_MOD_ASSIGN:
            if (cmd_is_addr(in.ops[0]) && cmd_addr_type(in.ops[0]) == ADDR_CONTAINER) {
                touch_addr(in.ops[0], false, reads, writes, side, trap);
                side = true;
            } else {
                touch_addr(in.ops[0], false, reads, writes, side, trap);
                touch_addr(in.ops[0], true, reads, writes, side, trap);
            }
            touch_addr(in.ops[1], false, reads, writes, side, trap);
            trap = true;
            break;
        case OPCODE_AND_JNE:
        case OPCODE_OR_JNE:
        case OPCODE_LESS_JNE:
        case OPCODE_MORE_JNE:
        case OPCODE_EQUAL_JNE:
        case OPCODE_MOREEQUAL_JNE:
        case OPCODE_LESSEQUAL_JNE:
        case OPCODE_NOTEQUAL_JNE:
            touch_addr(in.ops[0], false, reads, writes, side, trap);
            touch_addr(in.ops[1], false, reads, writes, side, trap);
            ctrl = true;
            if (in.op != OPCODE_EQUAL_JNE && in.op != OPCODE_NOTEQUAL_JNE) {
                trap = true;
            }
            break;
        case OPCODE_NOT_JNE:
        case OPCODE_JNE:
            touch_addr(in.ops[0], false, reads, writes, side, trap);
            ctrl = true;
            break;
        case OPCODE_JMP:
            ctrl = true;
            break;
        case OPCODE_FOR:
            touch_addr(in.ops[0], false, reads, writes, side, trap);
            touch_addr(in.ops[0], true, reads, writes, side, trap);
            touch_addr(in.ops[1], false, reads, writes, side, trap);
            touch_addr(in.ops[2], false, reads, writes, side, trap);
            ctrl = true;
            trap = true;
            break;
        case OPCODE_RETURN: {
            int retnum = (int) COMMAND_CODE(in.ops[0]);
            for (int i = 0; i < retnum && i + 1 < (int) in.ops.size(); i++) {
                touch_addr(in.ops[i + 1], false, reads, writes, side, trap);
            }
            ctrl = true;
        }
            break;
        case OPCODE_CALL: {
            side = true;
            touch_addr(in.ops[1], false, reads, writes, side, trap);
            int retnum = (int) COMMAND_CODE(in.ops[2]);
            int p = 3;
            for (int i = 0; i < retnum && p < (int) in.ops.size(); i++, p++) {
                touch_addr(in.ops[p], true, reads, writes, side, trap);
            }
            if (p < (int) in.ops.size()) {
                int argnum = (int) COMMAND_CODE(in.ops[p]);
                p++;
                for (int i = 0; i < argnum && p < (int) in.ops.size(); i++, p++) {
                    touch_addr(in.ops[p], false, reads, writes, side, trap);
                }
            }
        }
            break;
        default:
            side = true;
            break;
    }
}

void Opt::subst_use_ops(Insn &in, int insn_i) {
    auto subst = [&](command &c, int opidx) {
        if (insn_i < 0 || insn_i >= (int) use_ssa.size()) {
            return;
        }
        if (opidx < 0 || opidx >= (int) use_ssa[insn_i].size()) {
            return;
        }
        int id = use_ssa[insn_i][opidx];
        if (id < 0 || id >= (int) lattice.size()) {
            return;
        }
        if (lattice[id].kind == LAT_CONST) {
            c = lattice[id].c;
        }
    };
    if (in.op == OPCODE_ASSIGN && in.ops.size() >= 2) {
        subst(in.ops[1], 1);
    } else if (is_binop(in.op) && in.ops.size() >= 3) {
        subst(in.ops[0], 0);
        subst(in.ops[1], 1);
    } else if (in.op == OPCODE_NOT && in.ops.size() >= 2) {
        subst(in.ops[0], 0);
    } else if (is_math_assign(in.op) && in.ops.size() >= 2) {
        subst(in.ops[1], 1);
    } else if (is_cmp_jne(in.op) && in.ops.size() >= 2) {
        subst(in.ops[0], 0);
        subst(in.ops[1], 1);
    } else if ((in.op == OPCODE_JNE || in.op == OPCODE_NOT_JNE) && !in.ops.empty()) {
        subst(in.ops[0], 0);
    } else if (in.op == OPCODE_FOR && in.ops.size() >= 3) {
        subst(in.ops[1], 1);
        subst(in.ops[2], 2);
    } else if (in.op == OPCODE_RETURN && !in.ops.empty()) {
        int retnum = (int) COMMAND_CODE(in.ops[0]);
        for (int k = 0; k < retnum && k + 1 < (int) in.ops.size(); k++) {
            subst(in.ops[k + 1], k + 1);
        }
    } else if (in.op == OPCODE_CALL && in.ops.size() >= 3) {
        subst(in.ops[1], 1);
        int retnum = (int) COMMAND_CODE(in.ops[2]);
        int p = 3 + retnum;
        if (p < (int) in.ops.size()) {
            int argnum = (int) COMMAND_CODE(in.ops[p]);
            p++;
            for (int k = 0; k < argnum && p < (int) in.ops.size(); k++, p++) {
                subst(in.ops[p], p);
            }
        }
    }
}

void Opt::compute_dominators() {
    int nb = (int) blocks.size();
    idom.assign(nb, -1);
    dom_child.assign(nb, std::vector<int>());
    entry = 0;
    if (nb == 0) {
        return;
    }

    std::vector<int> post;
    std::vector<char> seen(nb, 0);
    std::vector<int> st;
    st.push_back(0);
    std::vector<int> it(nb, 0);
    seen[0] = 1;
    while (!st.empty()) {
        int b = st.back();
        if (it[b] < (int) blocks[b].succ.size()) {
            int s = blocks[b].succ[it[b]++];
            if (!seen[s]) {
                seen[s] = 1;
                st.push_back(s);
            }
        } else {
            post.push_back(b);
            st.pop_back();
        }
    }
    std::vector<int> rpo;
    for (int i = (int) post.size() - 1; i >= 0; i--) {
        rpo.push_back(post[i]);
    }
    std::vector<int> rpon(nb, -1);
    for (int i = 0; i < (int) rpo.size(); i++) {
        rpon[rpo[i]] = i;
    }

    auto intersect = [&](int b1, int b2) {
        int guard = 0;
        while (b1 != b2 && guard++ < nb + 2) {
            if (b1 < 0 || b2 < 0 || b1 >= nb || b2 >= nb) {
                return 0;
            }
            if (rpon[b1] < 0) {
                return (rpon[b2] >= 0) ? b2 : 0;
            }
            if (rpon[b2] < 0) {
                return b1;
            }
            while (b1 >= 0 && b1 < nb && rpon[b1] > rpon[b2]) {
                b1 = idom[b1];
                if (b1 < 0 || b1 >= nb) {
                    return 0;
                }
            }
            while (b2 >= 0 && b2 < nb && rpon[b2] > rpon[b1]) {
                b2 = idom[b2];
                if (b2 < 0 || b2 >= nb) {
                    return 0;
                }
            }
        }
        return (b1 >= 0 && b1 < nb) ? b1 : 0;
    };

    idom[0] = 0;
    bool ch = true;
    while (ch) {
        ch = false;
        for (int i = 1; i < (int) rpo.size(); i++) {
            int b = rpo[i];
            int new_idom = -1;
            for (int p : blocks[b].pred) {
                if (idom[p] < 0) {
                    continue;
                }
                if (new_idom < 0) {
                    new_idom = p;
                } else {
                    new_idom = intersect(p, new_idom);
                }
            }
            if (new_idom >= 0 && idom[b] != new_idom) {
                idom[b] = new_idom;
                ch = true;
            }
        }
    }
    for (int b = 1; b < nb; b++) {
        if (idom[b] >= 0 && idom[b] != b) {
            dom_child[idom[b]].push_back(b);
        }
    }
}

bool Opt::dominates(int a, int b) const {
    if (a < 0 || b < 0) {
        return false;
    }
    if (a == b) {
        return true;
    }
    int x = b;
    int guard = 0;
    while (x != entry && guard++ < (int) blocks.size() + 2) {
        if (x < 0 || x >= (int) idom.size()) {
            return false;
        }
        x = idom[x];
        if (x == a) {
            return true;
        }
        if (x < 0) {
            return false;
        }
    }
    return a == entry;
}

int Opt::new_ssa(int slot, int def_insn) {
    int id = (int) val_slot.size();
    val_slot.push_back(slot);
    val_def_insn.push_back(def_insn);
    lattice.push_back(Lat());
    ssa_insn_users.push_back(std::vector<int>());
    ssa_phi_users.push_back(std::vector<int>());
    return id;
}

void Opt::rename_block(int b, std::vector<std::vector<int> > &stk, std::vector<int> &saved) {
    if (b < 0 || b >= (int) blocks.size()) {
        return;
    }
    for (int pi : block_phis[b]) {
        Phi &ph = phis[pi];
        stk[ph.slot].push_back(ph.dest);
        saved.push_back(ph.slot);
    }
    for (int i = blocks[b].begin; i < blocks[b].end; i++) {
        Insn &in = insns[i];
        use_ssa[i].assign(in.ops.size(), -1);
        def_ssa[i] = -1;
        extra_defs[i].clear();
        auto use_op = [&](int opidx) {
            if (opidx < 0 || opidx >= (int) in.ops.size()) {
                return;
            }
            command c = in.ops[opidx];
            if (!cmd_is_addr(c) || cmd_addr_type(c) != ADDR_STACK) {
                return;
            }
            int slot = cmd_addr_pos(c);
            if (slot < 0 || slot >= nslots || stk[slot].empty()) {
                return;
            }
            int id = stk[slot].back();
            use_ssa[i][opidx] = id;
            ssa_insn_users[id].push_back(i);
        };
        auto def_op = [&](int opidx, bool extra) {
            if (opidx < 0 || opidx >= (int) in.ops.size()) {
                return;
            }
            command c = in.ops[opidx];
            if (!cmd_is_addr(c) || cmd_addr_type(c) != ADDR_STACK) {
                return;
            }
            int slot = cmd_addr_pos(c);
            if (slot < 0 || slot >= nslots) {
                return;
            }
            int id = new_ssa(slot, i);
            if (extra) {
                extra_defs[i].push_back(id);
            } else {
                def_ssa[i] = id;
            }
            stk[slot].push_back(id);
            saved.push_back(slot);
        };

        if (in.op == OPCODE_ASSIGN) {
            use_op(1);
            def_op(0, false);
        } else if (is_binop(in.op)) {
            use_op(0);
            use_op(1);
            def_op(2, false);
        } else if (in.op == OPCODE_NOT) {
            use_op(0);
            def_op(1, false);
        } else if (is_math_assign(in.op)) {
            use_op(0);
            use_op(1);
            def_op(0, false);
        } else if (is_cmp_jne(in.op)) {
            use_op(0);
            use_op(1);
        } else if (in.op == OPCODE_JNE || in.op == OPCODE_NOT_JNE) {
            use_op(0);
        } else if (in.op == OPCODE_FOR) {
            use_op(0);
            use_op(1);
            use_op(2);
            def_op(0, false);
        } else if (in.op == OPCODE_RETURN) {
            int retnum = in.ops.empty() ? 0 : (int) COMMAND_CODE(in.ops[0]);
            for (int k = 0; k < retnum; k++) {
                use_op(k + 1);
            }
        } else if (in.op == OPCODE_CALL && in.ops.size() >= 3) {
            use_op(1);
            int retnum = (int) COMMAND_CODE(in.ops[2]);
            int p = 3;
            for (int k = 0; k < retnum && p < (int) in.ops.size(); k++, p++) {
                def_op(p, k != 0);
                if (k == 0) {
                    /* def_ssa set */
                }
            }
            if (p < (int) in.ops.size()) {
                int argnum = (int) COMMAND_CODE(in.ops[p]);
                p++;
                for (int k = 0; k < argnum && p < (int) in.ops.size(); k++, p++) {
                    use_op(p);
                }
            }
        }
    }
    for (int s : blocks[b].succ) {
        int pred_i = 0;
        while (pred_i < (int) blocks[s].pred.size() && blocks[s].pred[pred_i] != b) {
            pred_i++;
        }
        if (pred_i >= (int) blocks[s].pred.size()) {
            continue;
        }
        for (int pi : block_phis[s]) {
            Phi &ph = phis[pi];
            if (pred_i < (int) ph.opnds.size() && ph.slot >= 0 && ph.slot < nslots && !stk[ph.slot].empty()) {
                ph.opnds[pred_i] = stk[ph.slot].back();
                ssa_phi_users[ph.opnds[pred_i]].push_back(pi);
            }
        }
    }
}

void Opt::build_ssa() {
    nslots = max_slot(fb, insns);
    phis.clear();
    block_phis.assign(blocks.size(), std::vector<int>());
    use_ssa.assign(n(), std::vector<int>());
    def_ssa.assign(n(), -1);
    extra_defs.assign(n(), std::vector<int>());
    val_slot.clear();
    val_def_insn.clear();
    lattice.clear();
    ssa_insn_users.clear();
    ssa_phi_users.clear();
    if (blocks.empty() || nslots <= 0) {
        return;
    }

    int nb = (int) blocks.size();
    std::vector<std::set<int> > df(nb);
    for (int b = 0; b < nb; b++) {
        if ((int) blocks[b].pred.size() < 2) {
            continue;
        }
        for (int p : blocks[b].pred) {
            int runner = p;
            int guard = 0;
            while (runner != idom[b] && runner >= 0 && guard++ < nb + 2) {
                df[runner].insert(b);
                runner = idom[runner];
            }
        }
    }

    std::vector<int> reads, writes;
    bool side, trap, ctrl;
    std::vector<std::vector<int> > assigns(nslots);
    for (int b = 0; b < nb; b++) {
        for (int i = blocks[b].begin; i < blocks[b].end; i++) {
            collect_rw(insns[i], reads, writes, side, trap, ctrl);
            for (int w : writes) {
                if (w >= 0 && w < nslots) {
                    if (assigns[w].empty() || assigns[w].back() != b) {
                        assigns[w].push_back(b);
                    }
                }
            }
        }
    }
    for (int s = 0; s < fb->m_paramnum && s < nslots; s++) {
        if (assigns[s].empty() || assigns[s].front() != 0) {
            assigns[s].insert(assigns[s].begin(), 0);
        }
    }

    for (int slot = 0; slot < nslots; slot++) {
        std::vector<int> work = assigns[slot];
        std::vector<char> has_phi(nb, 0);
        std::vector<char> on_work(nb, 0);
        for (int b : work) {
            on_work[b] = 1;
        }
        for (size_t wi = 0; wi < work.size(); wi++) {
            int b = work[wi];
            for (int d : df[b]) {
                if (has_phi[d]) {
                    continue;
                }
                has_phi[d] = 1;
                Phi ph;
                ph.slot = slot;
                ph.dest = -1;
                ph.block = d;
                ph.opnds.assign(blocks[d].pred.size(), -1);
                int pi = (int) phis.size();
                phis.push_back(ph);
                block_phis[d].push_back(pi);
                if (!on_work[d]) {
                    on_work[d] = 1;
                    work.push_back(d);
                }
            }
        }
    }

    for (int i = 0; i < (int) phis.size(); i++) {
        phis[i].dest = new_ssa(phis[i].slot, -1);
    }

    std::vector<std::vector<int> > stk(nslots);
    for (int s = 0; s < nslots; s++) {
        int id = new_ssa(s, -2);
        stk[s].push_back(id);
        lattice[id].kind = LAT_NAC;
    }
    std::vector<int> saved;
    struct Frame {
        int b;
        int ci;
        int mark;
    };
    std::vector<Frame> fr;
    auto pushf = [&](int b) {
        int mark = (int) saved.size();
        rename_block(b, stk, saved);
        fr.push_back(Frame{b, 0, mark});
    };
    pushf(0);
    while (!fr.empty()) {
        if (fr.back().ci < (int) dom_child[fr.back().b].size()) {
            int c = dom_child[fr.back().b][fr.back().ci++];
            pushf(c);
            continue;
        }
        int mark = fr.back().mark;
        while ((int) saved.size() > mark) {
            int slot = saved.back();
            saved.pop_back();
            if (slot >= 0 && slot < nslots && !stk[slot].empty()) {
                stk[slot].pop_back();
            }
        }
        fr.pop_back();
    }
}

void Opt::set_lat(int id, const Lat &nv, std::queue<int> &ssa_wl) {
    if (id < 0 || id >= (int) lattice.size()) {
        return;
    }
    Lat m = lat_meet(lattice[id], nv);
    if (lat_eq(lattice[id], m)) {
        return;
    }
    lattice[id] = m;
    ssa_wl.push(id);
}

Lat Opt::lat_of_op(const Insn &in, int insn_i, int opidx) const {
    Lat r;
    if (opidx < 0 || opidx >= (int) in.ops.size()) {
        r.kind = LAT_NAC;
        return r;
    }
    command c = in.ops[opidx];
    if (as_const(c)) {
        r.kind = LAT_CONST;
        r.c = c;
        return r;
    }
    if (insn_i < (int) use_ssa.size() && opidx < (int) use_ssa[insn_i].size()) {
        int id = use_ssa[insn_i][opidx];
        if (id >= 0 && id < (int) lattice.size()) {
            return lattice[id];
        }
    }
    r.kind = LAT_NAC;
    return r;
}

Lat Opt::eval_binop_lat(int op, const Lat &l, const Lat &r) {
    Lat o;
    if (l.kind == LAT_UNDEF || r.kind == LAT_UNDEF) {
        return o;
    }
    if (l.kind != LAT_CONST || r.kind != LAT_CONST) {
        o.kind = LAT_NAC;
        return o;
    }
    Insn tmp;
    tmp.op = op;
    tmp.ops.push_back(l.c);
    tmp.ops.push_back(r.c);
    tmp.ops.push_back(MAKE_ADDR(ADDR_STACK, 0));
    tmp.jump_to = -1;
    tmp.jump_op = -1;
    tmp.deleted = false;
    tmp.lineno = 0;
    if (!fold_binop(tmp) || tmp.op != OPCODE_ASSIGN) {
        o.kind = LAT_NAC;
        return o;
    }
    o.kind = LAT_CONST;
    o.c = tmp.ops[1];
    return o;
}

void Opt::mark_edge(int p, int s, std::queue<int> &cfg_wl) {
    if (p < 0 || s < 0 || p >= (int) exec_edge.size() || s >= (int) exec_edge[p].size()) {
        return;
    }
    if (exec_edge[p][s]) {
        return;
    }
    exec_edge[p][s] = 1;
    if (!exec_block[s]) {
        exec_block[s] = 1;
        cfg_wl.push(s);
    } else {
        cfg_wl.push(s);
    }
}

void Opt::visit_phi_sccp(int pi, std::queue<int> &ssa_wl) {
    if (pi < 0 || pi >= (int) phis.size()) {
        return;
    }
    Phi &ph = phis[pi];
    int b = ph.block;
    if (b < 0 || b >= (int) blocks.size() || !exec_block[b]) {
        return;
    }
    Lat m;
    for (int i = 0; i < (int) ph.opnds.size() && i < (int) blocks[b].pred.size(); i++) {
        int p = blocks[b].pred[i];
        if (!exec_edge[p][b]) {
            continue;
        }
        int id = ph.opnds[i];
        Lat in = (id >= 0 && id < (int) lattice.size()) ? lattice[id] : Lat();
        m = lat_meet(m, in);
    }
    set_lat(ph.dest, m, ssa_wl);
}

void Opt::visit_term_sccp(int b, std::queue<int> &cfg_wl) {
    if (blocks[b].begin >= blocks[b].end) {
        for (int s : blocks[b].succ) {
            mark_edge(b, s, cfg_wl);
        }
        return;
    }
    int last = blocks[b].end - 1;
    const Insn &in = insns[last];
    auto jump_block = [&]() {
        if (in.jump_to < 0 || in.jump_to >= n()) {
            return -1;
        }
        return insn_block[in.jump_to];
    };
    auto fall_block = [&]() {
        if (blocks[b].end >= n()) {
            return -1;
        }
        return insn_block[blocks[b].end];
    };
    if (in.op == OPCODE_JMP) {
        int jb = jump_block();
        if (jb >= 0) {
            mark_edge(b, jb, cfg_wl);
        }
        return;
    }
    if (in.op == OPCODE_RETURN) {
        return;
    }
    if (in.op == OPCODE_FOR) {
        for (int s : blocks[b].succ) {
            mark_edge(b, s, cfg_wl);
        }
        return;
    }

    bool is_br = (in.op == OPCODE_JNE || in.op == OPCODE_NOT_JNE || is_cmp_jne(in.op));
    if (!is_br) {
        int fb = fall_block();
        if (fb >= 0) {
            mark_edge(b, fb, cfg_wl);
        }
        return;
    }

    Lat cond;
    bool take_jump = false;
    bool known = false;
    if (in.op == OPCODE_JNE) {
        cond = lat_of_op(in, last, 0);
        if (cond.kind == LAT_CONST) {
            const variant *v = as_const(cond.c);
            if (v && v->type == variant::REAL) {
                known = true;
                take_jump = (v->data.real == 0);
            }
        }
    } else if (in.op == OPCODE_NOT_JNE) {
        cond = lat_of_op(in, last, 0);
        if (cond.kind == LAT_CONST) {
            const variant *v = as_const(cond.c);
            if (v && v->type == variant::REAL) {
                known = true;
                take_jump = (v->data.real != 0);
            }
        }
    } else {
        Lat l = lat_of_op(in, last, 0);
        Lat r = lat_of_op(in, last, 1);
        if (l.kind == LAT_CONST && r.kind == LAT_CONST) {
            if (eval_cmp_jne(in.op, as_const(l.c), as_const(r.c), take_jump)) {
                known = true;
            }
        } else if (l.kind == LAT_NAC || r.kind == LAT_NAC) {
            cond.kind = LAT_NAC;
        }
    }
    if (!known && cond.kind != LAT_NAC && !(is_cmp_jne(in.op))) {
        if (cond.kind == LAT_UNDEF) {
            return;
        }
    }
    if (!known && is_cmp_jne(in.op)) {
        Lat l = lat_of_op(in, last, 0);
        Lat r = lat_of_op(in, last, 1);
        if (l.kind == LAT_UNDEF || r.kind == LAT_UNDEF) {
            return;
        }
    }
    int jb = jump_block();
    int fl = fall_block();
    if (!known) {
        if (jb >= 0) {
            mark_edge(b, jb, cfg_wl);
        }
        if (fl >= 0) {
            mark_edge(b, fl, cfg_wl);
        }
        return;
    }
    if (take_jump) {
        if (jb >= 0) {
            mark_edge(b, jb, cfg_wl);
        }
    } else if (fl >= 0) {
        mark_edge(b, fl, cfg_wl);
    }
}

void Opt::visit_insn_sccp(int i, std::queue<int> &ssa_wl, std::queue<int> &cfg_wl) {
    (void) cfg_wl;
    if (i < 0 || i >= n()) {
        return;
    }
    int b = insn_block[i];
    if (b < 0 || !exec_block[b]) {
        return;
    }
    const Insn &in = insns[i];
    auto setdef = [&](const Lat &lv) {
        if (def_ssa[i] >= 0) {
            set_lat(def_ssa[i], lv, ssa_wl);
        }
        for (int d : extra_defs[i]) {
            set_lat(d, lv, ssa_wl);
        }
    };
    if (in.op == OPCODE_ASSIGN) {
        setdef(lat_of_op(in, i, 1));
    } else if (is_binop(in.op)) {
        setdef(eval_binop_lat(in.op, lat_of_op(in, i, 0), lat_of_op(in, i, 1)));
    } else if (in.op == OPCODE_NOT) {
        Lat l = lat_of_op(in, i, 0);
        Lat o;
        if (l.kind == LAT_CONST) {
            const variant *v = as_const(l.c);
            if (v && v->type == variant::REAL) {
                o.kind = LAT_CONST;
                o.c = MAKE_ADDR(ADDR_CONST, intern_real(v->data.real == 0 ? 1.0 : 0.0));
            } else {
                o.kind = LAT_NAC;
            }
        } else if (l.kind == LAT_NAC) {
            o.kind = LAT_NAC;
        }
        setdef(o);
    } else if (is_math_assign(in.op)) {
        int bop = math_assign_to_binop(in.op);
        setdef(eval_binop_lat(bop, lat_of_op(in, i, 0), lat_of_op(in, i, 1)));
    } else if (in.op == OPCODE_CALL || in.op == OPCODE_FOR) {
        Lat n;
        n.kind = LAT_NAC;
        setdef(n);
    }
}

bool Opt::fold_to_const_assign(Insn &in, const Lat &res) {
    if (res.kind != LAT_CONST) {
        return false;
    }
    command dest = 0;
    if (in.op == OPCODE_ASSIGN && in.ops.size() >= 1) {
        dest = in.ops[0];
    } else if (is_binop(in.op) && in.ops.size() >= 3) {
        dest = in.ops[2];
    } else if (in.op == OPCODE_NOT && in.ops.size() >= 2) {
        dest = in.ops[1];
    } else if (is_math_assign(in.op) && in.ops.size() >= 1) {
        dest = in.ops[0];
    } else {
        return false;
    }
    if (!cmd_is_addr(dest) || cmd_addr_type(dest) != ADDR_STACK) {
        return false;
    }
    replace_with_assign(in, dest, res.c);
    return true;
}

bool Opt::slot_still_holds(int def_i, int slot, int use_i) const {
    if (def_i < 0 || use_i < 0 || def_i >= n() || use_i >= n()) {
        return false;
    }
    int db = insn_block[def_i];
    int ub = insn_block[use_i];
    if (!dominates(db, ub)) {
        return false;
    }
    std::vector<int> reads, writes;
    bool side, trap, ctrl;
    for (int b = 0; b < (int) blocks.size(); b++) {
        if (!dominates(db, b) || !dominates(b, ub)) {
            continue;
        }
        int from = blocks[b].begin;
        int to = blocks[b].end;
        if (b == db) {
            from = def_i + 1;
        }
        if (b == ub) {
            to = use_i;
        }
        for (int i = from; i < to; i++) {
            collect_rw(insns[i], reads, writes, side, trap, ctrl);
            for (int w : writes) {
                if (w == slot) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool Opt::gvn() {
    if (blocks.empty()) {
        return false;
    }
    std::map<GvnKey, int> first;
    bool ch = false;
    std::vector<int> order;
    std::vector<int> st;
    st.push_back(0);
    while (!st.empty()) {
        int b = st.back();
        st.pop_back();
        order.push_back(b);
        for (int i = (int) dom_child[b].size() - 1; i >= 0; i--) {
            st.push_back(dom_child[b][i]);
        }
    }
    for (int b : order) {
        for (int i = blocks[b].begin; i < blocks[b].end; i++) {
            Insn &in = insns[i];
            if (!is_binop(in.op) || in.ops.size() < 3 || def_ssa[i] < 0) {
                continue;
            }
            if (!cmd_is_addr(in.ops[2]) || cmd_addr_type(in.ops[2]) != ADDR_STACK) {
                continue;
            }
            int a = (use_ssa[i].size() > 0) ? use_ssa[i][0] : -1;
            int bb = (use_ssa[i].size() > 1) ? use_ssa[i][1] : -1;
            GvnKey k;
            k.op = in.op;
            k.a = a;
            k.b = bb;
            std::map<GvnKey, int>::iterator it = first.find(k);
            if (it == first.end()) {
                first[k] = i;
                continue;
            }
            int prev = it->second;
            if (prev < 0 || prev >= n()) {
                continue;
            }
            command destp = insns[prev].ops.size() >= 3 ? insns[prev].ops[2] : 0;
            if (!cmd_is_addr(destp) || cmd_addr_type(destp) != ADDR_STACK) {
                continue;
            }
            int slot = cmd_addr_pos(destp);
            if (!slot_still_holds(prev, slot, i)) {
                continue;
            }
            replace_with_assign(in, in.ops[2], destp);
            ch = true;
        }
    }
    return ch;
}

bool Opt::ssa_sccp() {
    build_cfg();
    if (blocks.empty()) {
        return false;
    }
    compute_dominators();
    build_ssa();

    int nb = (int) blocks.size();
    exec_block.assign(nb, 0);
    exec_edge.assign(nb, std::vector<char>(nb, 0));
    std::queue<int> cfg_wl;
    std::queue<int> ssa_wl;
    exec_block[0] = 1;
    cfg_wl.push(0);

    int steps = 0;
    int limit = nb * (n() + 4) * 8 + 1024;
    while ((!cfg_wl.empty() || !ssa_wl.empty()) && steps++ < limit) {
        while (!cfg_wl.empty()) {
            int b = cfg_wl.front();
            cfg_wl.pop();
            if (b < 0 || b >= nb || !exec_block[b]) {
                continue;
            }
            for (int pi : block_phis[b]) {
                visit_phi_sccp(pi, ssa_wl);
            }
            for (int i = blocks[b].begin; i < blocks[b].end; i++) {
                visit_insn_sccp(i, ssa_wl, cfg_wl);
            }
            visit_term_sccp(b, cfg_wl);
        }
        while (!ssa_wl.empty()) {
            int id = ssa_wl.front();
            ssa_wl.pop();
            if (id < 0 || id >= (int) ssa_insn_users.size()) {
                continue;
            }
            for (int i : ssa_insn_users[id]) {
                visit_insn_sccp(i, ssa_wl, cfg_wl);
                int b = insn_block[i];
                if (b >= 0 && exec_block[b] && i == blocks[b].end - 1) {
                    visit_term_sccp(b, cfg_wl);
                }
            }
            for (int pi : ssa_phi_users[id]) {
                visit_phi_sccp(pi, ssa_wl);
            }
        }
    }

    bool ch = false;
    for (int i = 0; i < n(); i++) {
        int b = insn_block[i];
        if (b < 0 || !exec_block[b]) {
            if (!insns[i].deleted) {
                insns[i].deleted = true;
                ch = true;
            }
            continue;
        }
        subst_use_ops(insns[i], i);
        if (def_ssa[i] >= 0 && def_ssa[i] < (int) lattice.size()) {
            if (fold_to_const_assign(insns[i], lattice[def_ssa[i]])) {
                ch = true;
            }
        }
    }
    ch |= gvn();
    ch |= fold_constants();
    if (ch) {
        compact();
    }
    return ch;
}

bool Opt::copy_propagate() {
    build_cfg();
    bool ch = false;
    int ns = max_slot(fb, insns);
    for (int b = 0; b < (int) blocks.size(); b++) {
        std::vector<command> copies(ns, EMPTY_CMD);
        std::vector<char> has(ns, 0);
        std::vector<int> reads, writes;
        bool side, trap, ctrl;
        for (int i = blocks[b].begin; i < blocks[b].end; i++) {
            Insn &in = insns[i];
            auto subst = [&](command &c) {
                if (!cmd_is_addr(c) || cmd_addr_type(c) != ADDR_STACK) {
                    return;
                }
                int sp = cmd_addr_pos(c);
                if (sp >= 0 && sp < ns && has[sp]) {
                    c = copies[sp];
                    ch = true;
                }
            };
            if (in.op == OPCODE_ASSIGN && in.ops.size() >= 2) {
                subst(in.ops[1]);
            } else if (is_binop(in.op) && in.ops.size() >= 3) {
                subst(in.ops[0]);
                subst(in.ops[1]);
            } else if (in.op == OPCODE_NOT && in.ops.size() >= 2) {
                subst(in.ops[0]);
            } else if (is_math_assign(in.op) && in.ops.size() >= 2) {
                subst(in.ops[1]);
            } else if (is_cmp_jne(in.op) && in.ops.size() >= 2) {
                subst(in.ops[0]);
                subst(in.ops[1]);
            } else if ((in.op == OPCODE_JNE || in.op == OPCODE_NOT_JNE) && !in.ops.empty()) {
                subst(in.ops[0]);
            } else if (in.op == OPCODE_RETURN && !in.ops.empty()) {
                int retnum = (int) COMMAND_CODE(in.ops[0]);
                for (int k = 0; k < retnum && k + 1 < (int) in.ops.size(); k++) {
                    subst(in.ops[k + 1]);
                }
            } else if (in.op == OPCODE_CALL && in.ops.size() >= 3) {
                subst(in.ops[1]);
                int retnum = (int) COMMAND_CODE(in.ops[2]);
                int p = 3 + retnum;
                if (p < (int) in.ops.size()) {
                    int argnum = (int) COMMAND_CODE(in.ops[p]);
                    p++;
                    for (int k = 0; k < argnum && p < (int) in.ops.size(); k++, p++) {
                        subst(in.ops[p]);
                    }
                }
            }
            collect_rw(in, reads, writes, side, trap, ctrl);
            for (int w : writes) {
                if (w >= 0 && w < ns) {
                    has[w] = 0;
                }
                for (int s = 0; s < ns; s++) {
                    if (has[s] && cmd_is_addr(copies[s]) && cmd_addr_type(copies[s]) == ADDR_STACK &&
                        cmd_addr_pos(copies[s]) == w) {
                        has[s] = 0;
                    }
                }
            }
            if (in.op == OPCODE_ASSIGN && in.ops.size() >= 2 &&
                cmd_is_addr(in.ops[0]) && cmd_addr_type(in.ops[0]) == ADDR_STACK) {
                command src = in.ops[1];
                if (cmd_is_addr(src) && (cmd_addr_type(src) == ADDR_STACK || cmd_addr_type(src) == ADDR_CONST)) {
                    int d = cmd_addr_pos(in.ops[0]);
                    if (d >= 0 && d < ns) {
                        copies[d] = src;
                        has[d] = 1;
                    }
                }
            }
        }
    }
    return ch;
}

bool Opt::dest_forward() {
    int ns = max_slot(fb, insns);
    std::vector<int> reads(ns, 0);
    std::vector<int> writes(ns, 0);
    std::vector<int> last_def(ns, -1);
    std::vector<int> rtmp, wtmp;
    bool side, trap, ctrl;
    for (int i = 0; i < n(); i++) {
        collect_rw(insns[i], rtmp, wtmp, side, trap, ctrl);
        for (int r : rtmp) {
            if (r >= 0 && r < ns) {
                reads[r]++;
            }
        }
        for (int w : wtmp) {
            if (w >= 0 && w < ns) {
                writes[w]++;
                last_def[w] = i;
            }
        }
    }
    bool ch = false;
    build_cfg();
    for (int b = 0; b < (int) blocks.size(); b++) {
        for (int i = blocks[b].begin; i < blocks[b].end; i++) {
            Insn &in = insns[i];
            if (in.deleted || in.op != OPCODE_ASSIGN || in.ops.size() < 2) {
                continue;
            }
            if (!cmd_is_addr(in.ops[0]) || cmd_addr_type(in.ops[0]) != ADDR_STACK) {
                continue;
            }
            if (!cmd_is_addr(in.ops[1]) || cmd_addr_type(in.ops[1]) != ADDR_STACK) {
                continue;
            }
            int src = cmd_addr_pos(in.ops[1]);
            int dst = cmd_addr_pos(in.ops[0]);
            if (src < 0 || src >= ns || reads[src] != 1 || writes[src] != 1) {
                continue;
            }
            int def = last_def[src];
            if (def < 0 || def >= i || insn_block[def] != b) {
                continue;
            }
            Insn &d = insns[def];
            command *destp = 0;
            if (is_binop(d.op) && d.ops.size() >= 3 &&
                cmd_is_addr(d.ops[2]) && cmd_addr_type(d.ops[2]) == ADDR_STACK &&
                cmd_addr_pos(d.ops[2]) == src) {
                destp = &d.ops[2];
            } else if (d.op == OPCODE_NOT && d.ops.size() >= 2 &&
                       cmd_is_addr(d.ops[1]) && cmd_addr_type(d.ops[1]) == ADDR_STACK &&
                       cmd_addr_pos(d.ops[1]) == src) {
                destp = &d.ops[1];
            } else if (d.op == OPCODE_ASSIGN && d.ops.size() >= 2 &&
                       cmd_is_addr(d.ops[0]) && cmd_addr_type(d.ops[0]) == ADDR_STACK &&
                       cmd_addr_pos(d.ops[0]) == src) {
                destp = &d.ops[0];
            }
            if (!destp) {
                continue;
            }
            bool src_used = false;
            for (int k = def + 1; k < i; k++) {
                collect_rw(insns[k], rtmp, wtmp, side, trap, ctrl);
                for (int r : rtmp) {
                    if (r == src) {
                        src_used = true;
                    }
                }
                for (int w : wtmp) {
                    if (w == dst) {
                        src_used = true;
                    }
                }
            }
            if (src_used) {
                continue;
            }
            *destp = in.ops[0];
            in.deleted = true;
            ch = true;
        }
    }
    if (ch) {
        compact();
    }
    return ch;
}

bool Opt::dce() {
    int ns = max_slot(fb, insns);
    build_cfg();
    int nb = (int) blocks.size();
    if (nb == 0) {
        return false;
    }
    std::vector<std::vector<char> > live_in(nb, std::vector<char>(ns, 0));
    std::vector<std::vector<char> > live_out(nb, std::vector<char>(ns, 0));
    std::vector<int> reads, writes;
    bool side, trap, ctrl;

    bool changed = true;
    int guard = 0;
    while (changed && guard++ < 64) {
        changed = false;
        for (int b = nb - 1; b >= 0; b--) {
            std::vector<char> live(ns, 0);
            for (int s : blocks[b].succ) {
                for (int k = 0; k < ns; k++) {
                    if (live_in[s][k]) {
                        live[k] = 1;
                    }
                }
            }
            if (live != live_out[b]) {
                live_out[b] = live;
                changed = true;
            }
            for (int i = blocks[b].end - 1; i >= blocks[b].begin; i--) {
                collect_rw(insns[i], reads, writes, side, trap, ctrl);
                for (int w : writes) {
                    if (w >= 0 && w < ns) {
                        live[w] = 0;
                    }
                }
                for (int r : reads) {
                    if (r >= 0 && r < ns) {
                        live[r] = 1;
                    }
                }
            }
            if (live != live_in[b]) {
                live_in[b] = live;
                changed = true;
            }
        }
    }

    bool ch = false;
    for (int b = 0; b < nb; b++) {
        std::vector<char> live = live_out[b];
        for (int i = blocks[b].end - 1; i >= blocks[b].begin; i--) {
            collect_rw(insns[i], reads, writes, side, trap, ctrl);
            bool needed = ctrl || side || trap || writes.empty();
            if (!needed) {
                for (int w : writes) {
                    if (w < 0 || w >= ns || live[w]) {
                        needed = true;
                        break;
                    }
                }
            }
            if (!needed && !insns[i].deleted) {
                insns[i].deleted = true;
                ch = true;
            }
            for (int w : writes) {
                if (w >= 0 && w < ns) {
                    live[w] = 0;
                }
            }
            for (int r : reads) {
                if (r >= 0 && r < ns) {
                    live[r] = 1;
                }
            }
        }
    }
    if (ch) {
        compact();
    }
    return ch;
}

bool Opt::thread_jumps() {
    bool ch = false;
    for (int i = 0; i < n(); i++) {
        Insn &in = insns[i];
        if (in.deleted || in.jump_to < 0) {
            continue;
        }
        if (in.op != OPCODE_JMP && in.op != OPCODE_JNE && !is_cmp_jne(in.op) && in.op != OPCODE_NOT_JNE) {
            continue;
        }
        int t = in.jump_to;
        int guard = 0;
        while (t >= 0 && t < n() && insns[t].op == OPCODE_JMP && !insns[t].deleted && guard++ < n()) {
            if (insns[t].jump_to == t) {
                break;
            }
            t = insns[t].jump_to;
        }
        if (t != in.jump_to) {
            in.jump_to = t;
            ch = true;
        }
        if (in.op == OPCODE_JMP) {
            int next = i + 1;
            while (next < n() && insns[next].deleted) {
                next++;
            }
            if (in.jump_to == next || (in.jump_to >= n() && next >= n())) {
                in.deleted = true;
                ch = true;
            }
        }
    }
    if (ch) {
        compact();
    }
    return ch;
}

void Opt::rewrite_dummies() {
    command dummy = (fb->m_const_list_num > 0) ? MAKE_ADDR(ADDR_CONST, 0) : MAKE_ADDR(ADDR_STACK, 0);
    for (int i = 0; i < n(); i++) {
        Insn &in = insns[i];
        if (is_cmp_jne(in.op) && in.ops.size() >= 4) {
            in.ops[2] = dummy;
        } else if (in.op == OPCODE_NOT_JNE && in.ops.size() >= 3) {
            in.ops[1] = dummy;
        } else if (in.op == OPCODE_FOR && in.ops.size() >= 5) {
            in.ops[3] = dummy;
        }
    }
}

void Opt::remap_stack_cmd(command &c, const std::vector<int> &map) {
    if (!cmd_is_addr(c) || cmd_addr_type(c) != ADDR_STACK) {
        return;
    }
    int p = cmd_addr_pos(c);
    if (p >= 0 && p < (int) map.size() && map[p] >= 0) {
        c = MAKE_ADDR(ADDR_STACK, map[p]);
    }
}

void Opt::pack_stack() {
    int ns = max_slot(fb, insns);
    std::vector<char> used(ns, 0);
    for (int i = 0; i < fb->m_paramnum && i < ns; i++) {
        used[i] = 1;
    }
    auto mark = [&](command c) {
        if (cmd_is_addr(c) && cmd_addr_type(c) == ADDR_STACK) {
            int p = cmd_addr_pos(c);
            if (p >= 0 && p < ns) {
                used[p] = 1;
            }
        }
    };
    for (int i = 0; i < n(); i++) {
        for (size_t j = 0; j < insns[i].ops.size(); j++) {
            mark(insns[i].ops[j]);
        }
    }
    for (int i = 0; i < fb->m_container_addr_list_num; i++) {
        mark(fb->m_container_addr_list[i].con);
        mark(fb->m_container_addr_list[i].key);
    }
    std::vector<int> map(ns, -1);
    int next = 0;
    for (int i = 0; i < ns; i++) {
        if (used[i]) {
            map[i] = next++;
        }
    }
    bool hole = false;
    for (int i = 0; i < ns; i++) {
        if (map[i] != i && map[i] >= 0) {
            hole = true;
        }
        if (used[i] == 0 && i < next) {
            hole = true;
        }
    }
    if (!hole && next == ns) {
        fb->m_maxstack = ns;
        return;
    }
    for (int i = 0; i < n(); i++) {
        for (size_t j = 0; j < insns[i].ops.size(); j++) {
            remap_stack_cmd(insns[i].ops[j], map);
        }
    }
    for (int i = 0; i < fb->m_container_addr_list_num; i++) {
        remap_stack_cmd(fb->m_container_addr_list[i].con, map);
        remap_stack_cmd(fb->m_container_addr_list[i].key, map);
    }
    int dnum = fb->m_debug_stack_variant_info_num;
    if (dnum > 0) {
        stack_variant_info *nl = (stack_variant_info *) safe_fkmalloc(fk, dnum * sizeof(stack_variant_info),
                                                                      emt_func_binary);
        int k = 0;
        for (int i = 0; i < dnum; i++) {
            stack_variant_info info = fb->m_debug_stack_variant_info[i];
            if (info.pos >= 0 && info.pos < ns && map[info.pos] >= 0) {
                info.pos = map[info.pos];
                nl[k++] = info;
            }
        }
        safe_fkfree(fk, fb->m_debug_stack_variant_info);
        fb->m_debug_stack_variant_info = nl;
        fb->m_debug_stack_variant_info_num = k;
    }
    fb->m_maxstack = next;
}

void Opt::remap_container_cmd(command &c, const std::vector<int> &map) {
    if (!cmd_is_addr(c) || cmd_addr_type(c) != ADDR_CONTAINER) {
        return;
    }
    int p = cmd_addr_pos(c);
    if (p >= 0 && p < (int) map.size() && map[p] >= 0) {
        c = MAKE_ADDR(ADDR_CONTAINER, map[p]);
    }
}

void Opt::pack_containers() {
    int ncon = fb->m_container_addr_list_num;
    if (ncon <= 0) {
        return;
    }
    std::vector<char> used(ncon, 0);
    auto mark = [&](command c) {
        if (cmd_is_addr(c) && cmd_addr_type(c) == ADDR_CONTAINER) {
            int p = cmd_addr_pos(c);
            if (p >= 0 && p < ncon) {
                used[p] = 1;
            }
        }
    };
    for (int i = 0; i < n(); i++) {
        for (size_t j = 0; j < insns[i].ops.size(); j++) {
            mark(insns[i].ops[j]);
        }
    }
    for (int i = 0; i < ncon; i++) {
        mark(fb->m_container_addr_list[i].con);
        mark(fb->m_container_addr_list[i].key);
    }
    std::vector<int> map(ncon, -1);
    int next = 0;
    for (int i = 0; i < ncon; i++) {
        if (used[i]) {
            map[i] = next++;
        }
    }
    if (next == ncon) {
        return;
    }
    for (int i = 0; i < n(); i++) {
        for (size_t j = 0; j < insns[i].ops.size(); j++) {
            remap_container_cmd(insns[i].ops[j], map);
        }
    }
    container_addr *nl = 0;
    if (next > 0) {
        nl = (container_addr *) safe_fkmalloc(fk, next * sizeof(container_addr), emt_func_binary);
        int k = 0;
        for (int i = 0; i < ncon; i++) {
            if (used[i]) {
                nl[k] = fb->m_container_addr_list[i];
                remap_container_cmd(nl[k].con, map);
                remap_container_cmd(nl[k].key, map);
                k++;
            }
        }
    }
    safe_fkfree(fk, fb->m_container_addr_list);
    fb->m_container_addr_list = nl;
    fb->m_container_addr_list_num = next;
}

void Opt::run() {
    if (!fb->m_buff || fb->m_size <= 0) {
        return;
    }
    if (!decode()) {
        return;
    }
    for (int it = 0; it < 16; it++) {
        bool ch = false;
        ch |= fold_constants();
        ch |= ssa_sccp();
        ch |= copy_propagate();
        ch |= dest_forward();
        ch |= dce();
        ch |= thread_jumps();
        ch |= elim_unreachable();
        if (!ch) {
            break;
        }
    }
    rewrite_dummies();
    pack_stack();
    pack_containers();
    encode();
}

}  // namespace

optimizer::optimizer(fake *fk) {
    m_fk = fk;
}

optimizer::~optimizer() {
}

void optimizer::clear() {
}

void optimizer::optimize(func_binary &fb) {
    FKLOG("before %s", fb.dump(-1).c_str());
    Opt opt(m_fk, &fb);
    opt.run();
    FKLOG("after %s", fb.dump(-1).c_str());
}
