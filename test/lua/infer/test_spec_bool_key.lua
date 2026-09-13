function test_bool_key()
    local t = { [true] = 10, [false] = 20 }
    t[true] = 100
    return t[true] + t[false]
end
