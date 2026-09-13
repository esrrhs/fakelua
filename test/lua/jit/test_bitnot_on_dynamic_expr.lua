local function foo(v)
    return v
end

local function helper(x)
    local dummy = x + 1
    -- foo(x) is a dynamic expression; ~ on it exercises the dynamic bitnot path
    local not_val = ~foo(x)
    return not_val
end

function test(x)
    local temp = x + 0
    return helper(x)
end
