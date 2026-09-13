local function helper(x)
    return x + 1.0
end

local function test_impl(x)
    -- Both test_impl and helper are specialized float; exercises user-func float-ret path
    local dummy = x + 1.0
    return x + helper(x)
end

function test(x)
    local temp = x + 0.0
    return test_impl(x)
end
