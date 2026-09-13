local function my_abs(n)
    return n + 0
end

function test()
    -- args is kString, not kExpList -- tests the string-arg branch in function call compilation
    local val = my_abs "5"
    return val
end
