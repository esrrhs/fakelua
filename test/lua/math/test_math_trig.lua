function test_math_trig()
    local pi = math.pi
    local s = math.sin(0.0)
    local c = math.cos(0.0)
    local t = math.tan(0.0)
    local h = math.huge
    local h_valid = (h > 1000000.0) and 180.0 or 0.0
    return pi + s + c + t + h_valid
end
