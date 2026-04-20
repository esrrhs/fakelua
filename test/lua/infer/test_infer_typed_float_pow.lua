-- Type inference: POW always produces T_FLOAT.
-- 2 ^ 10 = 1024.0 (T_FLOAT even with int operands).
function test()
    local x = 2 ^ 10
    return x
end
