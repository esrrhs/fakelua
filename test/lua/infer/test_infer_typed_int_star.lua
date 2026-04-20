-- Type inference: INT * INT = INT specialization.
-- x=3, y=x*4 stays T_INT; result is 12.
function test()
    local x = 3
    local y = x * 4
    return y
end
