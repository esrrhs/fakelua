function test_mixed_keys()
    local t = {
        [1] = 10,
        x = 20,
        [true] = 30,
        [2.5] = 40,
        50 -- implicit index 2
    }

    local a = t[1]
    local b = t.x
    local c = t[true]
    local d = t[2.5]
    local e = t[2]

    t[1] = 100
    t.x = 200
    t[true] = 300
    t[2.5] = 400
    t[2] = 500

    return a + b + c + d + e + t[1] + t.x + t[true] + t[2.5] + t[2]
end
