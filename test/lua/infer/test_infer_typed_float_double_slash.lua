-- Type inference: FLOAT // INT = T_FLOAT (floor division with float operand).
-- 7.0 // 2 = 3.0 (T_FLOAT).
function test()
    local a = 7.0
    local x = a // 2
    return x
end
