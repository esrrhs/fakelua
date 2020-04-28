#pragma once

#include "types.h"
#include "variant.h"

class debuging {
public:
    force_inline debuging(fake *fk) : m_fk(fk) {
    }

    force_inline ~debuging() {
    }

    force_inline fake *getfake() {
        return m_fk;
    }

    force_inline void clear() {
    }

    struct breakpoint {
        bool enable;
        int id;
        std::string file;
        int line;
    };

    typedef std::vector<breakpoint> breakpoint_list;

    void debug();

private:
    void resume(bool &isend);

    void show_debug_code(fake *fk, int rid, int frame, int range);

    void show_watch_variant(fake *fk, int rid, int frame, std::vector<String> &watchvec);

    void check_show_func_header(fake *fk, int rid, int frame, int &lastrid, const char *&lastfunc);

    bool check_trigger_breakpoint(fake *fk, breakpoint_list &blist);

    void show_debug_help(fake *fk);

    void get_debug_command(fake *fk, int &command, std::vector<String> &paramvec);

private:
    fake *m_fk;
    variant m_ret;
};

