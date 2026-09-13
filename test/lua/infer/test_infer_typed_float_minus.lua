-- Type inference: INT - FLOAT promotes to T_FLOAT.
-- x=3 (T_INT), y=x-1.5 must be T_FLOAT; result is 1.5.
function test()
    local x = 3
    local y = x - 1.5
    return y
end
