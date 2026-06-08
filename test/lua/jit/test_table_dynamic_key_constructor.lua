function test(x)
    -- [x] is a dynamic/variable key in the table constructor
    local t = { [x] = 2 }
    return t[x]
end
