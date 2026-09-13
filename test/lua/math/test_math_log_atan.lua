-- Test math.log with base argument and math.atan with two arguments
-- Also covers math.atan2 explicitly

function test_math_log_with_base()
    -- math.log(1000, 10) should return ~3.0
    local r = math.log(1000, 10)
    if math.abs(r - 3.0) > 0.001 then return 1 end

    -- math.log(8, 2) should return 3.0
    local r2 = math.log(8, 2)
    if math.abs(r2 - 3.0) > 0.001 then return 2 end

    -- math.log(math.exp(1)) should return 1.0 (natural log)
    local r3 = math.log(math.exp(1))
    if math.abs(r3 - 1.0) > 0.001 then return 3 end

    return 5000
end

function test_math_atan_two_args()
    -- math.atan(y, x) is equivalent to math.atan2(y, x)
    local r = math.atan(1, 1)
    -- atan2(1, 1) = pi/4 ~ 0.785398
    if math.abs(r - math.pi / 4) > 0.001 then return 1 end

    -- math.atan(0, -1) should be pi
    local r2 = math.atan(0, -1)
    if math.abs(r2 - math.pi) > 0.001 then return 2 end

    -- math.atan(1) should be atan(1) ~ 0.785398
    local r3 = math.atan(1)
    if math.abs(r3 - math.pi / 4) > 0.001 then return 3 end

    return 5000
end

function test_math_modf_int()
    -- math.modf with integer input
    local int_part, frac = math.modf(7)
    if int_part ~= 7 then return 1 end
    if frac ~= 0.0 then return 2 end
    return 5000
end

function test_math_random_range()
    -- math.random(l, u) where l > u 应报错 "interval is empty"（对齐 Lua 5.4）
    local ok, err = pcall(function() return math.random(10, 5) end)
    if ok then return 1 end
    if not string.find(err, "interval is empty") then return 2 end

    -- math.random(l, u) where l == u should return l
    local r2 = math.random(7, 7)
    if r2 ~= 7 then return 3 end

    -- math.random(1) should return integer in [1, 1]
    local r3 = math.random(1)
    if r3 ~= 1 then return 4 end

    return 5000
end
