local function foo(v)
    return v
end

local function helper(x, xf)
    -- x is specialized int, xf is specialized float
    local dummy1 = x + 1
    local dummy2 = xf + 1.0
    -- Assign dynamic call result to specialized vars (fallback box-assignment path)
    x = foo(10)
    xf = foo(20.0)
    return x + xf
end

function test(x, xf)
    local temp = x + 0
    local tempf = xf + 0.0
    return helper(x, xf)
end
