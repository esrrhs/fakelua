function test_math_basic()
    local a = math.abs(-10)
    local c = math.floor(3.99)
    local d = math.ceil(3.01)
    local e = math.max(15, 42)
    local f = math.min(15, 42)
    local g = math.sqrt(16.0)
    local h = math.pow(2.0, 3.0)
    return a + c + d + e + f + g + h
end

function test_math_trig()
    local pi = math.pi
    local s = math.sin(0.0)
    local c = math.cos(0.0)
    local d = math.deg(pi)
    return s + c + d
end
