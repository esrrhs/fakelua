-- Type inference: FLOAT * INT promotes to T_FLOAT.
-- x=2.5 (T_FLOAT), y=x*2 must be T_FLOAT; result is 5.0.
function test()
    local x = 2.5
    local y = x * 2
    return y
end
