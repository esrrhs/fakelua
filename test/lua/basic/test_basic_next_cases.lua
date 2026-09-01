package "BasicNextCases"

-- 测试 next 带索引
function test_next_with_index()
    local t = {a=1, b=2, c=3}
    local k, v = next(t, "a")
    if k == nil then return 0 end
    return 1
end

-- 测试 next 无索引
function test_next_no_index()
    local t = {x=10, y=20}
    local k, v = next(t)
    if k == nil then return 0 end
    return 1
end

-- 测试 next 空表
function test_next_empty()
    local t = {}
    local k = next(t)
    if k ~= nil then return 0 end
    return 1
end

-- 测试 next 到末尾
function test_next_end()
    local t = {a=1}
    local k, v = next(t, "a")
    if k ~= nil then return 0 end
    return 1
end
