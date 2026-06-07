local function non_math_helper(x)
    return x
end

local function helper(x)
    -- non_math_helper returns its argument; exercises non-math-return branch
    return x + non_math_helper(5)
end

function test(x)
    local dummy = x + 1
    return helper(x)
end
