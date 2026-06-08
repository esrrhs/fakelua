local function helper(x)
    return x + 1
end

local function test_impl(x)
    -- Both test_impl and helper are specialized int; exercises user-func int-ret path
    local dummy = x + 1
    return x + helper(x)
end

function test(x)
    local temp = x + 0
    return test_impl(x)
end
