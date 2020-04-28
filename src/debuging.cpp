#include "debuging.h"
#include "fake.h"

void debuging::debug() {
    fake *fk = m_fk;
    bool isend = false;
    int range = 0;
    int command = debug_next;
    std::vector<String> paramvec;
    const char *lastfunc = 0;
    breakpoint_list blist;
    int bindex = 0;
    int frame = 0;
    int lastrid = fkgetcurroutineid(fk);
    int rid = lastrid;
    std::vector<String> watchvec;
    bool firsttime = true;
    while (1) {
        show_watch_variant(fk, rid, frame, watchvec);
        check_show_func_header(fk, rid, frame, lastrid, lastfunc);
        show_debug_code(fk, rid, frame, range);
        FAKEINPUT:
        get_debug_command(fk, command, paramvec);
        switch (command) {
            case debug_next: {
                frame = 0;
                rid = fkgetcurroutineid(fk);

                if (firsttime) {
                    if (check_trigger_breakpoint(fk, blist)) {
                        firsttime = false;
                        break;
                    }
                }
                firsttime = false;

                const char *lastfile = fkgetcurfile(fk);
                int lastline = fkgetcurline(fk);
                int laststacklength = fkgetcurcallstacklength(fk);
                int lastrid = rid;
                const char *curfile = fkgetcurfile(fk);
                int curline = fkgetcurline(fk);
                int curstacklength = fkgetcurcallstacklength(fk);
                int currid = rid;
                while (currid == lastrid &&
                       (curstacklength > laststacklength ||
                        (strcmp(lastfile, curfile) == 0 && lastline == curline))) {
                    resume(isend);
                    if (isend) {
                        break;
                    }
                    curfile = fkgetcurfile(fk);
                    curline = fkgetcurline(fk);
                    curstacklength = fkgetcurcallstacklength(fk);
                    currid = fkgetcurroutineid(fk);
                    if (check_trigger_breakpoint(fk, blist)) {
                        break;
                    }
                }

                rid = fkgetcurroutineid(fk);
            }
                break;
            case debug_step: {
                frame = 0;
                rid = fkgetcurroutineid(fk);

                if (firsttime) {
                    if (check_trigger_breakpoint(fk, blist)) {
                        firsttime = false;
                        break;
                    }
                }
                firsttime = false;

                const char *lastfile = fkgetcurfile(fk);
                int lastline = fkgetcurline(fk);
                int lastrid = rid;
                const char *curfile = fkgetcurfile(fk);
                int curline = fkgetcurline(fk);
                int currid = rid;
                while (currid == lastrid &&
                       (strcmp(lastfile, curfile) == 0 && lastline == curline)) {
                    resume(isend);
                    if (isend) {
                        break;
                    }
                    curfile = fkgetcurfile(fk);
                    curline = fkgetcurline(fk);
                    currid = fkgetcurroutineid(fk);
                    if (check_trigger_breakpoint(fk, blist)) {
                        break;
                    }
                }

                rid = fkgetcurroutineid(fk);
            }
                break;
            case debug_next_instruction: {
                frame = 0;
                rid = fkgetcurroutineid(fk);

                if (firsttime) {
                    if (check_trigger_breakpoint(fk, blist)) {
                        firsttime = false;
                        break;
                    }
                }
                firsttime = false;

                int laststacklength = fkgetcurcallstacklength(fk);
                int lastrid = rid;
                int curstacklength = fkgetcurcallstacklength(fk);
                int currid = rid;
                do {
                    resume(isend);
                    if (isend) {
                        break;
                    }
                    curstacklength = fkgetcurcallstacklength(fk);
                    currid = fkgetcurroutineid(fk);
                    if (check_trigger_breakpoint(fk, blist)) {
                        break;
                    }
                } while (currid == lastrid &&
                         curstacklength > laststacklength);

                rid = fkgetcurroutineid(fk);
            }
                break;
            case debug_step_instruction: {
                frame = 0;
                rid = fkgetcurroutineid(fk);

                if (firsttime) {
                    if (check_trigger_breakpoint(fk, blist)) {
                        firsttime = false;
                        break;
                    }
                }
                firsttime = false;

                resume(isend);

                rid = fkgetcurroutineid(fk);
                if (check_trigger_breakpoint(fk, blist)) {
                    break;
                }
            }
                break;
            case debug_continue: {
                frame = 0;
                rid = fkgetcurroutineid(fk);

                if (firsttime) {
                    if (check_trigger_breakpoint(fk, blist)) {
                        firsttime = false;
                        break;
                    }
                }
                firsttime = false;

                while (1) {
                    resume(isend);
                    rid = fkgetcurroutineid(fk);
                    if (check_trigger_breakpoint(fk, blist)) {
                        break;
                    }
                    if (isend) {
                        break;
                    }
                }
            }
                break;
            case debug_breakpoint: {
                breakpoint tmp;
                if (paramvec.empty()) {
                    tmp.file = fkgetcurfilebyroutinebyframe(fk, rid, frame);
                    tmp.line = fkgetcurlinebyroutinebyframe(fk, rid, frame);
                } else {
                    String str = paramvec[0];
                    int subpos = str.find(':');
                    if (subpos != -1) {
                        String filestr = str.substr(0, subpos);
                        String linestr = str.substr(subpos + 1);

                        tmp.file = filestr;
                        tmp.line = fkatoi(&linestr);
                    } else {
                        bool isnumber = true;
                        for (int i = 0; i < (int) str.size(); i++) {
                            if (!isdigit(str[i])) {
                                isnumber = false;
                                break;
                            }
                        }

                        if (isnumber) {
                            tmp.file = fkgetcurfilebyroutinebyframe(fk, rid, frame);
                            tmp.line = fkatoi(&str);
                        } else {
                            if (!fkisfunc(fk, str.c_str())) {
                                printf("%s is not func\n", str.c_str());
                                continue;
                            }

                            tmp.file = fkgetfuncfile(fk, str.c_str());
                            tmp.line = fkgetfuncstartline(fk, str.c_str());
                        }
                    }
                }
                tmp.enable = true;
                tmp.id = bindex;

                if ((int) tmp.file.find_last_of('/') != -1) {
                    tmp.file = tmp.file.substr(tmp.file.find_last_of('/') + 1);
                }

                blist.push_back(tmp);
                bindex++;
                printf("Breakpoint %d at file %s, line %d total %d\n", tmp.id, tmp.file.c_str(), tmp.line,
                       (int) blist.size());
            }
                break;
            case debug_enable: {
                if (paramvec.empty()) {
                    for (int i = 0; i < (int) blist.size(); i++) {
                        breakpoint &tmp = blist[i];
                        tmp.enable = true;
                    }
                } else {
                    int id = atoi(paramvec[0].c_str());
                    for (int i = 0; i < (int) blist.size(); i++) {
                        breakpoint &tmp = blist[i];
                        if (tmp.id == id) {
                            tmp.enable = true;
                        }
                    }
                }
            }
                break;
            case debug_disable: {
                if (paramvec.empty()) {
                    for (int i = 0; i < (int) blist.size(); i++) {
                        breakpoint &tmp = blist[i];
                        tmp.enable = false;
                    }
                } else {
                    int id = fkatoi(&paramvec[0]);
                    for (int i = 0; i < (int) blist.size(); i++) {
                        breakpoint &tmp = blist[i];
                        if (tmp.id == id) {
                            tmp.enable = false;
                        }
                    }
                }
            }
                break;
            case debug_delete: {
                if (paramvec.empty()) {
                    blist.clear();
                    watchvec.clear();
                    continue;
                } else {
                    int id = fkatoi(&paramvec[0]);
                    for (int i = 0; i < (int) blist.size(); i++) {
                        breakpoint &tmp = blist[i];
                        if (tmp.id == id) {
                            blist.erase(blist.begin() + i);
                        }
                    }
                }
            }
                break;
            case debug_info: {
                if (paramvec.empty()) {
                    printf("need arg, useage: i b, i r\n");
                    continue;
                }

                if (paramvec[0] == "b") {
                    printf("Id\tEnb\twhere\n");
                    for (int i = 0; i < (int) blist.size(); i++) {
                        breakpoint &tmp = blist[i];
                        printf("%d\t%s\tfile %s, line %d\n", tmp.id, tmp.enable ? "y" : "n", tmp.file.c_str(),
                               tmp.line);
                    }
                } else if (paramvec[0] == "r") {
                    for (int i = 0; i < (int) fkgetcurroutinenum(fk); i++) {
                        printf("%s%s%s\n", fkgetroutineidbyindex(fk, i) == rid ? "*" : "",
                               fkgetroutineidbyindex(fk, i) == fkgetcurroutineid(fk) ? "->" : "",
                               fkgetcurroutinebyindex(fk, i));
                    }
                }

                printf("\n");
            }
                break;
            case debug_finish: {
                frame = 0;
                rid = fkgetcurroutineid(fk);

                int laststacklength = fkgetcurcallstacklength(fk);
                int curstacklength = fkgetcurcallstacklength(fk);
                do {
                    resume(isend);
                    if (isend) {
                        break;
                    }
                    curstacklength = fkgetcurcallstacklength(fk);
                } while (curstacklength >= laststacklength);
            }
                break;
            case debug_list: {
                int listrange = 3;
                if (!paramvec.empty()) {
                    listrange = fkatoi(&paramvec[0]);
                }
                show_debug_code(fk, rid, frame, listrange);
                goto FAKEINPUT;
            }
                break;
            case debug_print: {
                if (paramvec.empty()) {
                    printf("need arg, useage: p variant\n");
                    continue;
                }

                String name = paramvec[0];
                printf("%s\n", fkgetcurvaiantbyroutinebyframe(fk, rid, frame, name.c_str()));
            }
                break;
            case debug_set: {
                if (paramvec.size() < 2) {
                    printf("need arg, useage: set variant value\n");
                    continue;
                }

                String name = paramvec[0];
                String value = paramvec[1];
                fksetcurvaiantbyroutinebyframe(fk, rid, frame, name.c_str(), value.c_str());
                printf("%s\n", fkgetcurvaiantbyroutinebyframe(fk, rid, frame, name.c_str()));
            }
                break;
            case debug_watch: {
                if (paramvec.empty()) {
                    printf("need arg, useage: wa variant\n");
                    continue;
                }

                String name = paramvec[0];
                watchvec.push_back(name);
            }
                break;
            case debug_backtrace: {
                int length = fkgetcurcallstacklengthbyroutine(fk, rid);
                for (int i = 0; i < length; i++) {
                    printf("%s%s\n", i == frame ? "*" : " ", fkgetcurcallstackbyroutinebyframe(fk, rid, i));
                }
                goto FAKEINPUT;
            }
                break;
            case debug_frame: {
                if (paramvec.empty()) {
                    frame = 0;
                } else {
                    int theframe = fkatoi(&paramvec[0]);
                    if (theframe < 0 || theframe >= fkgetcurcallstacklengthbyroutine(fk, rid)) {
                        printf("%d is invalid\n", theframe);
                    }
                    frame = theframe;
                }
            }
                break;
            case debug_disa: {
                int pos = fkgetcurbytecodeposbyroutine(fk, rid);
                const char *func = fkgetcurfuncbyroutinebyframe(fk, rid, frame);
                printf("%s\n", fkdumpfunc(fk, func, pos));
            }
                break;
            case debug_routine: {
                if (paramvec.empty()) {
                    printf("need arg, useage: r rid\n");
                    continue;
                }

                int id = fkatoi(&paramvec[0]);
                if (!fkishaveroutine(fk, id)) {
                    printf("no routine %d\n", id);
                    continue;
                }

                rid = id;
            }
                break;
            default:
                continue;
        }
        if (isend) {
            break;
        }
    }
    printf("end\n");

    // ret
    {
        fkpsclear(fk);
        variant *ret = 0;
        bool err = false;
        PS_PUSH_AND_GET(fk->ps, ret);
        *ret = m_ret;
        CHECK_ERR(err);
    }
}

