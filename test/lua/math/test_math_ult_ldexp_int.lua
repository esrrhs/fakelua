-- math.ult / math.ldexp 的整数参数必须能无损落成 int64。
-- 以前 CVarToInteger(2^63) 回落到 0，JIT 还会对 float 读 .data_.i 或 (int)强转。

function test_math_ult_ldexp_int()
    -- 整数快路径
    if math.ult(10, 20) ~= true then return 1 end
    if math.ult(20, 10) ~= false then return 2 end
    if math.ult(-1, 0) ~= false then return 3 end
    if math.ult(0, -1) ~= true then return 4 end

    -- 恰好是整数的 float
    if math.ult(10.0, 20.0) ~= true then return 5 end

    -- 数字串（native CheckIntegerArg）
    if math.ult("10", 20) ~= true then return 6 end

    local ld = math.ldexp(1.5, 3)
    if math.abs(ld - 12.0) > 1e-12 then return 7 end
    if math.abs(math.ldexp(1.5, 3.0) - 12.0) > 1e-12 then return 8 end

    return 5000
end

function test_math_ult_2pow63()
    math.ult(2^63, 1)
end

function test_math_ult_frac()
    math.ult(1.5, 2)
end

function test_math_ldexp_2pow63()
    math.ldexp(1, 2^63)
end
