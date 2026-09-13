-- Native XOR in CompileBinop via return expression (both operands T_INT).
-- local x = 5 (T_INT), return x ~ 3 goes through
-- CompileExp -> CompileBinop native fast path (not CompileNumericExp).
-- 5 ~ 3 = 6
function test()
    local x = 5
    return x ~ 3
end
