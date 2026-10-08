-- Lua 5.4 字符串转数字规则（luaO_str2d / l_str2d）：
-- 1) 不接受 "inf"/"nan" 字面量（即使大小写混合）；
-- 2) 接受指数溢出：1e999 -> inf（ERANGE 不算失败）。
function test_basic_tonumber_lua54()
    -- inf/nan 任何大小写组合都拒绝
    if tonumber("inf") ~= nil then return 1 end
    if tonumber("INF") ~= nil then return 2 end
    if tonumber("Infinity") ~= nil then return 3 end
    if tonumber("-inf") ~= nil then return 4 end
    if tonumber("nan") ~= nil then return 5 end
    if tonumber("NaN") ~= nil then return 6 end
    if tonumber("NAN") ~= nil then return 7 end
    if tonumber("  inf  ") ~= nil then return 8 end
    -- 但直接构造的 math.huge / 0/0 是合法 number，直通
    if tonumber(math.huge) ~= math.huge then return 9 end
    if tonumber(0 / 0) == tonumber(0 / 0) then return 10 end

    -- 指数溢出：接受为 ±inf
    if tonumber("1e999") ~= math.huge then return 11 end
    if tonumber("-1e999") ~= -math.huge then return 12 end
    if math.type(tonumber("1e999")) ~= "float" then return 13 end
    -- 下溢到 0 也算成功
    if tonumber("1e-999") ~= 0.0 then return 14 end
    -- 十六进制浮点溢出同理
    if tonumber("0x1p9999") ~= math.huge then return 15 end

    -- 常规解析不受影响
    if tonumber("3.14") ~= 3.14 then return 20 end
    if tonumber("  12  ") ~= 12 then return 21 end
    if tonumber("0x10") ~= 16 then return 22 end
    if tonumber("0x1p4") ~= 16.0 then return 23 end
    if tonumber("0x1.8p3") ~= 12.0 then return 24 end
    if tonumber("abc") ~= nil then return 25 end
    if tonumber("") ~= nil then return 26 end
    if tonumber("12a") ~= nil then return 27 end

    return 5000
end
