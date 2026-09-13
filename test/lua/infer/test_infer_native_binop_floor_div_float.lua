-- Native DOUBLE_SLASH float in CompileBinop via return expression.
-- local x = 7.0 is inferred T_FLOAT; x // 2 uses floor(double/double).
function test()
    local x = 7.0
    return x // 2
end
