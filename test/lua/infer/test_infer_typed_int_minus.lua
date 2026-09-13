-- Type inference: INT - INT = INT specialization.
-- x=10, x=x-3 stays T_INT; result is 7.
function test()
    local x = 10
    x = x - 3
    return x
end
