function test_dynamic(k1, k2, k3)
    local t = {
        [1] = 10,
        x = 20,
        [true] = 30
    }
    t[k1] = 100
    local a = t[k1]
    local b = t[k2]
    local c = t[k3]
    return a + b + c
end
