-- Type inference: INT % INT = T_INT (modulo).
-- 7 % 3 = 1 (T_INT).
function test()
    local x = 7 % 3
    return x
end