void debuging::resume(bool &isend) {
    fake *fk = m_fk;
    fkpsclear(fk);
    fkresumeps(fk, isend);
    variant *ret = 0;
    bool err = false;
    PS_POP_AND_GET(fk->ps, ret);
    m_ret = *ret;
    CHECK_ERR(err);
}

void debuging::show_debug_code(fake *fk, int rid, int frame, int range) {
    int curline = fkgetcurlinebyroutinebyframe(fk, rid, frame);
    for (int i = curline - range; i <= curline + range; i++) {
        if (i > 0) {
            const char *code = fkgetfilecode(fk, fkgetcurfilebyroutinebyframe(fk, rid, frame), i);
            if (code) {
                printf("%s%d\t%s", curline == i ? "*" : "", i, code);
            }
        }
    }
    printf("\n");
}

void debuging::show_watch_variant(fake *fk, int rid, int frame, std::vector<String> &watchvec) {
    for (int i = 0; i < (int) watchvec.size(); i++) {
        printf("%s\n", fkgetcurvaiantbyroutinebyframe(fk, rid, frame, watchvec[i].c_str()));
    }
}

void debuging::check_show_func_header(fake *fk, int rid, int frame, int &lastrid, const char *&lastfunc) {
    const char *curfunc = fkgetcurfuncbyroutinebyframe(fk, rid, frame);
    if (rid != lastrid) {
        printf("routine %s\n", fkgetcurroutinebyid(fk, rid));
        lastrid = rid;
    }
    if (curfunc != lastfunc || strcmp(curfunc, lastfunc) != 0) {
        printf("file %s, line %d, func %s\n",
               fkgetcurfilebyroutinebyframe(fk, rid, frame),
               fkgetcurlinebyroutinebyframe(fk, rid, frame),
               curfunc);
        lastfunc = curfunc;
    }
}

