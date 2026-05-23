-- Native MOD float in CompileBinop via return expression.
-- local x = 7.5 is inferred T_FLOAT; x % 3 uses FlModFloat.
function test()
    local x = 7.5
    return x % 3
end
