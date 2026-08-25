-- (double)INT64_MAX 会向上舍入成 2^63，用 <= 会把 2^63 强转成 int64（UB）。
-- 对齐 Lua 5.4：2^63 不能当整数；表键保持 float，且不得和 mininteger 撞槽。

function test_float_2pow63()
    local k = 2^63
    if math.type(k) ~= "float" then return 1 end
    if k ~= 9223372036854775808.0 then return 2 end

    -- math.tointeger：范围内的整值 float 仍转换；2^63 必须是 nil（不能 UB）
    if math.tointeger(15.0) ~= 15 then return 3 end
    if math.tointeger(k) ~= nil then return 4 end
    if math.tointeger(-k) ~= math.mininteger then return 5 end
    if math.tointeger(-math.mininteger) ~= nil then return 6 end

    -- 表键：2^63 不得归一成 mininteger
    local t = {}
    t[math.mininteger] = 1
    t[k] = 2
    if t[math.mininteger] ~= 1 then return 7 end
    if t[k] ~= 2 then return 8 end
    if t[-math.mininteger] ~= 2 then return 9 end

    -- 位运算：2^63 没有整数表示
    local ok, err = pcall(function() return 1 << k end)
    if ok then return 10 end
    if type(err) ~= "string" or not string.find(err, "integer representation") then return 11 end

    ok, err = pcall(function() return 1 & k end)
    if ok then return 12 end
    if type(err) ~= "string" or not string.find(err, "integer representation") then return 13 end

    return 100
end
