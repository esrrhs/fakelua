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
    DEBUG("fakelua_dofile start %s", file_path);
    std::ifstream f(file_path, std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        ERR("open fail %s", file_path);
        L->throw_err({FAKELUA_FILE_FAIL, string_format("can not open file %s", file_path)});
        return FAKELUA_FILE_FAIL;
    }
    const auto sz = std::filesystem::file_size(file_path);
    if (sz < 0) {
        ERR("file_size %s", file_path);
        L->throw_err({FAKELUA_FILE_FAIL, string_format("get file %s size error", file_path)});
        return FAKELUA_FILE_FAIL;
    }
    std::string content(sz, '\0');
    f.read(content.data(), sz);
    if (!f) {
        L->throw_err({FAKELUA_FILE_FAIL, string_format("read file %s fail", file_path)});
        ERR("read fail %s", file_path);
        return FAKELUA_FILE_FAIL;
    }
    f.close();

    parser p;
    int ret = p.parse(file_path, content);
    if (ret != FAKELUA_OK) {
        L->throw_err({FAKELUA_FILE_FAIL, string_format("parse file %s fail", file_path)});
        ERR("parse fail %s", file_path);
        return ret;
    }

    // TODO run

    DEBUG("fakelua_dofile ok %s", file_path);
    return FAKELUA_OK;
}

extern "C" int fakelua_dostring(fakelua_state *L, const char *content) {
    parser p;
    int ret = p.parse("", content);
    if (ret != FAKELUA_OK) {
        L->throw_err({FAKELUA_FILE_FAIL, string_format("parse content %s fail", content)});
        ERR("parse fail %s", content);
        return ret;
    }

    // TODO run

    return FAKELUA_OK;
}
