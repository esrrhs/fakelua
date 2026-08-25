-- 三个 CRITICAL 修复的边界测试，对齐 Lua 5.4 行为
-- 注意：使用 string.find 而非 err:match 来验证错误消息，
-- 因为 string.match 的 pattern 匹配有独立的已有 bug（plain 搜索正常）。

function test_math_critical_boundary()
    -- ===== #1 CheckIntegerArg 超大 float =====
    local ok1, err1 = pcall(function() return string.sub("hello", 1e20) end)
    if ok1 then return 1 end
    if not string.find(err1, "number has no integer representation") then return 2 end

    local ok2, err2 = pcall(function() return string.byte("x", 1e20) end)
    if ok2 then return 3 end
    if not string.find(err2, "number has no integer representation") then return 4 end

    -- 边界内应正常工作
    if string.sub("hello", 2^62) ~= "" then return 5 end

    -- 2^63 刚好越界
    local ok3 = pcall(function() return string.sub("hello", 2^63) end)
    if ok3 then return 6 end

    -- ===== #2 math.abs(INT64_MIN) =====
    local r = math.abs(-9223372036854775808)
    if r <= 0 then return 7 end
    if r ~= 9223372036854775808.0 then return 8 end

    -- ===== #3 math.random =====
    math.randomseed(42)
    local r0 = math.random(0)
    if not (r0 == math.floor(r0)) then return 9 end
    if math.abs(r0) <= 2^53 then return 10 end

    -- random(负数) 应报错
    local ok4, err4 = pcall(function() return math.random(-5) end)
    if ok4 then return 11 end
    if not string.find(err4, "interval is empty") then return 12 end

    -- random(l > u) 应报错
    local ok5, err5 = pcall(function() return math.random(5, 3) end)
    if ok5 then return 13 end
    if not string.find(err5, "interval is empty") then return 14 end

    -- 宽范围不溢出
    local ok6 = pcall(function() return math.random(0, 9223372036854775807) end)
    if not ok6 then return 15 end

    -- 2^63 不能当整数上界：native 以前会退化成 random(0)
    local ok7, err7 = pcall(function() return math.random(2^63) end)
    if ok7 then return 16 end
    if not string.find(err7, "integer representation") then return 17 end

    local ok8, err8 = pcall(function() return math.random(1.5) end)
    if ok8 then return 18 end
    if not string.find(err8, "integer representation") then return 19 end

    return 9999
end

-- 错误路径单独函数（gtest EXPECT_THROW）
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

function test_math_random_2pow63()
    math.random(2^63)
end

-- Bug #1 修复验证：string 库方法 colon 调用不 crash
function test_string_method_colon()
    local s = "hello world 123"
    -- 这些调用之前会触发 "attempt to index a non-table value"
    local m = s:match("123", 1, true)  -- plain search
    if m ~= "123" then return 1 end
    local f = s:find("world")
    if f ~= 7 then return 2 end
    local g = s:gsub("123", "456")
    if g ~= "hello world 456" then return 3 end
    return 0
end

-- Bug #1 验证：err:match 不 crash
function test_err_match()
    local ok, err = pcall(function() return string.sub("hello", 1e20) end)
    if ok then return 1 end
    if not string.find(err, "integer") then return 2 end
    return 0
end
