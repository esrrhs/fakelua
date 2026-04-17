-- Type inference: INT + FLOAT promotes to FLOAT.
-- a=3 (T_INT), b=a+1.5 must be T_FLOAT; result is 4.5.
function test()
    local a = 3
    local b = a + 1.5
    return b
end