bool debuging::check_trigger_breakpoint(fake *fk, breakpoint_list &blist) {
    String curfile = fkgetcurfile(fk);
    if ((int) curfile.find_last_of('/') != -1) {
        curfile = curfile.substr(curfile.find_last_of('/') + 1);
    }
    int line = fkgetcurline(fk);
    for (int i = 0; i < (int) blist.size(); i++) {
        breakpoint &tmp = blist[i];
        if (tmp.enable && tmp.line == line && tmp.file == curfile) {
            printf("Trigger Breakpoint %d at file %s, line %d\n", tmp.id, tmp.file.c_str(), tmp.line);
            return true;
        }
    }

    return false;
}

void debuging::show_debug_help(fake *fk) {
    printf("h help\n\
n\tnext\n\
s\tstep\n\
ni\tnext bytecode\n\
si\tstep bytecode\n\
c\tcontinue\n\
l\tlist\n\
p\tprint\n\
set\tset\n\
wa\twatch\n\
b\tbreakpoint\n\
en\tenable\n\
dis\tdisable\n\
d\tdelete\n\
i\tinfo\n\
bt\tbacktrace\n\
f\tframe\n\
fin\tfinish\n\
r\troutine\n\
disa\tview bytecode\n");
}

void debuging::get_debug_command(fake *fk, int &command, std::vector<String> &paramvec) {
    while (1) {
        char buff[100];
        printf("(fake) ");
        fgets(buff, sizeof(buff), stdin);
        std::remove(buff, buff + 100, '\n');
        std::remove(buff, buff + 100, '\r');
        if (!strlen(buff)) {
            if (command == -1) {
                continue;
            }
            return;
        }

        paramvec.clear();
        char *p = strtok(buff, " ");
        while (p) {
            paramvec.push_back(p);
            p = strtok(0, " ");
        }

        if (paramvec.empty()) {
            return;
        }

        String strcommand = paramvec[0];
        paramvec.erase(paramvec.begin());

        if (strcommand == "n") {
            command = debug_next;
        } else if (strcommand == "s") {
            command = debug_step;
        } else if (strcommand == "ni") {
            command = debug_next_instruction;
        } else if (strcommand == "si") {
            command = debug_step_instruction;
        } else if (strcommand == "c") {
            command = debug_continue;
        } else if (strcommand == "b") {
            command = debug_breakpoint;
        } else if (strcommand == "dis") {
            command = debug_disable;
        } else if (strcommand == "en") {
            command = debug_enable;
        } else if (strcommand == "d") {
            command = debug_delete;
        } else if (strcommand == "i") {
            command = debug_info;
        } else if (strcommand == "fin") {
            command = debug_finish;
        } else if (strcommand == "l") {
            command = debug_list;
        } else if (strcommand == "wa") {
            command = debug_watch;
        } else if (strcommand == "p") {
            command = debug_print;
        } else if (strcommand == "set") {
            command = debug_set;
        } else if (strcommand == "bt") {
            command = debug_backtrace;
        } else if (strcommand == "f") {
            command = debug_frame;
        } else if (strcommand == "disa") {
            command = debug_disa;
        } else if (strcommand == "r") {
            command = debug_routine;
        } else if (strcommand == "h") {
            show_debug_help(fk);
            continue;
        } else {
            printf("use h to get help\n");
            continue;
        }

        return;
    }
}

