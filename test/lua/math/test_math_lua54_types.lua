-- Lua 5.4 数值库返回类型对齐：
-- floor/ceil/modf 的整数部分是 integer；fmod 整数对返回 integer、除零报错。
function test_math_lua54_types()
    -- floor：整数输入原样返回 integer；浮点结果可整数化时是 integer
    if math.type(math.floor(2.5)) ~= "integer" then return 1 end
    if math.floor(2.5) ~= 2 then return 2 end
    if math.type(math.floor(-2.5)) ~= "integer" then return 3 end
    if math.floor(-2.5) ~= -3 then return 4 end
    if math.type(math.floor(2)) ~= "integer" then return 5 end
    if math.type(math.floor(-0.0)) ~= "integer" then return 6 end

    -- ceil 同理
    if math.type(math.ceil(2.5)) ~= "integer" then return 7 end
    if math.ceil(2.5) ~= 3 then return 8 end
    if math.type(math.ceil(-2.5)) ~= "integer" then return 9 end
    if math.ceil(-2.5) ~= -2 then return 10 end

    -- pushnumint：无法表示为 int64 的结果保留为 float（不报错）
    if math.type(math.floor(1e100)) ~= "float" then return 11 end
    if math.floor(1e100) ~= 1e100 then return 12 end
    if math.type(math.floor(math.huge)) ~= "float" then return 13 end
    if math.type(math.ceil(-math.huge)) ~= "float" then return 14 end
    if math.type(math.floor(0 / 0)) ~= "float" then return 15 end
    -- 2^63 是第一个无法落进 int64 的整数
    if math.type(math.floor(2.0 ^ 63)) ~= "float" then return 16 end
    if math.type(math.floor(2.0 ^ 63 - 2.0 ^ 11)) ~= "integer" then return 17 end

    -- fmod：两整数 -> integer，C 截断向零
    if math.type(math.fmod(7, 3)) ~= "integer" then return 20 end
    if math.fmod(7, 3) ~= 1 or math.fmod(-7, 3) ~= -1 then return 21 end
    if math.fmod(7, -3) ~= 1 or math.fmod(-7, -3) ~= -1 then return 22 end
    -- 任一 float -> 恒 float，除零为 NaN 不报错
    if math.type(math.fmod(7.0, 3)) ~= "float" then return 23 end
    if math.type(math.fmod(7, 3.0)) ~= "float" then return 24 end
    local nanf = math.fmod(5.0, 0.0)
    if nanf == nanf then return 25 end
    -- 整数除零必须报错
    if pcall(function() return math.fmod(5, 0) end) then return 26 end
    if pcall(function() return math.fmod(0, 0) end) then return 27 end
    -- mininteger % -1 规定为 0
    if math.fmod(math.mininteger, -1) ~= 0 then return 28 end

    -- modf：整数部分 integer，小数部分 float；整数值时小数部分 +0.0
    local a, b = math.modf(3.5)
    if math.type(a) ~= "integer" or a ~= 3 then return 30 end
    if math.type(b) ~= "float" or b ~= 0.5 then return 31 end
    local c, d = math.modf(-2.25)
    if c ~= -2 or math.abs(d + 0.25) > 1e-9 then return 32 end
    local e, f = math.modf(3)
    if math.type(e) ~= "integer" or e ~= 3 then return 33 end
    if math.type(f) ~= "float" or f ~= 0.0 then return 34 end
    local g, h = math.modf(math.huge)
    if math.type(g) ~= "float" or g ~= math.huge then return 35 end
    if h ~= 0.0 then return 36 end
    local i2, j2 = math.modf(-0.0)
    if math.type(i2) ~= "integer" or i2 ~= 0 then return 37 end
    if j2 ~= 0.0 then return 38 end

    return 5000
end
