package "BasicTest"

-- 测试 select 数字参数
function test_select_number()
    local a, b, c = 10, 20, 30
    local r1 = select(1, a, b, c)
    local r2 = select(2, a, b, c)
    local r3 = select(3, a, b, c)
    if r1 ~= 10 then return 0 end
    if r2 ~= 20 then return 0 end
    if r3 ~= 30 then return 0 end
    return 1
end

-- 测试 select "#" 参数
function test_select_hash()
    local a, b, c = 10, 20, 30
    local r = select("#", a, b, c)
    if r ~= 3 then return 0 end
    return 1
end

-- 测试 select 超出范围
function test_select_out_of_range()
    local a, b = 10, 20
    local r = select(5, a, b)
    if r ~= nil then return 0 end
    return 1
end

-- 测试 error 基本
function test_error_basic()
    local ok, err = pcall(function() error("test error") end)
    if ok then return 0 end
    if not string.find(err, "test error") then return 0 end
    return 1
end

-- 测试 error 带 level
function test_error_with_level()
    local ok, err = pcall(function() error("level error", 2) end)
    if ok then return 0 end
    if not string.find(err, "level error") then return 0 end
    return 1
end

-- 测试 assert 成功
function test_assert_success()
    local r = assert(true, "should not error")
    if r ~= true then return 0 end
    return 1
end

-- 测试 assert 失败
function test_assert_fail()
    local ok, err = pcall(function() assert(false, "assert failed") end)
    if ok then return 0 end
    if not string.find(err, "assert failed") then return 0 end
    return 1
end

-- 测试 assert 无消息
function test_assert_no_msg()
    local ok, err = pcall(function() assert(false) end)
    if ok then return 0 end
    return 1
end

-- 测试 pcall 成功
function test_pcall_success()
    local ok, result = pcall(function() return 42 end)
    if not ok then return 0 end
    if result ~= 42 then return 0 end
    return 1
end

-- 测试 pcall 失败
function test_pcall_fail()
    local ok, err = pcall(function() error("pcall error") end)
    if ok then return 0 end
    if not string.find(err, "pcall error") then return 0 end
    return 1
end

-- 测试 xpcall 成功
function test_xpcall_success()
    local ok, result = xpcall(function() return 100 end, function(err) return err end)
    if not ok then return 0 end
    if result ~= 100 then return 0 end
    return 1
end

-- 测试 xpcall 失败
function test_xpcall_fail()
    local ok, err = xpcall(function() error("xpcall error") end, function(e) return e end)
    if ok then return 0 end
    if not string.find(err, "xpcall error") then return 0 end
    return 1
end

-- 测试 type 各种类型
function test_type_various()
    if type(42) ~= "number" then return 0 end
    if type("hello") ~= "string" then return 0 end
    if type(true) ~= "boolean" then return 0 end
    if type(nil) ~= "nil" then return 0 end
    if type({}) ~= "table" then return 0 end
    if type(function() end) ~= "function" then return 0 end
    return 1
end

-- 测试 tostring 各种类型
function test_tostring_various()
    if tostring(42) ~= "42" then return 0 end
    if tostring(true) ~= "true" then return 0 end
    if tostring(false) ~= "false" then return 0 end
    if tostring(nil) ~= "nil" then return 0 end
    return 1
end

-- 测试 tonumber 各种输入
function test_tonumber_various()
    if tonumber("42") ~= 42 then return 0 end
    if tonumber("3.14") ~= 3.14 then return 0 end
    if tonumber("abc") ~= nil then return 0 end
    if tonumber(nil) ~= nil then return 0 end
    return 1
end

-- 测试 print 不报错
function test_print_basic()
    print("test print", 42, true, nil)
    return 1
end

-- 测试 pairs 迭代
function test_pairs_iter()
    local t = {a=1, b=2, c=3}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 ipairs 迭代
function test_ipairs_iter()
    local t = {10, 20, 30}
    local count = 0
    local sum = 0
    for i, v in ipairs(t) do
        count = count + 1
        sum = sum + v
    end
    if count ~= 3 then return 0 end
    if sum ~= 60 then return 0 end
    return 1
end

-- 测试 next 函数
function test_next_func()
    local t = {a=1, b=2}
    local k, v = next(t)
    if k == nil then return 0 end
    local k2, v2 = next(t, k)
    if k2 == nil then return 0 end
    local k3 = next(t, k2)
    if k3 ~= nil then return 0 end
    return 1
end

-- 测试 collectgarbage
function test_collectgarbage()
    collectgarbage("collect")
    return 1
end
