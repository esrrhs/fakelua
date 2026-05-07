-- Native SLASH in CompileBinop via return expression (result is T_FLOAT).
-- local x = 3 is inferred T_INT; x / 2 promotes to double.
function test()
    local x = 3
    return x / 2
end
