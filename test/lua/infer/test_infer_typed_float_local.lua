-- Type inference success: float literal propagates through addition.
-- 1.0 + 0.5 = 1.5; x and y must be T_FLOAT.
function test()
    local x = 1.0
    local y = x + 0.5
    return y
end
