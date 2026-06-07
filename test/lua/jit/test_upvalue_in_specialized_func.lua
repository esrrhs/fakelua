local upvalue_int = 10
local upvalue_float = 20.0

local function helper(x)
    -- x is specialized float via dummy; uses both int and float upvalues
    local dummy = x + 1.0
    return dummy + upvalue_int + upvalue_float
end

function test(x)
    local tempf = x + 0.0
    return helper(x)
end
