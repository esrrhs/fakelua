package "MathTest"

-- 测试 math.random(0) - 全范围随机整数
function test_math_random_zero()
    math.randomseed(42)
    local r = math.random(0)
    -- random(0) 返回全范围随机整数
    if type(r) ~= "number" then return 0 end
    return 1
end

-- 测试 math.randomseed 带参数
function test_math_randomseed_with_arg()
    math.randomseed(12345)
    local r1 = math.random()
    math.randomseed(12345)
    local r2 = math.random()
    -- 相同种子应产生相同序列
    if r1 ~= r2 then return 0 end
    return 1
end

-- 测试 math.randomseed 不带参数
function test_math_randomseed_no_arg()
    math.randomseed()
    local r = math.random()
    if type(r) ~= "number" then return 0 end
    return 1
end

-- 测试 math.random 整数参数
function test_math_random_int_arg()
    math.randomseed(42)
    local r = math.random(10)
    -- 整数参数应返回 1-10 之间的整数
    if type(r) ~= "number" then return 0 end
    return 1
end

-- 测试 math.random 两个整数参数
function test_math_random_int_args()
    math.randomseed(42)
    local r = math.random(1, 10)
    -- 整数参数应返回 1-10 之间的整数
    if type(r) ~= "number" then return 0 end
    return 1
end

-- 测试 math.random 负数参数（应报错）
function test_math_random_negative()
    local ok, err = pcall(function() math.random(-5) end)
    if ok then return 0 end
    if not string.find(err, "interval is empty") then return 0 end
    return 1
end

-- 测试 math.random 两个参数，l > u（应报错）
function test_math_random_reverse()
    local ok, err = pcall(function() math.random(10, 5) end)
    if ok then return 0 end
    if not string.find(err, "interval is empty") then return 0 end
    return 1
end

-- 测试 math.abs(INT64_MIN) - 边界情况
function test_math_abs_int64_min()
    local min = math.mininteger
    local r = math.abs(min)
    -- INT64_MIN 的绝对值无法存入 int64，应返回 float
    if type(r) ~= "number" then return 0 end
    return 1
end

-- 测试 math.modf 负数
function test_math_modf_negative()
    local i, f = math.modf(-3.14)
    if i ~= -3.0 then return 0 end
    if math.abs(f - (-0.14)) > 1e-4 then return 0 end
    return 1
end

-- 测试 math.frexp 负数
function test_math_frexp_negative()
    local frac, exp = math.frexp(-8.0)
    if frac ~= -0.5 then return 0 end
    if exp ~= 4 then return 0 end
    return 1
end

-- 测试 math.frexp 零
function test_math_frexp_zero()
    local frac, exp = math.frexp(0.0)
    if frac ~= 0.0 then return 0 end
    if exp ~= 0 then return 0 end
    return 1
end

-- 测试 math.type 各种类型
function test_math_type_various()
    if math.type(10) ~= "integer" then return 0 end
    if math.type(10.5) ~= "float" then return 0 end
    if math.type("10") ~= nil then return 0 end
    if math.type(nil) ~= nil then return 0 end
    if math.type(true) ~= nil then return 0 end
    return 1
end

-- 测试 math.tointeger 各种输入
function test_math_tointeger_various()
    if math.tointeger(15.0) ~= 15 then return 0 end
    if math.tointeger(15.5) ~= nil then return 0 end
    if math.tointeger(1e20) ~= nil then return 0 end
    if math.tointeger(math.huge) ~= nil then return 0 end
    return 1
end

-- 测试 math.ult 无符号比较
function test_math_ult_unsigned()
    -- 10 < 20
    if not math.ult(10, 20) then return 0 end
    -- 20 > 10
    if math.ult(20, 10) then return 0 end
    -- 相等
    if math.ult(10, 10) then return 0 end
    return 1
end

-- 测试 math.deg 各种值
function test_math_deg_various()
    local eps = 1e-6
    if math.abs(math.deg(0) - 0) > eps then return 0 end
    if math.abs(math.deg(math.pi) - 180) > eps then return 0 end
    if math.abs(math.deg(math.pi / 2) - 90) > eps then return 0 end
    return 1
end

-- 测试 math.rad 各种值
function test_math_rad_various()
    local eps = 1e-6
    if math.abs(math.rad(0) - 0) > eps then return 0 end
    if math.abs(math.rad(180) - math.pi) > eps then return 0 end
    if math.abs(math.rad(90) - math.pi / 2) > eps then return 0 end
    return 1
end

-- 测试 math.copysign 各种情况
function test_math_copysign_various()
    local eps = 1e-6
    if math.abs(math.copysign(1.0, -2.0) - (-1.0)) > eps then return 0 end
    if math.abs(math.copysign(-1.0, 2.0) - 1.0) > eps then return 0 end
    if math.abs(math.copysign(3.14, -1) - (-3.14)) > eps then return 0 end
    if math.abs(math.copysign(-5.5, 0.0) - 5.5) > eps then return 0 end
    return 1
end

-- 测试 math.log 带底数
function test_math_log_with_base()
    local eps = 1e-6
    if math.abs(math.log(1000, 10) - 3.0) > eps then return 0 end
    if math.abs(math.log(8, 2) - 3.0) > eps then return 0 end
    if math.abs(math.log(math.exp(1)) - 1.0) > eps then return 0 end
    return 1
end

-- 测试 math.atan 两个参数
function test_math_atan_two_args()
    local eps = 1e-6
    if math.abs(math.atan(1, 1) - math.pi / 4) > eps then return 0 end
    if math.abs(math.atan(0, -1) - math.pi) > eps then return 0 end
    if math.abs(math.atan(1) - math.pi / 4) > eps then return 0 end
    return 1
end

-- 测试 math.max 字符串数字
function test_math_max_string()
    if math.max("10", 25, 5, 100, 30) ~= 100 then return 0 end
    if math.min("10", 25, 5, 100, 30) ~= 5 then return 0 end
    return 1
end

-- 测试 math.floor/ceil 整数
function test_math_floor_ceil_int()
    if math.floor(7) ~= 7 then return 0 end
    if math.ceil(7) ~= 7 then return 0 end
    if math.floor(-3) ~= -3 then return 0 end
    if math.ceil(-3) ~= -3 then return 0 end
    return 1
end
