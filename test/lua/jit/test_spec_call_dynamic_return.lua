local function my_abs_dynamic(n)
    if n > 0 then
        return n
    else
        return "not a number"
    end
end

local function helper(x)
    -- my_abs_dynamic can return int or string; exercises dynamic-return path
    return x + my_abs_dynamic(5)
end

function test(x)
    local dummy = x + 1
    return helper(x)
end
