package "BasicPairsCases"

-- 测试 pairs 基本迭代
function test_pairs_basic()
    local t = {a=1, b=2, c=3}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 ipairs 基本迭代
function test_ipairs_basic()
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

-- 测试 ipairs 非连续
function test_ipairs_non_continuous()
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

-- 测试 pairs 空表
function test_pairs_empty()
    local t = {}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 0 then return 0 end
    return 1
end
