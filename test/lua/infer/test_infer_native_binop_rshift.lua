-- Native RIGHT_SHIFT in CompileBinop via return expression (both operands T_INT).
-- local x = 256 (T_INT), return x >> 3 goes through
-- CompileExp -> CompileBinop native fast path using FlRShiftInt.
-- 256 >> 3 = 32
function test()
    local x = 256
    return x >> 3
end
