#include "compile/c_gen.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

CGen::CGen(State *s) : s_(s) {
}

void CGen::Generate(const CompileResult &cr, const CompileConfig &cfg) {
    LOG_INFO("start CGen::Generate {}", cr.file_name);

    
}

}// namespace fakelua
