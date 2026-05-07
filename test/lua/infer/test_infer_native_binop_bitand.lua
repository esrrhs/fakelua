-- Native BITAND in CompileBinop via return expression (both operands T_INT).
-- local x = 12 (T_INT), return x & 10 goes through
-- CompileExp -> CompileBinop native fast path (not CompileNumericExp).
-- 12 & 10 = 8
function test()
    local x = 12
    return x & 10
end
