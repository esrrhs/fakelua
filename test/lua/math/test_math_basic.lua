function test_math_basic()
    local a = math.abs(-10)
    local b = math.floor(3.99)
    local c = math.ceil(3.01)
    local d = math.max(15, 20)
    local e = math.min(15, 20)
    local f = math.sqrt(16.0)
    local g = math.pow(2.0, 3.0)
    local h = math.deg(3.14159265358979323846 / 2)
    local i = math.rad(180)

    -- 数字字符串隐式类型转换测试
    if math.abs("-10") ~= 10 then return 0 end
    if math.floor("3.99") ~= 3 then return 0 end
    if math.ceil("3.01") ~= 4 then return 0 end

    return a + b + c + d + e + f + g + h + i
end
