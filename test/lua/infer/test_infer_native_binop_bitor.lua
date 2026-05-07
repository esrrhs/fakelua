-- Native BITOR in CompileBinop via return expression (both operands T_INT).
-- local x = 12 (T_INT), return x | 3 goes through
-- CompileExp -> CompileBinop native fast path (not CompileNumericExp).
-- 12 | 3 = 15
function test()
    local x = 12
    return x | 3
end
