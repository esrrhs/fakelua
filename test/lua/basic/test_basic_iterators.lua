package "BasicIterators"

-- 测试 pairs 遍历表
function test_pairs_basic()
    local t = {a = 1, b = 2, c = 3}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 pairs 空表
function test_pairs_empty()
    local count = 0
    for k, v in pairs({}) do
        count = count + 1
    end
    if count ~= 0 then return 0 end
    return 1
end

-- 测试 pairs 数组
function test_pairs_array()
    local arr = {10, 20, 30}
    local sum = 0
    for i, v in pairs(arr) do
        sum = sum + v
    end
    if sum ~= 60 then return 0 end
    return 1
end

-- 测试 ipairs 遍历数组
function test_ipairs_basic()
    local arr = {10, 20, 30}
    local sum = 0
    for i, v in ipairs(arr) do
        sum = sum + v
    end
    if sum ~= 60 then return 0 end
    return 1
end

-- 测试 ipairs 空表
function test_ipairs_empty()
    local count = 0
    for i, v in ipairs({}) do
        count = count + 1
    end
    if count ~= 0 then return 0 end
    return 1
end

-- 测试 ipairs 验证索引
function test_ipairs_index()
    local result = {}
    for i, v in ipairs({100, 200, 300}) do
        result[i] = v
    end
    if result[1] ~= 100 or result[2] ~= 200 or result[3] ~= 300 then return 0 end
    return 1
end

-- 测试 next 函数
function test_next_basic()
    local t = {a = 1, b = 2}
    local k, v = next(t)
    if k == nil then return 0 end
    return 1
end

-- 测试 next 空表
function test_next_empty()
    local k = next({})
    if k ~= nil then return 0 end
    return 1
end

-- 测试 select 函数
function test_select_basic()
    local s = select(2, "a", "b", "c")
    if s ~= "b" then return 0 end
    return 1
end

-- 测试 select 负数索引
function test_select_negative()
    local s = select(-1, "a", "b", "c")
    if s ~= "c" then return 0 end
    return 1
end
