local function my_mixed_func(n, name)
    return n + 1
end

local function helper(x)
    -- x is specialized int; called with x (int) and "hello" (string)
    return x + my_mixed_func(x, "hello")
end

function test(x)
    local dummy = x + 1
    return helper(x)
end
