-- Native DOUBLE_SLASH int in CompileBinop via return expression.
-- local x = 7 is inferred T_INT; x // 2 uses FlFloorDivInt.
function test()
    local x = 7
    return x // 2
end
