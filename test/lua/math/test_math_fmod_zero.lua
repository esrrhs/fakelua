-- Lua 5.4 math.fmod 的除零规则：
-- - 两个参数都是 integer 时，除数为 0 直接报错（"zero"）
-- - 任一参数是 float 时，结果为 NaN（IEEE 754），不报错

function test_math_fmod_zero()
    -- 整数 / 整数 / 0：必须抛错
    local ok1 = pcall(function() return math.fmod(5, 0) end)
    if ok1 then return 1 end
    local ok2 = pcall(function() return math.fmod(-7, 0) end)
    if ok2 then return 2 end

    -- 浮点 / 0：返回 NaN（NaN 不等于自身）
    local rf = math.fmod(5.0, 0.0)
    if rf == rf then return 3 end

    -- 混合类型也走浮点路径
    local rm = math.fmod(5, 0.0)
    if rm == rm then return 4 end

    -- 正常整数取模：结果是 integer
    local r2 = math.fmod(10, 3)
    if r2 ~= 1 then return 5 end
    if math.type(r2) ~= "integer" then return 6 end

    -- mininteger % -1 规定为 0（避开 C 有符号溢出）
    if math.fmod(math.mininteger, -1) ~= 0 then return 7 end

    -- 负数取模是 C 的截断向零语义
    if math.fmod(-7, 3) ~= -1 then return 8 end
    if math.fmod(7, -3) ~= 1 then return 9 end

    -- 浮点取模结果恒为 float
    local r3 = math.fmod(10.5, 3)
    if math.abs(r3 - 1.5) > 1e-9 then return 10 end
    if math.type(r3) ~= "float" then return 11 end

    return 5000
end
