package "BasicPairsCoverage"

-- 测试 pairs 迭代器 - 基本用法
function test_pairs_iterator_basic()
    local t = {a = 1, b = 2, c = 3}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 pairs 迭代器 - 空表
function test_pairs_iterator_empty()
    local t = {}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 0 then return 0 end
    return 1
end

-- 测试 pairs 迭代器 - 数组
function test_pairs_iterator_array()
    local t = {10, 20, 30}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 ipairs 迭代器 - 基本用法
function test_ipairs_iterator_basic()
    local t = {10, 20, 30}
    local count = 0
    for i, v in ipairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 ipairs 迭代器 - 空表
function test_ipairs_iterator_empty()
    local t = {}
    local count = 0
    for i, v in ipairs(t) do
        count = count + 1
    end
    if count ~= 0 then return 0 end
    return 1
end

-- 测试 ipairs 迭代器 - 索引正确性
function test_ipairs_iterator_index()
    local t = {"a", "b", "c"}
    local expected = {"a", "b", "c"}
    for i, v in ipairs(t) do
        if v ~= expected[i] then return 0 end
    end
    return 1
end

-- 测试 next 函数
function test_next_basic()
    local t = {x = 10, y = 20}
    local k, v = next(t)
    if k == nil then return 0 end
    return 1
end

-- 测试 next 空表
function test_next_empty()
    local t = {}
    local k, v = next(t)
    if k ~= nil then return 0 end
    return 1
end

-- 测试 select 函数
function test_select_basic()
    local result = select(2, "a", "b", "c")
    if result ~= "b" then return 0 end
    return 1
end

-- 测试 select 负数索引
function test_select_negative()
    local result = select(-1, "a", "b", "c")
    if result ~= "c" then return 0 end
    return 1
end

-- 测试 tonumber - 非字符串非数字返回 nil
function test_tonumber_non_string_non_number()
    if tonumber({}) ~= nil then return 0 end
    if tonumber(true) ~= nil then return 0 end
    if tonumber(false) ~= nil then return 0 end
    return 1
end

-- 测试 tonumber - 字符串数字
function test_tonumber_string_number()
    if tonumber("123") ~= 123 then return 0 end
    if tonumber("  42  ") ~= 42 then return 0 end
    if tonumber("-100") ~= -100 then return 0 end
    return 1
end

-- 测试 tonumber - 无效字符串返回 nil
function test_tonumber_invalid_string()
    if tonumber("notanumber") ~= nil then return 0 end
    if tonumber("   ") ~= nil then return 0 end
    if tonumber("") ~= nil then return 0 end
    return 1
end

-- 测试 tonumber - 十六进制
function test_tonumber_hex()
    if tonumber("0x1F", 16) ~= 31 then return 0 end
    if tonumber("FF", 16) ~= 255 then return 0 end
    return 1
end

-- 测试 tonumber - 进制中无效数字
function test_tonumber_invalid_digit_for_base()
    if tonumber("123", 2) ~= nil then return 0 end  -- 数字 2,3 在二进制中无效
    if tonumber("G", 16) ~= nil then return 0 end  -- G 在十六进制中无效
    return 1
end

-- 测试 tonumber - 浮点数
function test_tonumber_float()
    if math.abs(tonumber("1.5e10") - 1.5e10) > 1e-5 then return 0 end
    if math.abs(tonumber("3.14") - 3.14) > 1e-9 then return 0 end
    return 1
end

-- 测试 tostring - 已经是字符串
function test_tostring_already_string()
    if tostring("hello") ~= "hello" then return 0 end
    return 1
end

-- 测试 tostring - 数字
function test_tostring_number()
    if tostring(42) ~= "42" then return 0 end
    if tostring(3.14) ~= "3.14" then return 0 end
    return 1
end

-- 测试 type - 各种类型
function test_type_userdata()
    -- 测试基本类型
    if type(42) ~= "number" then return 0 end
    if type("hello") ~= "string" then return 0 end
    if type(true) ~= "boolean" then return 0 end
    if type(nil) ~= "nil" then return 0 end
    if type({}) ~= "table" then return 0 end
    return 1
end

-- 测试 type - 各种类型
function test_type_various()
    if type(42) ~= "number" then return 0 end
    if type("hello") ~= "string" then return 0 end
    if type(true) ~= "boolean" then return 0 end
    if type(nil) ~= "nil" then return 0 end
    if type({}) ~= "table" then return 0 end
    return 1
end

-- 测试 xpcall - 错误处理器
function test_xpcall_with_handler()
    local ok, err = xpcall(function() error("boom") end, function(e) return "caught: " .. e end)
    if ok then return 0 end  -- xpcall 返回 false 表示错误
    if not err:match("caught:") then return 0 end
    return 1
end

-- 测试 xpcall - 成功
function test_xpcall_success()
    local ok, result = xpcall(function() return 42 end, function(e) return e end)
    if not ok then return 0 end
    if result ~= 42 then return 0 end
    return 1
end
