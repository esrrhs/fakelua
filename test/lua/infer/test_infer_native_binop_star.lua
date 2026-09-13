-- Native STAR in CompileBinop via return expression (both operands T_INT).
-- local x = 3 is inferred T_INT; return x * 4 goes through
-- CompileExp -> CompileBinop native fast path (not CompileNumericExp).
function test()
    local x = 3
    return x * 4
end
