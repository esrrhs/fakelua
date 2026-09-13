-- Type inference: SLASH always produces T_FLOAT.
-- 7 / 2 = 3.5 (T_FLOAT even with int operands).
function test()
    local x = 7 / 2
    return x
end
