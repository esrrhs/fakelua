local function my_abs(x)
    if x > 0 then return x else return -x end
end

local function my_add(x)
    local y = x + 1.0
    do
        local y = my_abs(x)
        return y
    end
end

function test_shadow(x)
    return my_add(x)
end
