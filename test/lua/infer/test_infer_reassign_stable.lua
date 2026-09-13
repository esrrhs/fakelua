-- Type inference: reassigning an int variable to an int expression
-- keeps the variable at T_INT throughout (MergeType(T_INT, T_INT) = T_INT).
-- x starts as 1, x = x + 2 keeps x as T_INT; result is 3.
function test()
    local x = 1
    x = x + 2
    return x
end
