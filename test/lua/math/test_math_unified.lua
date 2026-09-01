package "MathUnified"

-- 测试 math.abs 各种输入
function test_math_abs_various()
    -- 整数
    if math.abs(-10) ~= 10 then return 0 end
    if math.abs(10) ~= 10 then return 0 end
    -- 浮点数
    if math.abs(-3.14) ~= 3.14 then return 0 end
    -- 零
    if math.abs(0) ~= 0 then return 0 end
    -- 字符串数字
    if math.abs("-5") ~= 5 then return 0 end
    return 1
end

-- 测试 math.floor/ceil 各种输入
function test_math_floor_ceil_various()
    if math.floor(3.7) ~= 3 then return 0 end
    if math.ceil(3.2) ~= 4 then return 0 end
    if math.floor(-3.7) ~= -4 then return 0 end
    if math.ceil(-3.2) ~= -3 then return 0 end
    if math.floor(5) ~= 5 then return 0 end
    if math.ceil(5) ~= 5 then return 0 end
    return 1
end

-- 测试 math.sqrt
function test_math_sqrt_various()
    if math.sqrt(16) ~= 4 then return 0 end
    if math.sqrt(0) ~= 0 then return 0 end
    if math.sqrt(2) ~= math.sqrt(2) then return 0 end
    return 1
end

-- 测试 math.pow
function test_math_pow_various()
    if math.pow(2, 3) ~= 8 then return 0 end
    if math.pow(3, 2) ~= 9 then return 0 end
    if math.pow(2, 0) ~= 1 then return 0 end
    return 1
end

-- 测试 math.sin/cos/tan
function test_math_trig_various()
    local eps = 1e-10
    if math.abs(math.sin(0) - 0) > eps then return 0 end
    if math.abs(math.cos(0) - 1) > eps then return 0 end
    if math.abs(math.tan(0) - 0) > eps then return 0 end
    return 1
end

-- 测试 math.asin/acos
function test_math_asin_acos()
    local eps = 1e-10
    if math.abs(math.asin(0) - 0) > eps then return 0 end
    if math.abs(math.acos(1) - 0) > eps then return 0 end
    if math.abs(math.asin(1) - math.pi/2) > eps then return 0 end
    return 1
end

-- 测试 math.atan
function test_math_atan_various()
    local eps = 1e-10
    if math.abs(math.atan(0) - 0) > eps then return 0 end
    if math.abs(math.atan(1) - math.pi/4) > eps then return 0 end
    if math.abs(math.atan(1, 1) - math.pi/4) > eps then return 0 end
    return 1
end

-- 测试 math.exp/log
function test_math_exp_log_various()
    local eps = 1e-10
    if math.abs(math.exp(0) - 1) > eps then return 0 end
    if math.abs(math.log(math.exp(1)) - 1) > eps then return 0 end
    if math.abs(math.log10(100) - 2) > eps then return 0 end
    return 1
end

-- 测试 math.sinh/cosh/tanh
function test_math_hyperbolic()
    local eps = 1e-10
    if math.abs(math.sinh(0) - 0) > eps then return 0 end
    if math.abs(math.cosh(0) - 1) > eps then return 0 end
    if math.abs(math.tanh(0) - 0) > eps then return 0 end
    return 1
end

-- 测试 math.fmod
function test_math_fmod_various()
    local eps = 1e-10
    if math.abs(math.fmod(5.5, 2.0) - 1.5) > eps then return 0 end
    if math.abs(math.fmod(-5.5, 2.0) - (-1.5)) > eps then return 0 end
    return 1
end

-- 测试 math.ldexp
function test_math_ldexp_various()
    local eps = 1e-10
    if math.abs(math.ldexp(1.5, 3) - 12.0) > eps then return 0 end
    if math.abs(math.ldexp(1.5, -1) - 0.75) > eps then return 0 end
    return 1
end

-- 测试 math.deg/rad
function test_math_deg_rad_various()
    local eps = 1e-6
    if math.abs(math.deg(math.pi) - 180) > eps then return 0 end
    if math.abs(math.rad(180) - math.pi) > eps then return 0 end
    return 1
end

-- 测试 math.copysign
function test_math_copysign_various()
    if math.copysign(1.0, -2.0) >= 0 then return 0 end
    if math.copysign(-1.0, 2.0) < 0 then return 0 end
    return 1
end

-- 测试 math.modf
function test_math_modf_various()
    local i, f = math.modf(3.14)
    if i ~= 3.0 then return 0 end
    if math.abs(f - 0.14) > 1e-4 then return 0 end
    return 1
end

-- 测试 math.frexp
function test_math_frexp_various()
    local frac, exp = math.frexp(8.0)
    if frac ~= 0.5 then return 0 end
    if exp ~= 4 then return 0 end
    return 1
end

-- 测试 math.type
function test_math_type_various()
    if math.type(10) ~= "integer" then return 0 end
    if math.type(10.5) ~= "float" then return 0 end
    if math.type("10") ~= nil then return 0 end
    return 1
end

-- 测试 math.tointeger
function test_math_tointeger_various()
    if math.tointeger(15.0) ~= 15 then return 0 end
    if math.tointeger(15.5) ~= nil then return 0 end
    return 1
end

-- 测试 math.ult
function test_math_ult_various()
    if not math.ult(10, 20) then return 0 end
    if math.ult(20, 10) then return 0 end
    return 1
end

-- 测试 math.max/min 多参数
function test_math_max_min_various()
    if math.max(1, 5, 3, 9, 2) ~= 9 then return 0 end
    if math.min(1, 5, 3, 9, 2) ~= 1 then return 0 end
    return 1
end

-- 测试 math.random 各种情况
function test_math_random_various()
    math.randomseed(42)
    local r = math.random()
    if r < 0 or r >= 1 then return 0 end
    return 1
end

-- 测试 math.random(m)
function test_math_random_m()
    math.randomseed(42)
    local r = math.random(10)
    if r < 1 or r > 10 then return 0 end
    return 1
end

-- 测试 math.random(m, n)
function test_math_random_m_n()
    math.randomseed(42)
    local r = math.random(5, 10)
    if r < 5 or r > 10 then return 0 end
    return 1
end

-- 测试 math.randomseed
function test_math_randomseed_various()
    math.randomseed(12345)
    local r1 = math.random()
    math.randomseed(12345)
    local r2 = math.random()
    if r1 ~= r2 then return 0 end
    return 1
end
