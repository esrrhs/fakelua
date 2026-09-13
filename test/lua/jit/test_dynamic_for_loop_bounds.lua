local function foo(v)
    return v
end

local function helper(x)
    local dummy = x + 1
    -- Both bounds are dynamic (foo() calls), not literal constants
    local sum = 0
    for i = foo(1), foo(5) do
        sum = sum + i
    end
    return sum
end

function test(x)
    local temp = x + 0
    return helper(x)
end
