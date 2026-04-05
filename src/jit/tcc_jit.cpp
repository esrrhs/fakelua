#include "jit/tcc_jit.h"
#include "state/state.h"
#include "util/logging.h"

// Assuming libtcc.h is available in the include path
#include <libtcc.h>

namespace fakelua {

static void TccStateFree(void *ctx) {
    if (ctx) {
        tcc_delete((TCCState *)ctx);
    }
}

TccJitter::TccJitter(State *s) : s_(s) {}

void TccJitter::Compile(CompileResult &cr, const CompileConfig &cfg) {
    TCCState *s = tcc_new();
    if (!s) {
        LOG_ERROR("tcc_new failed");
        return;
    }

    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    // Add symbols that the JIT code might call
    tcc_add_symbol(s, "FakeluaAllocTemp", (void *)FakeluaAllocTemp);
    tcc_add_symbol(s, "FakeluaThrowError", (void *)FakeluaThrowError);

    if (tcc_compile_string(s, cr.c_code.c_str()) == -1) {
        LOG_ERROR("tcc_compile_string failed for {}", cr.file_name);
        tcc_delete(s);
        return;
    }

    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
        LOG_ERROR("tcc_relocate failed for {}", cr.file_name);
        tcc_delete(s);
        return;
    }

    // Currently we don't have a main function yet in CGen, 
    // but we can try to find one if it exists.
    // For now, CGen only generates the header.
    void *func_ptr = tcc_get_symbol(s, "__fakelua_main__");
    
    // Create VmFunction. Even if func_ptr is null, we store the state so it's alive.
    cr.main_func = std::make_shared<VmFunction>(func_ptr, 0, false, (void *)s, TccStateFree);
    
    LOG_INFO("TCC JIT compilation finished for {}", cr.file_name);
}

} // namespace fakelua
