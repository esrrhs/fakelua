-- Native LEFT_SHIFT in CompileBinop via return expression (both operands T_INT).
-- local x = 1 (T_INT), return x << 4 goes through
-- CompileExp -> CompileBinop native fast path using FlLShiftInt.
-- 1 << 4 = 16
function test()
    local x = 1
    return x << 4
end
