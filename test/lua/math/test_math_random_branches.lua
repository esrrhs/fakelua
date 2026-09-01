package "MathRandomBranches"

-- 测试 math.random(m) - 单参数分支
function test_random_single_arg()
    math.randomseed(42)
    for i = 1, 100 do
        local r = math.random(10)
        if r < 1 or r > 10 then
            return 0
        end
    end
    return 1
end

-- 测试 math.random(m, n) - 双参数分支
function test_random_two_args()
    math.randomseed(42)
    for i = 1, 100 do
        local r = math.random(5, 20)
        if r < 5 or r > 20 then
            return 0
        end
    end
    return 1
end

-- 测试 math.random(0) - 全范围特殊情况
function test_random_zero()
    math.randomseed(42)
    local r = math.random(0)
    if r == nil then return 0 end
    return 1
end

-- 测试 math.random(m, m) - l == u 快捷路径
function test_random_same_bounds()
    math.randomseed(42)
    local r = math.random(7, 7)
    if r ~= 7 then return 0 end
    return 1
end

-- 测试 math.random 负数区间 - 应该报错 (exception test, use GCC backend)
function test_random_negative_interval()
    math.random(-5)
    return 0  -- should not reach here
end

-- 测试 math.random(l, n) 空区间 - l > n 应该报错 (exception test, use GCC backend)
function test_random_empty_interval()
    math.random(10, 3)
    return 0  -- should not reach here
end

-- 测试 math.random 错误参数类型 (exception test, use GCC backend)
function test_random_bad_type()
    math.random("bad")
    return 0  -- should not reach here
end

-- 测试 math.random 双参数错误类型 (exception test, use GCC backend)
function test_random_two_args_bad_type()
    math.random(1, "bad")
    return 0  -- should not reach here
end

-- 测试 math.random 大范围
function test_random_large_range()
    math.randomseed(12345)
    for i = 1, 50 do
        local r = math.random(1, 1000000)
        if r < 1 or r > 1000000 then
            return 0
        end
    end
    return 1
end

-- 测试 math.random 全 uint64 范围
function test_random_full_uint64()
    math.randomseed(99999)
    local r = math.random(0, 0)  -- 可能触发全范围
    if r == nil then return 0 end
    return 1
end

-- 测试 math.modf - 浮点数
function test_modf_float()
    local intpart, fracpart = math.modf(3.7)
    if intpart ~= 3.0 then return 0 end
    if math.abs(fracpart - 0.7) > 1e-9 then return 0 end
    return 1
end

-- 测试 math.modf - 负数
function test_modf_negative()
    local intpart, fracpart = math.modf(-2.3)
    if intpart ~= -2.0 then return 0 end
    if math.abs(fracpart + 0.3) > 1e-9 then return 0 end
    return 1
end

-- 测试 math.modf - 整数输入
function test_modf_integer()
    local intpart, fracpart = math.modf(5)
    if intpart ~= 5 then return 0 end
    if fracpart ~= 0.0 then return 0 end
    return 1
end

-- 测试 math.modf - 零
function test_modf_zero()
    local intpart, fracpart = math.modf(0)
    if intpart ~= 0 then return 0 end
    if fracpart ~= 0.0 then return 0 end
    return 1
end

-- 测试 math.frexp - 正数
function test_frexp_positive()
    local mantissa, exponent = math.frexp(8.0)
    if mantissa ~= 0.5 then return 0 end
    if exponent ~= 4 then return 0 end
    return 1
end

-- 测试 math.frexp - 1.0
function test_frexp_one()
    local mantissa, exponent = math.frexp(1.0)
    if mantissa ~= 0.5 then return 0 end
    if exponent ~= 1 then return 0 end
    return 1
end

-- 测试 math.frexp - 零
function test_frexp_zero()
    local mantissa, exponent = math.frexp(0.0)
    if mantissa ~= 0.0 then return 0 end
    if exponent ~= 0 then return 0 end
    return 1
end

-- 测试 math.frexp - 小数
function test_frexp_fraction()
    local mantissa, exponent = math.frexp(0.75)
    if mantissa ~= 0.75 then return 0 end
    if exponent ~= 0 then return 0 end
    return 1
end

-- 测试 math.pow
function test_pow_basic()
    if math.abs(math.pow(2, 10) - 1024) > 1e-9 then return 0 end
    if math.abs(math.pow(9, 0.5) - 3.0) > 1e-9 then return 0 end
    return 1
end

-- 测试 math.pow - 负指数
function test_pow_negative_exp()
    if math.abs(math.pow(2, -1) - 0.5) > 1e-9 then return 0 end
    return 1
end

-- 测试 math.pow - 零指数
function test_pow_zero_exp()
    if math.abs(math.pow(5, 0) - 1) > 1e-9 then return 0 end
    return 1
end

-- 测试 math.tan
function test_tan()
    local eps = 1e-10
    if math.abs(math.tan(0) - 0) > eps then return 0 end
    return 1
end

-- 测试 math.tan(π/4)
function test_tan_pi_over_4()
    local eps = 1e-9
    if math.abs(math.tan(math.pi / 4) - 1) > eps then return 0 end
    return 1
end

-- 测试 math.exp
function test_exp()
    local eps = 1e-10
    if math.abs(math.exp(0) - 1) > eps then return 0 end
    if math.abs(math.exp(1) - 2.718281828) > 1e-5 then return 0 end
    return 1
end

-- 测试 math.log10
function test_log10()
    local eps = 1e-10
    if math.abs(math.log10(100) - 2.0) > eps then return 0 end
    if math.abs(math.log10(1000) - 3.0) > eps then return 0 end
    return 1
end

-- 测试 math.log10(1)
function test_log10_one()
    local eps = 1e-10
    if math.abs(math.log10(1) - 0) > eps then return 0 end
    return 1
end

-- 测试 math.sinh
function test_sinh()
    local eps = 1e-10
    if math.abs(math.sinh(0) - 0) > eps then return 0 end
    return 1
end

-- 测试 math.cosh
function test_cosh()
    local eps = 1e-10
    if math.abs(math.cosh(0) - 1.0) > eps then return 0 end
    return 1
end

-- 测试 math.tanh
function test_tanh()
    local eps = 1e-10
    if math.abs(math.tanh(0) - 0) > eps then return 0 end
    return 1
end

-- 测试 math.deg
function test_deg()
    local eps = 1e-9
    if math.abs(math.deg(math.pi) - 180.0) > eps then return 0 end
    if math.abs(math.deg(math.pi / 2) - 90.0) > eps then return 0 end
    return 1
end

-- 测试 math.rad
function test_rad()
    local eps = 1e-9
    if math.abs(math.rad(180) - math.pi) > eps then return 0 end
    if math.abs(math.rad(90) - math.pi / 2) > eps then return 0 end
    return 1
end

-- 测试 math.sqrt 边界
function test_sqrt_edge()
    if math.sqrt(0) ~= 0 then return 0 end
    if math.sqrt(1) ~= 1 then return 0 end
    return 1
end
