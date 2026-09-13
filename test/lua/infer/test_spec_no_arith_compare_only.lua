-- Pure equality comparison with no arithmetic: n and m are only compared via ==
-- and returned as-is.  There is no arithmetic operation on either parameter, so
-- neither is a "math param" and no numeric specialisation should be generated.
-- The function must work with integers, floats, and strings alike.
function test(n, m)
    if n == m then
        return n
    end
    return m
end
