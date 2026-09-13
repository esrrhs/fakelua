local function foo(v)
    return v
end

local function helper_bitwise(x)
    -- x & 1 is a bitwise expr; result is passed as CVar to foo
    return foo(x & 1)
end

function test(y)
    -- Force y as int specialization
    local temp = y + 0
    return helper_bitwise(y)
end
