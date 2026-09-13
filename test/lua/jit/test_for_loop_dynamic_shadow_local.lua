-- Shadow the loop variable with a local inside the dynamic for-loop body.
-- helper() forces the step to T_DYNAMIC (not typed), selecting the CVar
-- for-loop code path in c_gen.cpp.
-- i=1,3,5 (step=2); inner_i=2,6,10  => sum=18
function helper()
    return 2
end

function test()
    local sum = 0
    for i = 1, 5, helper() do
        local i = i * 2
        sum = sum + i
    end
    return sum
end
