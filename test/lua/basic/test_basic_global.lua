function test_basic_global()
    -- === type ===
    if type(nil) ~= "nil" then return 1 end
    if type(true) ~= "boolean" then return 2 end
    if type(false) ~= "boolean" then return 3 end
    if type(42) ~= "number" then return 4 end
    if type(3.14) ~= "number" then return 5 end
    if type("hello") ~= "string" then return 6 end
    if type({}) ~= "table" then return 7 end
    if type(function() end) ~= "function" then return 8 end

    -- === tostring ===
    if tostring(nil) ~= "nil" then return 10 end
    if tostring(true) ~= "true" then return 11 end
    if tostring(false) ~= "false" then return 12 end
    if tostring(42) ~= "42" then return 13 end
    if tostring("hello") ~= "hello" then return 14 end
    if tostring(-100) ~= "-100" then return 15 end

    -- === tonumber ===
    if tonumber("42") ~= 42 then return 20 end
    if math.abs(tonumber("3.14") - 3.14) > 0.001 then return 21 end
    if tonumber("-100") ~= -100 then return 22 end
    if tonumber(42) ~= 42 then return 23 end
    if math.abs(tonumber(3.14) - 3.14) > 0.001 then return 24 end
    if tonumber("abc") ~= nil then return 25 end
    if tonumber("") ~= nil then return 26 end
    if tonumber("abc123") ~= nil then return 27 end
    -- 带 base 参数
    if tonumber("ff", 16) ~= 255 then return 28 end
    if tonumber("1010", 2) ~= 10 then return 29 end
    if tonumber("z", 36) ~= 35 then return 30 end
    if tonumber("123", 1) ~= nil then return 31 end -- 非法 base
    if tonumber("123", 37) ~= nil then return 32 end -- 非法 base

    -- === select ===
    local a, b, c = select(2, 10, 20, 30, 40)
    if a ~= 20 or b ~= 30 or c ~= 40 then return 40 end

    local count = select("#", 10, 20, 30)
    if count ~= 3 then return 41 end

    local first = select(1, 100, 200)
    if first ~= 100 then return 42 end

    -- select 超出范围
    local x = select(5, 1, 2, 3)
    if x ~= nil then return 43 end

    -- === next ===
    local t2 = {a = 1, b = 2, c = 3}
    local k, v = next(t2)
    if k == nil then return 80 end -- 至少有一个键

    -- 遍历所有键值对
    local count_pairs = 0
    local k1 = nil
    while true do
        local key, val = next(t2, k1)
        if key == nil then break end
        count_pairs = count_pairs + 1
        k1 = key
    end
    if count_pairs ~= 3 then return 81 end

    -- 空表
    local ek, ev = next({})
    if ek ~= nil then return 82 end

    -- === assert (直接调用，不通过 pcall) ===
    -- assert 成功时不返回（返回 nil）
    local a1 = assert(42)
    if a1 ~= 42 then return 95 end

    -- === pcall (测试正常调用) ===
    -- 注意：fakelua 中原生函数不是 first-class value，不能作为参数传递
    -- 这里只测试基本的 pcall 调用（不使用 JIT 编译的函数）

    -- === print (只测试不崩溃) ===
    print("test", 123, true, nil)
    print() -- 空打印

    return 5000
end
