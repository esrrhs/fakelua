local function helper_bool_ret(x)
    return x > 0
end

local function test_impl(x)
    -- x is specialized int; exercises bool-return branch
    local dummy = x + 1
    local val = 0
    if helper_bool_ret(x) then
        val = 1
    end
    return val
end

function test(x)
    local temp = x + 0
    return test_impl(x)
end
