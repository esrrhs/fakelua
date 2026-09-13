local function foo(v)
    return v
end

local function helper(xf)
    local dummy = xf + 1.0
    -- xf is specialized float, foo(xf) is dynamic; exercises mixed-type <= path
    local le_val = xf <= foo(xf)
    return le_val and 1 or 0
end

function test(xf)
    local tempf = xf + 0.0
    return helper(xf)
end
