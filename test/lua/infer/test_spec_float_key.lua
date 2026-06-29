function test_float_key()
    local t = { [2.5] = 10, [3.5] = 20 }
    t[2.5] = 100
    return t[2.5] + t[3.5]
end
