#include "fakelua.h"
#include "types.h"
#include "state.h"
#include "parser.h"

extern "C" fakelua_state *fakelua_newstate() {
    fakelua_state * L= new fakelua_state();
    return L;
}

extern "C" void fakelua_close(fakelua_state *L) {
    delete L;
}

extern "C" int fakelua_dofile(fakelua_state *L, const char *file_path) {
    std::ifstream f(file_path, std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        return FAKELUA_FILE_FAIL;
    }
    const auto sz = std::filesystem::file_size(file_path);
    if (sz < 0) {
        return FAKELUA_FILE_FAIL;
    }
    std::string content(sz, '\0');
    f.read(content.data(), sz);
    if (!f) {
        return FAKELUA_FILE_FAIL;
    }
    parser p;
    int ret = p.parse(file_path, content);
    if (ret != FAKELUA_OK) {
        return ret;
    }

    // TODO run

    return FAKELUA_OK;
}

extern "C" int fakelua_dostring(fakelua_state *L, const char *content) {
    parser p;
    int ret = p.parse("", content);
    if (ret != FAKELUA_OK) {
        return ret;
    }

    // TODO run

    return FAKELUA_OK;
}
