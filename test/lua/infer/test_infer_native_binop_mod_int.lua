-- Native MOD int in CompileBinop via return expression.
-- local x = 7 is inferred T_INT; x % 3 uses FlModInt.
function test()
    local x = 7
    return x % 3
end
