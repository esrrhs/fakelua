-- Type inference: INT // INT = T_INT (floor division).
-- 7 // 2 = 3 (T_INT).
function test()
    local x = 7 // 2
    return x
end
