local function foo()
    return 5
end

local function my_abs(n)
    return n + 0
end

function test_math_specializations(y)
    local val1 = my_abs(foo()) + 1
    return val1
end

function test_math_string_arg()
    local val4 = my_abs "5"
    return val4
end
