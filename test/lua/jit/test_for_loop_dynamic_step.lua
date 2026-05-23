-- Untyped for loop with explicit dynamic step.
-- helper() returns T_DYNAMIC (not typed), so the step expression is T_DYNAMIC.
-- This prevents typed_int_for optimization and exercises the CVar for-loop
-- path with a non-nil ExpStep (c_gen.cpp lines 2011-2012).
-- Iterations: i=1,3,5,7,9 => 5 iterations => result=5.
function helper()
    return 2
end

function test()
    local result = 0
    for i = 1, 10, helper() do
        result = result + 1
    end
    return result
end
