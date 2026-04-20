-- Bottom-up / mixed-type inference:
-- 1. math_res uses *, - and // which are all typed, promoting to T_FLOAT.
-- 2. dynamic_res involves an unknown function call, forcing T_DYNAMIC.
function unknown_func()
    return 5
end

function test()
    -- Pure calculation: (10+20)*3.14 - (50//2) = 30*3.14 - 25 = 94.2 - 25 = 69.2
    -- All ops are typed: +→T_INT, *→T_FLOAT, //→T_INT, -→T_FLOAT
    local math_res = (10 + 20) * 3.14 - (50 // 2)

    -- Contains a function-call result → T_DYNAMIC
    local dynamic_res = 100 + unknown_func() * 2

    return dynamic_res   -- 100 + 5*2 = 110
end
