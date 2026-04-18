-- Bottom-up / mixed-type inference:
-- 1. math_res uses *, - and // which are not PLUS, so the expression is T_DYNAMIC.
-- 2. dynamic_res involves an unknown function call, forcing T_DYNAMIC too.
-- Both variables must be compiled via CVar arithmetic.
function unknown_func()
    return 5
end

function test()
    -- Pure calculation but involves non-PLUS operators → T_DYNAMIC
    local math_res = (10 + 20) * 3.14 - (50 // 2)

    -- Contains a function-call result → T_DYNAMIC
    local dynamic_res = 100 + unknown_func() * 2

    return dynamic_res   -- 100 + 5*2 = 110
end
