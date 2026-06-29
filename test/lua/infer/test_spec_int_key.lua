function test_int_key()
    local t = { [1] = 10, [2] = 20, [3] = 30 }
    t[1] = 100
    return t[1] + t[2] + t[3]
end
