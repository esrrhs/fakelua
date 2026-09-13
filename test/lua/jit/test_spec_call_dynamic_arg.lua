local function foo(v)
    return v
end

local function my_abs(n)
    return n + 0
end

local function helper(x)
    -- foo(x) is a dynamic call whose result is passed to my_abs; exercises dynamic-arg path
    return x + my_abs(foo(x))
end

function test(x)
    local dummy = x + 1
    return helper(x)
end
