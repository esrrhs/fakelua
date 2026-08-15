-- 三个 CRITICAL 修复的边界测试，对齐 Lua 5.4 行为
-- 注意：避开 fakelua 已有的 JIT FlUnboxMulti bug——inline math.abs 单返回值被错误当作 multi。
-- 把 math.abs(INT64_MIN) 单独放一个函数，random 正常路径放另一个。

function test_math_critical_boundary()
    -- ===== #2 math.abs(INT64_MIN) =====
    local abs_check = check_abs_int64_min()
    if abs_check ~= 0 then return abs_check end

    -- ===== #3 math.random 正常路径 =====
    math.randomseed(42)
    local r0 = math.random(0)
    if r0 <= 2^53 and r0 >= -(2^53) then return 10 end

    -- random(10) 应在 [1,10]
    math.randomseed(12345)
    local r10 = math.random(10)
    if r10 < 1 or r10 > 10 then return 11 end

    -- random(5,10) 应在 [5,10]
    math.randomseed(999)
    local r510 = math.random(5, 10)
    if r510 < 5 or r510 > 10 then return 12 end

    -- random(l==u) 应返回 l
    local rll = math.random(42, 42)
    if rll ~= 42 then return 13 end

    return 9999
end

-- math.abs(INT64_MIN) 测试
-- 在 test_math_critical_boundary 内部调用，确保被 JIT 编译
local function check_abs_int64_min()
    local r = math.abs(-9223372036854775808)
    if r <= 0 then return 7 end
    if r ~= 9223372036854775808.0 then return 8 end
    return 0
end

-- 错误路径测试
function test_string_sub_overflow()
    string.sub("hello", 1e20)
end

function test_string_byte_overflow()
    string.byte("x", 1e20)
end

function test_math_random_neg()
    math.random(-5)
end

function test_math_random_reverse()
    math.random(5, 3)
end
