-- Type inference: FLOAT % INT = T_FLOAT (modulo with float operand).
-- 7.5 % 2 = 1.5 (T_FLOAT).
function test()
    local a = 7.5
    local x = a % 2
    return x
end
