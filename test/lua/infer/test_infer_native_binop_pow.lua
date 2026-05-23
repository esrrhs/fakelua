-- Native POW in CompileBinop via return expression (result is T_FLOAT).
-- local x = 2 is inferred T_INT; x ^ 10 calls pow() with doubles.
function test()
    local x = 2
    return x ^ 10
end
