#include "fakelua.h"
#include "types.h"
#include "state.h"
#include "parser.h"

extern "C" fakelua_state *fakelua_newstate() {
    auto L = new fakelua_state();
    return L;
}

extern "C" void fakelua_close(fakelua_state *L) {
    delete L;
}

extern "C" int fakelua_dofile(fakelua_state *L, const char *file_path) {
    std::ifstream f(file_path, std::ios::in);
    if (!f.is_open()) {
        ERR("open fail %s", file_path);
        return FAKELUA_FILE_FAIL;
    }
    const auto sz = std::filesystem::file_size(file_path);
    if (sz < 0) {
        ERR("file_size %s", file_path);
        return FAKELUA_FILE_FAIL;
    }
    std::string content(sz, '\0');
    f.read(content.data(), sz);
    if (!f) {
        ERR("read fail %s", file_path);
        return FAKELUA_FILE_FAIL;
    }
    f.close();

    parser p;
    int ret = p.parse(file_path, content);
    if (ret != FAKELUA_OK) {
        ERR("parse fail %s", file_path);
        return ret;
    }

    // TODO run

    return FAKELUA_OK;
}

extern "C" int fakelua_dostring(fakelua_state *L, const char *content) {
    parser p;
    int ret = p.parse("", content);
    if (ret != FAKELUA_OK) {
        ERR("parse fail %s", content);
        return ret;
    }

    // TODO run

    return FAKELUA_OK;
}
