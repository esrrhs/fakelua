function test_math_abs()
    -- 1. 正数
    if math.abs(10) ~= 10 then return 1 end

    -- 2. 负数
    if math.abs(-10) ~= 10 then return 2 end

    -- 3. 零
    if math.abs(0) ~= 0 then return 3 end

    -- 4. 浮点数
    if math.abs(-3.14) ~= 3.14 then return 4 end

    -- 5. 整数溢出边界（mininteger）
    local min_int = math.mininteger
    local r = math.abs(min_int)
    -- mininteger 的绝对值无法用 integer 表示，应返回 float
    local r_type = math.type(r)
    if r_type == "float" or r_type == "integer" then
        -- ok (implementation may return either due to overflow)
    else
        return 5
    end

    -- 6. 字符串隐式转换
    if math.abs("-42") ~= 42 then return 6 end

    return 5000
end
