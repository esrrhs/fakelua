local MY_CONST = 100

local function helper(x)
    local dummy = x + 1
    -- Assign global const of type T_INT to a fresh local variable
    local val_const = 0
    val_const = MY_CONST
    return val_const
end

function test(x)
    local temp = x + 0
    return helper(x)
end
